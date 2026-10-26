#pragma once

// Remastered lighting: the shader sources, the LitConstants uniform layout and
// the texture units of the remastered inputs. See EngineGL33_LightingShaders.cpp
// for the shading model and Poseidon/Graphics/Remaster/LightRig.hpp for the
// light rig.

namespace Poseidon::gl33lighting
{

// Standalone vertex shader of the remastered set for a VertexShaderID, or
// nullptr.
const char* RemasterVertexSource(int vs);
// Fragment shaders are the common prelude followed by a main body. A null main
// means the pixel shader reuses its classic source (PSFlat, PSShadow).
const char* RemasterFragmentCommon();
const char* RemasterFragmentMain(int ps);

// The common prelude, split by concern (INTEGRATION-PLAN.md section 3.5):
// RemasterFragmentCommon() is these pieces joined in this order. A feature
// edits the piece of its concern. Overrides still replace the joined prelude
// whole (psRemasterCommon.glsl).
enum class FragmentPiece : int
{
    Header,        // #version and the classic PSConstants block
    LitConstants,  // LitConstantsBlock()
    Inputs,        // samplers, varyings, colour space and tonemap helpers
    Shadows,       // shadowMapVisibility
    TerrainClouds, // terrainSunVisibility, cloudSunVisibility
    Sky,           // skyIrradiance, fogColorToward (EngineGL33_LightingShadersSky.cpp)
    Lights,        // localLights (EngineGL33_LightingShadersLights.cpp)
    Surface,       // glossy (EngineGL33_LightingShadersSurface.cpp)
    Shade,         // shadeRemastered
    Finish,        // finishFragment
    Count,
};
const char* RemasterFragmentPiece(FragmentPiece piece);

// The pieces kept in files of their own (read them through RemasterFragmentPiece).
extern const char kRemasterSkyGLSL[];
extern const char kRemasterLightsGLSL[];
extern const char kRemasterSurfaceGLSL[];

// The LitConstants block (binding 3): the one GLSL declaration of the LitConst
// layout below. Every shader that reads the rig pastes this string instead of
// a copy of its own (today the fragment prelude).
const char* LitConstantsBlock();

// The post-pass shaders (EngineGL33_PostShaders.cpp; EngineGL33_PostFX.cpp
// runs the passes), each assembled from its pieces.
enum class PostShader : int
{
    Vertex, // the fullscreen triangle every pass draws
    Ao,
    AoBlur,
    BloomDown,
    BloomUp,
    Shafts,
    Composite,
    Count,
};
// The override file stem (vsPost, psPostAo, ...) and the assembled source.
const char* PostShaderName(PostShader shader);
const char* PostShaderSource(PostShader shader);

// Uniform-buffer binding point of LitConstants (VS = 0, PS = 1, WorldInstances = 2).
constexpr unsigned int kLitBinding = 3;

// Texture units of the remastered inputs (INTEGRATION-PLAN.md section 3.4).
// Units 0 and 1 hold the material textures and 7 is the upload unit
// (EngineGL33::kUploadUnit); GL 3.3 guarantees 16 fragment units. GL33Bind
// caches units 0 to 7 only, so units 8 and up bind on every call.
constexpr int kShadowMapUnit = 2; // cascade array; a complete depth array from init on, never texture 0
constexpr int kTerrainHorizonUnit = 3;
constexpr int kTerrainHeightUnit = 4;
constexpr int kCloudNoiseUnit = 5;
// Allocated to the features still to land; no shader samples them yet.
constexpr int kShadowRawUnit = 6;     // the cascade array again, non-compare NEAREST sampler (shadows, temporal)
constexpr int kSkyLutUnit = 8;        // sky-view table (sky table)
constexpr int kSkyDensityUnit = 9;    // cloud density map from the weather sky texture (clouds)
constexpr int kCanopyUnit = 10;       // canopy heightfield (hfrt)
constexpr int kSeaNextFrameUnit = 11; // next sea animation frame (water)
constexpr int kLightDataUnit = 12;    // light buffer, samplerBuffer (light list)
constexpr int kLightTileUnit = 13;    // per-tile light indices (light list)
constexpr int kCombatMaskUnit = 14;   // combat shadow mask (combat)
constexpr int kCanopyVisUnit = 16;    // canopy sun visibility (hfrt)

namespace detail
{
constexpr int kRemasterUnits[] = {kShadowMapUnit,    kTerrainHorizonUnit, kTerrainHeightUnit, kCloudNoiseUnit,
                                  kShadowRawUnit,    kSkyLutUnit,         kSkyDensityUnit,    kCanopyUnit,
                                  kSeaNextFrameUnit, kLightDataUnit,      kLightTileUnit,     kCombatMaskUnit,
                                  kCanopyVisUnit};
constexpr bool RemasterUnitsValid()
{
    constexpr int n = static_cast<int>(sizeof(kRemasterUnits) / sizeof(kRemasterUnits[0]));
    for (int i = 0; i < n; ++i)
    {
        const int unit = kRemasterUnits[i];
        if (unit < 2 || unit == 7 || unit > 15)
            return false;
        for (int j = 0; j < i; ++j)
            if (kRemasterUnits[j] == unit)
                return false;
    }
    return true;
}
} // namespace detail
static_assert(detail::RemasterUnitsValid(),
              "remastered texture units: 2 to 15, never the material units 0 and 1 or the upload unit 7, all distinct");

// Sampler uniform -> texture unit for every lit program, classic and
// remastered; InitPixelShaders binds them from this table and skips a name the
// program lacks. A shader that adds a sampler adds its row here: a sampler
// left unbound reads unit 0, the material texture.
struct LitSampler
{
    const char* name;
    int unit;
};
constexpr LitSampler kLitSamplers[] = {
    {"terrainHorizon", kTerrainHorizonUnit},
    {"terrainHeight", kTerrainHeightUnit},
    {"cloudNoise", kCloudNoiseUnit},
};

// LitConstants vec4 slots; must match LitConstantsBlock() (the layout test
// checks it member by member).
namespace LitConst
{
enum : int
{
    SunDir = 0,
    Sun = 1,
    Zenith = 2,
    Horizon = 3,
    Ground = 4,
    MoonDir = 5,
    Moon = 6,
    CamPos = 7,
    MatDiffuse = 8,
    MatAmbient = 9,
    MatForced = 10,
    MatEmissive = 11,
    MatSpecular = 12,
    Terrain = 13,
    TerrainSel0 = 14,
    TerrainSel1 = 15,
    TerrainSun = 16,
    TerrainLift = 17,
    Cloud = 18,
    CloudOffset = 19,
    ShadowDelta = 20,
    Flags = 21,
    LightCount = 22,
    HorizonSun = 23,
    FogSun = 24,
    FogCap = 25,
    LightPos = 26,   // MaxLights vec4
    LightColor = 34, // MaxLights vec4
    LightAmb = 42,   // MaxLights vec4
    LightDir = 50,   // MaxLights vec4
    Count = 58,
};
constexpr int MaxLights = 8;
static_assert(LightPos == FogCap + 1, "LitConst slots out of order");
static_assert(LightColor == LightPos + MaxLights, "LitConst light ranges overlap");
static_assert(LightAmb == LightColor + MaxLights, "LitConst light ranges overlap");
static_assert(LightDir == LightAmb + MaxLights, "LitConst light ranges overlap");
static_assert(Count == LightDir + MaxLights, "LitConst size out of sync");
} // namespace LitConst

// The slots allocated to the features still to land (INTEGRATION-PLAN.md
// section 3.3), so no two steps take the same one. The per-draw slot comes
// first, next to the per-draw material and light slots. A step that lands a
// feature moves its rows into LitConst, declares them in LitConstantsBlock()
// (a slot below it that has not landed yet becomes a padding member there) and
// raises LitConst::Count, which the upload and the shaders follow.
namespace LitConstPlanned
{
constexpr int MatClass = 58;     // materials, per draw: class, object kind, blend, hash
constexpr int Taa = 59;          // temporal: frame index, stochastic flags, taps, penumbra per metre
constexpr int Pcss = 60;         // shadows: search radius, minimum texels
constexpr int SkyRight = 61;     // sky pass, shared by the sky table and clouds: camera right * tan(fov x / 2)
constexpr int SkyUp = 62;        // sky pass: camera up * tan(fov y / 2)
constexpr int SkyFwd = 63;       // sky pass: camera forward
constexpr int Atmo = 64;         // sky table: radiance scale, overcast floor, aerial perspective, cloud mask
constexpr int AtmoCloudSun = 65; // sky table: sun light on the clouds (linear)
constexpr int AtmoCloudAmb = 66; // sky table: sky light on the clouds (linear)
constexpr int AtmoDome = 67;     // sky table: classic dome light (linear), hue blend
constexpr int Sky = 68;          // clouds: sky mode, lit-cloud strength, shadow source
constexpr int SkyVp = 69;        // clouds: viewport origin and 1 / size (pixels)
constexpr int LightTiles = 70;   // light list: tile size, tile counts, light count
constexpr int Night = 71;        // night: flags, beam profile, moon key
constexpr int CombatShadow = 72; // combat: mask scale, strength, shadowed count
constexpr int Weather = 73;      // materials: wetness, rain, puddles, time
constexpr int Water0 = 74;       // water: wave, swell, ripple, sky weight
constexpr int Water1 = 75;       // water: sheen, glint, sparkle
constexpr int Canopy = 76;       // hfrt: canopy heightfield constants
constexpr int Detail = 77;       // texsharp: terrain detail constants
constexpr int Haze = 78;         // haze: the analytic fallback for blended surfaces
constexpr int End = 79;          // one past the last planned slot
static_assert(MatClass == LitConst::Count, "the planned slots follow the declared ones");
// std140 vec4 slots; GL 3.3 guarantees uniform blocks of 16384 bytes.
static_assert(End * 16 <= 16384, "LitConstants stays within the GL minimum uniform block size");
} // namespace LitConstPlanned

} // namespace Poseidon::gl33lighting
