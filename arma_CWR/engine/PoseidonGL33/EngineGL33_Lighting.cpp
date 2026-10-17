// Remastered lighting: GL resources and the LitConstants uploads.
//
// The remastered program set (EngineGL33_LightingShaders.cpp) reads the light
// rig, the per-draw material and the local lights from the LitConstants block
// (binding 3), the shadow map through a comparison sampler on unit 2, the
// terrain horizon map on units 3 and 4 and the cloud noise on unit 5. Selection
// between the remastered and classic sets happens in DoSelectPixelShader via
// SelectProgram; Classic leaves every classic upload and program untouched.

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <Poseidon/Graphics/Remaster/CloudNoise.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>

#include <cmath>
#include <cstring>

using namespace Poseidon::gl33lighting;
namespace rm = Poseidon::remaster;

namespace
{
float s_lit[LitConst::Count * 4] = {};
GLuint s_litUBO = 0;
// Cloud drift integrated from the wind, so a wind change bends the drift
// instead of jumping the whole field.
float s_cloudDrift[2] = {1830.0f, 970.0f};
float s_cloudDriftTime = -1.0f;

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

void EngineGL33::FlushLitConstants()
{
    if (!s_litUBO)
        return;
    glBindBuffer(GL_UNIFORM_BUFFER, s_litUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(s_lit), s_lit);
}

void EngineGL33::InitLightingResources()
{
    glGenBuffers(1, &s_litUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, s_litUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(s_lit), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, kLitBinding, s_litUBO);

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

    glGenSamplers(1, &_shadowCompareSampler);
    glSamplerParameteri(_shadowCompareSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(_shadowCompareSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(_shadowCompareSampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(_shadowCompareSampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(_shadowCompareSampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glSamplerParameteri(_shadowCompareSampler, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    // The map (if any) belongs to the landscape, not the context: re-upload it.
    _terrainMapDirty = _terrainMap != nullptr;
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
    _post.unavailable = false;
    if (_shadowCompareSampler)
    {
        glBindSampler(2, 0);
        glDeleteSamplers(1, &_shadowCompareSampler);
        _shadowCompareSampler = 0;
    }
    if (s_litUBO)
    {
        glDeleteBuffers(1, &s_litUBO);
        s_litUBO = 0;
    }
}

void EngineGL33::SetLightingTuning(const rm::LightingTuning& tuning)
{
    const bool wasActive = _lightingActive;
    _lighting = tuning;
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
    glBindSampler(2, on ? _shadowCompareSampler : 0);
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
    GL33Bind::ActiveUnit(2);
    if (!(ShadowMapsEnabled() && _shadowMapActive && _shadowMapTex && _shadowCascades > 0))
        glBindTexture(GL_TEXTURE_2D_ARRAY, _dummyDepthArray);
    glBindSampler(2, _shadowCompareSampler);
    GL33Bind::ActiveUnit(kTerrainHorizonUnit);
    glBindTexture(GL_TEXTURE_2D_ARRAY, map ? _terrainHorizonTex : _dummyHorizonTex);
    GL33Bind::Tex2D(kTerrainHeightUnit, map ? _terrainHeightTex : _dummyHeightTex);
    GL33Bind::Tex2D(kCloudNoiseUnit, _cloudNoiseTex);
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
    if (s_litUBO)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, s_litUBO);
        glBufferSubData(GL_UNIFORM_BUFFER, LitConst::Flags * 16, 16, s_lit + LitConst::Flags * 4);
    }
}

void EngineGL33::UploadLitFrameConstants()
{
    if (!_remasterPrograms || !GScene)
        return;
    if (_terrainMapDirty)
        UploadTerrainHorizonMap();

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
    const rm::LightRig rig = rm::ComputeLightRig(in, _lighting);
    _lightRigSun = Color(rig.sun.r, rig.sun.g, rig.sun.b, 1.0f);

    SetSlot(LitConst::SunDir, toSun.X(), toSun.Y(), toSun.Z(), _sunEnabled ? 1.0f : 0.0f);
    SetSlot(LitConst::Sun, rig.sun.r, rig.sun.g, rig.sun.b, rig.sunElevationDeg);
    SetSlot(LitConst::Zenith, rig.skyZenith.r, rig.skyZenith.g, rig.skyZenith.b, rig.exposure);
    SetSlot(LitConst::Horizon, rig.skyHorizon.r, rig.skyHorizon.g, rig.skyHorizon.b, _lighting.specular);
    SetSlot(LitConst::HorizonSun, rig.skyHorizonSun.r, rig.skyHorizonSun.g, rig.skyHorizonSun.b, 0.0f);

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
    if (s_cloudDriftTime >= 0.0f)
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
    const float debug =
        _lighting.debugView >= 1 && _lighting.debugView <= 3 ? static_cast<float>(_lighting.debugView) : 0.0f;
    SetSlot(LitConst::Flags, _litOpaque ? 1.0f : 0.0f, _worldEffectsOn ? 1.0f : 0.0f, debug, _lighting.saturation);
    FlushLitConstants();
    BindLightingTextures();
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
