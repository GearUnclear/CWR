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
// Every kind is shaded with the classic local-light model today (full strength
// inside the start distance, a quadratic falloff past it ending in a smooth
// window at kRangeFactor times it, a smoothed spotlight cone, the sum taken in
// display space and clamped at 1, as the per-draw path does). The kind travels
// with each light so the steps that give a kind its own model (B3 lamps and
// low beams, B4 fire and combat light) find it here.

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
    // The object the light is attached to (Object::ID, 0 = none), for the
    // self-exclusion the night lamps need (B3). The shaders do not read it yet.
    std::uint32_t owner = 0;
    int shadowSlot = -1; // combat shadow mask channel (B4); -1 = none

    bool IsSpot() const { return kind == FrameLightKind::Spot; }
};

// True when the light adds nothing (no diffuse and no ambient colour).
bool LightIsDark(const FrameLight& light);

// The sphere a light is binned by (camera-relative centre, radius in metres):
// its reach, kRangeFactor start distances around it, for every kind. A
// spotlight lights only the sector inside its cone edge, whose enclosing sphere
// (centred reach / (2 cos a) ahead of the lamp, radius the same, a the cone
// edge's angle) is about half as large; but the classic budget ranks a
// spotlight wherever it is in reach, cone or not (as Scene::SelectLights does),
// so every tile in reach must list it. The step that lifts the budget for
// headlights (B3) can bin them by the cone.
struct LightSphere
{
    float c[3];
    float r;
};
LightSphere LightBounds(const FrameLight& light);

// Ranks the lights as Light::Compare ranks the scene's active lights: by sort
// brightness over the squared distance from the camera, strongest first
// (stable for ties). Drops lights that add nothing and keeps at most
// `capacity`.
void RankFrameLights(std::vector<FrameLight>& lights, int capacity = kMaxFrameLights);

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
bool LightScreenRect(const FrameLight& light, const TileView& view, float rect[4]);

// Bins the ranked lights into the grid (sized beforehand): every tile lists
// the lights whose reach covers it, in rank order, up to kMaxLightsPerTile.
void BinLightsToTiles(const std::vector<FrameLight>& lights, const TileView& view, TileGrid& grid);

// The lights listed in the tile that holds frame-target pixel (px, py): their
// indices into the ranked list, in order. Returns how many.
int TileLightsAt(const TileGrid& grid, int px, int py, int out[kMaxLightsPerTile]);

// The light block's contents, kTexelsPerLight vec4s per light:
//   0: xyz position (camera-relative), w start distance
//   1: rgb diffuse, w 1 for a spotlight (the per-draw litLightColor layout)
//   2: rgb ambient, w sort brightness (the classic budget's ranking)
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

} // namespace Poseidon::lighting
