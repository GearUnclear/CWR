#include <Poseidon/World/Scene/TerrainOcclusion.hpp>

#include <algorithm>
#include <cmath>

namespace Poseidon::TerrainOcclusion
{

namespace
{
constexpr float kTwoPi = 6.2831853f;
// Rounding slack on a sphere's angular span (radians), so a sphere that ends
// exactly on a wedge edge also asks the wedge beyond it.
constexpr float kSpanSlack = 1e-4f;
} // namespace

// ---- HeightGrid ----

void HeightGrid::Clear()
{
    _size = 0;
    _v.clear();
}

void HeightGrid::BuildFromVertices(int size, float cellSize, float outside, std::vector<float> vertices)
{
    Clear();
    if (size < 2 || !(cellSize > 0.0f) || vertices.size() != static_cast<size_t>(size) * size)
        return;
    _size = size;
    _cellSize = cellSize;
    _invCellSize = 1.0f / cellSize;
    _outside = outside;
    _v = std::move(vertices);
}

float HeightGrid::SurfaceCells(float xr, float zr) const
{
    // Landscape::SurfaceY's triangles, over the vertices as the renderer
    // draws them (an off-grid vertex at the outside height).
    const int xi = static_cast<int>(std::floor(xr));
    const int zi = static_cast<int>(std::floor(zr));
    const float xIn = xr - static_cast<float>(xi);
    const float zIn = zr - static_cast<float>(zi);
    const float y00 = Vertex(xi, zi);
    const float y01 = Vertex(xi + 1, zi);
    const float y10 = Vertex(xi, zi + 1);
    const float y11 = Vertex(xi + 1, zi + 1);
    if (xIn <= 1.0f - zIn)
        return y00 + (y10 - y00) * zIn + (y01 - y00) * xIn;
    return y10 + (y01 - y11) - (y10 - y11) * xIn - (y01 - y11) * zIn;
}

float HeightGrid::SegmentMin(float x0, float z0, float x1, float z1) const
{
    if (_size <= 0)
        return -1e30f;
    const float ax = x0 * _invCellSize, az = z0 * _invCellSize;
    const float bx = x1 * _invCellSize, bz = z1 * _invCellSize;
    float m = std::min(SurfaceCells(ax, az), SurfaceCells(bx, bz));
    // The surface is linear inside each triangle, so along the segment its
    // corners are where the segment crosses a line x = i, z = j or a cell
    // diagonal x + z = k (cell units): the lowest point is one of those or an
    // end. A segment many cells long (never a chord of the table) falls back
    // to the lowest vertex of the cells it may touch.
    constexpr int kMaxCrossings = 256;
    if (std::fabs(bx - ax) + std::fabs(bz - az) > kMaxCrossings / 2)
    {
        const int gx0 = static_cast<int>(std::floor(std::min(ax, bx))),
                  gx1 = static_cast<int>(std::floor(std::max(ax, bx))) + 1;
        const int gz0 = static_cast<int>(std::floor(std::min(az, bz))),
                  gz1 = static_cast<int>(std::floor(std::max(az, bz))) + 1;
        for (int z = gz0; z <= gz1; ++z)
            for (int x = gx0; x <= gx1; ++x)
                m = std::min(m, Vertex(x, z));
        return m;
    }
    auto crossings = [&](float a, float b)
    {
        if (a == b)
            return;
        const float lo = std::min(a, b), hi = std::max(a, b);
        const float inv = 1.0f / (b - a);
        for (float k = std::floor(lo) + 1.0f; k <= hi; k += 1.0f)
        {
            const float t = (k - a) * inv;
            m = std::min(m, SurfaceCells(ax + (bx - ax) * t, az + (bz - az) * t));
        }
    };
    crossings(ax, bx);
    crossings(az, bz);
    crossings(ax + az, bx + bz);
    return m;
}

// ---- HorizonTable ----

void HorizonTable::Build(const HeightGrid& terrain, float eyeX, float eyeY, float eyeZ, float range,
                         const Params& params)
{
    _valid = false;
    _terrain = &terrain;
    _p = params;
    if (!terrain.Valid() || _p.rays < 8 || !(range > 0.0f) || !(_p.nearStep > 0.0f) || !(_p.midStep > 0.0f) ||
        !(_p.farStep > 0.0f))
        return;
    _ex = eyeX;
    _ey = eyeY;
    _ez = eyeZ;
    _range = range;
    _wedge = kTwoPi / static_cast<float>(_p.rays);
    _cosHalf = std::cos(0.5f * _wedge);

    _dist.clear();
    for (float d = _p.nearStep; d <= range;)
    {
        _dist.push_back(d);
        d += d < _p.nearEnd ? _p.nearStep : (d < _p.midEnd ? _p.midStep : _p.farStep);
    }
    if (_sin.size() != static_cast<size_t>(_p.rays) + 1)
    {
        _sin.resize(static_cast<size_t>(_p.rays) + 1);
        _cos.resize(static_cast<size_t>(_p.rays) + 1);
        for (int b = 0; b <= _p.rays; ++b)
        {
            const float a = static_cast<float>(b) * _wedge;
            _sin[static_cast<size_t>(b)] = std::sin(a);
            _cos[static_cast<size_t>(b)] = std::cos(a);
        }
    }
    _slope.resize(static_cast<size_t>(_p.rays) * _dist.size());
    if (_gen.size() != static_cast<size_t>(_p.rays))
    {
        _gen.assign(static_cast<size_t>(_p.rays), 0u);
        _buildGen = 0;
    }
    if (++_buildGen == 0)
    {
        // The counter wrapped: forget every wedge.
        std::fill(_gen.begin(), _gen.end(), 0u);
        _buildGen = 1;
    }
    _wedgesBuilt = 0;
    _valid = !_dist.empty();
}

bool HorizonTable::Matches(float eyeX, float eyeY, float eyeZ, float range) const
{
    return _valid && eyeX == _ex && eyeY == _ey && eyeZ == _ez && range == _range;
}

void HorizonTable::BuildWedge(int b)
{
    const size_t k = _dist.size();
    float* row = &_slope[static_cast<size_t>(b) * k];
    const float s0 = _sin[static_cast<size_t>(b)], c0 = _cos[static_cast<size_t>(b)];
    const float s1 = _sin[static_cast<size_t>(b) + 1], c1 = _cos[static_cast<size_t>(b) + 1];
    float steepest = -1e30f;
    for (size_t j = 0; j < k; ++j)
    {
        const float d = _dist[j];
        // The chord between the wedge's edge rays at distance d.
        const float ax = _ex + d * s0, az = _ez + d * c0;
        const float bx = _ex + d * s1, bz = _ez + d * c1;
        const float low = _terrain->SegmentMin(ax, az, bx, bz) - _p.terrainMargin;
        // Sight lines cross the chord between d cos(half wedge) and d: the
        // slope the chord blocks for all of them.
        const float rise = low - _ey;
        const float slope = rise >= 0.0f ? rise / d : rise / (d * _cosHalf);
        steepest = std::max(steepest, slope);
        row[j] = steepest;
    }
    _gen[static_cast<size_t>(b)] = _buildGen;
    ++_wedgesBuilt;
}

Result HorizonTable::Test(float cx, float cy, float cz, float radius)
{
    if (!_valid)
        return Result::NotTested;
    const float r = radius * _p.radiusScale + _p.radiusMargin;
    const float dx = cx - _ex, dz = cz - _ez;
    const float dc = std::sqrt(dx * dx + dz * dz);
    const float nearD = dc - r;
    if (!(nearD >= _p.minTestDistance))
        return Result::NotTested;
    // The last chord wholly in front of the sphere's nearest point.
    const auto it = std::upper_bound(_dist.begin(), _dist.end(), nearD);
    if (it == _dist.begin())
        return Result::Visible;
    const size_t k = static_cast<size_t>(it - _dist.begin()) - 1;
    // The steepest slope from the eye to any point of the sphere: its top over
    // the nearest distance when the top is above the eye, over the farthest
    // when below.
    const float top = cy + r - _ey;
    const float objSlope = top >= 0.0f ? top / nearD : top / (dc + r);
    float azimuth = std::atan2(dx, dz);
    if (azimuth < 0.0f)
        azimuth += kTwoPi;
    const float half = std::asin(std::min(1.0f, r / dc)) + kSpanSlack;
    const float inv = 1.0f / _wedge;
    const int b0 = static_cast<int>(std::floor((azimuth - half) * inv));
    const int b1 = static_cast<int>(std::floor((azimuth + half) * inv));
    if (b1 - b0 + 1 >= _p.rays)
        return Result::Visible;
    const size_t chords = _dist.size();
    for (int b = b0; b <= b1; ++b)
    {
        const int w = ((b % _p.rays) + _p.rays) % _p.rays;
        if (_gen[static_cast<size_t>(w)] != _buildGen)
            BuildWedge(w);
        if (!(_slope[static_cast<size_t>(w) * chords + k] > objSlope))
            return Result::Visible;
    }
    return Result::Hidden;
}

// ---- CullGovernor ----

void CullGovernor::Reset()
{
    *this = CullGovernor();
}

bool CullGovernor::ShouldRun()
{
    if (!_idle)
        return true;
    if (++_framesIdle >= kWindow)
    {
        _framesIdle = 0;
        return true; // a probe
    }
    return false;
}

void CullGovernor::Report(int tested, int hidden)
{
    if (_idle)
    {
        // A probe: a view worth culling starts the cull again.
        if (tested >= kMinTested && static_cast<float>(hidden) >= kMinShare * static_cast<float>(tested))
        {
            _idle = false;
            _reports = 0;
            _next = 0;
        }
        return;
    }
    _tested[_next] = tested;
    _hidden[_next] = hidden;
    _next = (_next + 1) % kWindow;
    if (_reports < kWindow)
        ++_reports;
    if (_reports < kWindow)
        return;
    long long sumTested = 0, sumHidden = 0;
    for (int i = 0; i < kWindow; ++i)
    {
        sumTested += _tested[i];
        sumHidden += _hidden[i];
    }
    const bool crowded = sumTested >= static_cast<long long>(kMinTested) * kWindow;
    const bool pays = static_cast<float>(sumHidden) >= kMinShare * static_cast<float>(sumTested);
    if (!crowded || !pays)
    {
        _idle = true;
        _framesIdle = 0;
    }
}

} // namespace Poseidon::TerrainOcclusion
