#include <Poseidon/Graphics/Remaster/CloudNoise.hpp>

#include <algorithm>
#include <cmath>
#include <numeric>

namespace Poseidon::remaster
{

namespace
{
std::uint32_t Hash(std::uint32_t x, std::uint32_t y, std::uint32_t seed)
{
    std::uint32_t h = x * 0x8da6b343u ^ y * 0xd8163841u ^ seed * 0xcb1ab31fu;
    h ^= h >> 13;
    h *= 0x5bd1e995u;
    h ^= h >> 15;
    return h;
}

float Lattice(int x, int y, int period, std::uint32_t seed)
{
    const std::uint32_t wx = static_cast<std::uint32_t>(((x % period) + period) % period);
    const std::uint32_t wy = static_cast<std::uint32_t>(((y % period) + period) % period);
    return static_cast<float>(Hash(wx, wy, seed) & 0xffffu) / 65535.0f;
}

// Smooth value noise with lattice period `period` cells across the tile.
float ValueNoise(float u, float v, int period, std::uint32_t seed)
{
    const float x = u * static_cast<float>(period);
    const float y = v * static_cast<float>(period);
    const int x0 = static_cast<int>(std::floor(x));
    const int y0 = static_cast<int>(std::floor(y));
    float tx = x - static_cast<float>(x0);
    float ty = y - static_cast<float>(y0);
    tx = tx * tx * tx * (tx * (tx * 6.0f - 15.0f) + 10.0f);
    ty = ty * ty * ty * (ty * (ty * 6.0f - 15.0f) + 10.0f);
    const float a = Lattice(x0, y0, period, seed);
    const float b = Lattice(x0 + 1, y0, period, seed);
    const float c = Lattice(x0, y0 + 1, period, seed);
    const float d = Lattice(x0 + 1, y0 + 1, period, seed);
    const float ab = a + (b - a) * tx;
    const float cd = c + (d - c) * tx;
    return ab + (cd - ab) * ty;
}
} // namespace

std::vector<std::uint8_t> BuildCloudNoise(int size, std::uint32_t seed)
{
    if (size < 16 || (size & (size - 1)) != 0)
        return {};
    const size_t count = static_cast<size_t>(size) * size;
    std::vector<float> value(count);
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            const float u = (static_cast<float>(x) + 0.5f) / static_cast<float>(size);
            const float v = (static_cast<float>(y) + 0.5f) / static_cast<float>(size);
            float sum = 0.0f;
            float amp = 1.0f;
            int period = 4;
            for (int octave = 0; octave < 5; ++octave)
            {
                sum += amp * ValueNoise(u, v, period, seed + static_cast<std::uint32_t>(octave) * 7919u);
                amp *= 0.5f;
                period *= 2;
            }
            value[static_cast<size_t>(y) * size + x] = sum;
        }
    }

    // Histogram equalisation: rank every texel, so the output is uniform.
    std::vector<std::uint32_t> order(count);
    std::iota(order.begin(), order.end(), 0u);
    std::sort(order.begin(), order.end(), [&](std::uint32_t a, std::uint32_t b) { return value[a] < value[b]; });
    std::vector<std::uint8_t> out(count);
    for (size_t rank = 0; rank < count; ++rank)
        out[order[rank]] = static_cast<std::uint8_t>((rank * 256) / count);
    return out;
}

} // namespace Poseidon::remaster
