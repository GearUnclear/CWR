#include <Poseidon/Graphics/Remaster/TerrainHorizon.hpp>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <thread>

namespace Poseidon::remaster
{

namespace
{
constexpr float kPi = 3.14159265358979323846f;
constexpr float kDegPerRad = 180.0f / kPi;

// Bilinear height at a continuous texel coordinate; the caller keeps (fx, fz)
// inside [0, size - 1].
float SampleHeight(const std::vector<float>& h, int size, float fx, float fz)
{
    const int x0 = std::min(static_cast<int>(fx), size - 2);
    const int z0 = std::min(static_cast<int>(fz), size - 2);
    const float tx = fx - static_cast<float>(x0);
    const float tz = fz - static_cast<float>(z0);
    const float* row0 = h.data() + static_cast<size_t>(z0) * size;
    const float* row1 = row0 + size;
    const float a = row0[x0] + (row0[x0 + 1] - row0[x0]) * tx;
    const float b = row1[x0] + (row1[x0 + 1] - row1[x0]) * tx;
    return a + (b - a) * tz;
}

// Box-downsample to at most maxSize texels per side.
void Downsample(const float* src, int size, float cellSize, int maxSize, std::vector<float>& dst, int& dstSize,
                float& dstCell)
{
    const int factor = (size + maxSize - 1) / maxSize;
    if (factor <= 1)
    {
        dst.assign(src, src + static_cast<size_t>(size) * size);
        dstSize = size;
        dstCell = cellSize;
        return;
    }
    dstSize = (size + factor - 1) / factor;
    dstCell = cellSize * static_cast<float>(factor);
    dst.assign(static_cast<size_t>(dstSize) * dstSize, 0.0f);
    for (int z = 0; z < dstSize; ++z)
    {
        for (int x = 0; x < dstSize; ++x)
        {
            float sum = 0.0f;
            int n = 0;
            for (int dz = 0; dz < factor; ++dz)
            {
                const int sz = z * factor + dz;
                if (sz >= size)
                    break;
                for (int dx = 0; dx < factor; ++dx)
                {
                    const int sx = x * factor + dx;
                    if (sx >= size)
                        break;
                    sum += src[static_cast<size_t>(sz) * size + sx];
                    ++n;
                }
            }
            dst[static_cast<size_t>(z) * dstSize + x] = n > 0 ? sum / static_cast<float>(n) : 0.0f;
        }
    }
}

// Highest elevation angle (degrees) seen from texel (x, z) along (dx, dz).
// Starts two texels out: the slope of the ground itself is already in the
// shader's N.L, and counting it again here turns every facet of the grid into a
// visible step when the sun is low. Steps one texel at a time near the start,
// then grows geometrically out to maxDistance.
float MarchHorizon(const std::vector<float>& h, int size, float cell, int x, int z, float dx, float dz,
                   float maxDistance)
{
    const float h0 = h[static_cast<size_t>(z) * size + x];
    const float limit = static_cast<float>(size - 1);
    float best = -1e30f; // tan of the elevation angle
    float dist = cell * 2.0f;
    while (dist <= maxDistance)
    {
        const float fx = static_cast<float>(x) + dx * dist / cell;
        const float fz = static_cast<float>(z) + dz * dist / cell;
        if (fx < 0.0f || fz < 0.0f || fx > limit || fz > limit)
            break;
        const float t = (SampleHeight(h, size, fx, fz) - h0) / dist;
        if (t > best)
            best = t;
        dist += std::max(cell, dist * 0.08f);
    }
    if (best < -1e29f)
        return 0.0f; // edge of the map: open horizon
    return std::atan(best) * kDegPerRad;
}
} // namespace

std::uint16_t EncodeHorizonDeg(float deg)
{
    const float range = TerrainHorizonMap::kMaxAngleDeg - TerrainHorizonMap::kMinAngleDeg;
    const float t = (deg - TerrainHorizonMap::kMinAngleDeg) / range;
    const float c = std::min(1.0f, std::max(0.0f, t));
    return static_cast<std::uint16_t>(std::lround(c * 65535.0f));
}

float DecodeHorizonDeg(std::uint16_t value)
{
    const float range = TerrainHorizonMap::kMaxAngleDeg - TerrainHorizonMap::kMinAngleDeg;
    return TerrainHorizonMap::kMinAngleDeg + range * (static_cast<float>(value) / 65535.0f);
}

float TerrainHorizonMap::HorizonDeg(int x, int z, int dir) const
{
    const int layer = dir / 4;
    const int comp = dir % 4;
    const size_t texel = (static_cast<size_t>(layer) * size + z) * size + x;
    return DecodeHorizonDeg(horizon[texel * 4 + comp]);
}

float TerrainHorizonMap::SkyVisibility(int x, int z) const
{
    return static_cast<float>(skyVisibility[static_cast<size_t>(z) * size + x]) / 255.0f;
}

bool BuildTerrainHorizonMap(const float* heights, int size, float cellSize, const TerrainHorizonOptions& options,
                            TerrainHorizonMap& out)
{
    out = TerrainHorizonMap{};
    if (!heights || size < 2 || !(cellSize > 0.0f))
        return false;

    std::vector<float> h;
    int n = 0;
    float cell = 0.0f;
    Downsample(heights, size, cellSize, std::max(2, options.maxSize), h, n, cell);

    out.size = n;
    out.cellSize = cell;
    out.horizon.assign(static_cast<size_t>(TerrainHorizonMap::kLayers) * n * n * 4, 0);
    out.skyVisibility.assign(static_cast<size_t>(n) * n, 255);
    out.height = h;

    float dirX[TerrainHorizonMap::kDirections];
    float dirZ[TerrainHorizonMap::kDirections];
    for (int i = 0; i < TerrainHorizonMap::kDirections; ++i)
    {
        const float a = 2.0f * kPi * static_cast<float>(i) / static_cast<float>(TerrainHorizonMap::kDirections);
        dirX[i] = std::sin(a);
        dirZ[i] = std::cos(a);
    }

    // Pass 1: raw angles per direction (float) and the sky visibility.
    const size_t texels = static_cast<size_t>(n) * n;
    std::vector<float> angles(static_cast<size_t>(TerrainHorizonMap::kDirections) * texels);
    std::atomic<int> nextRow{0};
    auto march = [&]()
    {
        for (int z = nextRow.fetch_add(1); z < n; z = nextRow.fetch_add(1))
        {
            for (int x = 0; x < n; ++x)
            {
                const size_t t = static_cast<size_t>(z) * n + x;
                float visible = 0.0f;
                for (int i = 0; i < TerrainHorizonMap::kDirections; ++i)
                {
                    const float deg = MarchHorizon(h, n, cell, x, z, dirX[i], dirZ[i], options.maxDistance);
                    angles[static_cast<size_t>(i) * texels + t] = deg;
                    // Cosine-weighted sky seen above a horizon at `deg`: cos^2.
                    const float c = std::cos(std::max(0.0f, deg) / kDegPerRad);
                    visible += c * c;
                }
                visible /= static_cast<float>(TerrainHorizonMap::kDirections);
                out.skyVisibility[t] =
                    static_cast<std::uint8_t>(std::lround(std::min(1.0f, std::max(0.0f, visible)) * 255.0f));
            }
        }
    };
    // Pass 2: 3x3 binomial smoothing per direction, then encode.
    auto smooth = [&]()
    {
        for (int z = nextRow.fetch_add(1); z < n; z = nextRow.fetch_add(1))
        {
            for (int i = 0; i < TerrainHorizonMap::kDirections; ++i)
            {
                const float* a = angles.data() + static_cast<size_t>(i) * texels;
                const int layer = i / 4;
                for (int x = 0; x < n; ++x)
                {
                    float sum = 0.0f;
                    float wsum = 0.0f;
                    for (int dz = -1; dz <= 1; ++dz)
                    {
                        const int zz = std::min(n - 1, std::max(0, z + dz));
                        for (int dx = -1; dx <= 1; ++dx)
                        {
                            const int xx = std::min(n - 1, std::max(0, x + dx));
                            const float w = static_cast<float>((2 - std::abs(dx)) * (2 - std::abs(dz)));
                            sum += w * a[static_cast<size_t>(zz) * n + xx];
                            wsum += w;
                        }
                    }
                    const size_t texel = (static_cast<size_t>(layer) * n + z) * n + x;
                    out.horizon[texel * 4 + (i % 4)] = EncodeHorizonDeg(sum / wsum);
                }
            }
        }
    };

    int threads = options.threads > 0 ? options.threads : static_cast<int>(std::thread::hardware_concurrency());
    threads = std::max(1, std::min(threads, 16));
    auto runPass = [&](auto& pass)
    {
        nextRow = 0;
        std::vector<std::thread> pool;
        pool.reserve(static_cast<size_t>(threads - 1));
        for (int t = 1; t < threads; ++t)
            pool.emplace_back(pass);
        pass();
        for (auto& th : pool)
            th.join();
    };
    runPass(march);
    runPass(smooth);
    return true;
}

float SunVisibility(const TerrainHorizonMap& map, int x, int z, float towardSunX, float towardSunY, float towardSunZ,
                    float softnessDeg)
{
    if (!map.Valid())
        return 1.0f;
    const float horiz = std::sqrt(towardSunX * towardSunX + towardSunZ * towardSunZ);
    const float elevation = std::atan2(towardSunY, horiz) * kDegPerRad;
    float azimuth = std::atan2(towardSunX, towardSunZ) / (2.0f * kPi); // turns, 0 = +Z
    if (azimuth < 0.0f)
        azimuth += 1.0f;
    const float f = azimuth * static_cast<float>(TerrainHorizonMap::kDirections);
    const int i0 = static_cast<int>(f) % TerrainHorizonMap::kDirections;
    const int i1 = (i0 + 1) % TerrainHorizonMap::kDirections;
    const float w = f - std::floor(f);
    const float horizon = map.HorizonDeg(x, z, i0) * (1.0f - w) + map.HorizonDeg(x, z, i1) * w;
    const float soft = std::max(softnessDeg, 1e-3f);
    const float t = std::min(1.0f, std::max(0.0f, (elevation - (horizon - soft)) / (2.0f * soft)));
    return t * t * (3.0f - 2.0f * t);
}

std::uint64_t HashHeights(const float* heights, int size, float cellSize)
{
    std::uint64_t h = 1469598103934665603ull;
    auto mix = [&h](std::uint32_t v)
    {
        for (int i = 0; i < 4; ++i)
        {
            h ^= (v >> (8 * i)) & 0xffu;
            h *= 1099511628211ull;
        }
    };
    mix(static_cast<std::uint32_t>(size));
    std::uint32_t bits = 0;
    static_assert(sizeof(bits) == sizeof(cellSize), "float is 32-bit");
    std::memcpy(&bits, &cellSize, sizeof(bits));
    mix(bits);
    if (heights)
    {
        const size_t count = static_cast<size_t>(size) * static_cast<size_t>(std::max(size, 0));
        for (size_t i = 0; i < count; ++i)
        {
            std::memcpy(&bits, &heights[i], sizeof(bits));
            mix(bits);
        }
    }
    return h;
}

} // namespace Poseidon::remaster
