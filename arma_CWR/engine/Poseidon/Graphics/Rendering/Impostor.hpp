#pragma once

// XP-impostor (experimental, POSEIDON_XP_IMPOSTOR=1, default off): far trees
// and bushes drawn as baked billboards.
//
// Each vegetation shape (map type tree, small tree or bush) seen beyond the
// impostor distance is baked once, from kFrames directions around it at the
// horizon, into layers of two texture arrays: albedo and alpha, and the
// model-space normal with the depth toward the camera. Every far instance is
// then one camera-facing quad of one instanced draw per render state; the
// fragment shader rebuilds the point and normal of the tree under the pixel
// from the atlas and lights it with the remastered object lighting itself
// (shadeRemastered: sun, crown canopy, shadows, sky, fog), so a far tree takes
// the light of the near one. The mip chain keeps each frame's leaf coverage
// (alpha-tested cards thin out under plain box mips).
//
// This header is the backend-free half: settings, the per-instance record the
// scene hands the engine, and the pure helpers the bake and the shader share
// (frame selection, the coverage-keeping mip chain). EngineGL33_Impostor.cpp
// is the GL half.

#include <cstdint>
#include <vector>

namespace Poseidon::render::impostor
{

constexpr int kFrames = 8;        // bake directions around the shape (45 degrees apart)
constexpr int kTile = 128;        // atlas tile, texels a side (level 0)
constexpr int kBakeSize = 256;    // bake target, texels a side (downsampled 2x to the tile)
constexpr int kMaxSlots = 32;     // baked shapes
constexpr float kAlphaRef = 0.5f; // bake cutout threshold and the coverage the mips keep

struct Settings
{
    bool on = false;         // POSEIDON_XP_IMPOSTOR=1
    float distance = 600.0f; // POSEIDON_XP_IMPOSTOR_DIST (m): impostors from here out
    bool lod0 = true;        // POSEIDON_XP_IMPOSTOR_LOD0=0 bakes the coarsest level instead of the finest
    // POSEIDON_XP_IMPOSTOR_FAR=1: vegetation past the object distance (objectsZ,
    // 2/3 of the view distance) is drawn too, as impostors, out to the fog's
    // end: the far hills keep their forests instead of going bare.
    bool far = false;
};

// Read once from the environment.
const Settings& Get();

// One far instance: the model's frame in camera-relative world space (the
// position, then the model's x, y, z axes with their scale) and its slot.
struct Instance
{
    float pos[3];
    float slot;
    float ax[4];
    float ay[4];
    float az[4];
};
static_assert(sizeof(Instance) == 64, "four vec4 attributes");

// The frame pair and blend for a view direction in model space (horizontal
// components, any length): frames are at azimuth 2 pi k / frames, measured
// from +z toward +x (atan2(x, z)). Mirrors the impostor vertex shader.
struct FramePick
{
    int f0 = 0;
    int f1 = 0;
    float blend = 0.0f; // weight of f1
};
FramePick PickFrames(float dirX, float dirZ, int frames);

// One RGBA8 image, rows bottom-up (GL order).
struct Image
{
    int w = 0;
    int h = 0;
    std::vector<std::uint8_t> rgba;
};

// Halve a bake (albedo + alpha, normal + depth pair) to the next size:
// colour, normal and depth averaged by alpha, alpha averaged. Texels with no
// alpha at all take `fill` (albedo) or `fillN` (normal + depth).
void Downsample(const Image& albedo, const Image& normal, Image& outAlbedo, Image& outNormal,
                const std::uint8_t fill[4], const std::uint8_t fillN[4]);

// Share of texels whose alpha reaches `ref` (0..1).
float Coverage(const Image& albedo, float ref);

// Scales the alpha of `albedo` so its coverage at `ref` comes as close to
// `target` as a search over the scale finds (Castano's coverage-preserving
// mips). Returns the scale used.
float KeepCoverage(Image& albedo, float ref, float target);

// The whole chain from level 0 (kTile) down to 1x1, each level coverage-kept
// against level 0.
void BuildChain(const Image& albedo0, const Image& normal0, const std::uint8_t fill[4], const std::uint8_t fillN[4],
                std::vector<Image>& albedoChain, std::vector<Image>& normalChain);

} // namespace Poseidon::render::impostor
