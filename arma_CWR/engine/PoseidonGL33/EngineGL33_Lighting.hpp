#pragma once

// Remastered lighting: the shader sources, the LitConstants uniform layout and
// the texture units of the remastered inputs. See EngineGL33_LightingShaders.cpp
// for the shading model and Poseidon/Graphics/Remaster/LightRig.hpp for the
// light rig.

namespace Poseidon::gl33lighting
{

// Vertex shader of the remastered set for a VertexShaderID, or nullptr. The
// screen vertex shader is assembled from its pieces (kScreenVertexPieces: it
// pastes LitConstantsBlock() for the sky pass); the others are standalone.
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
// The sky table's passes (EngineGL33_PostSkyTableShaders.cpp; read them through
// PostShaderSource).
extern const char kPostSkyTableHeaderGLSL[];
extern const char kPostSkyTableModelGLSL[];
extern const char kPostSkyViewMainGLSL[];
extern const char kPostSkyViewMeanMainGLSL[];

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
    LinearDepth, // half-resolution view depth (the LinearDepth post stage)
    DebugDepth,  // that depth drawn over the frame (the DebugView stage, debug view 5)
    Taa,         // the TAA resolve into the history (the Temporal stage)
    TaaSharpen,  // the resolved frame back into the frame target (the Sharpen stage)
    // The sky table's two fullscreen passes. Not post stages: they run before
    // the world when the sun has moved (EngineGL33_SkyTable.cpp).
    SkyView,     // regions 0 and 1 of the sky-view table
    SkyViewMean, // the whole table: those two copied and each row's mean
    LumaSharpen, // the composited frame, sharpened in brightness only, into the frame target (Sharpen, TAA off)
    Count,
};
// The override file stem (vsPost, psPostAo, ...) and the assembled source.
const char* PostShaderName(PostShader shader);
const char* PostShaderSource(PostShader shader);

// Uniform-buffer binding point of LitConstants (VS = 0, PS = 1, WorldInstances = 2).
constexpr unsigned int kLitBinding = 3;
// Uniform-buffer binding point of FrameLights, the frame's tiled light list
// (EngineGL33_LightingShadersLights.cpp, Graphics/Rendering/Lighting/FrameLights.hpp).
constexpr unsigned int kLightListBinding = 4;

// Texture units of the remastered inputs (INTEGRATION-PLAN.md section 3.4).
// Units 0 and 1 hold the material textures and 7 is the upload unit
// (EngineGL33::kUploadUnit); GL 3.3 guarantees 16 fragment units. GL33Bind
// caches units 0 to 7 only, so units 8 and up bind on every call.
constexpr int kShadowMapUnit = 2; // cascade array; a complete depth array from init on, never texture 0
constexpr int kTerrainHorizonUnit = 3;
constexpr int kTerrainHeightUnit = 4;
constexpr int kCloudNoiseUnit = 5;
// The cascade array again under a non-compare NEAREST sampler (the soft
// shadows' blocker search, B1): like unit 2, a complete depth array from init
// on (the stand-in, then each cascade array as it is created).
constexpr int kShadowRawUnit = 6;
constexpr int kSkyLutUnit = 8;     // sky-view table (the sky table, EngineGL33_SkyTable.cpp); a stand-in until built
constexpr int kLightTileUnit = 13; // per-tile light indices, usampler2D RGBA8UI (light list)
// The sea animation frame the water program cross-fades into (the sea,
// EngineGL33_Sea.cpp); a white stand-in outside the sea's draws.
constexpr int kSeaNextFrameUnit = 11;
// Allocated to the features still to land; no shader samples them yet.
// kLightDataUnit is free: the light list keeps its lights in a uniform block
// (kLightListBinding), which the fragments of a tile read faster.
constexpr int kLightDataUnit = 12;
constexpr int kSkyDensityUnit = 9;  // cloud density map from the weather sky texture (clouds)
constexpr int kCanopyUnit = 10;     // canopy heightfield (hfrt)
constexpr int kCombatMaskUnit = 14; // combat shadow mask (combat)
constexpr int kCanopyVisUnit = 15;  // canopy sun visibility (hfrt)

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

// Texture units the post stages bind their inputs to (EngineGL33_PostFX.cpp,
// EngineGL33_PostTemporal.cpp).
// The post pass sets their samplers to none before the first fullscreen stage
// and unbinds all of them at its end; a stage that binds any other unit
// restores that unit itself. Never unit 1 (its sampler object belongs to
// ApplyPipeline) or the upload unit 7.
constexpr int kPostUnits[] = {0, 2, 3, 4, 5};
namespace detail
{
constexpr bool PostUnitsValid()
{
    constexpr int n = static_cast<int>(sizeof(kPostUnits) / sizeof(kPostUnits[0]));
    for (int i = 0; i < n; ++i)
    {
        const int unit = kPostUnits[i];
        if (unit < 0 || unit == 1 || unit == 7 || unit > 15)
            return false;
        for (int j = 0; j < i; ++j)
            if (kPostUnits[j] == unit)
                return false;
    }
    return true;
}
} // namespace detail
static_assert(detail::PostUnitsValid(), "post texture units: 0 to 15, never unit 1 or the upload unit 7, all distinct");

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
    {"shadowMap", kShadowMapUnit},           // the cascade array (comparison sampler in remastered)
    {"terrainHorizon", kTerrainHorizonUnit}, // horizon angles
    {"terrainHeight", kTerrainHeightUnit},   // height, sky visibility, ground tint
    {"cloudNoise", kCloudNoiseUnit},         // cloud-shadow noise
    {"shadowDepth", kShadowRawUnit},         // the cascade array as raw depth (soft shadows)
    {"skyLut", kSkyLutUnit},                 // sky-view table (the sky table)
    {"seaNext", kSeaNextFrameUnit},          // next sea animation frame (the water program only)
    {"lightTiles", kLightTileUnit},          // per-tile light indices, unsigned integer (the light list)
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
    // 58: allocated to the materials' per-draw class (LitConstPlanned
    // MatClass); one padding member in the block until it lands.
    Reserved = 58,
    // Sun shadows (B1, Graphics/Remaster/SunShadows.hpp): x per-frame turn of
    // the soft shadows' taps, y 1 in a TAA frame, z taps per frame there,
    // w tan(sun angular radius) (0 = the stock filter).
    Taa = 59,
    // x blocker search reach (m), y smallest filter radius (texels), z soft
    // filter strength, w leaf lift toward the sun (texels).
    Pcss = 60,
    // The sky pass (the sky table; clouds read the camera basis too).
    SkyRight = 61,     // xyz camera right * tan(fov x / 2), w Mie asymmetry g
    SkyUp = 62,        // xyz camera up * tan(fov y / 2)
    SkyFwd = 63,       // xyz camera forward, w sky mode of the draws that follow (RemasterSky; 0 = world)
    Atmo = 64,         // x dome blend, y overcast cloud floor, z aerial perspective blend, w cloud mask dead zone
    AtmoCloudSun = 65, // rgb sun light on the sky clouds (linear), w sky light on them (linear; the table's colour)
    AtmoGlow = 66,     // rgb classic dome sun glow (display, SunSkyColor), w sky saturation
    AtmoDome = 67,     // rgb classic dome light (display, SkyColor), w hue (0 the original hue relit, 1 physical)
    // 68-73: allocated to features still to land (LitConstPlanned Sky to
    // Weather); one padding member in the block until they do.
    Planned = 68,
    // The sea (EngineGL33_Sea.cpp, Graphics/Remaster/SeaWaves.hpp).
    Water0 = 74, // x wave relief scale, y share of the next animation frame, z mean sky weight, w sky tint
    Water1 =
        75, // rgb sheen light (linear: the sun in the original sun's hue, times the sheen), w sheen's flat-normal share
    // 76: allocated to the canopy heightfield (LitConstPlanned Canopy); a
    // second padding member in the block until it lands.
    Planned2 = 76,
    Detail = 77, // terrain detail (B11): x self-detail strength, y detail kept, z anti-tiling, w land draw
    Count = 78,
};
constexpr int MaxLights = 8;
constexpr int ReservedCount = 1;
constexpr int PlannedCount = Water0 - Planned;
constexpr int Planned2Count = Detail - Planned2;
static_assert(LightPos == FogCap + 1, "LitConst slots out of order");
static_assert(LightColor == LightPos + MaxLights, "LitConst light ranges overlap");
static_assert(LightAmb == LightColor + MaxLights, "LitConst light ranges overlap");
static_assert(LightDir == LightAmb + MaxLights, "LitConst light ranges overlap");
static_assert(Reserved == LightDir + MaxLights, "LitConst reserved slots follow the lights");
static_assert(Taa == Reserved + ReservedCount && Pcss == Taa + 1, "LitConst sun shadow slots follow the reserved one");
static_assert(SkyRight == Pcss + 1, "LitConst sky slots follow the sun shadow ones");
static_assert(Planned == AtmoDome + 1, "the planned padding follows the sky slots");
static_assert(Water0 == Planned + PlannedCount && Water1 == Water0 + 1,
              "LitConst water slots follow the planned padding");
static_assert(Planned2 == Water1 + 1, "the second planned padding follows the water slots");
static_assert(Count == Detail + 1, "LitConst size out of sync");
} // namespace LitConst

// The slots allocated to the features still to land (INTEGRATION-PLAN.md
// section 3.3), so no two steps take the same one. The per-draw slot comes
// first, next to the per-draw material and light slots. A step that lands a
// feature moves its rows into LitConst, declares them in LitConstantsBlock()
// (a slot below it that has not landed yet becomes a padding member there) and
// raises LitConst::Count, which the upload and the shaders follow. A7 landed
// the sky slots 61 to 67 and B1 the sun shadow slots 59 and 60, so 58 is the
// padding member litReserved; B5 landed the water slots 74 and 75 and B11 the
// terrain detail at 77, so 68 to 73 are the padding member litPlanned and 76
// the padding member litPlanned2.
namespace LitConstPlanned
{
constexpr int MatClass = 58; // materials, per draw: class, object kind, blend, hash
// 59, 60: the sun shadows landed (LitConst::Taa, LitConst::Pcss, B1).
constexpr int Sky = 68;   // clouds: sky mode, lit-cloud strength, shadow source
constexpr int SkyVp = 69; // clouds: viewport origin and 1 / size (pixels)
// 70 is free: the light list (A6) needs only a tile size and a night factor,
// which ride in litLightCount.yz (LitConst::LightCount).
constexpr int Night = 71;        // night: flags, beam profile, moon key
constexpr int CombatShadow = 72; // combat: mask scale, strength, shadowed count
constexpr int Weather = 73;      // materials: wetness, rain, puddles, time
// 74, 75: the sea landed (LitConst::Water0, LitConst::Water1, B5).
constexpr int Canopy = 76; // hfrt: canopy heightfield constants
// 77: texsharp's terrain detail landed (LitConst::Detail, B11).
constexpr int Haze = 78; // haze: the analytic fallback for blended surfaces
constexpr int End = 79;  // one past the last planned slot
static_assert(MatClass == LitConst::Reserved && LitConst::ReservedCount == 1,
              "the planned slot below the sun shadows is the reserved padding");
static_assert(Sky == LitConst::Planned && Weather == LitConst::Planned + LitConst::PlannedCount - 1,
              "the planned slots between the sky pass and the water are the planned padding");
static_assert(Canopy == LitConst::Planned2 && LitConst::Planned2Count == 1 && Canopy == LitConst::Detail - 1,
              "the planned slot between the water and the terrain detail is the second planned padding");
static_assert(Haze == LitConst::Count, "the other planned slots follow the declared ones");
// std140 vec4 slots; GL 3.3 guarantees uniform blocks of 16384 bytes.
static_assert(End * 16 <= 16384, "LitConstants stays within the GL minimum uniform block size");
} // namespace LitConstPlanned

} // namespace Poseidon::gl33lighting
