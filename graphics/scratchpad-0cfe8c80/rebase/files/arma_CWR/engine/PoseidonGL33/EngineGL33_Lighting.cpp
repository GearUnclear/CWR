// Remastered lighting: GL resources and the LitConstants uploads.
//
// The remastered program set (EngineGL33_LightingShaders.cpp) reads the light
// rig, the per-draw material and the local lights from the LitConstants block
// (binding 3), the shadow map through a comparison sampler on unit 2 (and as
// raw depth on unit 6), the terrain horizon map on units 3 and 4, the cloud
// noise on unit 5, the sky table on unit 8, the sky's cloud density on unit 9,
// the frame's tiled light list from the FrameLights block (binding 4) and
// its tile indices on unit 13, and the canopy's sun-visibility map on unit 15
// (EngineGL33_Canopy.cpp). Selection between the remastered and classic sets
// happens in DoSelectPixelShader via SelectProgram; Classic leaves every
// classic upload and program untouched.

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <Poseidon/Graphics/Remaster/PhotoCamera.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <Poseidon/Graphics/Core/PerfCounters.hpp>
#include <Poseidon/Graphics/Core/UboMirror.hpp>
#include <Poseidon/Graphics/Remaster/CloudNoise.hpp>
#include <Poseidon/Graphics/Remaster/SunShadows.hpp>
#include <Poseidon/Graphics/Remaster/TerrainDetail.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>

#include <array>
#include <chrono>
#include <cmath>
#include <cstring>

using namespace Poseidon::gl33lighting;
namespace rm = Poseidon::remaster;
namespace fl = Poseidon::lighting;

namespace
{
float s_lit[LitConst::Count * 4] = {};
GLuint s_litUBO = 0;
// What s_litUBO holds (UploadLitSlots keeps it; a new buffer starts unknown).
Poseidon::gpu::UboMirror s_litMirror;
// Cloud drift integrated from the wind, so a wind change bends the drift
// instead of jumping the whole field. Integrated per frame, so re-pinning the
// sim clock (triSetSimTime) does not put it back; captures pin it instead
// (PinCloudDrift, triPinCloudDrift), and it holds still while pinned.
float s_cloudDrift[2] = {1830.0f, 970.0f};
float s_cloudDriftTime = -1.0f;
bool s_cloudDriftPinned = false;

constexpr float kCloudDeckAltitude = 1500.0f; // metres
constexpr float kCloudScale = 1.0f / 6000.0f; // noise tile = 6 km
constexpr float kCloudEdge = 0.08f;
constexpr float kTerrainPenumbraDeg = 1.2f; // half width of the hill-shadow edge
constexpr float kShadowFilterTexels = 1.0f; // shadow-map filter radius at the default softness

void SetSlot(int slot, float x, float y, float z, float w)
{
    float* p = s_lit + slot * 4;
    p[0] = x;
    p[1] = y;
    p[2] = z;
    p[3] = w;
}

GLuint MakeDummyTexture(GLenum target, GLenum internalFormat, GLenum format, GLenum type, const void* texel)
{
    GLuint t = 0;
    glGenTextures(1, &t);
    glBindTexture(target, t);
    if (target == GL_TEXTURE_2D_ARRAY)
        glTexImage3D(target, 0, internalFormat, 1, 1, 1, 0, format, type, texel);
    else
        glTexImage2D(target, 0, internalFormat, 1, 1, 0, format, type, texel);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 0);
    glBindTexture(target, 0);
    return t;
}
} // namespace

// The one place the LitConstants buffer is written: `count` slots of s_lit
// from `first` on, recorded in the mirror.
void EngineGL33::UploadLitSlots(int first, int count)
{
    if (!s_litUBO || count <= 0)
        return;
    glBindBuffer(GL_UNIFORM_BUFFER, s_litUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, first * 16, count * 16, s_lit + first * 4);
    s_litMirror.Store(s_lit, first, count);
    Poseidon::perf::FrameCounters& c = Poseidon::perf::GPerfCounters().Frame();
    ++c.litUploads;
    c.litBytes += count * 16;
}

void EngineGL33::FlushLitConstants()
{
    if (!s_litUBO)
        return;
    // drawGrouping: send only the slots that changed since the last upload
    // (a draw's material and lights, mostly); off, the whole block as before.
    const bool dedupe = _featureGates[rm::Feature::DrawGrouping].on;
    int first = 0, count = 0;
    if (!s_litMirror.Plan(s_lit, dedupe, first, count))
    {
        ++Poseidon::perf::GPerfCounters().Frame().litSkips;
        return;
    }
    UploadLitSlots(first, count);
}

void EngineGL33::InitLightingResources()
{
    glGenBuffers(1, &s_litUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, s_litUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(s_lit), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, kLitBinding, s_litUBO);
    s_litMirror.Reset(LitConst::Count); // a new buffer holds nothing known yet

    // Stand-ins so every sampler of the remastered shaders always has a complete
    // texture of the right kind, map or no map. Created on the upload unit; the
    // raw binds are followed by a bind-cache invalidation.
    GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
    const float one = 1.0f;
    _dummyDepthArray = MakeDummyTexture(GL_TEXTURE_2D_ARRAY, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT, &one);
    const unsigned short open[4] = {0, 0, 0, 0}; // horizon at the bottom of the range: open sky
    _dummyHorizonTex = MakeDummyTexture(GL_TEXTURE_2D_ARRAY, GL_RGBA16, GL_RGBA, GL_UNSIGNED_SHORT, open);
    const float heightSky[4] = {-1e6f, 1.0f, 1.0f, 1.0f}; // height, sky visibility, neutral ground tint
    _dummyHeightTex = MakeDummyTexture(GL_TEXTURE_2D, GL_RGBA32F, GL_RGBA, GL_FLOAT, heightSky);

    const std::vector<std::uint8_t> noise = rm::BuildCloudNoise(256, 1);
    glGenTextures(1, &_cloudNoiseTex);
    glBindTexture(GL_TEXTURE_2D, _cloudNoiseTex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 256, 256, 0, GL_RED, GL_UNSIGNED_BYTE, noise.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    const unsigned char white[4] = {255, 255, 255, 255};
    const unsigned char black[4] = {0, 0, 0, 0};
    _post.whiteTex = MakeDummyTexture(GL_TEXTURE_2D, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, white);
    _post.blackTex = MakeDummyTexture(GL_TEXTURE_2D, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, black);
    GL33Bind::Invalidate();
    // Every lit program, classic and remastered, has its shadowMap sampler on
    // unit 2, and none may meet texture 0 there (an incomplete texture; under
    // the comparison sampler, undefined behaviour). Park the stand-in on the
    // unit until a shadow map replaces it.
    GL33Bind::ActiveUnit(kShadowMapUnit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _dummyDepthArray);
    // The same rule for unit 6, where the remastered programs read the array
    // as raw depth (the soft shadows' blocker search).
    GL33Bind::ActiveUnit(kShadowRawUnit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _dummyDepthArray);
    GL33Bind::ActiveUnit(0);
    // The sky table's unit 8, the clouds' unit 9 and the canopy's units 10 and
    // 15 get their stand-ins the same way.
    InitSkyTable();
    InitSkyClouds();
    InitCanopy();
    // So does the sea's next-frame unit (EngineGL33_Sea.cpp binds an animation
    // frame there only inside the sea's draws).
    GL33Bind::Tex2D(kSeaNextFrameUnit, _post.whiteTex);
    GL33Bind::ActiveUnit(0);

    glGenSamplers(1, &_shadowCompareSampler);
    glSamplerParameteri(_shadowCompareSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(_shadowCompareSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(_shadowCompareSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(_shadowCompareSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(_shadowCompareSampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glSamplerParameteri(_shadowCompareSampler, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    // Raw depth: nearest texel, no comparison. Unit 6 keeps it for good; no
    // other program samples that unit.
    glGenSamplers(1, &_shadowRawSampler);
    glSamplerParameteri(_shadowRawSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(_shadowRawSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(_shadowRawSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(_shadowRawSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(_shadowRawSampler, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glBindSampler(kShadowRawUnit, _shadowRawSampler);

    InitLightTiles();

    // The map (if any) belongs to the landscape, not the context: re-upload it.
    _terrainMapDirty = _terrainMap != nullptr;
}

void EngineGL33::InitLightTiles()
{
    LightTileState& t = _lightTiles;
    // The lights: a uniform block of kMaxFrameLights lights on its own binding
    // point, zeroed, so a remastered program always meets a complete buffer.
    const std::vector<float> none(static_cast<size_t>(fl::kMaxFrameLights) * fl::kTexelsPerLight * 4, 0.0f);
    glGenBuffers(1, &t.dataBuf);
    glBindBuffer(GL_UNIFORM_BUFFER, t.dataBuf);
    glBufferData(GL_UNIFORM_BUFFER, static_cast<GLsizeiptr>(none.size() * sizeof(float)), none.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, kLightListBinding, t.dataBuf);
    // The tile indices: created on the upload unit, then parked on their own
    // unit, one tile with no light until the first build sizes it for the frame
    // target. Integer textures sample only with NEAREST filtering.
    GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
    const std::uint8_t empty[fl::kTileTexels * 4] = {};
    glGenTextures(1, &t.tileTex);
    glBindTexture(GL_TEXTURE_2D, t.tileTex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI, fl::kTileTexels, 1, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, empty);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    GL33Bind::Invalidate();
    t.texW = fl::kTileTexels;
    t.texH = 1;
    t.uploadedTexels.clear();
    t.uploadedIndices.clear();
    t.frame = 0xffffffffu;
    GL33Bind::Tex2D(kLightTileUnit, t.tileTex);
    GL33Bind::ActiveUnit(0);
    _lightTilesActive = false;
}

void EngineGL33::DeinitLightTiles()
{
    LightTileState& t = _lightTiles;
    if (t.tileTex)
    {
        GL33Bind::OnTexDeleted(t.tileTex);
        glDeleteTextures(1, &t.tileTex);
    }
    t.tileTex = 0;
    if (t.dataBuf)
        glDeleteBuffers(1, &t.dataBuf);
    t.dataBuf = 0;
    t.texW = t.texH = 0;
    t.uploadedTexels.clear();
    t.uploadedIndices.clear();
    t.frame = 0xffffffffu;
    _lightTilesActive = false;
}

void EngineGL33::UpdateLightTiles()
{
    LightTileState& t = _lightTiles;
    const std::uint32_t frame = GetFrameCounter();
    if (t.frame == frame || !t.dataBuf || !t.tileTex)
        return;
    t.frame = frame;
    const auto start = std::chrono::steady_clock::now();

    // The scene's active lights (on, within the horizon range plus 500 m, the
    // ENGINE_CONFIG.maxLights strongest at the camera): the set every draw's
    // own list is chosen from. Positions camera-relative as in UploadLitLights;
    // colours as authored, the draw's night factor scales them in the shader.
    const float* camPos = _frameState.cameraPos;
    t.lights.clear();
    const LightList& active = GScene->ActiveLights();
    for (int i = 0; i < active.Size(); i++)
    {
        Light* light = active[i];
        if (!light)
            continue;
        LightDescription desc;
        light->GetDescription(desc);
        const bool spot = desc.type == LTSpotLight;
        if (desc.type != LTPoint && !spot)
            continue;
        fl::FrameLight f;
        f.kind = spot ? fl::FrameLightKind::Spot : light->Kind();
        f.pos[0] = desc.pos.X() - camPos[0];
        f.pos[1] = desc.pos.Y() - camPos[1];
        f.pos[2] = desc.pos.Z() - camPos[2];
        f.start = desc.startAtten;
        Vector3 beam = desc.dir;
        beam.Normalize();
        f.dir[0] = beam.X();
        f.dir[1] = beam.Y();
        f.dir[2] = beam.Z();
        f.diffuse[0] = desc.diffuse.R();
        f.diffuse[1] = desc.diffuse.G();
        f.diffuse[2] = desc.diffuse.B();
        f.ambient[0] = desc.ambient.R();
        f.ambient[1] = desc.ambient.G();
        f.ambient[2] = desc.ambient.B();
        f.sortBrightness = light->SortBrightness();
        if (auto* attached = dynamic_cast<AttachedOnVehicle*>(light))
        {
            if (const Object* owner = attached->AttachedOn())
                f.owner = static_cast<std::uint32_t>(owner->ID());
        }
        t.lights.push_back(f);
    }
    fl::RankFrameLights(t.lights);

    // The camera as the world pass draws it: the view rotation of
    // BuildFrameState, the projection's scale and the 3D viewport of
    // ApplyWorldViewport, over the whole frame target.
    fl::TileView view;
    if (Camera* camera = GScene->GetCamera())
    {
        const Matrix4& toView = camera->InverseScaled();
        const Vector3 axes[3] = {Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1)};
        for (int c = 0; c < 3; ++c)
        {
            const Vector3 col = toView.Rotate(axes[c]);
            view.rot[0][c] = col.X();
            view.rot[1][c] = col.Y();
            view.rot[2][c] = col.Z();
        }
    }
    view.p11 = _frameState.projection._11;
    view.p22 = _frameState.projection._22;
    int targetW = 0, targetH = 0;
    RenderTargetSize(targetW, targetH);
    const Poseidon::AspectSettings& a = _aspectSettings;
    const bool full = a.worldLeft <= 0.0f && a.worldTop <= 0.0f && a.worldRight >= 1.0f && a.worldBottom >= 1.0f;
    int x0 = 0, x1 = targetW, yTop = 0, yBottom = targetH;
    if (!full)
    {
        x0 = static_cast<int>(a.worldLeft * targetW + 0.5f);
        yTop = static_cast<int>(a.worldTop * targetH + 0.5f);
        x1 = static_cast<int>(a.worldRight * targetW + 0.5f);
        yBottom = static_cast<int>(a.worldBottom * targetH + 0.5f);
    }
    view.viewport[0] = static_cast<float>(x0);
    view.viewport[1] = static_cast<float>(targetH - yBottom);
    view.viewport[2] = static_cast<float>(x1 - x0);
    view.viewport[3] = static_cast<float>(yBottom - yTop);

    const int tilesX = (targetW + fl::kTileSize - 1) / fl::kTileSize;
    const int tilesY = (targetH + fl::kTileSize - 1) / fl::kTileSize;
    if (t.grid.tilesX != tilesX || t.grid.tilesY != tilesY)
        fl::ResizeTileGrid(t.grid, targetW, targetH);
    fl::BinLightsToTiles(t.lights, view, t.grid);
    fl::PackFrameLights(t.lights, t.texels);

    // Upload what changed since the last build (a still camera sends nothing).
    if (t.texels != t.uploadedTexels && !t.texels.empty())
    {
        // The indexed binding stays; only the store's contents change.
        glBindBuffer(GL_UNIFORM_BUFFER, t.dataBuf);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, static_cast<GLsizeiptr>(t.texels.size() * sizeof(float)),
                        t.texels.data());
        t.uploadedTexels = t.texels;
    }
    const int texW = t.grid.tilesX * fl::kTileTexels, texH = t.grid.tilesY;
    if (texW > 0 && texH > 0 && (texW != t.texW || texH != t.texH || t.grid.indices != t.uploadedIndices))
    {
        // The upload unit, through the bind cache (it caches units 0 to 7).
        GL33Bind::Tex2D(kUploadUnit - GL_TEXTURE0, t.tileTex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        if (texW != t.texW || texH != t.texH)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI, texW, texH, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE,
                         t.grid.indices.data());
            t.texW = texW;
            t.texH = texH;
        }
        else
        {
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texW, texH, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE,
                            t.grid.indices.data());
        }
        GL33Bind::ActiveUnit(0);
        t.uploadedIndices = t.grid.indices;
    }

    Poseidon::perf::FrameCounters& c = Poseidon::perf::GPerfCounters().Frame();
    c.tileLights = static_cast<int>(t.lights.size());
    c.tileRefs = t.grid.refs;
    c.tileMs = std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - start).count();
}

void EngineGL33::DeinitLightingResources()
{
    DestroyGpuQueries();
    DestroyPostTargets();
    DestroyPostPrograms();
    auto delTex = [](unsigned int& t)
    {
        if (t)
        {
            GL33Bind::OnTexDeleted(t);
            glDeleteTextures(1, &t);
        }
        t = 0;
    };
    delTex(_terrainHorizonTex);
    delTex(_terrainHeightTex);
    delTex(_cloudNoiseTex);
    delTex(_dummyDepthArray);
    delTex(_dummyHorizonTex);
    delTex(_dummyHeightTex);
    delTex(_post.whiteTex);
    delTex(_post.blackTex);
    DeinitSkyTable();
    DeinitLightTiles();
    DeinitSkyClouds();
    DeinitCanopy();
    _post.unavailable = false;
    if (_shadowCompareSampler)
    {
        glBindSampler(kShadowMapUnit, 0);
        glDeleteSamplers(1, &_shadowCompareSampler);
        _shadowCompareSampler = 0;
    }
    if (_shadowRawSampler)
    {
        glBindSampler(kShadowRawUnit, 0);
        glDeleteSamplers(1, &_shadowRawSampler);
        _shadowRawSampler = 0;
    }
    if (s_litUBO)
    {
        glDeleteBuffers(1, &s_litUBO);
        s_litUBO = 0;
    }
    s_litMirror.Reset(LitConst::Count);
}

void EngineGL33::SetLightingTuning(const rm::LightingTuning& tuning)
{
    const bool wasActive = _lightingActive;
    _lighting = tuning;
    // A lighting change: the camera's exposure snaps to the next meter.
    _camera.snap = true;
    _camera.snapFrame = GetFrameCounter();
    _lightingActive = tuning.remastered && _lightingProgramsBuilt;
    if (wasActive != _lightingActive && _glContext)
    {
        // Switch program sets and the unit-2 sampler pairing right away; the
        // next pass re-uploads the rig.
        FlushQueues();
        _litOpaque = false;
        UpdateRemasterPrograms();
        LOG_INFO(Graphics, "GL33: lighting {}", _lightingActive ? "remastered" : "classic");
    }
}

void EngineGL33::UpdateRemasterPrograms()
{
    const bool on = _lightingActive && _worldEffectsOn;
    if (on == _remasterPrograms)
        return;
    _remasterPrograms = on;
    if (!_glContext)
        return;
    // Unit 2 holds the shadow-map array for both sets; only the remastered
    // shaders read it through a comparison sampler (the classic ones read raw
    // depth, and compare mode on a plain sampler is undefined).
    glBindSampler(kShadowMapUnit, on ? _shadowCompareSampler : 0);
    InvalidatePipelineCache();
    // Re-bind the current program from the other set (water reads the scene's
    // sun when selected; without a scene, the next selection re-binds).
    if (GScene || _pixelShaderSel != PSWater)
        DoSelectPixelShader(_pixelShaderSel, _pixelShaderModeSel, _pixelShaderSpecularSel);
    else
        _pixelShaderSel = PSNone;
}

bool EngineGL33::WantsTerrainHorizonMap() const
{
    return _lightingActive && (_lighting.terrainShadows > 0.0f || _lighting.terrainOcclusion > 0.0f);
}

void EngineGL33::SetTerrainHorizonMap(std::shared_ptr<const rm::TerrainHorizonMap> map)
{
    _terrainMap = std::move(map);
    _terrainMapDirty = true;
}

void EngineGL33::UploadTerrainHorizonMap()
{
    _terrainMapDirty = false;
    auto delTex = [](unsigned int& t)
    {
        if (t)
        {
            GL33Bind::OnTexDeleted(t);
            glDeleteTextures(1, &t);
        }
        t = 0;
    };
    delTex(_terrainHorizonTex);
    delTex(_terrainHeightTex);
    if (!_terrainMap || !_terrainMap->Valid())
        return;
    const rm::TerrainHorizonMap& m = *_terrainMap;
    const int n = m.size;

    GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &_terrainHorizonTex);
    glBindTexture(GL_TEXTURE_2D_ARRAY, _terrainHorizonTex);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA16, n, n, rm::TerrainHorizonMap::kLayers, 0, GL_RGBA, GL_UNSIGNED_SHORT,
                 m.horizon.data());
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL, 0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    // r = height, g = sky visibility, ba = the ground-bounce tint (red, blue).
    const bool tint = m.groundTint.size() == static_cast<size_t>(n) * n * 2;
    std::vector<float> hs(static_cast<size_t>(n) * n * 4);
    for (size_t i = 0; i < static_cast<size_t>(n) * n; ++i)
    {
        hs[i * 4 + 0] = m.height[i];
        hs[i * 4 + 1] = static_cast<float>(m.skyVisibility[i]) / 255.0f;
        hs[i * 4 + 2] = tint ? m.groundTint[i * 2 + 0] : 1.0f;
        hs[i * 4 + 3] = tint ? m.groundTint[i * 2 + 1] : 1.0f;
    }
    glGenTextures(1, &_terrainHeightTex);
    glBindTexture(GL_TEXTURE_2D, _terrainHeightTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, n, n, 0, GL_RGBA, GL_FLOAT, hs.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    GL33Bind::Invalidate();
    LOG_INFO(Graphics, "GL33: terrain horizon map {}x{} at {} m uploaded", n, n, m.cellSize);
}

void EngineGL33::BindLightingTextures()
{
    if (!_remasterPrograms)
        return;
    const bool map = _terrainHorizonTex && _terrainHeightTex;
    const bool shadowMap = ShadowMapsEnabled() && _shadowMapActive && _shadowMapTex && _shadowCascades > 0;
    GL33Bind::ActiveUnit(kShadowMapUnit);
    if (!shadowMap)
        glBindTexture(GL_TEXTURE_2D_ARRAY, _dummyDepthArray);
    glBindSampler(kShadowMapUnit, _shadowCompareSampler);
    GL33Bind::ActiveUnit(kShadowRawUnit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, shadowMap ? _shadowMapTex : _dummyDepthArray);
    glBindSampler(kShadowRawUnit, _shadowRawSampler);
    GL33Bind::ActiveUnit(kTerrainHorizonUnit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, map ? _terrainHorizonTex : _dummyHorizonTex);
    GL33Bind::Tex2D(kTerrainHeightUnit, map ? _terrainHeightTex : _dummyHeightTex);
    GL33Bind::Tex2D(kCloudNoiseUnit, _cloudNoiseTex);
    BindSkyTable();
    // The tiled light list's indices (read only while litLightCount.y is set,
    // complete either way).
    GL33Bind::Tex2D(kLightTileUnit, _lightTiles.tileTex);
    BindSkyDensity();
    BindCanopy();
    GL33Bind::ActiveUnit(0);
}

unsigned int EngineGL33::SelectProgram(int vs, int spec, int mode, int ps) const
{
    if (_remasterPrograms && _shaderProgramRemaster[vs][spec][mode][ps])
        return _shaderProgramRemaster[vs][spec][mode][ps];
    return _shaderProgram[vs][spec][mode][ps];
}

void EngineGL33::SetLitOpaque(bool opaque)
{
    if (!_lightingActive || opaque == _litOpaque)
        return;
    _litOpaque = opaque;
    s_lit[LitConst::Flags * 4 + 0] = opaque ? 1.0f : 0.0f;
    // Blend modes switch per draw group: upload just the flags slot.
    UploadLitSlots(LitConst::Flags, 1);
}

void EngineGL33::UpdateFeatureGates(const rm::LightRig* rig)
{
    // Once per frame, in the world's own passes: a portrait capture or a 3D
    // preview drawn after the world must not overwrite the world's gates.
    if (!GScene || _portraitTargetSize || _worldSceneEnded)
        return;
    if (_featureGates.computed && _featureGates.frame == GetFrameCounter())
        return;
    LightSun* sun = GScene->MainLight();
    if (!sun)
        return;
    Camera* camera = GScene->GetCamera();
    Landscape* land = GScene->GetLandscape();

    constexpr float kDeg = 57.29578f;
    auto clampUnit = [](float v) { return v < -1.0f ? -1.0f : (v > 1.0f ? 1.0f : v); };
    rm::GateInputs in;
    const Vector3 toSun = -sun->Direction();
    const Vector3 toMoon = -sun->MoonDirection();
    in.sunElevationDeg = std::asin(clampUnit(toSun.Y())) * kDeg;
    float azimuth = std::atan2(toSun.X(), toSun.Z()) * kDeg;
    in.sunAzimuthDeg = azimuth < 0.0f ? azimuth + 360.0f : azimuth;
    if (camera)
        in.sunViewAngleDeg = std::acos(clampUnit(camera->Direction() * toSun)) * kDeg;
    in.moonElevationDeg = std::asin(clampUnit(toMoon.Y())) * kDeg;
    // The moon light share the rig uses (LightSun::Recalculate's phase).
    const float cosMoonPhase = -(sun->Direction() * sun->MoonDirection());
    in.moonPhase = cosMoonPhase + 1.0f < 1.0f ? cosMoonPhase + 1.0f : 1.0f;
    in.nightEffect = sun->NightEffect();
    in.skyThrough = land ? land->SkyThrough() : 1.0f;
    in.cloudCover = rig ? rig->cloudCover : 0.0f;
    in.rainDensity = land ? land->GetRainDensity() : 0.0f;
    in.fog = land ? land->GetFog() : 0.0f;
    in.worldDrawn = true; // only the world's passes get here
    in.interior = _prevWorldDepthCleared;
    in.optics = _prevWorldOverlayDrawn;
    in.nightVision = _nightVision;
    in.skyTablesReady = SkyTablesReady();
    in.skyDensityReady = SkyDensityReady();
    const LightList& lights = GScene->ActiveLights();
    for (int i = 0; i < lights.Size(); i++)
    {
        Light* light = lights[i];
        if (!light)
            continue;
        LightDescription desc;
        light->GetDescription(desc);
        if (desc.type == LTPoint || desc.type == LTSpotLight)
            in.localLights++;
        if (desc.type == LTSpotLight)
            in.spotLights++;
    }
    _featureGates = rm::ComputeFeatureGates(in, _lighting);
    _featureGates.frame = GetFrameCounter();
}

void EngineGL33::UploadLitFrameConstants()
{
    if (!_remasterPrograms || !GScene)
    {
        UpdateFeatureGates(nullptr);
        // Classic programs light each draw from its own list.
        _lightTilesActive = false;
        return;
    }
    if (_terrainMapDirty)
        UploadTerrainHorizonMap();
    // The sky's cloud-density map: collect a finished build, start one when the
    // weather sky changed (while a clouds switch is on).
    PollSkyDensity();

    LightSun* sun = GScene->MainLight();
    Camera* camera = GScene->GetCamera();
    Landscape* land = GScene->GetLandscape();

    rm::LightRigInputs in;
    const Vector3 toSun = -sun->Direction();
    const Vector3 toMoon = -sun->MoonDirection();
    in.towardSun[0] = toSun.X();
    in.towardSun[1] = toSun.Y();
    in.towardSun[2] = toSun.Z();
    in.towardMoon[0] = toMoon.X();
    in.towardMoon[1] = toMoon.Y();
    in.towardMoon[2] = toMoon.Z();
    // LightSun::Recalculate: full moon when moonlight travels against sunlight.
    const float cosMoonPhase = -(sun->Direction() * sun->MoonDirection());
    in.moonPhaseLight = cosMoonPhase + 1.0f < 1.0f ? cosMoonPhase + 1.0f : 1.0f;
    const Color dif = sun->Diffuse();
    const Color amb = sun->Ambient();
    in.classicSunDiffuse = rm::Rgb(dif.R(), dif.G(), dif.B());
    in.classicAmbient = rm::Rgb(amb.R(), amb.G(), amb.B());
    in.fogColor = rm::Rgb(_fogColor.R(), _fogColor.G(), _fogColor.B());
    in.skyThrough = land ? land->SkyThrough() : 1.0f;
    in.nightEffect = sun->NightEffect();
    in.cloudShadowsActive = _worldEffectsOn && _lighting.cloudShadows > 0.0f;
    // The photoreal camera (photoLight): while it can run, the rig is the
    // physical one, from the sky table's atmosphere (EngineGL33_Camera.cpp).
    if (CameraWanted())
    {
        rm::PhysicalSky physical;
        const float y = toSun.Y() < -1.0f ? -1.0f : (toSun.Y() > 1.0f ? 1.0f : toSun.Y());
        if (PhysicalSkyFor(std::asin(y), physical))
            in.physical = physical;
    }
    rm::LightRig rig = rm::ComputeLightRig(in, _lighting);
    UpdateFeatureGates(&rig);
    // Sky-matched cloud shadows lie over the sky's own covered share of the
    // ground: the rig keeps the mean dimming over it (same cover, so the gates
    // computed above stand).
    if (_featureGates[rm::Feature::SkyCloudShadows].on && SkyDensityReady())
    {
        in.skyShadowCoverage = SkyShadowCoverageAt(in.skyThrough, rig.sunElevationDeg);
        rig = rm::ComputeLightRig(in, _lighting);
    }
    _lightRigSun = Color(rig.sun.r, rig.sun.g, rig.sun.b, 1.0f);
    // The photoreal camera: whether this frame's world passes write scene
    // light, the white balance, and the clear sky's gain (only where the dome
    // is the sky table's).
    UpdateCameraFrame(rig, _featureGates[rm::Feature::Atmosphere].on && SkyTablesReady());
    SetSlot(LitConst::Camera, _camera.frameOn ? 1.0f : 0.0f, rm::kSunDiskGain, _camera.frameOn ? _camera.skyGain : 1.0f,
            0.0f);

    // The tiled light list: built once per frame while its gate is on, and
    // then read by every draw of the world's passes instead of the draw's own
    // list. Off, the per-draw slots carry each draw's lights as before.
    const bool wasTiled = _lightTilesActive;
    _lightTilesActive =
        _featureGates.computed && _featureGates.frame == GetFrameCounter() && _featureGates[rm::Feature::LightTiles].on;
    if (_lightTilesActive)
    {
        UpdateLightTiles();
        _lightTilesActive = _lightTiles.frame == GetFrameCounter();
    }
    // While the tiles light the draws the per-draw light slots go stale, and
    // slot 22 is set here for the pass until each draw's material upload sets
    // its own night factor: whenever the tiles are or were on, the next
    // material upload must happen.
    if (wasTiled || _lightTilesActive)
        _materialSetSpec = -1;
    float* lightCount = s_lit + LitConst::LightCount * 4;
    lightCount[1] = _lightTilesActive ? static_cast<float>(fl::kTileSize) : 0.0f;
    if (_lightTilesActive)
    {
        lightCount[0] = 0.0f;
        lightCount[2] = sun->NightEffect();
    }

    SetSlot(LitConst::SunDir, toSun.X(), toSun.Y(), toSun.Z(), _sunEnabled ? 1.0f : 0.0f);
    SetSlot(LitConst::Sun, rig.sun.r, rig.sun.g, rig.sun.b, rig.sunElevationDeg);
    SetSlot(LitConst::Zenith, rig.skyZenith.r, rig.skyZenith.g, rig.skyZenith.b, rig.exposure);
    SetSlot(LitConst::Horizon, rig.skyHorizon.r, rig.skyHorizon.g, rig.skyHorizon.b, _lighting.specular);
    // w: tree crowns lit as a canopy (B2, crownCanopy), 1 while its switch and
    // gate are on; 0 leaves shadeRemastered's foliage as it was before.
    const float crownCanopy = _featureGates[rm::Feature::CrownCanopy].on ? 1.0f : 0.0f;
    SetSlot(LitConst::HorizonSun, rig.skyHorizonSun.r, rig.skyHorizonSun.g, rig.skyHorizonSun.b, crownCanopy);

    // Fog toward the sun: the sky dome adds SunSkyColor * cos^6 to SkyColor
    // before modulating the sky texture (TLVertexTable::DoSkyLighting), while
    // the fog colour is the sky texture times SkyColor alone (Scene::
    // MainLightChanged). Recover the sky texture colour from the fog colour and
    // hand the shader the glow it should add toward the sun, capped at the sky
    // texture (the dome's vertex colour saturates at 1).
    {
        const Color sky = sun->SkyColor() * _accomodateEye;
        const Color glow = sun->SunSkyColor() * _accomodateEye;
        float tex[3] = {0.0f, 0.0f, 0.0f};
        const float fog[3] = {_fogColor.R(), _fogColor.G(), _fogColor.B()};
        const float lit[3] = {sky.R(), sky.G(), sky.B()};
        for (int i = 0; i < 3; ++i)
        {
            const float l = lit[i] > 1.0f ? 1.0f : lit[i];
            tex[i] = l > 1e-3f ? fog[i] / l : 0.0f;
            tex[i] = tex[i] > 1.0f ? 1.0f : tex[i];
        }
        const bool on = _worldEffectsOn;
        SetSlot(LitConst::FogSun, tex[0] * glow.R(), tex[1] * glow.G(), tex[2] * glow.B(), on ? 1.0f : 0.0f);
        SetSlot(LitConst::FogCap, tex[0], tex[1], tex[2], 0.0f);
    }
    SetSlot(LitConst::Ground, rig.skyGround.r, rig.skyGround.g, rig.skyGround.b, _lighting.localLights);
    SetSlot(LitConst::MoonDir, toMoon.X(), toMoon.Y(), toMoon.Z(), 0.0f);
    SetSlot(LitConst::Moon, rig.moon.r, rig.moon.g, rig.moon.b, 0.0f);
    const Vector3 cam = camera ? camera->Position() : VZero;
    SetSlot(LitConst::CamPos, cam.X(), cam.Y(), cam.Z(), 0.0f);

    // Terrain horizon: the two stored azimuths around the sun and the blend.
    const bool terrain = _worldEffectsOn && _terrainHorizonTex && _terrainMap && _terrainMap->Valid() &&
                         (_lighting.terrainShadows > 0.0f || _lighting.terrainOcclusion > 0.0f);
    if (terrain)
    {
        float azimuth = std::atan2(toSun.X(), toSun.Z()) / (2.0f * 3.14159265f);
        if (azimuth < 0.0f)
            azimuth += 1.0f;
        const float f = azimuth * static_cast<float>(rm::TerrainHorizonMap::kDirections);
        const int i0 = static_cast<int>(f) % rm::TerrainHorizonMap::kDirections;
        const int i1 = (i0 + 1) % rm::TerrainHorizonMap::kDirections;
        const float w = f - std::floor(f);
        SetSlot(LitConst::Terrain, 1.0f, 1.0f / _terrainMap->cellSize, 1.0f / static_cast<float>(_terrainMap->size),
                _lighting.terrainOcclusion);
        SetSlot(LitConst::TerrainSel0, i0 % 4 == 0, i0 % 4 == 1, i0 % 4 == 2, i0 % 4 == 3);
        SetSlot(LitConst::TerrainSel1, i1 % 4 == 0, i1 % 4 == 1, i1 % 4 == 2, i1 % 4 == 3);
        SetSlot(LitConst::TerrainSun, static_cast<float>(i0 / 4), static_cast<float>(i1 / 4), w, kTerrainPenumbraDeg);
        SetSlot(LitConst::TerrainLift, 3.0f, 30.0f, _lighting.terrainShadows, 0.0f);
    }
    else
    {
        SetSlot(LitConst::Terrain, 0.0f, 1.0f, 1.0f, 0.0f);
    }

    // Cloud field drifting with the wind.
    const float now = Glob.time.toFloat();
    if (s_cloudDriftTime >= 0.0f && !s_cloudDriftPinned)
    {
        float dt = now - s_cloudDriftTime;
        dt = dt < 0.0f ? 0.0f : (dt > 0.5f ? 0.5f : dt); // pauses and time skips do not jump the field
        Vector3 wind = land ? land->GetWind() : VZero;
        // Clouds move faster than the ground wind; keep a gentle drift in a calm.
        float wx = wind.X() * 4.0f, wz = wind.Z() * 4.0f;
        if (wx * wx + wz * wz < 9.0f)
        {
            wx += 2.4f;
            wz += 1.1f;
        }
        s_cloudDrift[0] -= wx * dt;
        s_cloudDrift[1] -= wz * dt;
    }
    s_cloudDriftTime = now;
    const bool clouds = _worldEffectsOn && rig.cloudCover > 0.0f;
    SetSlot(LitConst::Cloud, rig.cloudCover, rig.cloudTransmit, kCloudScale, clouds ? 1.0f : 0.0f);
    SetSlot(LitConst::CloudOffset, s_cloudDrift[0], s_cloudDrift[1], kCloudDeckAltitude, kCloudEdge);

    // The shadow map was rendered from where the camera was then.
    SetSlot(LitConst::ShadowDelta, cam.X() - _shadowCamPos[0], cam.Y() - _shadowCamPos[1], cam.Z() - _shadowCamPos[2],
            kShadowFilterTexels * _lighting.shadowSoftness);
    // Sun shadows (B1): the soft filter with its per-frame turn under TAA, and
    // the leaves' lift toward the sun; zero while their gates are off.
    const rm::SunShadowConstants sunShadows =
        rm::ComputeSunShadowConstants(_lighting, _featureGates, TaaWantedThisFrame(), _taa.sequence);
    SetSlot(LitConst::Taa, sunShadows.taa[0], sunShadows.taa[1], sunShadows.taa[2], sunShadows.taa[3]);
    SetSlot(LitConst::Pcss, sunShadows.pcss[0], sunShadows.pcss[1], sunShadows.pcss[2], sunShadows.pcss[3]);
    // Debug views of the forward shaders: 1 to 3 and the albedo (7, drawn as 4)
    // by rm::ShaderDebugView (shadeRemastered), and the lights per tile (6);
    // 4 and 5 are the post pass's.
    const int dv = _lighting.debugView;
    const float debug = dv == fl::kDebugViewLightTiles ? static_cast<float>(dv) : rm::ShaderDebugView(dv);
    SetSlot(LitConst::Flags, _litOpaque ? 1.0f : 0.0f, _worldEffectsOn ? 1.0f : 0.0f, debug, _lighting.saturation);
    // The sky table: the camera basis, the dome and fog blends, the clouds' light.
    UploadLitSky(toSun, in.skyThrough);
    // The sky's own clouds: lit clouds, the dome's turn, the sky-matched shadows.
    UploadLitClouds(in.skyThrough, clouds);
    // Terrain detail (B11): zero while the switch or its gate is off, which
    // leaves the terrain main the stock one. w (land or sea) is the draw's,
    // set by SetLitTerrainDraw.
    const std::array<float, 3> detail =
        rm::terrain_detail::FrameConstants(_lighting, _featureGates[rm::Feature::TerrainDetail].on);
    SetSlot(LitConst::Detail, detail[0], detail[1], detail[2], s_lit[LitConst::Detail * 4 + 3]);
    // Long tree shadows (B9): the sun-visibility map, rebuilt when the sun or
    // the camera moved enough; zero while the switch or its gate is off.
    UploadLitCanopy(toSun, _featureGates.inputs.sunElevationDeg, _featureGates.inputs.skyThrough);
    FlushLitConstants();
    BindLightingTextures();
}

void EngineGL33::SetLitSlot(int slot, float x, float y, float z, float w)
{
    SetSlot(slot, x, y, z, w);
}

void EngineGL33::SetLitSkyMode(int mode)
{
    // Twice a frame at most (the sky pass opening and closing): the whole
    // block through the one flush, so any record of what the buffer holds
    // stays true.
    s_lit[LitConst::SkyFwd * 4 + 3] = static_cast<float>(mode);
    FlushLitConstants();
}

void EngineGL33::SetLitTerrainDraw(bool land)
{
    // Per draw, but it changes only between the land and the sea (both drawn
    // by PSDetail), a few times a frame: upload the one slot when it does.
    if (!_lightingActive)
        return;
    float& w = s_lit[LitConst::Detail * 4 + 3];
    const float value = land ? 1.0f : 0.0f;
    if (w == value)
        return;
    w = value;
    UploadLitSlots(LitConst::Detail, 1);
}

void EngineGL33::PinCloudDrift(bool pinned, float x, float z)
{
    s_cloudDriftPinned = pinned;
    PinSkyShadowAnchor(pinned);
    if (pinned)
    {
        s_cloudDrift[0] = x;
        s_cloudDrift[1] = z;
    }
}

bool EngineGL33::GetCloudDrift(float& x, float& z) const
{
    x = s_cloudDrift[0];
    z = s_cloudDrift[1];
    return true;
}

void EngineGL33::UploadLitShadowDelta()
{
    if (!_remasterPrograms || !GScene || !GScene->GetCamera())
        return;
    const Vector3 cam = GScene->GetCamera()->Position();
    SetSlot(LitConst::ShadowDelta, cam.X() - _shadowCamPos[0], cam.Y() - _shadowCamPos[1], cam.Z() - _shadowCamPos[2],
            kShadowFilterTexels * _lighting.shadowSoftness);
    FlushLitConstants();
}

void EngineGL33::UploadLitMaterial(const Poseidon::TLMaterial& mat)
{
    if (!_lightingActive)
        return;
    SetSlot(LitConst::MatDiffuse, mat.diffuse.R(), mat.diffuse.G(), mat.diffuse.B(), 1.0f);
    SetSlot(LitConst::MatAmbient, mat.ambient.R(), mat.ambient.G(), mat.ambient.B(), 0.0f);
    SetSlot(LitConst::MatForced, mat.forcedDiffuse.R(), mat.forcedDiffuse.G(), mat.forcedDiffuse.B(), 0.0f);
    SetSlot(LitConst::MatEmissive, mat.emmisive.R(), mat.emmisive.G(), mat.emmisive.B(), 0.0f);
    SetSlot(LitConst::MatSpecular, mat.specular.R(), mat.specular.G(), mat.specular.B(),
            mat.specularPower > 0 ? static_cast<float>(mat.specularPower) : 0.0f);
    s_lit[LitConst::SunDir * 4 + 3] = _sunEnabled ? 1.0f : 0.0f;
    // The flush happens in UploadLitLights, which DoSetMaterial always calls next.
}

void EngineGL33::UploadLitLights(const LightList& lights, float nightEffect)
{
    if (!_lightingActive)
        return;
    if (_lightTilesActive)
    {
        // The frame's tiled list lights every fragment of the pass; the draw
        // adds only the factor this path scales its lights by (the night
        // effect, 1 for a material without sun, nothing by day).
        SetSlot(LitConst::LightCount, 0.0f, static_cast<float>(fl::kTileSize), nightEffect > 0.0f ? nightEffect : 0.0f,
                0.0f);
        FlushLitConstants();
        return;
    }
    // Same selection as the classic UploadVSLights, but the colours are the raw
    // light colours (the shader multiplies by the material) and positions are
    // camera-relative like vWorldRel.
    const float* camPos = _frameState.cameraPos;
    int n = 0;
    if (nightEffect > 0.0f)
    {
        for (int i = 0; i < lights.Size() && n < LitConst::MaxLights; i++)
        {
            Light* light = lights[i];
            if (!light)
                continue;
            LightDescription desc;
            light->GetDescription(desc);
            const bool isSpot = desc.type == LTSpotLight;
            if (desc.type != LTPoint && !isSpot)
                continue;
            SetSlot(LitConst::LightPos + n, desc.pos.X() - camPos[0], desc.pos.Y() - camPos[1],
                    desc.pos.Z() - camPos[2], desc.startAtten);
            Vector3 beam = desc.dir;
            beam.Normalize();
            SetSlot(LitConst::LightDir + n, beam.X(), beam.Y(), beam.Z(), 0.0f);
            const Color dif = desc.diffuse * nightEffect;
            const Color am = desc.ambient * nightEffect;
            SetSlot(LitConst::LightColor + n, dif.R(), dif.G(), dif.B(), isSpot ? 1.0f : 0.0f);
            SetSlot(LitConst::LightAmb + n, am.R(), am.G(), am.B(), 0.0f);
            n++;
        }
    }
    SetSlot(LitConst::LightCount, static_cast<float>(n), 0.0f, 0.0f, 0.0f);
    FlushLitConstants();
}
