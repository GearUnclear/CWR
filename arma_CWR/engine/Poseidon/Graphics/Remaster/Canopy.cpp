#include <Poseidon/Graphics/Remaster/Canopy.hpp>

#include <Poseidon/World/MapTypes.hpp>

#include <algorithm>
#include <cmath>
#include <utility>

namespace Poseidon::remaster
{

namespace
{

constexpr float kDegToRad = 0.017453292519943295f;

float Smoothstep(float e0, float e1, float x)
{
    const float t = std::clamp((x - e0) / (e1 - e0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

float Mix(float a, float b, float t)
{
    return a + (b - a) * t;
}

std::uint16_t Unorm16(float v)
{
    return static_cast<std::uint16_t>(std::lround(std::clamp(v, 0.0f, 1.0f) * 65535.0f));
}

// The covered threshold of a mip child, in the stored density.
constexpr int kCoveredU16 = static_cast<int>(kCanopyMipCovered * 65535.0f);

} // namespace

// ---- The canopy -------------------------------------------------------------------

CanopyKind CanopyKindOfMapType(int mapType)
{
    switch (mapType)
    {
        case MapTree:
            return CanopyKind::Tree;
        case MapSmallTree:
            return CanopyKind::SmallTree;
        case MapBush:
            return CanopyKind::Bush;
        case MapForestBorder:
        case MapForestTriangle:
        case MapForestSquare:
            return CanopyKind::Forest;
        case MapBuilding:
        case MapHouse:
        case MapChurch:
        case MapChapel:
        case MapBunker:
        case MapFortress:
        case MapViewTower:
        case MapLighthouse:
        case MapFuelstation:
        case MapHospital:
        case MapRock:
            return CanopyKind::Solid;
        default:
            return CanopyKind::None;
    }
}

float CanopyOpacity(CanopyKind kind)
{
    switch (kind)
    {
        case CanopyKind::Tree:
            return 0.8f;
        case CanopyKind::SmallTree:
            return 0.65f;
        case CanopyKind::Bush:
            return 0.5f;
        case CanopyKind::Forest:
            return 0.95f;
        case CanopyKind::Solid:
            return 1.0f;
        case CanopyKind::None:
            break;
    }
    return 0.0f;
}

bool CanopyRound(CanopyKind kind)
{
    return kind == CanopyKind::Tree || kind == CanopyKind::SmallTree || kind == CanopyKind::Bush;
}

void CanopyItemBounds(CanopyItem& item)
{
    if (!item.hull.empty())
    {
        item.minX = item.maxX = item.hull[0];
        item.minZ = item.maxZ = item.hull[1];
        for (size_t i = 0; i + 1 < item.hull.size(); i += 2)
        {
            item.minX = std::min(item.minX, item.hull[i]);
            item.maxX = std::max(item.maxX, item.hull[i]);
            item.minZ = std::min(item.minZ, item.hull[i + 1]);
            item.maxZ = std::max(item.maxZ, item.hull[i + 1]);
        }
        return;
    }
    const float ex = std::fabs(item.ux) * item.a + std::fabs(item.vx) * item.b;
    const float ez = std::fabs(item.uz) * item.a + std::fabs(item.vz) * item.b;
    item.minX = item.cx - ex;
    item.maxX = item.cx + ex;
    item.minZ = item.cz - ez;
    item.maxZ = item.cz + ez;
}

bool CanopyInside(const CanopyItem& item, float x, float z)
{
    if (!item.hull.empty())
    {
        const size_t n = item.hull.size() / 2;
        for (size_t i = 0; i < n; ++i)
        {
            const float x0 = item.hull[i * 2], z0 = item.hull[i * 2 + 1];
            const float x1 = item.hull[((i + 1) % n) * 2], z1 = item.hull[((i + 1) % n) * 2 + 1];
            if ((x1 - x0) * (z - z0) - (z1 - z0) * (x - x0) < 0.0f)
                return false;
        }
        return true;
    }
    if (!(item.a > 0.0f) || !(item.b > 0.0f))
        return false;
    const float dx = x - item.cx, dz = z - item.cz;
    const float s = (dx * item.ux + dz * item.uz) / item.a;
    const float t = (dx * item.vx + dz * item.vz) / item.b;
    return item.round ? s * s + t * t <= 1.0f : (std::fabs(s) <= 1.0f && std::fabs(t) <= 1.0f);
}

std::vector<float> CanopyConvexHull(std::vector<float> xz)
{
    const size_t n = xz.size() / 2;
    if (n < 3)
        return {};
    std::vector<std::pair<float, float>> p(n);
    for (size_t i = 0; i < n; ++i)
        p[i] = {xz[i * 2], xz[i * 2 + 1]};
    std::sort(p.begin(), p.end());
    p.erase(std::unique(p.begin(), p.end()), p.end());
    if (p.size() < 3)
        return {};
    auto cross =
        [](const std::pair<float, float>& o, const std::pair<float, float>& a, const std::pair<float, float>& b)
    { return (a.first - o.first) * (b.second - o.second) - (a.second - o.second) * (b.first - o.first); };
    std::vector<std::pair<float, float>> h(2 * p.size());
    size_t k = 0;
    for (size_t i = 0; i < p.size(); ++i)
    {
        while (k >= 2 && cross(h[k - 2], h[k - 1], p[i]) <= 0.0f)
            --k;
        h[k++] = p[i];
    }
    for (size_t i = p.size() - 1, t = k + 1; i > 0; --i)
    {
        while (k >= t && cross(h[k - 2], h[k - 1], p[i - 1]) <= 0.0f)
            --k;
        h[k++] = p[i - 1];
    }
    h.resize(k - 1);
    if (h.size() < 3)
        return {};
    std::vector<float> out;
    out.reserve(h.size() * 2);
    for (const auto& q : h)
    {
        out.push_back(q.first);
        out.push_back(q.second);
    }
    return out;
}

int CanopyField::LevelSize(int level) const
{
    int s = size;
    for (int l = 0; l < level; ++l)
        s = std::max(1, s / 2);
    return s;
}

float CanopyField::Top(int level, int x, int z) const
{
    const int s = LevelSize(level);
    const std::uint16_t r = levels[static_cast<size_t>(level)][(static_cast<size_t>(z) * s + x) * 2];
    return heightBase + static_cast<float>(r) / 65535.0f * heightRange;
}

float CanopyField::Density(int level, int x, int z) const
{
    const int s = LevelSize(level);
    return static_cast<float>(levels[static_cast<size_t>(level)][(static_cast<size_t>(z) * s + x) * 2 + 1]) / 65535.0f;
}

int CanopyTexelsPerCell(float cellSize, int range)
{
    if (!(cellSize > 0.0f) || range < 1)
        return 1;
    int k = std::max(1, static_cast<int>(std::lround(cellSize / kCanopyTargetTexel)));
    while (k > 1 && range * k > kCanopyMaxSize)
        --k;
    return k;
}

float CanopyRaster::Ground(float x, float z) const
{
    const float fx = std::clamp(x / _grid, 0.0f, static_cast<float>(_range - 1));
    const float fz = std::clamp(z / _grid, 0.0f, static_cast<float>(_range - 1));
    const int x0 = std::min(static_cast<int>(fx), std::max(0, _range - 2));
    const int z0 = std::min(static_cast<int>(fz), std::max(0, _range - 2));
    const int x1 = std::min(x0 + 1, _range - 1), z1 = std::min(z0 + 1, _range - 1);
    const float tx = fx - static_cast<float>(x0), tz = fz - static_cast<float>(z0);
    auto at = [&](int xx, int zz) { return _heights[static_cast<size_t>(zz) * _range + xx]; };
    return (at(x0, z0) * (1.0f - tx) + at(x1, z0) * tx) * (1.0f - tz) +
           (at(x0, z1) * (1.0f - tx) + at(x1, z1) * tx) * tz;
}

std::uint16_t CanopyRaster::Encode(float height) const
{
    return Unorm16((height - _field->heightBase) / _field->heightRange);
}

float CanopyRaster::Decode(std::uint16_t value) const
{
    return _field->heightBase + static_cast<float>(value) / 65535.0f * _field->heightRange;
}

bool CanopyRaster::Build(std::vector<CanopyItem> items, std::vector<float> heights, int range, float grid, int k,
                         const std::atomic<bool>* cancel)
{
    _field.reset();
    if (range < 2 || !(grid > 0.0f) || heights.size() < static_cast<size_t>(range) * range)
        return false;
    k = std::max(1, k);
    while (k > 1 && range * k > kCanopyMaxSize)
        --k;
    _items = std::move(items);
    _heights = std::move(heights);
    _range = range;
    _grid = grid;
    _removed.assign(_items.size(), false);
    _byId.clear();
    for (size_t i = 0; i < _items.size(); ++i)
    {
        CanopyItemBounds(_items[i]);
        if (_items[i].id >= 0)
            _byId.emplace(_items[i].id, static_cast<int>(i));
    }

    auto field = std::make_shared<CanopyField>();
    field->size = range * k;
    field->texel = grid / static_cast<float>(k);
    const auto [lo, hi] = std::minmax_element(_heights.begin(), _heights.begin() + static_cast<long>(range) * range);
    field->heightBase = *lo - 1.0f;
    field->heightRange = (*hi + kCanopyMaxRelTop + 1.0f) - field->heightBase;
    int levels = 1;
    for (int s = field->size; s > 1; s = std::max(1, s / 2))
        ++levels;
    field->levels.resize(static_cast<size_t>(levels));
    _ground.assign(static_cast<size_t>(levels), {});
    for (int l = 0; l < levels; ++l)
    {
        const size_t s = static_cast<size_t>(field->LevelSize(l));
        field->levels[static_cast<size_t>(l)].assign(s * s * 2, 0);
        _ground[static_cast<size_t>(l)].assign(s * s, 0);
    }
    _field = field;

    // Items into square bins of level-0 texels (indices ascending in each).
    const int n = field->size;
    const float c = field->texel;
    _bins1 = (n + kBinTexels - 1) / kBinTexels;
    _bins.assign(static_cast<size_t>(_bins1) * _bins1, {});
    for (size_t i = 0; i < _items.size(); ++i)
    {
        const CanopyItem& it = _items[i];
        if (!(it.opacity > 0.0f))
            continue;
        const int x0 = std::clamp(static_cast<int>(std::floor(it.minX / c)), 0, n - 1) / kBinTexels;
        const int x1 = std::clamp(static_cast<int>(std::floor(it.maxX / c)), 0, n - 1) / kBinTexels;
        const int z0 = std::clamp(static_cast<int>(std::floor(it.minZ / c)), 0, n - 1) / kBinTexels;
        const int z1 = std::clamp(static_cast<int>(std::floor(it.maxZ / c)), 0, n - 1) / kBinTexels;
        for (int bz = z0; bz <= z1; ++bz)
            for (int bx = x0; bx <= x1; ++bx)
                _bins[static_cast<size_t>(bz) * _bins1 + bx].push_back(static_cast<int>(i));
    }

    // The ground of every level (fixed), then the canopy, a band of bins at a
    // time so a cancel stops it within one band.
    for (int z = 0; z < n; ++z)
        for (int x = 0; x < n; ++x)
            _ground[0][static_cast<size_t>(z) * n + x] =
                Encode(Ground((static_cast<float>(x) + 0.5f) * c, (static_cast<float>(z) + 0.5f) * c));
    for (int l = 1; l < levels; ++l)
    {
        const int s = field->LevelSize(l), sc = field->LevelSize(l - 1);
        const std::vector<std::uint16_t>& child = _ground[static_cast<size_t>(l - 1)];
        std::vector<std::uint16_t>& dst = _ground[static_cast<size_t>(l)];
        for (int z = 0; z < s; ++z)
            for (int x = 0; x < s; ++x)
            {
                unsigned sum = 0;
                for (int dz = 0; dz < 2; ++dz)
                    for (int dx = 0; dx < 2; ++dx)
                        sum += child[static_cast<size_t>(std::min(sc - 1, z * 2 + dz)) * sc +
                                     std::min(sc - 1, x * 2 + dx)];
                dst[static_cast<size_t>(z) * s + x] = static_cast<std::uint16_t>((sum + 2) / 4);
            }
    }
    for (int bz = 0; bz < _bins1; ++bz)
    {
        if (cancel && cancel->load(std::memory_order_relaxed))
        {
            _field.reset();
            return false;
        }
        RasterLevel0(0, bz * kBinTexels, n, std::min(n, (bz + 1) * kBinTexels));
    }
    for (int l = 1; l < levels; ++l)
    {
        const int s = field->LevelSize(l);
        Downsample(l, 0, 0, s, s);
    }
    field->version = 1;
    return true;
}

void CanopyRaster::RasterLevel0(int x0, int z0, int x1, int z1)
{
    CanopyField& f = *_field;
    const int n = f.size;
    const float c = f.texel;
    const float sub = 1.0f / static_cast<float>(kCanopySamples);
    std::vector<std::uint16_t>& dst = f.levels[0];
    for (int z = z0; z < z1; ++z)
    {
        for (int x = x0; x < x1; ++x)
        {
            const std::vector<int>& bin = _bins[static_cast<size_t>(z / kBinTexels) * _bins1 + x / kBinTexels];
            const float tx0 = static_cast<float>(x) * c, tz0 = static_cast<float>(z) * c;
            float height = 0.0f;
            float trans = 1.0f;
            bool covered = false;
            for (int idx : bin)
            {
                if (_removed[static_cast<size_t>(idx)])
                    continue;
                const CanopyItem& it = _items[static_cast<size_t>(idx)];
                if (it.maxX < tx0 || it.minX > tx0 + c || it.maxZ < tz0 || it.minZ > tz0 + c)
                    continue;
                int inside = 0;
                for (int sz = 0; sz < kCanopySamples; ++sz)
                    for (int sx = 0; sx < kCanopySamples; ++sx)
                        inside += CanopyInside(it, tx0 + (static_cast<float>(sx) + 0.5f) * sub * c,
                                               tz0 + (static_cast<float>(sz) + 0.5f) * sub * c)
                                      ? 1
                                      : 0;
                if (!inside)
                    continue;
                covered = true;
                height = std::max(height, it.height);
                trans *= 1.0f -
                         it.opacity * static_cast<float>(inside) / static_cast<float>(kCanopySamples * kCanopySamples);
            }
            const size_t i = static_cast<size_t>(z) * n + x;
            const std::uint16_t ground = _ground[0][i];
            dst[i * 2] = covered ? Encode(Decode(ground) + height) : ground;
            dst[i * 2 + 1] = covered ? Unorm16(1.0f - trans) : 0;
        }
    }
}

void CanopyRaster::Downsample(int level, int x0, int z0, int x1, int z1)
{
    CanopyField& f = *_field;
    const int s = f.LevelSize(level), sc = f.LevelSize(level - 1);
    const std::vector<std::uint16_t>& child = f.levels[static_cast<size_t>(level - 1)];
    const std::vector<std::uint16_t>& childGround = _ground[static_cast<size_t>(level - 1)];
    std::vector<std::uint16_t>& dst = f.levels[static_cast<size_t>(level)];
    const std::vector<std::uint16_t>& ground = _ground[static_cast<size_t>(level)];
    for (int z = z0; z < z1; ++z)
    {
        for (int x = x0; x < x1; ++x)
        {
            unsigned den = 0;
            float height = 0.0f;
            for (int dz = 0; dz < 2; ++dz)
            {
                for (int dx = 0; dx < 2; ++dx)
                {
                    const size_t ci =
                        static_cast<size_t>(std::min(sc - 1, z * 2 + dz)) * sc + std::min(sc - 1, x * 2 + dx);
                    const std::uint16_t g = child[ci * 2 + 1];
                    den += g;
                    if (g > kCoveredU16)
                        height = std::max(height, Decode(child[ci * 2]) - Decode(childGround[ci]));
                }
            }
            const size_t i = static_cast<size_t>(z) * s + x;
            dst[i * 2] = height > 0.0f ? Encode(Decode(ground[i]) + height) : ground[i];
            dst[i * 2 + 1] = static_cast<std::uint16_t>((den + 2) / 4);
        }
    }
}

std::vector<CanopyRect> CanopyRaster::Remove(int id)
{
    std::vector<CanopyRect> rects;
    if (!_field)
        return rects;
    const auto found = _byId.find(id);
    if (found == _byId.end() || _removed[static_cast<size_t>(found->second)])
        return rects;
    _removed[static_cast<size_t>(found->second)] = true;
    const CanopyItem& it = _items[static_cast<size_t>(found->second)];
    CanopyField& f = *_field;
    const int n = f.size;
    const float c = f.texel;
    CanopyRect r;
    r.x0 = std::clamp(static_cast<int>(std::floor(it.minX / c)), 0, n - 1);
    r.x1 = std::clamp(static_cast<int>(std::floor(it.maxX / c)), 0, n - 1) + 1;
    r.z0 = std::clamp(static_cast<int>(std::floor(it.minZ / c)), 0, n - 1);
    r.z1 = std::clamp(static_cast<int>(std::floor(it.maxZ / c)), 0, n - 1) + 1;
    RasterLevel0(r.x0, r.z0, r.x1, r.z1);
    rects.push_back(r);
    for (int l = 1; l < f.Levels(); ++l)
    {
        const int s = f.LevelSize(l);
        CanopyRect p;
        p.level = l;
        p.x0 = std::min(r.x0 / 2, s - 1);
        p.z0 = std::min(r.z0 / 2, s - 1);
        p.x1 = std::clamp((r.x1 + 1) / 2, p.x0 + 1, s);
        p.z1 = std::clamp((r.z1 + 1) / 2, p.z0 + 1, s);
        Downsample(l, p.x0, p.z0, p.x1, p.z1);
        rects.push_back(p);
        r = p;
    }
    ++f.version;
    return rects;
}

// ---- Sampling and the trace ---------------------------------------------------------

void CanopySample(const CanopyField& field, float x, float z, float lod, float& top, float& density)
{
    const int levels = field.Levels();
    const float extent = static_cast<float>(field.size) * field.texel;
    const float u = x / extent, v = z / extent;
    auto bilinear = [&](int level, float& t, float& d)
    {
        const int s = field.LevelSize(level);
        const float fx = u * static_cast<float>(s) - 0.5f, fz = v * static_cast<float>(s) - 0.5f;
        const float flx = std::floor(fx), flz = std::floor(fz);
        const float ax = fx - flx, az = fz - flz;
        const int ix = static_cast<int>(flx), iz = static_cast<int>(flz);
        auto cx = [s](int i) { return std::clamp(i, 0, s - 1); };
        const std::vector<std::uint16_t>& l = field.levels[static_cast<size_t>(level)];
        float r[2] = {0.0f, 0.0f};
        for (int c = 0; c < 2; ++c)
        {
            auto at = [&](int xx, int zz)
            { return static_cast<float>(l[(static_cast<size_t>(cx(zz)) * s + cx(xx)) * 2 + c]); };
            r[c] = ((at(ix, iz) * (1.0f - ax) + at(ix + 1, iz) * ax) * (1.0f - az) +
                    (at(ix, iz + 1) * (1.0f - ax) + at(ix + 1, iz + 1) * ax) * az) /
                   65535.0f;
        }
        t = field.heightBase + r[0] * field.heightRange;
        d = r[1];
    };
    const float l = std::clamp(lod, 0.0f, static_cast<float>(levels - 1));
    const int l0 = static_cast<int>(std::floor(l));
    const float w = l - static_cast<float>(l0);
    bilinear(l0, top, density);
    if (w > 0.0f && l0 + 1 < levels)
    {
        float t1 = 0.0f, d1 = 0.0f;
        bilinear(l0 + 1, t1, d1);
        top = Mix(top, t1, w);
        density = Mix(density, d1, w);
    }
}

CanopySunRay CanopySunRayToward(float x, float y, float z)
{
    CanopySunRay ray;
    const float sl = std::sqrt(x * x + z * z);
    if (sl < 1e-3f || y <= 0.0f)
        return ray; // no trace: the sun overhead or under the horizon
    ray.dirX = x / sl;
    ray.dirZ = z / sl;
    ray.tanSun = y / sl;
    ray.tanSoft = std::tan(kCanopySoftDeg * kDegToRad);
    ray.length = std::min(kCanopyReach / std::max(ray.tanSun, 0.02f), kCanopyMaxTrace);
    return ray;
}

CanopySunTexel CanopySunAt(const CanopyField& field, float x, float z, float groundY, const CanopySunRay& ray)
{
    CanopySunTexel out;
    if (!field.Valid() || ray.length <= 0.0f)
        return out;
    const float c0 = field.texel;
    float t = 0.5f * c0;
    float upper = kCanopyMinSoft, lower = -kCanopyMinSoft;
    for (int i = 0; i < kCanopyMaxSteps; ++i)
    {
        if (t > ray.length)
            break;
        const float step = std::max(c0, kCanopyGrowth * t);
        const float lod = std::log2(step / c0);
        float top = 0.0f, den = 0.0f;
        CanopySample(field, x + ray.dirX * t, z + ray.dirZ * t, lod, top, den);
        if (den > kCanopyMinDensity)
        {
            const float soft = t * ray.tanSoft + 0.25f * step + 0.5f;
            const float crossing = std::sqrt(den - kCanopyMinDensity);
            const float rise = t * ray.tanSun;
            out.ground *=
                std::max(1.0f - crossing * Smoothstep(-0.5f, 1.0f, (top - (groundY + kCanopyGroundLift + rise)) / soft),
                         kCanopyMinTransmit);
            if (t >= kCanopyRaisedSkip)
            {
                out.raisedLow *= std::max(
                    1.0f - crossing * Smoothstep(-0.5f, 1.0f, (top - (groundY + kCanopyRaisedLift + rise)) / soft),
                    kCanopyMinTransmit);
                // The raised receivers' shadow edge: the envelope of every
                // sample's edge, plus and minus its softness. A thin sample
                // sinks its edge out of the envelope as its crossing falls
                // (continuously, so the edge never jumps where a density
                // crosses a threshold).
                const float w = Smoothstep(kCanopyEdgeCrossFrom, kCanopyEdgeCrossTo, crossing);
                const float h = (top - rise - groundY + soft + 1.0f) * w - soft - 1.0f;
                upper = std::max(upper, h + soft);
                lower = std::max(lower, h - soft);
            }
        }
        t += step;
    }
    out.edge = 0.5f * (upper + lower);
    out.soft = 0.5f * (upper - lower);
    return out;
}

float CanopyReceiverVisibility(const CanopySunTexel& texel, float heightAboveGround)
{
    const float h = texel.edge;
    const float s = std::max(texel.soft, kCanopyMinReadSoft);
    const float raised =
        Mix(texel.raisedLow, 1.0f, Smoothstep(h - s, h + s, std::max(heightAboveGround, kCanopyRaisedFrom)));
    return Mix(texel.ground, raised, Smoothstep(kCanopyGroundTo, kCanopyRaisedFrom, heightAboveGround));
}

// ---- The sun-visibility map --------------------------------------------------------

float CanopySunTexelFor(float viewDistance)
{
    const float t = std::clamp(viewDistance * kCanopySunTexelPerVd, kCanopySunMinTexel, kCanopySunMaxTexel);
    return std::round(t * 32.0f) / 32.0f;
}

void CanopySunOrigin(float camX, float camZ, float texel, float& originX, float& originZ)
{
    const float half = 0.5f * static_cast<float>(kCanopySunSize) * texel;
    originX = std::floor(camX / texel) * texel - half;
    originZ = std::floor(camZ / texel) * texel - half;
}

bool CanopySunNeedsRebuild(const CanopySunKey& built, const CanopySunKey& want, float camX, float camZ)
{
    if (!built.valid || built.canopyVersion != want.canopyVersion || built.texel != want.texel)
        return true;
    const float dot = built.sun[0] * want.sun[0] + built.sun[1] * want.sun[1] + built.sun[2] * want.sun[2];
    if (dot < std::cos(kCanopySunRebuildDeg * kDegToRad))
        return true;
    const float extent = static_cast<float>(kCanopySunSize) * built.texel;
    const float cx = built.originX + 0.5f * extent, cz = built.originZ + 0.5f * extent;
    const float limit = kCanopySunRecentre * extent;
    return std::fabs(camX - cx) > limit || std::fabs(camZ - cz) > limit;
}

float CanopyStrength(float sunElevationDeg, float skyThrough)
{
    if (sunElevationDeg <= kGateCanopyMinSunDeg || sunElevationDeg >= kGateCanopyMaxSunDeg ||
        skyThrough <= kGateCanopyMinSkyThrough)
        return 0.0f;
    return Smoothstep(kGateCanopyMinSunDeg, kCanopyFadeInToSunDeg, sunElevationDeg) *
           (1.0f - Smoothstep(kCanopyFadeOutFromSunDeg, kGateCanopyMaxSunDeg, sunElevationDeg)) *
           Smoothstep(kGateCanopyMinSkyThrough, kCanopyFadeInToSkyThrough, skyThrough);
}

CanopyFrameConstants ComputeCanopyFrameConstants(const LightingTuning& tuning, const FeatureGates& gates, bool mapReady,
                                                 float sunElevationDeg, float skyThrough, float originX, float originZ,
                                                 float texel)
{
    CanopyFrameConstants out;
    if (!gates[Feature::CanopyShadows].on || !mapReady || !(texel > 0.0f))
        return out;
    const float strength =
        tuning.gates ? CanopyStrength(sunElevationDeg, skyThrough) : (sunElevationDeg > 0.0f ? 1.0f : 0.0f);
    if (!(strength > 0.0f))
        return out;
    out.canopy[0] = strength;
    out.canopy[1] = originX;
    out.canopy[2] = originZ;
    out.canopy[3] = 1.0f / (static_cast<float>(kCanopySunSize) * texel);
    return out;
}

} // namespace Poseidon::remaster
