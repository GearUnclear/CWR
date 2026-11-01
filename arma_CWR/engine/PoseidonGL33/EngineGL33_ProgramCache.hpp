#pragma once

// The world program set's binary cache layout and its slot sharing
// (EngineGL33_Shaders.cpp: InitPixelShaders, DeinitPixelShaders,
// DoSelectPixelShader). No GL here, so the unit tests can pin it.
//
// The [specular][mode] slots of one (set, vertex shader, pixel shader) link the
// same vertex and fragment shader objects: their programs are identical. What
// differs between the slots lives outside the program (the night eye's
// rgbEyeCoef in the PSConstants block; specular selects nothing at all), so the
// set links one program per (set, vs, ps), 36 in all instead of 144, and every
// slot of it holds the same GL name.

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace Poseidon::gl33programs
{

// gl33_shaders.bin layout version. 2: one entry per (set, vs, ps), keyed by
// WorldProgramKey; version 1 held one entry per (set, vs, specular, mode, ps).
// Later cache-key changes (the shader salt, the non-world program registry)
// share this bump instead of taking another.
constexpr uint32_t kShaderCacheVersion = 2;

// Set 0 is classic, set 1 the remastered lighting.
constexpr uint32_t WorldProgramKey(int set, int vs, int ps)
{
    return (static_cast<uint32_t>(set) << 28) | (static_cast<uint32_t>(vs) << 24) | static_cast<uint32_t>(ps);
}

// Stores `prog` in every [specular][mode] slot of (vs, ps) of a
// [vs][specular][mode][ps] program table.
template <class Table>
void ShareAcrossSpecularAndMode(Table& table, int vs, int ps, unsigned int prog)
{
    for (auto& bySpecular : table[vs])
        for (auto& byMode : bySpecular)
            byMode[ps] = prog;
}

// Appends each non-zero name of `slots` that `out` does not hold yet, in the
// order first seen, so a shared program is deleted once.
inline void CollectDistinctPrograms(const unsigned int* slots, size_t count, std::vector<unsigned int>& out)
{
    for (size_t i = 0; i < count; ++i)
        if (slots[i] && std::find(out.begin(), out.end(), slots[i]) == out.end())
            out.push_back(slots[i]);
}

} // namespace Poseidon::gl33programs
