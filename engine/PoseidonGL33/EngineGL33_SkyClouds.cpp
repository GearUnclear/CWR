// Remastered sky: the sky's own clouds (feature switches `litClouds` and
// `skyCloudShadows`; the math and its constants are
// Graphics/Remaster/SkyDensity.hpp).
//
//   - the cloud-density map on unit 9, built from the weather sky texture
//     whenever the weather sky changes: the texture files are read here, once
//     per weather sky; the decode, the mix of two weather skies and the map are
//     a worker thread's (std::async, as the atmosphere tables). A stand-in (no
//     cloud) holds the unit until the first map is built, and while a new
//     weather sky builds the last map stays in use;
//   - the anchor the sky-matched shadows are projected from (SkyShadowAnchor);
//   - the LitConstants slots Sky and SkyShadow: the lit-cloud strength, the
//     shadows' switch, the dome's turn with the sky clouds' drift
//     (Landscape::CloudsPosition) and the anchor.
//
// Off (both switches, classic lighting, night vision or a 3D
// preview) both slots stay zero, nothing is built, and the shaders take the
// paths of the frame before the feature.

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <PoseidonGL33/TextureGL33.hpp>
#include <Poseidon/Graphics/Remaster/Atmosphere.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/Graphics/Remaster/SkyDensity.hpp>
#include <Poseidon/Graphics/Textures/PAADecoder.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/IO/FileServer.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <future>
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace Poseidon::gl33lighting;
namespace rm = Poseidon::remaster;

namespace
{

constexpr float kDeg = 57.29578f;

// A weather sky texture's file, read once.
struct SkyFile
{
    std::vector<char> bytes;
    bool isPaa = false;
};

// What the dome draws: the weather sky texture, or two of them mixed.
struct SkyKey
{
    std::string a;
    std::string b;
    float f = 0.0f;
    bool operator==(const SkyKey& o) const { return a == o.a && b == o.b && f == o.f; }
    bool operator!=(const SkyKey& o) const { return !(*this == o); }
};

struct SkyDensityBuild
{
    rm::SkyDensityMap map;
    rm::SkyShadowCoverageTable coverage;
};

struct SkyCloudsState
{
    std::map<std::string, std::shared_ptr<const SkyFile>> files; // by texture name; null = unreadable
    std::future<SkyDensityBuild> pending;
    bool building = false;
    SkyKey pendingKey;
    // The map on unit 9 (built) and the key it was built from.
    bool built = false;
    bool failed = false; // the current key could not be read or decoded
    SkyKey builtKey;
    rm::SkyShadowCoverageTable coverage;
    unsigned builds = 0;

    GLuint standIn = 0;
    GLuint mapTex = 0;

    rm::SkyShadowAnchor anchor;
    float anchorTime = -1.0f;
    std::uint32_t anchorFrame = 0;
    bool anchorStepped = false;
    bool pinned = false; // captures hold the clouds (triPinCloudDrift)
};

SkyCloudsState s_clouds;

std::shared_ptr<const SkyFile> ReadSkyFile(const char* name)
{
    if (!name || !*name)
        return nullptr;
    const auto it = s_clouds.files.find(name);
    if (it != s_clouds.files.end())
        return it->second;
    std::shared_ptr<SkyFile> file;
    QIFStream in;
    GFileServer->Open(in, name);
    const int size = in.fail() ? 0 : in.rest();
    if (size > 0)
    {
        file = std::make_shared<SkyFile>();
        file->bytes.resize(static_cast<size_t>(size));
        in.read(file->bytes.data(), size);
        const size_t len = std::strlen(name);
        file->isPaa = len >= 4 && (name[len - 1] == 'a' || name[len - 1] == 'A'); // .paa vs .pac
    }
    else
    {
        LOG_WARN(Graphics, "GL33 sky clouds: cannot read the weather sky {}", name);
    }
    s_clouds.files[name] = file;
    return file;
}

// The top mip of a weather sky as display RGB floats, row 0 at v = 0.
std::vector<float> DecodeSkyRgb(const SkyFile& file, int& w, int& h)
{
    const Poseidon::DecodedImage img = Poseidon::DecodePAABuffer(file.bytes.data(), file.bytes.size(), file.isPaa);
    w = h = 0;
    if (!img.valid())
        return {};
    w = img.width;
    h = img.height;
    std::vector<float> rgb(static_cast<size_t>(w) * h * 3);
    for (size_t i = 0; i < static_cast<size_t>(w) * h; ++i)
        for (int c = 0; c < 3; ++c)
            rgb[i * 3 + c] = img.rgba[i * 4 + c] / 255.0f;
    return rgb;
}

// The worker's part: decode, mix as the engine mixes the dome's texture, build.
SkyDensityBuild BuildSkyDensity(std::shared_ptr<const SkyFile> a, std::shared_ptr<const SkyFile> b, float f)
{
    SkyDensityBuild out;
    int w = 0, h = 0;
    std::vector<float> rgb = DecodeSkyRgb(*a, w, h);
    if (rgb.empty())
        return out;
    if (b && f > 0.0f)
    {
        int wb = 0, hb = 0;
        const std::vector<float> other = DecodeSkyRgb(*b, wb, hb);
        if (wb == w && hb == h)
            rgb = rm::MixSkyTexels(rgb, other, f);
    }
    out.map = rm::BuildSkyDensityMap(rgb.data(), w, h);
    out.coverage = rm::BuildSkyShadowCoverageTable(out.map);
    return out;
}

void DeleteTexture(GLuint& t)
{
    if (t)
    {
        Poseidon::GL33Bind::OnTexDeleted(t);
        glDeleteTextures(1, &t);
    }
    t = 0;
}

} // namespace

void EngineGL33::InitSkyClouds()
{
    // Unit 9 always holds a complete texture (a shader must never sample an
    // incomplete texture): no cloud until the first map is built.
    GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
    const unsigned char none[4] = {0, 0, 0, 255};
    glGenTextures(1, &s_clouds.standIn);
    glBindTexture(GL_TEXTURE_2D, s_clouds.standIn);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, none);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    GL33Bind::Invalidate();
    GL33Bind::Tex2D(kSkyDensityUnit, s_clouds.standIn);
    GL33Bind::ActiveUnit(0);
}

void EngineGL33::DeinitSkyClouds()
{
    // GL objects only; the next frame that wants the map builds it again (the
    // files stay read).
    DeleteTexture(s_clouds.standIn);
    DeleteTexture(s_clouds.mapTex);
    s_clouds.built = false;
    s_clouds.failed = false;
    s_clouds.builtKey = SkyKey{};
}

bool EngineGL33::SkyCloudsWanted() const
{
    return _lightingActive && (rm::FeatureOn(_lighting.features, rm::Feature::LitClouds) ||
                               rm::FeatureOn(_lighting.features, rm::Feature::SkyCloudShadows));
}

bool EngineGL33::SkyDensityReady() const
{
    return s_clouds.built;
}

bool EngineGL33::SkyDensityPending() const
{
    if (s_clouds.building)
        return true;
    // Wanted, and the weather sky has no map yet (the next world pass starts
    // its build).
    if (!SkyCloudsWanted() || !GScene || !GScene->GetLandscape())
        return false;
    const TextureGL33* sky = static_cast<const TextureGL33*>(GScene->GetLandscape()->SkyTexture());
    if (!sky || s_clouds.failed)
        return false;
    SkyKey key;
    key.a = sky->Name() ? sky->Name() : "";
    if (sky->_interpolate)
    {
        key.b = sky->_interpolate->Name() ? sky->_interpolate->Name() : "";
        key.f = sky->_iFactor;
    }
    return !s_clouds.built || key != s_clouds.builtKey;
}

void EngineGL33::PollSkyDensity()
{
    // Collect a finished build whatever the switches say now, so nothing
    // stays pending.
    if (s_clouds.building && s_clouds.pending.valid() &&
        s_clouds.pending.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
    {
        SkyDensityBuild done = s_clouds.pending.get();
        s_clouds.building = false;
        if (done.map.Valid())
        {
            if (!s_clouds.mapTex)
                glGenTextures(1, &s_clouds.mapTex);
            GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, s_clouds.mapTex);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, done.map.width, done.map.height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                         done.map.rgba.data());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
            GL33Bind::Invalidate();
            GL33Bind::ActiveUnit(0);
            s_clouds.built = true;
            s_clouds.failed = false;
            s_clouds.builtKey = s_clouds.pendingKey;
            s_clouds.coverage = done.coverage;
            ++s_clouds.builds;
            const rm::SkyShadowCoverageTable& c = s_clouds.coverage;
            LOG_INFO(Graphics,
                     "GL33 sky clouds: density map {}x{} of {}{}{} (mix {:.3f}) built; shadow coverage at sun 10/30/60 "
                     "deg {:.3f}/{:.3f}/{:.3f} broken, {:.3f}/{:.3f}/{:.3f} overcast",
                     done.map.width, done.map.height, s_clouds.builtKey.a, s_clouds.builtKey.b.empty() ? "" : " + ",
                     s_clouds.builtKey.b, s_clouds.builtKey.f, c.At(0.0f, 10.0f), c.At(0.0f, 30.0f), c.At(0.0f, 60.0f),
                     c.At(1.0f, 10.0f), c.At(1.0f, 30.0f), c.At(1.0f, 60.0f));
        }
        else
        {
            s_clouds.failed = true;
            s_clouds.builtKey = s_clouds.pendingKey;
            LOG_WARN(Graphics, "GL33 sky clouds: the weather sky {} did not decode; the noise field keeps the shadows",
                     s_clouds.pendingKey.a);
        }
    }
    if (s_clouds.building || !SkyCloudsWanted() || !GScene || !GScene->GetLandscape())
        return;
    const TextureGL33* sky = static_cast<const TextureGL33*>(GScene->GetLandscape()->SkyTexture());
    if (!sky)
        return;
    SkyKey key;
    key.a = sky->Name() ? sky->Name() : "";
    if (sky->_interpolate)
    {
        key.b = sky->_interpolate->Name() ? sky->_interpolate->Name() : "";
        key.f = sky->_iFactor;
    }
    if ((s_clouds.built || s_clouds.failed) && key == s_clouds.builtKey)
        return;
    std::shared_ptr<const SkyFile> a = ReadSkyFile(key.a.c_str());
    std::shared_ptr<const SkyFile> b = key.b.empty() ? nullptr : ReadSkyFile(key.b.c_str());
    if (!a)
    {
        s_clouds.failed = true;
        s_clouds.builtKey = key;
        return;
    }
    const float f = key.f;
    s_clouds.pendingKey = key;
    s_clouds.building = true;
    s_clouds.pending = std::async(std::launch::async, [a, b, f] { return BuildSkyDensity(a, b, f); });
}

float EngineGL33::SkyShadowCoverageAt(float skyThrough, float sunElevationDeg) const
{
    if (!s_clouds.built)
        return -1.0f;
    return s_clouds.coverage.At(rm::SkyDeckWeight(skyThrough), sunElevationDeg);
}

void EngineGL33::BindSkyDensity()
{
    GL33Bind::Tex2D(kSkyDensityUnit, s_clouds.built && s_clouds.mapTex ? s_clouds.mapTex : s_clouds.standIn);
}

void EngineGL33::PinSkyShadowAnchor(bool pinned)
{
    s_clouds.pinned = pinned;
    // A pinned view sees the sky from where it stands: the anchor at the
    // camera, held there until released.
    Camera* camera = GScene ? GScene->GetCamera() : nullptr;
    if (pinned && camera)
    {
        const Vector3 cam = camera->Position();
        s_clouds.anchor.Snap(cam.X(), cam.Z());
    }
}

void EngineGL33::UploadLitClouds(float skyThrough, bool cloudField)
{
    namespace LC = LitConst;
    Camera* camera = GScene ? GScene->GetCamera() : nullptr;
    LightSun* sun = GScene ? GScene->MainLight() : nullptr;
    Landscape* land = GScene ? GScene->GetLandscape() : nullptr;
    const bool wanted = SkyCloudsWanted() && _worldEffectsOn && camera && sun && land;
    if (!wanted)
    {
        SetLitSlot(LC::Sky, 0.0f, 0.0f, 0.0f, 0.0f);
        SetLitSlot(LC::SkyShadow, 0.0f, 0.0f, 0.0f, 0.0f);
        return;
    }
    // Lit clouds fade out with the sky table's own night fade (-3 to -9 deg).
    const float sunElevDeg = std::asin(std::max(-1.0f, std::min(1.0f, -sun->Direction().Y()))) * kDeg;
    const float lit = _featureGates[rm::Feature::LitClouds].on ? rm::SkyDayFade(sunElevDeg) : 0.0f;
    const bool shadows = _featureGates[rm::Feature::SkyCloudShadows].on && cloudField;
    // The dome turns with the sky clouds' drift whenever a clouds switch is on
    // (not only when gated on: the dome would jump when a gate opened).
    const float turn = rm::SkyDomeTurnU(land->CloudsPosition());
    SetLitSlot(LC::Sky, lit, shadows ? 1.0f : 0.0f, turn, skyThrough);

    // The shadow anchor, once per frame (the sky pass and the world pass both
    // upload the frame's constants).
    const Vector3 cam = camera->Position();
    const std::uint32_t frame = GetFrameCounter();
    if (!s_clouds.anchorStepped || s_clouds.anchorFrame != frame)
    {
        const float now = Glob.time.toFloat();
        float dt = s_clouds.anchorTime >= 0.0f ? now - s_clouds.anchorTime : 0.0f;
        dt = dt < 0.0f ? 0.0f : (dt > 0.5f ? 0.5f : dt); // pauses and time skips do not run the fade
        s_clouds.anchor.Update(cam.X(), cam.Z(), dt, s_clouds.pinned);
        s_clouds.anchorTime = now;
        s_clouds.anchorFrame = frame;
        s_clouds.anchorStepped = true;
    }
    const rm::SkyShadowAnchor& a = s_clouds.anchor;
    if (shadows)
        SetLitSlot(LC::SkyShadow, a.x - cam.X(), a.z - cam.Z(), a.prevAngle, a.prevWeight);
    else
        SetLitSlot(LC::SkyShadow, 0.0f, 0.0f, 0.0f, 0.0f);
}
