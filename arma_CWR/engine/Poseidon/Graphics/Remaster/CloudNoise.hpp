#pragma once

#include <cstdint>
#include <vector>

// Tileable noise for the remastered cloud shadows: pure CPU, no GL.
//
// Fractal value noise, wrapped so the texture tiles, then histogram-equalised so
// its values are uniformly distributed over [0, 1]. Uniform values make a
// threshold exact: the fraction of texels above (1 - c) is c, so the shader can
// set the covered fraction of the ground straight from the overcast.

namespace Poseidon::remaster
{

// size x size bytes, row-major. size must be a power of two >= 16.
std::vector<std::uint8_t> BuildCloudNoise(int size, std::uint32_t seed);

} // namespace Poseidon::remaster
