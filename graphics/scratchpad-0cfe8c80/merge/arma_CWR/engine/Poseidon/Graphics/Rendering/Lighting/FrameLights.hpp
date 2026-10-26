#pragma once

#include <cstdint>
#include <vector>

// The frame's light list (INTEGRATION-PLAN.md section 5 A6, the lightTiles
// switch): every local light the remastered shaders shade, in one block,
// binned into screen tiles so a fragment loops over only the lights whose
// reach covers its tile. One list for every surface: terrain, roads and
// objects see the same lights, instead of the list each draw selected for
// itself (Scene::SelectLights: up to 7 lights, ranked from the object's or the
// terrain segment's centre, with a brightness cut at its bounding sphere).
//
// Pure: no GL, no engine state. The GL33 backend fills FrameLight records from
// the scene's active lights once per frame, ranks them, bins them against the
// frame's camera and uploads the packed data (EngineGL33_Lighting.cpp,
// UpdateLightTiles); the shader side is localLightsTiled() in
// EngineGL33_LightingShadersLights.cpp.
//
// The classic local-light model (full strength inside the start distance, a
// quadratic falloff past it ending in a smooth window at kRangeFactor times
// it, a smoothed spotlight cone, the sum taken in display space and clamped at
// 1, as the per-draw path does) shades every kind, except that at night, with
// the nightLights switch on (B3), street lamps and headlights take the
// physical model below: light falling off with the squared distance from the
// bulb, a low-beam pattern for headlights, summed in linear light. The kind
// travels with each light so the steps that give a kind its own model (B4 fire
// and combat light) find it here.

namespace Poseidon::lighting
{

enum class FrameLightKind : std::uint8_t
{
    Classic, // any other point light: vehicle marker and cargo lights, lightning, script lights
    Lamp,    // a street lamp (StreetLamp)
    Spot,    // a headlight (LightReflector)
    Fire,    // a camp fire (Fireplace)
    Combat,  // a muzzle flash, an explosion or burning wreck, a flare, a missile motor
};

const char* FrameLightKindName(FrameLightKind kind);

struct PhysicalLightParams; // the physical model's intensities (below)

// The classic local-light model the shaders apply (EngineGL33_LightingShadersLights.cpp
// localLight; the test pins the shader's numbers to these).
constexpr float kRangeFactor = 10.0f;     // the reach ends at this many start distances (end2 = 100 * start2)
constexpr float kSpotOuterCos2 = 0.9415f; // squared cosine of the cone edge (about 14 deg): nothing outside
constexpr float kSpotInnerCos2 = 0.9806f; // full strength inside this (about 8 deg)

// The classic engine lights a surface with at most this many local lights:
// the ones its own selection ranks strongest by sort brightness over the
// squared distance (Scene::SelectLights, per object or terrain segment). The
// tiled list keeps that budget, ranked at the fragment (ClassicBudgetWeights).
// Without it a lamp-lit wall in a town took the ambient term of every lamp in
// reach (a street lamp's start distance is 50 m, its reach 500 m) and went
// pale and flat; Saint Louis at night has 10 lights on.
constexpr int kClassicLightsPerSurface = 7;
// Where the 7th and 8th strongest are within this share of the 7th's
// importance of each other, they share a blend instead of a cut, so the set
// changes smoothly across a surface.
constexpr float kClassicBudgetSoftness = 0.25f;

// Capacities. The GL buffer and texture are sized from these.
constexpr int kMaxFrameLights = 64;   // lights in the frame's buffer; an index + 1 fits one byte
constexpr int kTexelsPerLight = 4;    // vec4s (four floats) per light in the light block
constexpr int kMaxLightsPerTile = 16; // light indices per tile
constexpr int kIndicesPerTexel = 4;   // RGBA8UI tile texture: four indices per texel
constexpr int kTileTexels = kMaxLightsPerTile / kIndicesPerTexel; // texels per tile, side by side in a row
constexpr int kTileSize = 32;                                     // tile side in frame-target pixels
static_assert(kMaxFrameLights <= 255, "a light index + 1 must fit the tile texture's byte");
static_assert(kMaxLightsPerTile % kIndicesPerTexel == 0, "whole texels per tile");

// LightingTuning::debugView value that shows the number of lights per tile over
// the frame (shadeRemastered), with the tile grid.
constexpr int kDebugViewLightTiles = 6;

struct FrameLight
{
    FrameLightKind kind = FrameLightKind::Classic;
    float pos[3] = {};                 // camera-relative world position (m), like the shaders' vWorldRel
    float start = 1.0f;                // start distance (m): full strength inside it (LightDescription::startAtten)
    float diffuse[3] = {};             // display-space colour the light adds at full strength
    float ambient[3] = {};             // display-space colour it adds whatever the surface faces
    float dir[3] = {0.0f, 0.0f, 1.0f}; // unit beam direction (spotlights)
    float sortBrightness = 0.0f;       // Light::SortBrightness: the ranking weight (Light::Compare)
    // The object the light is attached to (Object::ID, 0 = none). The shaders
    // do not read it: a draw in an instanced run has no single object, so the
    // physical model keeps a light off its owner by geometry instead
    // (kPhysicalNearEnd and the low beam's back plane).
    std::uint32_t owner = 0;
    int shadowSlot = -1; // combat shadow mask channel (B4); -1 = none
    // A combat light (B4, CombatLights.hpp): linear HDR, outside the classic
    // model and its budget. `start` is then the ball radius r0, `diffuse` the
    // linear radiance at the core (intensity times the unit-luminance colour),
    // `range` the window end; `ambient` and `sortBrightness` are unused.
    bool hdr = false;
    float range = 0.0f;

    bool IsSpot() const { return kind == FrameLightKind::Spot && !hdr; }
};

// True when the light adds nothing (no diffuse and no ambient colour).
bool LightIsDark(const FrameLight& light);

// The sphere a light is binned by (camera-relative centre, radius in metres):
// its reach, kRangeFactor start distances around it, for every classic kind,
// and its window (FrameLight::range) for a combat light. A
// spotlight lights only the sector inside its cone edge, whose enclosing sphere
// (centred reach / (2 cos a) ahead of the lamp, radius the same, a the cone
// edge's angle) is about half as large; but the classic budget ranks a
// spotlight wherever it is in reach, cone or not (as Scene::SelectLights does),
// so every tile in reach must list it. With `physical` (the physical model
// lights the frame, B3) a street lamp or headlight is binned by its physical
// reach instead (PhysicalReach), still centred on the bulb: the low beam's
// floor and spill light most of the half-space ahead, so its cone is no
// tighter bound.
struct LightSphere
{
    float c[3];
    float r;
};
LightSphere LightBounds(const FrameLight& light, const PhysicalLightParams* physical = nullptr);

// Ranks the lights as Light::Compare ranks the scene's active lights: by sort
// brightness over the squared distance from the camera, strongest first
// (stable for ties). Drops lights that add nothing and keeps at most
// `capacity`.
void RankFrameLights(std::vector<FrameLight>& lights, int capacity = kMaxFrameLights);

// Combat lights in the frame's buffer: at most this many, the strongest at the
// camera (CombatLights.hpp ShadowScore), and always first, so a tile lists them
// before its classic lights (a full tile drops a far classic light, never a
// fire) and the classic budget sees the same lights as without them.
constexpr int kMaxCombatFrameLights = 12;

// The frame's buffer with combat lights: `combat` (ranked by the caller,
// strongest first, at most kMaxCombatFrameLights kept) followed by the ranked
// classic lights, at most kMaxFrameLights in all. Returns how many combat
// lights lead the list.
int MergeCombatFrameLights(std::vector<FrameLight>& classicRanked, const std::vector<FrameLight>& combat);

// The camera and the screen the tiles cover.
struct TileView
{
    // Camera-relative world to view space, as rows: view x right, y up, z
    // forward (the camera's InverseScaled rotation).
    float rot[3][3] = {{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};
    float p11 = 1.0f, p22 = 1.0f; // projection: ndc.x = p11 * x / z, ndc.y = p22 * y / z
    // The 3D viewport in frame-target pixels, lower-left origin: x, y, width,
    // height (the aspect pillarbox may leave bars around it).
    float viewport[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    // A reach that comes closer than this to the eye plane covers the whole
    // viewport (its projection is unbounded or nearly so).
    float nearZ = 0.05f;
};

// Tiles over the whole frame target from its pixel (0, 0), lower-left origin,
// so a fragment finds its tile from gl_FragCoord alone.
struct TileGrid
{
    int tileSize = kTileSize;
    int tilesX = 0, tilesY = 0;
    // kMaxLightsPerTile bytes per tile, tiles row by row from the bottom. A
    // tile lists light index + 1 in rank order; 0 follows the last.
    std::vector<std::uint8_t> indices;
    int refs = 0;     // entries written
    int dropped = 0;  // entries a full tile could not take
    int litTiles = 0; // tiles with at least one light
};

// Sizes the grid for a frame target and clears it.
void ResizeTileGrid(TileGrid& grid, int targetW, int targetH, int tileSize = kTileSize);

// The light's reach on screen: the pixel rectangle x0, y0, x1, y1 (frame-target
// pixels, lower-left origin, inclusive) that holds every pixel it can light,
// clipped to the viewport. False when the reach is off screen.
bool LightScreenRect(const FrameLight& light, const TileView& view, float rect[4],
                     const PhysicalLightParams* physical = nullptr);

// Bins the ranked lights into the grid (sized beforehand): every tile lists
// the lights whose reach covers it, in rank order, up to kMaxLightsPerTile.
void BinLightsToTiles(const std::vector<FrameLight>& lights, const TileView& view, TileGrid& grid,
                      const PhysicalLightParams* physical = nullptr);

// The lights listed in the tile that holds frame-target pixel (px, py): their
// indices into the ranked list, in order. Returns how many.
int TileLightsAt(const TileGrid& grid, int px, int py, int out[kMaxLightsPerTile]);

// The light block's contents, kTexelsPerLight vec4s per light:
//   0: xyz position (camera-relative), w start distance (a combat light: its core radius)
//   1: rgb diffuse, w 1 for a spotlight (the per-draw litLightColor layout),
//      2 for a combat light (rgb its linear radiance at the core)
//   2: rgb ambient, w sort brightness (the classic budget's ranking); a combat
//      light: x its range, y its shadow mask channel + 1 (0 = none)
//   3: xyz beam direction, w kind
void PackFrameLights(const std::vector<FrameLight>& lights, std::vector<float>& texels);

// How strongly the classic budget ranks `light` at camera-relative point `p`,
// as localLightsTiled computes it: sort brightness over squared distance
// (Light::Compare), faded to 0 from half the squared radius of LightBounds'
// sphere to its edge. Outside that sphere a tile does not list the light, so it
// must not rank there either, or the budget would change at a tile edge.
float ClassicImportance(const FrameLight& light, const float p[3]);

// The classic budget at one fragment, as localLightsTiled applies it: the
// weight (0..1) of each of `n` lights given their importance there (sort
// brightness over squared distance). Up to kClassicLightsPerSurface lights all
// weigh 1. Past it the strongest keep 1 and the rest 0, except that the 7th and
// 8th strongest blend when they are within kClassicBudgetSoftness of the 7th's
// importance; the weights change continuously with the importances.
void ClassicBudgetWeights(const float* importance, int n, float* weight);

// ---- The physical lamps and low beams (B3, the nightLights switch) ----------
//
// A street lamp is a point source of intensity kLampIntensityPerStart times
// its start distance (the start distance is 50 m times the configured
// brightness, so the intensity follows the brightness linearly); a headlight
// is a low beam of hot-spot intensity kLowBeamPeak times the nightSpot knob.
// The irradiance at distance d is I / (d^2 + kPhysicalSoftening): no flat
// disc of full light out to the start distance, so lamps lay pools and the
// country away from them stays dark. A smooth window ends every light at its
// physical reach (PhysicalReach, at most the classic reach), the sphere the
// tiles bin it by. Colours (PhysicalColour): a street lamp takes its
// configured colour to linear (F9b gives lamps their authored warm colour); a
// headlight is white at its configured colour's linear luminance. No warming:
// a warmed headlight turned verges mustard in the research, and the configured
// rosy beam (0.9, 0.8, 0.8) still turned the khaki grass of a verge orange-tan
// in the beam (hue 39 deg, saturation 0.29, at b3_jeep_side). The shaders'
// copy is physicalLight in EngineGL33_LightingShadersLights.cpp; the test pins
// its numbers to these.
// The research tuned its lamps at 4 * 0.3 = 1.2 per start metre on lamps the
// engine then read as white (the colour bug F9b fixed); the authored (0.9,
// 0.8, 0.6) is 0.63 as bright in linear light, so the same pools take 1.9.
constexpr float kLampIntensityPerStart = 1.9f; // street lamp: I = 1.9 * start
// The research's low beams (4000 * 0.3 * nightSpot 1.5 = 1800) blew the hot
// spot out at 1080p; 800 keeps the hot spot under white (about 193 levels on
// the moonless road) and the far verges at a night level. A verge in the beam
// core 10 to 20 m out still reads brighter than the noon lawn (b3_jeep_side).
constexpr float kLowBeamPeak = 800.0f;      // headlight hot spot at nightSpot 1
constexpr float kPhysicalSoftening = 0.25f; // m^2 added to d^2: a bulb is not a point
// Light reaching a surface closer to the bulb than kNearEnd is the lamp's own
// housing, post or vehicle (bumper, grille): it fades out from kNearEnd to
// kNearStart. With the low beam's back plane (nothing behind the lamp) this is
// the owner exclusion: the shaders cannot tell the draw's object in an
// instanced run.
constexpr float kPhysicalNearStart = 0.25f; // metres
constexpr float kPhysicalNearEnd = 1.0f;
// The low beam, in radians about its axis (vertical: up positive). Wide
// horizontally (a core of kLowBeamCoreWidth and a spill of kLowBeamSpillWidth,
// kLowBeamSpill strong), a hot band kLowBeamHotBand under the axis
// (kLowBeamHotWidth wide), and a cut-off that closes between kLowBeamCutLow and
// kLowBeamCutHigh above it (0.3 to 1.4 degrees: a hard top edge, a little
// softer than the research's 0.2 to 0.9, which hill roads put right on the
// road). Under the hot band a faint kLowBeamFloor reaches down to
// kLowBeamFloorEnd (34 deg), so the road just ahead of the bumper is not black.
constexpr float kLowBeamCutLow = -0.005f;
constexpr float kLowBeamCutHigh = 0.025f;
constexpr float kLowBeamHotBand = -0.03f; // 1.7 deg under the axis
constexpr float kLowBeamHotWidth = 0.05f;
constexpr float kLowBeamFloor = 0.02f;
constexpr float kLowBeamFloorEnd = -0.6f;
constexpr float kLowBeamFloorStart = -0.1f;
constexpr float kLowBeamCoreWidth = 0.25f; // +-14 deg
constexpr float kLowBeamSpill = 0.15f;
constexpr float kLowBeamSpillWidth = 0.6f; // +-34 deg
// A headlight is aimed as fixed to the body, this much (tan) under the body's
// own pitch along the lamp's heading: 1%, the usual low-beam setting. On a
// flat road a lamp 0.9 m up lays its hot band about 23 m ahead, and on any
// steady slope the same distance up or down it.
constexpr float kLowBeamTilt = 0.01f;

// True for the kinds the physical model shades (street lamps and headlights).
bool UsesPhysicalModel(FrameLightKind kind);

// The low beam's relative intensity toward unit direction `toPoint` from the
// lamp, for the unit axis `beam`: (1 + kLowBeamFloor)(1 + kLowBeamSpill) on
// the hot band's centre line straight ahead, 0 behind the lamp's plane.
float LowBeamPattern(const float toPoint[3], const float beam[3]);

// The headlight's axis, unit: the horizontal heading of the model's beam axis
// `lampDir` (its vertical part is ignored: a model's beam axis can point
// degrees up or down), pitched as the body is along that heading
// (`bodyForward`, the owner's forward axis) minus kLowBeamTilt.
void LowBeamAim(const float lampDir[3], const float bodyForward[3], float out[3]);

// The physical model's parameters for a frame (NightLights.hpp
// ComputeNightConstants fills the shaders' copy).
struct PhysicalLightParams
{
    float lampPerStart = kLampIntensityPerStart;
    float spotPeak = kLowBeamPeak; // times the nightSpot knob
};

// A physical light reaches as far as its strongest irradiance (a lamp's
// intensity, a low beam's hot spot) stays above kPhysicalMinIrradiance, and no
// farther than the classic reach (kRangeFactor start distances); the window
// that ends it starts at half its square, where the light is twice that. At
// this level a light adds about one display level to the darkest night
// ground, so a lamp 260 m off no longer lists itself in every tile of a
// country road. The shaders compute the same reach.
constexpr float kPhysicalMinIrradiance = 0.002f;
float PhysicalReach(const FrameLight& light, const PhysicalLightParams& params);

// Linear irradiance (per colour channel, before any surface term) that a
// light of the physical model puts on camera-relative point `p`: its intensity
// toward the point over the softened squared distance, the near fade, the
// reach window and the light's PhysicalColour. 0 for a kind the physical model
// does not shade.
void PhysicalIrradiance(const FrameLight& light, const float p[3], const PhysicalLightParams& params, float out[3]);

// A physical light's linear colour: a lamp's configured colour taken to
// linear; a headlight's, white at that colour's linear luminance (Rec. 709).
void PhysicalColour(const FrameLight& light, float out[3]);

// The linear light the frame's physical lamps and headlights put on a particle
// (a smoke or dust puff, drawn as a camera-facing decal lit on the CPU,
// Object::DrawDecal) at camera-relative point `p`: the sum of
// PhysicalIrradiance over `lights`. A puff scatters light every way, so there
// is no surface term. The classic kinds keep lighting particles their own way.
void ParticleLight(const std::vector<FrameLight>& lights, const float p[3], const PhysicalLightParams& params,
                   float out[3]);

} // namespace Poseidon::lighting
