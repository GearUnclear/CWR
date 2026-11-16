#pragma once

// Shore ribbons (perf squad SURF-E, under the Shore Water option): breaking
// waves drawn as thin ribbons along the coastline.
//
// The coastline is found once per island, on the CPU, from the terrain height
// map the shore water already reads (TerrainHorizonMap::height, the
// landscape's own triangles: each grid square split along its (x+1, z) to
// (x, z+1) diagonal). The tide moves the waterline (Landscape::Simulate: 0 to
// kShoreRibbonLevelTop m), so the waterline contour is traced at a few sea
// levels kShoreRibbonLevelStep apart; the backend draws the level nearest the
// tide and its vertex shader slides every point along its sea normal onto the
// drawn sea level (the tide plus the sea level's own wave).
//
// Each level is a list of points: resampled every kShoreRibbonSpacing metres
// along the contour, each with its sea normal (land on the left of the walk,
// sea on the right) and the beach slope there. The points are cut into
// pieces at the edges of square tiles and sorted by tile, so the points of a
// row of tiles are one contiguous range: the frame draws the few rows of
// tiles around the camera with one glMultiDrawArrays and no CPU rebuild.
// A piece ends with a point flagged last, which starts no segment.

#include <cstdint>
#include <vector>

namespace Poseidon::remaster
{

constexpr float kShoreRibbonLevelBase = 0.0f;  // m: the lowest traced sea level (the lowest tide)
constexpr float kShoreRibbonLevelStep = 0.5f;  // m between traced levels
constexpr int kShoreRibbonLevels = 12;         // 0 to 5.5 m: the tide (0 to 5 m) and a little over
constexpr float kShoreRibbonSpacing = 3.0f;    // m between the points of a contour
constexpr float kShoreRibbonTile = 128.0f;     // m: the side of a culling tile (at least)
constexpr int kShoreRibbonMaxTiles = 160;      // tiles per side at most (larger maps take larger tiles)
constexpr float kShoreRibbonMinLength = 24.0f; // m: shorter contours (a rock, a puddle) are left out
constexpr float kShoreRibbonMinSlope = 0.005f; // the slope stored is at least this (and at most 2)

struct ShoreRibbonSet
{
    // 4 floats per point: x, z (m), the sea normal's angle atan2(nz, nx), and
    // the beach slope (m per m), negated on the last point of a piece.
    std::vector<float> texels;
    // Per level, tilesPerSide^2 + 1 entries: the first point of each tile
    // (row-major, z rows), the last entry the level's end. Absolute point
    // indices into texels.
    std::vector<std::uint32_t> tileStart;
    int levels = 0;
    float levelBase = 0.0f;
    float levelStep = 0.0f;
    int tilesPerSide = 0;
    float tile = 0.0f;
    // Statistics (the log line).
    int contours = 0;
    double length = 0.0; // m, over every level

    int Points() const { return static_cast<int>(texels.size() / 4); }
    bool Empty() const { return texels.empty(); }
    // The level nearest a sea level (clamped to the traced ones).
    int LevelFor(float seaLevel) const;
    // [first, end) points of tile (tx, tz) of a level.
    std::uint32_t TileFirst(int level, int tx, int tz) const
    {
        return tileStart[static_cast<size_t>(level) * (static_cast<size_t>(tilesPerSide) * tilesPerSide + 1) +
                         static_cast<size_t>(tz) * tilesPerSide + tx];
    }
};

// Traces the waterline of every level over a height map of n x n heights
// (row-major, z rows) cellSize metres apart.
ShoreRibbonSet BuildShoreRibbons(const std::vector<float>& height, int n, float cellSize,
                                 float levelBase = kShoreRibbonLevelBase, float levelStep = kShoreRibbonLevelStep,
                                 int levels = kShoreRibbonLevels);

} // namespace Poseidon::remaster
