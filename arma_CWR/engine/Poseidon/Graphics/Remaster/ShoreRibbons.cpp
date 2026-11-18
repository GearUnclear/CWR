#include <Poseidon/Graphics/Remaster/ShoreRibbons.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace Poseidon::remaster
{

namespace
{

struct P2
{
    float x = 0.0f, z = 0.0f;
};

// One piece of contour through one triangle, from the crossing on edge e0 to
// the crossing on edge e1, the land on its left.
struct Seg
{
    std::uint32_t e0 = 0, e1 = 0;
    P2 p, q;
};

struct Sample
{
    float x, z, angle, slope;
    bool last;
};

struct Piece
{
    std::uint32_t tile, first, count;
};

class Tracer
{
  public:
    Tracer(const std::vector<float>& h, int n, float cell) : _h(h), _n(n), _cell(cell)
    {
        _startOf.assign(static_cast<size_t>(n) * n * 3, -1);
    }

    float H(int x, int z) const { return _h[static_cast<size_t>(z) * _n + x]; }

    // The ground's slope at world xz, smooth across the triangles: the
    // central-difference slopes of the cell's four corners, mixed bilinearly
    // (the shore water's xpFloorSlope).
    float Slope(float wx, float wz) const
    {
        const float gx = std::clamp(wx / _cell, 0.0f, static_cast<float>(_n - 1) - 1e-3f);
        const float gz = std::clamp(wz / _cell, 0.0f, static_cast<float>(_n - 1) - 1e-3f);
        const int ix = std::min(static_cast<int>(gx), _n - 2), iz = std::min(static_cast<int>(gz), _n - 2);
        const float fx = gx - static_cast<float>(ix), fz = gz - static_cast<float>(iz);
        float s[4];
        for (int k = 0; k < 4; ++k)
        {
            const int cx = ix + (k & 1), cz = iz + (k >> 1);
            const int xm = std::max(cx - 1, 0), xp = std::min(cx + 1, _n - 1);
            const int zm = std::max(cz - 1, 0), zp = std::min(cz + 1, _n - 1);
            const float dx = H(xp, cz) - H(xm, cz), dz = H(cx, zp) - H(cx, zm);
            s[k] = std::sqrt(dx * dx + dz * dz) * 0.5f / _cell;
        }
        const float a = s[0] + (s[1] - s[0]) * fx, b = s[2] + (s[3] - s[2]) * fx;
        return a + (b - a) * fz;
    }

    // The contours of level L as polylines (world metres), each with its
    // closed flag.
    void Trace(float L, std::vector<std::vector<P2>>& lines, std::vector<bool>& closed)
    {
        _segs.clear();
        for (std::uint32_t e : _touched)
            _startOf[e] = -1;
        _touched.clear();
        for (int z = 0; z + 1 < _n; ++z)
        {
            for (int x = 0; x + 1 < _n; ++x)
            {
                const float h00 = H(x, z), h10 = H(x + 1, z), h01 = H(x, z + 1), h11 = H(x + 1, z + 1);
                const float lo = std::min(std::min(h00, h10), std::min(h01, h11));
                const float hi = std::max(std::max(h00, h10), std::max(h01, h11));
                if (lo > L || hi <= L)
                    continue;
                // The lower triangle (x, z), (x + 1, z), (x, z + 1) and the
                // upper one (x + 1, z + 1), (x, z + 1), (x + 1, z), with
                // their edges ab, bc, ca.
                Triangle(L, x, z, x + 1, z, x, z + 1, Edge(x, z, 0), Edge(x, z, 2), Edge(x, z, 1));
                Triangle(L, x + 1, z + 1, x, z + 1, x + 1, z, Edge(x, z + 1, 0), Edge(x, z, 2), Edge(x + 1, z, 1));
            }
        }
        // Chains: every segment's successor starts on the edge it ends on.
        const int ns = static_cast<int>(_segs.size());
        std::vector<int> next(ns, -1);
        std::vector<char> hasPrev(ns, 0), seen(ns, 0);
        for (int s = 0; s < ns; ++s)
        {
            const int t = _startOf[_segs[s].e1];
            if (t >= 0 && t != s)
            {
                next[s] = t;
                hasPrev[t] = 1;
            }
        }
        auto walk = [&](int s0)
        {
            std::vector<P2> line;
            line.push_back(_segs[s0].p);
            int s = s0;
            bool loop = false;
            while (s >= 0 && !seen[s])
            {
                seen[s] = 1;
                line.push_back(_segs[s].q);
                s = next[s];
                if (s == s0)
                    loop = true;
            }
            if (loop && line.size() > 1)
                line.pop_back(); // the last point is the first again
            lines.push_back(std::move(line));
            closed.push_back(loop);
        };
        for (int s = 0; s < ns; ++s)
            if (!hasPrev[s] && !seen[s])
                walk(s);
        for (int s = 0; s < ns; ++s)
            if (!seen[s])
                walk(s);
    }

  private:
    std::uint32_t Edge(int x, int z, int type) const
    {
        return static_cast<std::uint32_t>((static_cast<size_t>(z) * _n + x) * 3 + type);
    }

    // The level's crossing on an edge, from its canonical end points (so both
    // triangles of the edge get the same point).
    P2 Crossing(std::uint32_t e, float L) const
    {
        const int type = static_cast<int>(e % 3);
        const int cellIdx = static_cast<int>(e / 3);
        const int x = cellIdx % _n, z = cellIdx / _n;
        int ax = x, az = z, bx = x + 1, bz = z;
        if (type == 1)
        {
            bx = x;
            bz = z + 1;
        }
        else if (type == 2)
        {
            ax = x + 1;
            az = z;
            bx = x;
            bz = z + 1;
        }
        const float ha = H(ax, az), hb = H(bx, bz);
        const float t = std::clamp((L - ha) / (hb - ha), 0.0f, 1.0f);
        return P2{(static_cast<float>(ax) + t * static_cast<float>(bx - ax)) * _cell,
                  (static_cast<float>(az) + t * static_cast<float>(bz - az)) * _cell};
    }

    // A triangle with its corners counter-clockwise (x east, z north) and its
    // edges ab, bc, ca: the contour leaves it through the edge that runs from
    // land to sea and enters it through the edge that runs from sea to land,
    // which keeps the land on its left (no geometry, so a crossing on a
    // corner or a piece of zero length is oriented as its neighbours are).
    void Triangle(float L, int ax, int az, int bx, int bz, int cx, int cz, std::uint32_t eab, std::uint32_t ebc,
                  std::uint32_t eca)
    {
        const bool la = H(ax, az) > L, lb = H(bx, bz) > L, lc = H(cx, cz) > L;
        const int land = static_cast<int>(la) + static_cast<int>(lb) + static_cast<int>(lc);
        if (land == 0 || land == 3)
            return;
        const bool from[3] = {la, lb, lc}, to[3] = {lb, lc, la};
        const std::uint32_t edge[3] = {eab, ebc, eca};
        Seg s;
        for (int i = 0; i < 3; ++i)
        {
            if (from[i] && !to[i])
                s.e0 = edge[i];
            else if (!from[i] && to[i])
                s.e1 = edge[i];
        }
        s.p = Crossing(s.e0, L);
        s.q = Crossing(s.e1, L);
        if (_startOf[s.e0] >= 0)
            return; // cannot happen on a triangulated field; kept as a guard
        _startOf[s.e0] = static_cast<int>(_segs.size());
        _touched.push_back(s.e0);
        _segs.push_back(s);
    }

    const std::vector<float>& _h;
    int _n;
    float _cell;
    std::vector<int> _startOf;
    std::vector<std::uint32_t> _touched;
    std::vector<Seg> _segs;
};

// Resamples a contour every `spacing` metres: the samples, with the closing
// sample (the first again) at the end of a closed contour.
std::vector<P2> Resample(const std::vector<P2>& line, bool closedLine, float spacing, double& length)
{
    std::vector<P2> pts = line;
    if (closedLine)
        pts.push_back(line.front());
    std::vector<double> d(pts.size(), 0.0);
    for (size_t i = 1; i < pts.size(); ++i)
        d[i] = d[i - 1] +
               std::hypot(static_cast<double>(pts[i].x - pts[i - 1].x), static_cast<double>(pts[i].z - pts[i - 1].z));
    length = d.back();
    std::vector<P2> out;
    if (length <= 0.0)
        return out;
    const int m = std::max(closedLine ? 3 : 1, static_cast<int>(std::lround(length / spacing)));
    out.reserve(static_cast<size_t>(m) + 1);
    size_t j = 0;
    for (int k = 0; k <= m; ++k)
    {
        const double s = length * static_cast<double>(k) / static_cast<double>(m);
        while (j + 2 < pts.size() && d[j + 1] < s)
            ++j;
        const double span = d[j + 1] - d[j];
        const float t = span > 0.0 ? static_cast<float>(std::clamp((s - d[j]) / span, 0.0, 1.0)) : 0.0f;
        out.push_back(P2{pts[j].x + t * (pts[j + 1].x - pts[j].x), pts[j].z + t * (pts[j + 1].z - pts[j].z)});
    }
    if (closedLine)
        out.back() = out.front();
    return out;
}

} // namespace

int ShoreRibbonSet::LevelFor(float seaLevel) const
{
    if (levels <= 0 || levelStep <= 0.0f)
        return 0;
    const int k = static_cast<int>(std::lround((seaLevel - levelBase) / levelStep));
    return std::clamp(k, 0, levels - 1);
}

ShoreRibbonSet BuildShoreRibbons(const std::vector<float>& height, int n, float cellSize, float levelBase,
                                 float levelStep, int levels)
{
    ShoreRibbonSet set;
    if (n < 2 || !(cellSize > 0.0f) || height.size() < static_cast<size_t>(n) * n || levels <= 0)
        return set;
    const float extent = static_cast<float>(n - 1) * cellSize;
    set.tile = std::max(kShoreRibbonTile, extent / static_cast<float>(kShoreRibbonMaxTiles));
    set.tilesPerSide = std::max(1, static_cast<int>(std::ceil(extent / set.tile)));
    set.levels = levels;
    set.levelBase = levelBase;
    set.levelStep = levelStep;
    const int T = set.tilesPerSide;
    const size_t tiles = static_cast<size_t>(T) * T;
    set.tileStart.reserve((tiles + 1) * static_cast<size_t>(levels));
    auto tileOf = [&](float x, float z)
    {
        const int tx = std::clamp(static_cast<int>(std::floor(x / set.tile)), 0, T - 1);
        const int tz = std::clamp(static_cast<int>(std::floor(z / set.tile)), 0, T - 1);
        return static_cast<std::uint32_t>(tz * T + tx);
    };

    Tracer tracer(height, n, cellSize);
    std::vector<std::vector<P2>> lines;
    std::vector<bool> closed;
    std::vector<Sample> samples;
    std::vector<Piece> pieces;
    std::vector<std::uint32_t> counts(tiles);
    for (int level = 0; level < levels; ++level)
    {
        const float L = levelBase + levelStep * static_cast<float>(level);
        lines.clear();
        closed.clear();
        tracer.Trace(L, lines, closed);
        samples.clear();
        pieces.clear();
        for (size_t c = 0; c < lines.size(); ++c)
        {
            double len = 0.0;
            const std::vector<P2> pts = Resample(lines[c], closed[c], kShoreRibbonSpacing, len);
            if (len < kShoreRibbonMinLength || pts.size() < 2)
                continue;
            ++set.contours;
            set.length += len;
            const int m = static_cast<int>(pts.size()); // the closing sample included
            const bool loop = closed[c];
            // Unit tangents, then smoothed over +-3 samples; the sea normal is
            // the tangent turned right (the land is on the left).
            std::vector<P2> tan(m);
            for (int i = 0; i < m; ++i)
            {
                int a = i - 1, b = i + 1;
                if (loop)
                {
                    a = (a + (m - 1)) % (m - 1);
                    b = b % (m - 1);
                }
                a = std::clamp(a, 0, m - 1);
                b = std::clamp(b, 0, m - 1);
                const float dx = pts[b].x - pts[a].x, dz = pts[b].z - pts[a].z;
                const float l = std::sqrt(dx * dx + dz * dz);
                tan[i] = l > 1e-6f ? P2{dx / l, dz / l} : P2{1.0f, 0.0f};
            }
            std::vector<float> slope(m);
            for (int i = 0; i < m; ++i)
                slope[i] = tracer.Slope(pts[i].x, pts[i].z);
            const size_t first = samples.size();
            for (int i = 0; i < m; ++i)
            {
                P2 t{0.0f, 0.0f};
                float s = 0.0f;
                int ns = 0;
                for (int j = -3; j <= 3; ++j)
                {
                    int k = i + j;
                    if (loop)
                        k = ((k % (m - 1)) + (m - 1)) % (m - 1);
                    else
                        k = std::clamp(k, 0, m - 1);
                    const float w = 4.0f - static_cast<float>(std::abs(j));
                    t.x += tan[k].x * w;
                    t.z += tan[k].z * w;
                    if (std::abs(j) <= 2)
                    {
                        s += slope[k];
                        ++ns;
                    }
                }
                const float l = std::sqrt(t.x * t.x + t.z * t.z);
                if (l > 1e-6f)
                {
                    t.x /= l;
                    t.z /= l;
                }
                else
                    t = tan[i];
                const float angle = std::atan2(-t.x, t.z); // the normal (t.z, -t.x)
                const float sl = std::clamp(s / static_cast<float>(ns), kShoreRibbonMinSlope, 2.0f);
                samples.push_back(Sample{pts[i].x, pts[i].z, angle, sl, false});
            }
            // Pieces: the longest runs of segments whose first point is in one
            // tile, each closed by the next segment's first point (flagged last).
            const int segs = m - 1;
            int a = 0;
            while (a < segs)
            {
                const std::uint32_t t = tileOf(samples[first + a].x, samples[first + a].z);
                int b = a;
                while (b + 1 < segs && tileOf(samples[first + b + 1].x, samples[first + b + 1].z) == t)
                    ++b;
                pieces.push_back(
                    Piece{t, static_cast<std::uint32_t>(first + a), static_cast<std::uint32_t>(b - a + 2)});
                a = b + 1;
            }
        }
        // The level's points by tile.
        std::stable_sort(pieces.begin(), pieces.end(), [](const Piece& x, const Piece& y) { return x.tile < y.tile; });
        std::fill(counts.begin(), counts.end(), 0u);
        const std::uint32_t levelFirst = static_cast<std::uint32_t>(set.texels.size() / 4);
        for (const Piece& p : pieces)
        {
            counts[p.tile] += p.count;
            for (std::uint32_t i = 0; i < p.count; ++i)
            {
                const Sample& s = samples[p.first + i];
                const bool last = i + 1 == p.count;
                set.texels.push_back(s.x);
                set.texels.push_back(s.z);
                set.texels.push_back(s.angle);
                set.texels.push_back(last ? -s.slope : s.slope);
            }
        }
        std::uint32_t at = levelFirst;
        for (size_t t = 0; t < tiles; ++t)
        {
            set.tileStart.push_back(at);
            at += counts[t];
        }
        set.tileStart.push_back(at);
    }
    return set;
}

} // namespace Poseidon::remaster
