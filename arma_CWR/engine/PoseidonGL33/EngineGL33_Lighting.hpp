#pragma once

// Remastered lighting: shader sources and the LitConstants uniform layout.
// See EngineGL33_LightingShaders.cpp for the shading model and
// Poseidon/Graphics/Remaster/LightRig.hpp for the light rig.

namespace Poseidon::gl33lighting
{

// Standalone vertex shader of the remastered set for a VertexShaderID, or
// nullptr.
const char* RemasterVertexSource(int vs);
// Fragment shaders are the common prelude followed by a main body. A null main
// means the pixel shader reuses its classic source (PSFlat, PSShadow).
const char* RemasterFragmentCommon();
const char* RemasterFragmentMain(int ps);

// Uniform-buffer binding point of LitConstants (VS = 0, PS = 1, WorldInstances = 2).
constexpr unsigned int kLitBinding = 3;

// Texture units of the remastered inputs (0/1 = material textures, 2 = shadow
// map array, 7 = the upload unit).
constexpr int kTerrainHorizonUnit = 3;
constexpr int kTerrainHeightUnit = 4;
constexpr int kCloudNoiseUnit = 5;

// LitConstants vec4 slots; must match the GLSL block in s_psRemasterCommon.
namespace LitConst
{
enum : int
{
    SunDir = 0,
    Sun = 1,
    Zenith = 2,
    Horizon = 3,
    Ground = 4,
    MoonDir = 5,
    Moon = 6,
    CamPos = 7,
    MatDiffuse = 8,
    MatAmbient = 9,
    MatForced = 10,
    MatEmissive = 11,
    MatSpecular = 12,
    Terrain = 13,
    TerrainSel0 = 14,
    TerrainSel1 = 15,
    TerrainSun = 16,
    TerrainLift = 17,
    Cloud = 18,
    CloudOffset = 19,
    ShadowDelta = 20,
    Flags = 21,
    LightCount = 22,
    HorizonSun = 23,
    FogSun = 24,
    FogCap = 25,
    LightPos = 26,   // MaxLights vec4
    LightColor = 34, // MaxLights vec4
    LightAmb = 42,   // MaxLights vec4
    LightDir = 50,   // MaxLights vec4
    Count = 58,
};
constexpr int MaxLights = 8;
static_assert(LightPos == FogCap + 1, "LitConst slots out of order");
static_assert(LightColor == LightPos + MaxLights, "LitConst light ranges overlap");
static_assert(LightAmb == LightColor + MaxLights, "LitConst light ranges overlap");
static_assert(LightDir == LightAmb + MaxLights, "LitConst light ranges overlap");
static_assert(Count == LightDir + MaxLights, "LitConst size out of sync");
} // namespace LitConst

} // namespace Poseidon::gl33lighting
