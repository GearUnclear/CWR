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

// The common prelude, split by concern:
// RemasterFragmentCommon() is these pieces joined in this order. A feature
// edits the piece of its concern. Overrides still replace the joined prelude
// whole (psRemasterCommon.glsl).
enum class FragmentPiece : int
{
    Header,        // #version and the classic PSConstants block
    LitConstants,  // LitConstantsBlock()
    Inputs,        // samplers, varyings, colour space and tonemap helpers
    Shadows,       // shadowMapVisibility
    TerrainClouds, // terrainSunVisibility, cloudSunVisibility, the dome mapping and the sky-matched shadows
    Sky,           // skyIrradiance, fogColorToward, the sky table, lit clouds (EngineGL33_LightingShadersSky.cpp)
    Puddles,       // wetPuddle, where standing water collects (EngineGL33_LightingShadersSurface.cpp)
    Lights,        // localLights (EngineGL33_LightingShadersLights.cpp)
    Surface,       // glossy, the wet weather's response (EngineGL33_LightingShadersSurface.cpp)
    Shade,         // shadeRemastered
    Finish,        // finishFragment
    Count,
};
const char* RemasterFragmentPiece(FragmentPiece piece);

// The pieces kept in files of their own (read them through RemasterFragmentPiece).
// The LitConstants block and the terrain and clouds piece are named here too:
// the post air and fog passes paste them, and the reflections' trace pastes the
// block, the sky piece and the puddles piece (kPostShaders).
extern const char kLitConstantsBlockGLSL[];
// The photoreal camera's frame encoding (camFold, camDecode; photoLight), which
// the post passes that read or write the frame's colour paste.
extern const char kCameraCodecGLSL[];
extern const char kRemasterTerrainCloudsGLSL[];
extern const char kRemasterSkyGLSL[];
extern const char kRemasterPuddlesGLSL[];
extern const char kRemasterLightsGLSL[];
extern const char kRemasterSurfaceGLSL[];
// The sky table's passes (EngineGL33_PostSkyTableShaders.cpp; read them through
// PostShaderSource).
extern const char kPostSkyTableHeaderGLSL[];
extern const char kPostSkyTableModelGLSL[];
extern const char kPostSkyViewMainGLSL[];
extern const char kPostSkyViewMeanMainGLSL[];
// The canopy's sun-visibility pass (EngineGL33_PostCanopyShaders.cpp; read it
// through PostShaderSource).
extern const char kPostCanopySunGLSL[];
// The air pass (EngineGL33_PostAirShaders.cpp; read them through
// PostShaderSource).
extern const char kPostAirMarchInputsGLSL[];
extern const char kPostAirCommonGLSL[];
extern const char kPostAirMarchHelpersGLSL[];
extern const char kPostAirMarchMainGLSL[];
extern const char kPostAirBlurGLSL[];
extern const char kPostAirMeanGLSL[];
extern const char kPostAirApplyInputsGLSL[];
extern const char kPostAirApplyMainGLSL[];
// The screen-space reflections' passes (EngineGL33_PostSsrShaders.cpp; read
// them through PostShaderSource).
extern const char kPostSsrTagGLSL[];
extern const char kPostSsrShareGLSL[];
extern const char kPostSsrTraceInputsGLSL[];
extern const char kPostSsrTraceMainGLSL[];
extern const char kPostSsrBlurGLSL[];
extern const char kPostSsrCompositeInputsGLSL[];
extern const char kPostSsrCompositeMainGLSL[];
// The combat shadow mask's passes (EngineGL33_PostCombatShaders.cpp; read them
// through PostShaderSource).
extern const char kPostCombatMaskGLSL[];
extern const char kPostCombatMaskBlurGLSL[];
// The surfaces' horizon occlusion (EngineGL33_PostSurfaceShaders.cpp;
// read it through PostShaderSource).
extern const char kPostGtaoInputsGLSL[];
extern const char kPostGtaoMainGLSL[];
extern const char kPostGtaoAccumGLSL[];
// The volumetric fog (the Air stage, EngineGL33_PostFogShaders.cpp; read them
// through PostShaderSource).
extern const char kPostFogInjectHeadGLSL[];
extern const char kPostFogSharedInputsGLSL[];
extern const char kPostFogSkyInputsGLSL[];
extern const char kPostFogMediumGLSL[];
extern const char kPostFogInjectMainGLSL[];
extern const char kPostFogIntegrateGLSL[];
extern const char kPostFogNearHeadGLSL[];
extern const char kPostFogNearMainGLSL[];
extern const char kPostFogRingGLSL[];
extern const char kPostFogRingMainGLSL[];
extern const char kPostFogApplyGLSL[];
extern const char kPostFogApplyMainGLSL[];
extern const char kPostFogGlowGLSL[];
extern const char kPostFogGlowMainGLSL[];

// The LitConstants block (binding 5): the one GLSL declaration of the LitConst
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
    // The canopy's sun-visibility map. Not a post stage: it runs before
    // the world when the sun or the camera moved (EngineGL33_Canopy.cpp).
    CanopySun,
    // The Air stage (EngineGL33_PostAir.cpp): the half-resolution march
    // (beams, haze sun visibility, valley mist top), its depth-aware blur, the
    // beams' local mean, and the full-resolution apply (haze, mist, beams).
    AirMarch,
    AirBlur,
    AirMean,
    AirApply,
    // The screen-space reflections (the TagReadout and Ssr stages).
    SsrTag,       // a constant where the stencil test passes: the tags read out bit by bit
    SsrShare,     // the screen shares of water and wettable ground, on a small grid
    SsrTrace,     // the half-resolution trace: the reflection as a multiply and an add, and its spread
    SsrBlur,      // the trace gathered sideways over its spread (rough reflections have no hard outline)
    SsrComposite, // the reflections laid on the full-resolution frame
    // The combat shadow mask. Not post stages: they run at the start of
    // the world, from the previous frame's linear depth (EngineGL33_PostCombat.cpp).
    CombatMask,     // the trace toward each shadowed combat light, half resolution
    CombatMaskBlur, // its depth-aware blur, one axis per pass
    // The horizon-based ambient occlusion of the surfaces switch (the Ao stage
    // with the switch on; EngineGL33_PostSurfaceShaders.cpp).
    Gtao,
    GtaoAccum, // its occlusion accumulated over TAA frames, reprojected by depth
    // The photoreal camera's meter (photoLight): the resolved frame's log
    // luminance in kMeterWidth x kMeterHeight texels, read back by the CPU
    // (EngineGL33_Camera.cpp); run inside the Composite stage.
    Meter,
    // The volumetric fog (the Air stage, EngineGL33_PostFog.cpp): the froxel
    // volume's lighting, eight slices per draw; its front-to-back integration;
    // the half-resolution near field (the sun's beams, the lamps' light in the
    // air); and the full-resolution apply by each pixel's depth.
    FogInject,
    FogIntegrate,
    FogNear,
    FogApply,
    FogRing, // the horizon ring: the sky's colour above the land in every azimuth
    FogGlow, // the night glow: the frame's light blurred, laid into the fog at night
    Count,
};
// The override file stem (vsPost, psPostAo, ...) and the assembled source.
const char* PostShaderName(PostShader shader);
const char* PostShaderSource(PostShader shader);

// Uniform-buffer binding point of LitConstants (VS = 0, PS = 1, WorldInstances = 2,
// LightIndices = 3, LocalLights = 4).
constexpr unsigned int kLitBinding = 5;
// Uniform-buffer binding point of FrameLights, the frame's tiled light list
// (EngineGL33_LightingShadersLights.cpp, Graphics/Rendering/Lighting/FrameLights.hpp).
constexpr unsigned int kLightListBinding = 6;

// Texture units of the remastered inputs.
// Units 0 and 1 hold the material textures, 3 to 6 the instanced terrain's
// inputs (3, the height map, also the land clip of every lit program) and 7 is
// the upload unit (EngineGL33::kUploadUnit). GL 3.3 guarantees 16 units per
// stage and 48 combined, so the remastered inputs take 2, 8 to 15 and 16 to 19
// (15 fragment samplers). GL33Bind caches units 0 to 7 only, so units 8 and up
// bind on every call.
constexpr int kShadowMapUnit = 2; // cascade array; a complete depth array from init on, never texture 0
constexpr int kTerrainHorizonUnit = 16;
constexpr int kTerrainHeightUnit = 17;
constexpr int kCloudNoiseUnit = 18;
// The cascade array again under a non-compare NEAREST sampler (the soft
// shadows' blocker search): like unit 2, a complete depth array from init
// on (the stand-in, then each cascade array as it is created).
constexpr int kShadowRawUnit = 19;
constexpr int kSkyLutUnit = 8;     // sky-view table (the sky table, EngineGL33_SkyTable.cpp); a stand-in until built
constexpr int kSkyDensityUnit = 9; // the weather sky's cloud density (EngineGL33_SkyClouds.cpp); a stand-in
constexpr int kLightTileUnit = 13; // per-tile light indices, usampler2D RGBA8UI (light list)
// The sea animation frame the water program cross-fades into (the sea,
// EngineGL33_Sea.cpp); a white stand-in outside the sea's draws.
constexpr int kSeaNextFrameUnit = 11;
// The canopy heightfield (EngineGL33_Canopy.cpp): sampled by the pass that
// builds the sun-visibility map and, for the sky it hides (the surfaces
// switch), by the world programs; holds a stand-in until the first canopy
// is uploaded.
constexpr int kCanopyUnit = 10;
// The canopy's sun-visibility map: a world-space map around the camera
// the world programs read beyond the shadow maps; a stand-in (all lit) until
// the first map is built.
constexpr int kCanopyVisUnit = 15;
// The combat shadow mask: RGBA8 at half resolution, one shadowed combat
// light per channel, in the previous frame's screen; a 1x1 white stand-in
// (nothing shadowed) until the first mask is built.
constexpr int kCombatMaskUnit = 14;
// The ground field (the wet weather's land classes; TerrainHorizonMap::
// groundColor): RGBA16, the land's own colour per terrain vertex, a = 1 where
// built; a transparent black stand-in (a = 0) until a map with ground colours
// is uploaded. The world programs and the reflections' trace read it (the
// trace binds it itself and unbinds it). It takes the unit the light list's
// data was allocated (the list keeps its lights in a uniform block,
// kLightListBinding, which the fragments of a tile read faster).
constexpr int kGroundFieldUnit = 12;
// The volumetric fog's floor map (the lowest ground nearby) shares
// the ground field's unit: only the fog's injection pass samples it, binds it
// itself (no world program declares it) and unbinds it at its end; the world
// binds the ground field again at each frame's lighting upload
// (BindLightingTextures).
constexpr int kFogFloorUnit = kGroundFieldUnit;
// The surface's share of each pixel's light after the fog (FogState::veilTex),
// read by the post composite on the ground field's unit after the fog pass
// has handed it back; the composite unbinds it again, and the world binds the
// ground field again at the next lighting upload (BindLightingTextures).
constexpr int kFogVeilUnit = kGroundFieldUnit;
// The volumetric fog's horizon ring (EngineGL33_PostFog.cpp), read by its
// injection in the post pass on the light list's tile unit, which the world
// binds again at each frame's lighting upload (BindLightingTextures).
constexpr int kFogRingUnit = kLightTileUnit;
// The fog's night glow, read by the fog apply on the same unit (the apply
// reads no ring). It has no sampler object, so the glow keeps its own
// clamped filtering; the apply unbinds it at its end.
constexpr int kFogGlowUnit = kLightTileUnit;

namespace detail
{
constexpr int kRemasterUnits[] = {kShadowMapUnit,    kTerrainHorizonUnit, kTerrainHeightUnit, kCloudNoiseUnit,
                                  kShadowRawUnit,    kSkyLutUnit,         kSkyDensityUnit,    kCanopyUnit,
                                  kSeaNextFrameUnit, kGroundFieldUnit,    kLightTileUnit,     kCombatMaskUnit,
                                  kCanopyVisUnit};
constexpr bool RemasterUnitsValid()
{
    constexpr int n = static_cast<int>(sizeof(kRemasterUnits) / sizeof(kRemasterUnits[0]));
    for (int i = 0; i < n; ++i)
    {
        const int unit = kRemasterUnits[i];
        if (unit < 2 || (unit >= 3 && unit <= 7) || unit > 19)
            return false;
        for (int j = 0; j < i; ++j)
            if (kRemasterUnits[j] == unit)
                return false;
    }
    return true;
}
} // namespace detail
static_assert(detail::RemasterUnitsValid(),
              "remastered texture units: 2 or 8 to 19, never the material units 0 and 1, the terrain units 3 to 6 or "
              "the upload unit 7, all distinct");

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
    {"heightMap", 3},                        // the terrain height map of the land clip (vsTransform)
    {"terrainHorizon", kTerrainHorizonUnit}, // horizon angles
    {"terrainHeight", kTerrainHeightUnit},   // height, sky visibility, ground tint
    {"cloudNoise", kCloudNoiseUnit},         // cloud-shadow noise
    {"shadowDepth", kShadowRawUnit},         // the cascade array as raw depth (soft shadows)
    {"skyLut", kSkyLutUnit},                 // sky-view table (the sky table)
    {"skyDensity", kSkyDensityUnit},         // the weather sky's cloud density (the clouds)
    {"seaNext", kSeaNextFrameUnit},          // next sea animation frame (the water program only)
    {"lightTiles", kLightTileUnit},          // per-tile light indices, unsigned integer (the light list)
    {"canopySun", kCanopyVisUnit},           // the canopy's sun-visibility map (long tree shadows)
    {"combatMask", kCombatMaskUnit},         // the combat shadow mask (combat light)
    {"canopyHeight", kCanopyUnit},           // the canopy heightfield (the sky it hides: surfaces)
    {"groundField", kGroundFieldUnit},       // the land's own colour (the wet weather's land classes)
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
    // Wet weather (Graphics/Remaster/SurfaceMaterials.hpp): per draw x the
    // surface class, y the object kind (SetLitSurface, only while the wet
    // gate is on); per frame z 1 while the lamp streaks run, w 1 while the
    // reflections' pass reflects the wet ground (UploadLitWeather).
    MatClass = 58,
    // Sun shadows (Graphics/Remaster/SunShadows.hpp): x per-frame turn of
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
    // The sky's own clouds (EngineGL33_SkyClouds.cpp, Graphics/Remaster/SkyDensity.hpp).
    Sky = 68,       // x lit-cloud strength, y sky-matched shadows on (1), z the dome's turn (u offset), w sky-through
    SkyShadow = 69, // xy shadow anchor - camera (m, east and north), z previous anchor's angle (rad), w its weight
    // The night (Graphics/Remaster/NightLights.hpp). SunDir and Sun carry
    // the key light, which under the moon key is the moon; the sky reads the
    // sun from SkySun: xyz toward the sun, w its elevation (degrees). Equal to
    // SunDir.xyz and Sun.w whenever the sun is the key.
    SkySun = 70,
    // x 1 while the physical lamps and low beams light the frame, y lamp
    // intensity per metre of start distance, z low-beam hot-spot intensity,
    // w the moon key's share (0: the sun is the key).
    Night = 71,
    // 72: allocated to the combat shadow mask (LitConstPlanned CombatShadow);
    // one padding member in the block until it lands.
    Planned = 72,
    // Wet weather (Graphics/Remaster/Wetness.hpp): x surface wetness, y
    // rain density, z puddle level, w time (s, wrapped); zero while the
    // wetWeather gate is off, which leaves every fragment as before.
    Weather = 73,
    // The sea (EngineGL33_Sea.cpp, Graphics/Remaster/SeaWaves.hpp).
    Water0 = 74, // x wave relief scale, y share of the next animation frame, z mean sky weight, w sky tint
    Water1 =
        75, // rgb sheen light (linear: the sun in the original sun's hue, times the sheen), w sheen's flat-normal share
    // Long tree shadows (Graphics/Remaster/Canopy.hpp): x strength (0 =
    // off), yz the sun-visibility map's origin (m), w 1 / its extent (1/m).
    Canopy = 76,
    Detail = 77, // terrain detail: x self-detail strength, y detail kept, z anti-tiling, w land draw
    // Aerial haze (Graphics/Remaster/AirMedium.hpp) or the volumetric fog
    // (Graphics/Remaster/VolumetricFog.hpp): x the classic fog factor's scale
    // minus 1 (0 = the classic fog: the zero the block starts with leaves every
    // fragment as before), yzw free.
    Haze = 78,
    // Surfaces and ambient light (the surfaces switch, SurfaceLight.hpp):
    // x on (the switch and its gate), y specular strength, z canopy occlusion
    // strength, w the spherical-harmonics sky light's weight.
    Surface = 79,
    // The canopy heightfield on unit 10 as the world programs read it: x its
    // height base (m), y its height range (m), z 1 / its extent (1/m), w 1 when
    // a canopy is uploaded.
    SurfaceCanopy = 80,
    Surface2 = 81,  // surfaces: x relief strength, y debug view (1 sky light, 2 canopy sky, 3 specular), z sky tint
    SurfaceSh = 82, // kSkyShCoeffs vec4: the sky light's irradiance coefficients (rgb), the sun frame
    Surface3 = 88,  // surfaces: x cavity strength
    // The photoreal camera (photoLight; Graphics/Remaster/ PhotoCamera.hpp): x
    // 1 = the world's passes write scene light for the camera (camEncode), y
    // the additive pre-lit draws' code scale, z the clear sky's scale
    // (LightRig::skyScale), w the frame's pre-exposure.
    Camera = 89,
    // The camera's look of the world (photoLight; zero while it is off): x the
    // weather's veil (0 clear .. 1 a grey sky over broken cloud), y a street
    // lamp's classic ambient kept (kCameraLampAmbient), z a street lamp's
    // intensity gain (kCameraLampGain), w how far a low sun turns the upper
    // sky toward the zenith's blue.
    CameraLook = 90,
    Count = 91,
};
constexpr int MaxLights = 8;
constexpr int PlannedCount = Weather - Planned;
constexpr int SurfaceShCount = 6;
static_assert(LightPos == FogCap + 1, "LitConst slots out of order");
static_assert(LightColor == LightPos + MaxLights, "LitConst light ranges overlap");
static_assert(LightAmb == LightColor + MaxLights, "LitConst light ranges overlap");
static_assert(LightDir == LightAmb + MaxLights, "LitConst light ranges overlap");
static_assert(MatClass == LightDir + MaxLights, "LitConst material class follows the lights");
static_assert(Taa == MatClass + 1 && Pcss == Taa + 1, "LitConst sun shadow slots follow the material class");
static_assert(SkyRight == Pcss + 1, "LitConst sky slots follow the sun shadow ones");
static_assert(Sky == AtmoDome + 1 && SkyShadow == Sky + 1, "LitConst clouds slots follow the sky table's");
static_assert(SkySun == SkyShadow + 1 && Night == SkySun + 1, "LitConst night slots follow the clouds slots");
static_assert(Planned == Night + 1, "the planned padding follows the night slots");
static_assert(Weather == Planned + PlannedCount, "the weather follows the planned padding");
static_assert(Water0 == Weather + 1 && Water1 == Water0 + 1, "LitConst water slots follow the weather");
static_assert(Canopy == Water1 + 1, "the canopy follows the water slots");
static_assert(Detail == Canopy + 1, "the terrain detail follows the canopy");
static_assert(Haze == Detail + 1, "LitConst haze slot follows the terrain detail");
static_assert(Surface == Haze + 1, "the surface slots follow the haze");
static_assert(SurfaceCanopy == Surface + 1 && Surface2 == SurfaceCanopy + 1 && SurfaceSh == Surface2 + 1,
              "LitConst surface slots out of order");
static_assert(Surface3 == SurfaceSh + SurfaceShCount, "LitConst surface slots out of order");
static_assert(Camera == Surface3 + 1, "the camera slot follows the surface slots");
static_assert(CameraLook == Camera + 1, "the camera's look follows the camera slot");
static_assert(Count == CameraLook + 1, "LitConst size out of sync");
} // namespace LitConst

// The slots allocated to features not yet in the block, so no two features
// take the same one. The per-draw slot comes first, next to the per-draw
// material and light slots. A feature that lands moves its rows into LitConst,
// declares them in LitConstantsBlock() (a slot below it that has not landed yet
// becomes a padding member there) and raises LitConst::Count, which the upload
// and the shaders follow. Landed so far: the material class at 58 and the
// weather at 73, the sky slots 61 to 67, the sun shadow slots 59 and 60, the
// clouds slots 68 and 69 (Sky, SkyShadow), the night slots 70 and 71 (SkySun,
// Night), the water slots 74 and 75, the canopy at 76 and the terrain detail
// at 77, so 72 is the padding member litPlanned; the haze at 78, the surface
// slots after it (79 to 88) and the photoreal camera at 89 and 90.
namespace LitConstPlanned
{
// 58: the wet weather's material class landed (LitConst::MatClass).
// 59, 60: the sun shadows landed (LitConst::Taa, LitConst::Pcss).
// 68, 69: the sky's clouds landed (LitConst::Sky, LitConst::SkyShadow).
// 70, 71: the night landed (LitConst::SkySun, LitConst::Night); 70 was
// free (the light list rides in litLightCount.yz, LitConst::LightCount).
constexpr int CombatShadow = 72; // combat: mask scale, strength, shadowed count
// 73: the wet weather landed (LitConst::Weather).
// 74, 75: the sea landed (LitConst::Water0, LitConst::Water1).
// 76: the canopy landed (LitConst::Canopy); 77: the terrain detail landed
// (LitConst::Detail); 78: the haze landed (LitConst::Haze).
constexpr int End = 79; // one past the last planned slot
static_assert(CombatShadow == LitConst::Planned && LitConst::PlannedCount == 1,
              "the planned slot between the night and the weather is the planned padding");
static_assert(End == LitConst::Haze + 1, "every planned slot above the padding has landed");
// std140 vec4 slots; GL 3.3 guarantees uniform blocks of 16384 bytes.
static_assert(End * 16 <= 16384 && LitConst::Count * 16 <= 16384,
              "LitConstants stays within the GL minimum uniform block size");
} // namespace LitConstPlanned

} // namespace Poseidon::gl33lighting
