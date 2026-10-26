// Remastered lighting shaders: the local-lights piece of the fragment
// prelude (FragmentPiece::Lights, joined by RemasterFragmentCommon() in
// EngineGL33_LightingShaders.cpp): headlights, lamps and fires per fragment.
// Further kinds of local light belong here.
//
// Two sources of lights, one model (localLight): the draw's own list in the
// per-draw LitConstants slots (up to 8, the classic path and the lightTiles=0
// fallback), or the frame's tiled light list (the lightTiles switch,
// Graphics/Rendering/Lighting/FrameLights.hpp): a uniform block of lights and a
// texture of light indices per 32-pixel screen tile, so every surface draws
// from the same lights, those whose reach covers its tile, within the classic
// budget of 7 per surface. litLightCount.y holds the tile size while the tiled
// list lights the pass and is 0 otherwise.
//
// At night, with the nightLights switch (B3, litNight.x), the tiled path gives
// street lamps and headlights the physical model instead (physicalLight:
// inverse-square light from the bulb, a low-beam pattern, rough diffuse,
// leaves wrapped as for the sun), summed in linear light into g_physicalLocal
// and g_physicalSpec outside the classic clamp and budget.

#include <PoseidonGL33/EngineGL33_Lighting.hpp>

namespace Poseidon::gl33lighting
{

const char kRemasterLightsGLSL[] = R"(
// The frame's lights, kTexelsPerLight vec4 each (FrameLights.hpp
// PackFrameLights), binding 4 (kLightListBinding). A uniform block, not a
// buffer texture: the fragments of a tile read the same lights, and at a
// 10-light town view the block cost less GPU time than texel fetches.
layout(std140) uniform FrameLights {
    vec4 frameLights[256]; // kMaxFrameLights * kTexelsPerLight
};
uniform usampler2D lightTiles; // unit 13: per tile kTileTexels texels of 4 light indices + 1, 0 after the last
const int kTexelsPerLight = 4;
const int kTileTexels = 4;
const int kMaxTileLights = 16;
// The classic engine's budget of local lights per surface (FrameLights.hpp
// kClassicLightsPerSurface and ClassicBudgetWeights, its C++ mirror), kept per
// fragment by the tiled list.
const int kClassicBudget = 7;
// A light's reach in start distances: the sphere the tiles bin it by
// (FrameLights.hpp LightBounds).
const float kReachFactor = 10.0;

// One point light or spotlight at this fragment: the classic quadratic falloff
// past the start distance, ending in a smooth window instead of a hard cut at
// 10x the start distance, and a smoothed spotlight cone. pos: xyz
// camera-relative, w start distance; color: rgb diffuse, w > 0.5 spotlight;
// amb: rgb ambient; dir: the beam's unit direction.
vec3 localLight(vec4 pos, vec4 color, vec3 amb, vec3 dir, vec3 n, vec3 kd, vec3 ka) {
    vec3 toLight = pos.xyz - vWorldRel;
    float d2 = dot(toLight, toLight);
    float start2 = pos.w * pos.w;
    float end2 = start2 * 100.0;
    if (d2 >= end2) return vec3(0.0);
    float cone = 1.0;
    if (color.w > 0.5) {
        float inside = -dot(toLight, dir);
        if (inside <= 0.0) return vec3(0.0);
        float cos2 = inside * inside / max(d2, 1e-6);
        cone = smoothstep(0.9415, 0.9806, cos2); // 14 deg soft edge, full inside 8 deg
        if (cone <= 0.0) return vec3(0.0);
    }
    float atten = (d2 > start2) ? start2 / d2 : 1.0;
    atten *= 1.0 - smoothstep(0.5 * end2, end2, d2);
    float cosFi = max(dot(toLight, n), 0.0) * inversesqrt(max(d2, 1e-6));
    return (kd * color.rgb * cosFi + ka * amb) * (atten * cone);
}

// This fragment's tile list, its kTileTexels (4) texels read once
// (loadTileList), and entry k of it: a light index + 1, 0 past the last.
uvec4 g_tileIds0 = uvec4(0u), g_tileIds1 = uvec4(0u), g_tileIds2 = uvec4(0u), g_tileIds3 = uvec4(0u);

int loadTileList() {
    ivec2 tile = ivec2(gl_FragCoord.xy) / int(litLightCount.y);
    int x = tile.x * kTileTexels;
    g_tileIds0 = texelFetch(lightTiles, ivec2(x, tile.y), 0);
    if (g_tileIds0.w == 0u) return int(g_tileIds0.x != 0u) + int(g_tileIds0.y != 0u) + int(g_tileIds0.z != 0u);
    g_tileIds1 = texelFetch(lightTiles, ivec2(x + 1, tile.y), 0);
    if (g_tileIds1.w == 0u) return 4 + int(g_tileIds1.x != 0u) + int(g_tileIds1.y != 0u) + int(g_tileIds1.z != 0u);
    g_tileIds2 = texelFetch(lightTiles, ivec2(x + 2, tile.y), 0);
    if (g_tileIds2.w == 0u) return 8 + int(g_tileIds2.x != 0u) + int(g_tileIds2.y != 0u) + int(g_tileIds2.z != 0u);
    g_tileIds3 = texelFetch(lightTiles, ivec2(x + 3, tile.y), 0);
    return 12 + int(g_tileIds3.x != 0u) + int(g_tileIds3.y != 0u) + int(g_tileIds3.z != 0u) + int(g_tileIds3.w != 0u);
}

int tileEntry(int k) {
    uvec4 ids = k < 4 ? g_tileIds0 : (k < 8 ? g_tileIds1 : (k < 12 ? g_tileIds2 : g_tileIds3));
    return int(ids[k & 3]);
}

// How strongly the classic selection ranks the light with index + 1 `id` at
// this fragment: its sort brightness over the squared distance (Light::Compare),
// faded to 0 toward the edge of its reach, the sphere the tiles bin it by
// (FrameLights.hpp LightBounds and ClassicImportance): where a tile does not
// list the light it must not rank either, or the budget would change at a tile
// edge.
float lightImportance(int id) {
    int at = (id - 1) * kTexelsPerLight;
    vec4 pos = frameLights[at];
    vec3 toLight = pos.xyz - vWorldRel;
    float d2 = dot(toLight, toLight);
    float r2 = kReachFactor * kReachFactor * pos.w * pos.w;
    float fade = 1.0 - smoothstep(0.5 * r2, r2, d2);
    return fade * frameLights[at + 2].w / max(d2, 1e-4);
}

// The light with index + 1 `id` of the frame's buffer at this fragment.
vec3 tiledLight(int id, vec3 n, vec3 kd, vec3 ka) {
    int at = (id - 1) * kTexelsPerLight;
    return localLight(frameLights[at], frameLights[at + 1], frameLights[at + 2].rgb, frameLights[at + 3].xyz, n, kd, ka);
}

// ---- The physical lamps and low beams (B3, nightLights; litNight.x on) ------
// Street lamps (kind 1) and headlights (kind 2) as physical lights: intensity
// over the softened squared distance from the bulb, in linear light, summed
// outside the classic budget and clamp. FrameLights.hpp holds the model and
// these constants (the test pins them); NightLights.hpp the slot.
const float kNightSoftening = 0.25;  // kPhysicalSoftening
const float kNightMinIrradiance = 0.002; // kPhysicalMinIrradiance
const float kNightNearStart = 0.25;  // kPhysicalNearStart
const float kNightNearEnd = 1.0;     // kPhysicalNearEnd
const float kBeamCutLow = -0.005;    // kLowBeamCutLow
const float kBeamCutHigh = 0.025;    // kLowBeamCutHigh
const float kBeamHotBand = -0.03;    // kLowBeamHotBand
const float kBeamHotWidth = 0.05;    // kLowBeamHotWidth
const float kBeamFloor = 0.02;       // kLowBeamFloor
const float kBeamFloorEnd = -0.6;    // kLowBeamFloorEnd
const float kBeamFloorStart = -0.1;  // kLowBeamFloorStart
const float kBeamCoreWidth = 0.25;   // kLowBeamCoreWidth
const float kBeamSpill = 0.15;       // kLowBeamSpill
const float kBeamSpillWidth = 0.6;   // kLowBeamSpillWidth

// What shadeRemastered hands the physical lights besides the normal: the view
// direction, and for foliage the crown-blended normal and the forced diffuse
// (leaves take a lamp as they take the sun: wrapped, through the crown).
vec3 g_lightView = vec3(0.0, 1.0, 0.0);
vec3 g_lightLeafNormal = vec3(0.0, 1.0, 0.0);
vec3 g_lightForced = vec3(0.0);
bool g_lightLeaves = false;
// The physical lights' sums at this fragment, linear: the diffuse (times the
// material) and the gleam on glossy materials.
vec3 g_physicalLocal = vec3(0.0);
vec3 g_physicalSpec = vec3(0.0);

// The low beam's relative intensity toward unit direction L from the lamp, for
// the unit axis `beam` (FrameLights.hpp LowBeamPattern): wide horizontally, a
// hot band 1.7 degrees under the axis and a hard top edge just above it.
float lowBeamPattern(vec3 L, vec3 beam) {
    float c = dot(L, beam);
    if (c <= 0.0) return 0.0;
    vec3 right = vec3(beam.z, 0.0, -beam.x) * inversesqrt(max(beam.x * beam.x + beam.z * beam.z, 1e-12));
    vec3 u = cross(beam, right);
    float ah = atan(dot(L, right), c);
    float av = atan(dot(L, u), c);
    float cut = 1.0 - smoothstep(kBeamCutLow, kBeamCutHigh, av);
    float hv = (av - kBeamHotBand) / kBeamHotWidth;
    float vert = exp(-hv * hv) + kBeamFloor * smoothstep(kBeamFloorEnd, kBeamFloorStart, av);
    float h1 = ah / kBeamCoreWidth;
    float h2 = ah / kBeamSpillWidth;
    float horiz = exp(-h1 * h1) + kBeamSpill * exp(-h2 * h2);
    return vert * horiz * cut;
}

// One lamp or headlight at this fragment, added to g_physicalLocal and
// g_physicalSpec. A rough diffuse (Oren-Nayar, sigma 0.5, Fujii's form,
// normalised to 1 at normal incidence) lets gravel, asphalt and plaster scatter
// a grazing low beam back toward the driver; leaves wrap their terminator as
// they do for the sun. A lamp keeps a little of its classic ambient (capped);
// a headlight none.
void physicalLight(int id, vec3 n, vec3 kd, vec3 ka) {
    int at = (id - 1) * kTexelsPerLight;
    vec4 pos = frameLights[at];
    vec4 color = frameLights[at + 1];
    vec3 toLight = pos.xyz - vWorldRel;
    float d2 = dot(toLight, toLight);
    bool spot = color.w > 0.5;
    // The physical reach (FrameLights.hpp PhysicalReach), which the tiles bin by.
    float peak = spot ? litNight.z : litNight.y * pos.w;
    float end2 = min(kReachFactor * kReachFactor * pos.w * pos.w, peak / kNightMinIrradiance);
    if (d2 >= end2) return;
    float d = sqrt(max(d2, 1e-12));
    vec3 L = toLight / d;
    float I = spot ? peak * lowBeamPattern(-L, frameLights[at + 3].xyz) : peak;
    if (I <= 0.0) return;
    I *= (1.0 - smoothstep(0.5 * end2, end2, d2)) * smoothstep(kNightNearStart, kNightNearEnd, d);
    float E = I / (d2 + kNightSoftening);
    vec3 col = toLinear(color.rgb) * E;
    if (g_lightLeaves) {
        float wrap = clamp((dot(g_lightLeafNormal, L) + 0.35) / 1.35, 0.0, 1.0);
        g_physicalLocal += col * (kd * wrap + g_lightForced);
        return;
    }
    float nl = max(dot(n, L), 0.0);
    if (nl > 0.0) {
        float cosR = max(dot(n, g_lightView), 0.0);
        float s = dot(L, g_lightView) - nl * cosR;
        float t = s > 0.0 ? max(max(nl, cosR), 0.1) : 1.0;
        nl *= max((0.72 + 0.33 * s / t) / 0.72, 0.0);
    }
    g_physicalLocal += kd * col * nl;
    if (!spot) g_physicalLocal += ka * toLinear(frameLights[at + 2].rgb) * (0.3 * min(E, 0.3));
    float power = litMatSpecular.w;
    if (power > 0.0 && nl > 0.0) {
        vec3 h = normalize(L + g_lightView);
        float p = max(power, 1.0);
        g_physicalSpec += litMatSpecular.rgb * col * (pow(max(dot(n, h), 0.0), p) * (p + 8.0) / 28.0 * nl);
    }
}

// The kinds the physical model shades: a lamp (1) or a headlight (2), while
// litNight.x is on.
bool physicalKind(int id) {
    float kind = frameLights[(id - 1) * kTexelsPerLight + 3].w;
    return litNight.x > 0.5 && kind > 0.5 && kind < 2.5;
}

// The frame's tiled list: the lights whose reach covers this fragment's tile
// (an empty tile costs one fetch), within the classic budget. The classic
// selection (Scene::SelectLights) ranks lights by their sort brightness over
// the squared distance and keeps kClassicBudget of them per object; here the
// ranking is taken at the fragment, and the 7th and 8th strongest share a
// blend instead of a cut, so the set changes smoothly across a surface rather
// than at object edges. The buffer holds the lights' own colours; the draw's
// night factor (litLightCount.z, the factor the per-draw upload scales them
// by) applies to the sum. With the physical lamps and low beams on
// (litNight.x), lamps and headlights leave the classic sum and its budget:
// each adds its own light to g_physicalLocal (and gleam to g_physicalSpec),
// linear and unclamped, and the budget ranks the other kinds among
// themselves.
vec3 localLightsTiled(vec3 n, vec3 kd, vec3 ka) {
    int count = loadTileList();
    int classic = count;
    if (litNight.x > 0.5) {
        classic = 0;
        for (int k = 0; k < kMaxTileLights; ++k) {
            if (k >= count) break;
            int id = tileEntry(k);
            if (physicalKind(id)) physicalLight(id, n, kd, ka);
            else ++classic;
        }
        g_physicalLocal *= litLightCount.z;
        g_physicalSpec *= litLightCount.z;
    }
    // The budget's edge: the importance of the 7th (edgeIn) and the 8th
    // (edgeOut) strongest, from the eight strongest kept sorted in registers.
    float edgeIn = 0.0;
    float edgeOut = 0.0;
    if (classic > kClassicBudget) {
        float t0 = 0.0, t1 = 0.0, t2 = 0.0, t3 = 0.0, t4 = 0.0, t5 = 0.0, t6 = 0.0, t7 = 0.0;
        for (int k = 0; k < kMaxTileLights; ++k) {
            if (k >= count) break;
            if (physicalKind(tileEntry(k))) continue;
            float v = lightImportance(tileEntry(k));
            float m;
            m = max(t0, v); v = min(t0, v); t0 = m;
            m = max(t1, v); v = min(t1, v); t1 = m;
            m = max(t2, v); v = min(t2, v); t2 = m;
            m = max(t3, v); v = min(t3, v); t3 = m;
            m = max(t4, v); v = min(t4, v); t4 = m;
            m = max(t5, v); v = min(t5, v); t5 = m;
            m = max(t6, v); v = min(t6, v); t6 = m;
            t7 = max(t7, v);
        }
        edgeIn = t6;
        edgeOut = t7;
    }
    float mid = 0.5 * (edgeIn + edgeOut);
    float band = max(edgeIn - edgeOut, 0.25 * edgeIn);
    vec3 sum = vec3(0.0);
    for (int k = 0; k < kMaxTileLights; ++k) {
        if (k >= count) break;
        int id = tileEntry(k);
        if (physicalKind(id)) continue;
        float w = classic > kClassicBudget ? clamp(0.5 + (lightImportance(id) - mid) / band, 0.0, 1.0) : 1.0;
        if (w > 0.0) sum += w * tiledLight(id, n, kd, ka);
    }
    return sum * litLightCount.z;
}

// Headlights, lamps and fires at this fragment (display space, before the
// clamp and the linear conversion in shadeRemastered).
vec3 localLights(vec3 n, vec3 kd, vec3 ka) {
    if (litLightCount.y > 0.5) return localLightsTiled(n, kd, ka);
    vec3 sum = vec3(0.0);
    int count = int(litLightCount.x);
    for (int i = 0; i < 8; ++i) {
        if (i >= count) break;
        sum += localLight(litLightPos[i], litLightColor[i], litLightAmb[i].rgb, litLightDir[i].xyz, n, kd, ka);
    }
    return sum;
}

// Debug view 6 (FrameLights.hpp kDebugViewLightTiles): how many lights the
// fragment's tile lists, blue for one to red for eight or more, over half of
// the shaded colour, with the tile grid. Without the tiled list the frame
// shows as it is.
vec3 lightTileView(vec3 display) {
    if (litLightCount.y < 0.5) return display;
    int size = int(litLightCount.y);
    ivec2 px = ivec2(gl_FragCoord.xy);
    ivec2 tile = px / size;
    int count = 0;
    for (int t = 0; t < kTileTexels; ++t) {
        uvec4 ids = texelFetch(lightTiles, ivec2(tile.x * kTileTexels + t, tile.y), 0);
        count += int(ids.x != 0u) + int(ids.y != 0u) + int(ids.z != 0u) + int(ids.w != 0u);
    }
    vec3 heat = mix(vec3(0.0, 0.35, 1.0), vec3(1.0, 0.1, 0.0), clamp(float(count - 1) / 7.0, 0.0, 1.0));
    vec3 c = count > 0 ? mix(display, heat, 0.5) : display;
    ivec2 edge = px - tile * size;
    return (edge.x == 0 || edge.y == 0) ? c * 0.6 : c;
}
)";

} // namespace Poseidon::gl33lighting
