#pragma once

#include <cstdint>
#include <utility>
#include <vector>

// Hill culling: objects that a ridge hides from the camera are not drawn.
// Pure, no engine state; Scene::DrawObjectsAndShadowsPass1 builds the table
// from the landscape and asks it about every queued object (the hillCull
// feature switch).
//
// The test is conservative by construction: an object is reported hidden only
// when the terrain as the renderer draws it blocks every sight line from the
// eye to every point of the object's (enlarged) bounding sphere.
//
//   - The terrain is a triangulated height grid (HeightGrid: the vertices of
//     Landscape::ClippedData, split into triangles as Landscape::SurfaceY and
//     the terrain segments split them, drawn at full detail: LandCache asks
//     for segment LOD 0). Along a straight segment that surface is piecewise
//     linear, with its corners where the segment crosses a grid line or a
//     cell diagonal, so its lowest point is the lowest of those crossings and
//     the two ends (HeightGrid::SegmentMin, exact).
//   - From the eye, `rays` azimuth wedges cover the circle. In plan view a
//     sight line keeps its azimuth, so every sight line inside wedge b crosses
//     the chord that joins the wedge's two edge rays at distance d (at a
//     distance between d cos(half wedge) and d). The lowest terrain along that
//     chord blocks every such sight line that passes below it; the table
//     keeps, per wedge and chord, the steepest such blocking slope met so far
//     (a running maximum).
//   - An object is hidden when, in every wedge its sphere spans, a chord in
//     front of the sphere's nearest point blocks a slope steeper than the
//     steepest one from the eye to any point of the sphere.
//
// A notch between two rays cannot hide anything: the chord takes the lowest
// terrain across the whole width of the wedge, not the terrain under one ray. An earlier
// version marched single rays and compared the sphere's top over its
// near-edge distance, which let narrow notches and objects below the eye
// through; both are cases in test_terrain_occlusion.cpp.

namespace Poseidon::TerrainOcclusion
{

struct Params
{
    int rays = 512; // azimuth wedges (0.70 degrees each)
    // Chord distances: nearStep apart up to nearEnd, then midStep up to midEnd,
    // then farStep up to the range.
    float nearStep = 25.0f;
    float nearEnd = 500.0f;
    float midStep = 50.0f;
    float midEnd = 1500.0f;
    float farStep = 100.0f;
    // Objects whose (enlarged) sphere comes closer than this are never hidden.
    float minTestDistance = 75.0f;
    // The sphere tested is the object's bounding sphere times radiusScale plus
    // radiusMargin metres (animated parts, proxies, rounding).
    float radiusScale = 1.25f;
    float radiusMargin = 1.0f;
    // Metres taken off every terrain bound (depth precision at a ridge line).
    float terrainMargin = 0.5f;
};

// The terrain surface as the renderer draws it: a height grid split into
// triangles along the (x + 1, z)-(x, z + 1) diagonal of every cell.
class HeightGrid
{
  public:
    // `size` vertices per side, `cellSize` metres apart; heightAt(x, z) is the
    // height of vertex (x, z) for 0 <= x, z < size. Vertices outside the grid
    // sit at `outside` (the engine's YOutsideMap), as the renderer draws them.
    template <class HeightAt>
    void Build(int size, float cellSize, float outside, HeightAt&& heightAt)
    {
        std::vector<float> v(static_cast<size_t>(size) * size);
        for (int z = 0; z < size; ++z)
            for (int x = 0; x < size; ++x)
                v[static_cast<size_t>(z) * size + x] = heightAt(x, z);
        BuildFromVertices(size, cellSize, outside, std::move(v));
    }
    void BuildFromVertices(int size, float cellSize, float outside, std::vector<float> vertices);
    void Clear();
    bool Valid() const { return _size > 0; }
    int Size() const { return _size; }
    float CellSize() const { return _cellSize; }

    float Vertex(int x, int z) const
    {
        if (x < 0 || z < 0 || x >= _size || z >= _size)
            return _outside;
        return _v[static_cast<size_t>(z) * _size + x];
    }
    // The drawn surface at (x, z) metres.
    float SurfaceY(float x, float z) const { return SurfaceCells(x * _invCellSize, z * _invCellSize); }
    // The lowest point of the drawn surface along the segment from (x0, z0)
    // to (x1, z1), metres.
    float SegmentMin(float x0, float z0, float x1, float z1) const;

  private:
    float SurfaceCells(float xr, float zr) const; // position in cells

    int _size = 0;
    float _cellSize = 0.0f;
    float _invCellSize = 0.0f;
    float _outside = 0.0f;
    std::vector<float> _v;
};

enum class Result : std::uint8_t
{
    NotTested, // too close to the eye, or no table
    Visible,
    Hidden,
};

// The horizon from one eye: per wedge and chord, the steepest slope the
// terrain blocks so far. Wedges are built when an object first asks about
// them, so a rebuild costs only the wedges in view.
class HorizonTable
{
  public:
    void Build(const HeightGrid& terrain, float eyeX, float eyeY, float eyeZ, float range, const Params& params);
    void Invalidate() { _valid = false; }
    bool Valid() const { return _valid; }
    // The table was built for exactly this eye and range.
    bool Matches(float eyeX, float eyeY, float eyeZ, float range) const;

    // Is the sphere (centre, radius: the object's bounding sphere; the
    // margins of Params are added here) hidden by the terrain?
    Result Test(float cx, float cy, float cz, float radius);

    int Chords() const { return static_cast<int>(_dist.size()); }
    float ChordDistance(int k) const { return _dist[static_cast<size_t>(k)]; }
    int WedgesBuilt() const { return _wedgesBuilt; }
    const Params& GetParams() const { return _p; }

  private:
    void BuildWedge(int b);

    const HeightGrid* _terrain = nullptr;
    Params _p;
    bool _valid = false;
    float _ex = 0.0f, _ey = 0.0f, _ez = 0.0f, _range = 0.0f;
    float _wedge = 0.0f;             // wedge angle, radians
    float _cosHalf = 1.0f;           // cos(wedge / 2)
    std::vector<float> _dist;        // chord distances, ascending
    std::vector<float> _sin, _cos;   // edge ray b (0..rays): sin and cos of b * wedge
    std::vector<float> _slope;       // [b * chords + k]: steepest blocked slope over chords 0..k
    std::vector<std::uint32_t> _gen; // wedge b is built when _gen[b] == _buildGen
    std::uint32_t _buildGen = 0;
    int _wedgesBuilt = 0;
};

// The adaptive rule: hill culling pays for itself only where hills hide a
// good share of a crowded view. After a full window of frames that tested
// fewer than kMinTested objects on average or hid less than kMinShare of
// them, the cull idles and runs once every kWindow frames as a probe; a probe
// that finds a view worth culling starts it again.
class CullGovernor
{
  public:
    static constexpr int kWindow = 30;
    static constexpr int kMinTested = 200;
    static constexpr float kMinShare = 0.10f;

    // Called once per frame: run the cull this frame?
    bool ShouldRun();
    // The counts of a frame that ran.
    void Report(int tested, int hidden);
    void Reset();
    bool Idle() const { return _idle; }

  private:
    bool _idle = false;
    int _framesIdle = 0;
    int _reports = 0;
    int _next = 0;
    int _tested[kWindow] = {};
    int _hidden[kWindow] = {};
};

} // namespace Poseidon::TerrainOcclusion
