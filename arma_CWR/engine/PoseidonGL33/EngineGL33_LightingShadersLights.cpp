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
// At night, with the nightLights switch (litNight.x), the tiled path gives
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
    // Combat light (CombatLights.hpp): x the combat lights at the head of
    // the list (light indices 1 to x), y the shadow mask's strength (0 = no
    // mask this frame).
    vec4 combatFrame;
    // The shadow mask's mapping (CombatMaskRows): u = (combatPrevU.xyz . p +
    // combatPrevU.w) / z, v likewise, z = combatPrevZ.xyz . p + combatPrevZ.w
    // (last frame's view depth), for a camera-relative point p.
    vec4 combatPrevU;
    vec4 combatPrevV;
    vec4 combatPrevZ;
};
uniform usampler2D lightTiles; // unit 13: per tile kTileTexels texels of 4 light indices + 1, 0 after the last
uniform sampler2D combatMask;  // unit 14: the combat shadow mask, one shadowed light per channel (last frame's screen)
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
// The list's length as localLightsTiled read it (-1 before it has; the lamp
// streaks reuse the list instead of reading it again).
int g_tileCount = -1;

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

// ---- The physical lamps and low beams (nightLights; litNight.x on) ----------
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

// ---- Experiment xp-nightlight (litXpNight, all 0 while its switches are off) --
// litXpNight.x the window glow's gain (times the night's fade), y the light
// bounce's gain, z 1 while street lamps take channels of the combat shadow
// mask (their channel + 1 in their beam's x, 0 for none).
const float kXpGroundAlbedo = 0.2;
const float kXpCombatBounce = 0.25;
float combatShadowVis(int channel);

// The light a lamp, a headlight or a fire throws on the ground bounced onto
// this fragment from below: the ground the surface faces (under it, pushed
// out along its normal by about its height over the ground) takes the light's
// irradiance there, kXpGroundAlbedo of it (in the ground's own tint) comes
// back up, and a surface sees the ground with its lower hemisphere's share,
// (1 - n.y) / 2: none on flat ground, half on a wall, all on an overhang.
// No occlusion is known (not the light's toward that ground, nor the ground's
// toward the fragment). xpBounceSetup finds that ground once per fragment.
vec3 g_xpBounceFoot = vec3(0.0);
float g_xpBounceView = -1.0;
float g_xpBounceHeight = 0.0;
bool xpBounceSetup(vec3 n) {
    if (g_xpBounceView >= 0.0) return g_xpBounceView > 0.0;
    g_xpBounceView = 0.0;
    if (litXpNight.y <= 0.0 || g_groundHeight < -1e5) return false;
    float view = 0.5 - 0.5 * n.y;
    if (view <= 0.02) return false;
    float hr = max(vWorldRel.y + litCamPos.y - g_groundHeight, 0.0);
    vec2 side = n.xz * inversesqrt(max(dot(n.xz, n.xz), 1e-8));
    float push = 0.5 + 0.5 * min(hr, 6.0);
    g_xpBounceFoot = vec3(vWorldRel.x + side.x * push, g_groundHeight - litCamPos.y, vWorldRel.z + side.y * push);
    g_xpBounceHeight = hr;
    g_xpBounceView = view * kXpGroundAlbedo * litXpNight.y;
    return true;
}

// A physical lamp's (or headlight's) bounce: intensity `I0` at the bulb
// `lightRel`; a headlight's toward the ground point by its low beam.
float xpLampBounce(vec3 lightRel, float I0, bool spot, vec3 beam);

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
    // (xp-nightlight) The ground's bounce, whichever way the surface faces.
    if (litXpNight.y > 0.0 && xpBounceSetup(g_lightLeaves ? g_lightLeafNormal : n)) {
        float I0 = spot ? peak : peak * (litCamera.x > 0.5 ? litCameraLook.z : 1.0);
        float eb = xpLampBounce(pos.xyz, I0, spot, frameLights[at + 3].xyz) * (1.0 - smoothstep(0.5 * end2, end2, d2));
        vec3 cb = toLinear(color.rgb);
        if (spot) cb = vec3(dot(cb, kLuma));
        g_physicalLocal += cb * g_groundTint * eb * (g_lightLeaves ? kd + g_lightForced : kd);
    }
    // A low beam on a surface that faces away adds nothing: everything below
    // is times nl for it (a headlight has no ambient line), so its pattern is
    // not worked out. Leaves wrap past the terminator and take it all.
    if (spot && !g_lightLeaves && dot(n, L) <= 0.0) return;
    float I = spot ? peak * lowBeamPattern(-L, frameLights[at + 3].xyz) : peak;
    if (I <= 0.0) return;
    I *= (1.0 - smoothstep(0.5 * end2, end2, d2)) * smoothstep(kNightNearStart, kNightNearEnd, d);
    // Under the photoreal camera a street lamp lays a stronger pool
    // (litCameraLook.z; the night's exposure sits far under the day's), and
    // its head shades what is within a few metres of the bulb (its own pole,
    // which the stronger lamp lit white from top to bottom).
    if (litCamera.x > 0.5 && !spot) I *= litCameraLook.z * smoothstep(1.5, 4.5, d);
    float E = I / (d2 + kNightSoftening);
    // (xp-nightlight) A street lamp with a channel of the shadow mask.
    if (!spot && litXpNight.z > 0.5) {
        float ch = frameLights[at + 3].x;
        if (ch > 0.5) E *= combatShadowVis(int(ch + 0.5));
    }
    // A lamp's authored colour; a headlight white at its colour's luminance
    // (FrameLights.hpp PhysicalColour: the rosy beam turned khaki verges tan).
    vec3 col = toLinear(color.rgb);
    if (spot) col = vec3(dot(col, kLuma));
    col *= E;
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
    // (Under the photoreal camera the lamp keeps litCameraLook.y of this glow:
    // it lit a lamp's pole evenly along its whole height.)
    if (!spot) g_physicalLocal += ka * toLinear(frameLights[at + 2].rgb) * (0.3 * min(E, 0.3)) *
                                  (litCamera.x > 0.5 ? litCameraLook.y : 1.0);
    float power = litMatSpecular.w;
    if (power > 0.0 && nl > 0.0) {
        vec3 h = normalize(L + g_lightView);
        float p = max(power, 1.0);
        g_physicalSpec += litMatSpecular.rgb * col * (pow(max(dot(n, h), 0.0), p) * (p + 8.0) / 28.0 * nl);
    }
}

float xpLampBounce(vec3 lightRel, float I0, bool spot, vec3 beam) {
    vec3 toL = lightRel - g_xpBounceFoot;
    if (toL.y <= 0.05) return 0.0;
    float d2 = dot(toL, toL);
    float d = sqrt(d2);
    float I = spot ? I0 * lowBeamPattern(-toL / d, beam) : I0;
    if (I <= 0.0) return 0.0;
    float Eg = I * (toL.y / d) / (d2 + kNightSoftening + 0.5 * g_xpBounceHeight * g_xpBounceHeight);
    return Eg * g_xpBounceView;
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
// The combat lights listed at the head of this fragment's tile (set by
// localLightsTiled, read by combatLights).
int g_combatHead = 0;

vec3 localLightsTiled(vec3 n, vec3 kd, vec3 ka) {
    int count = loadTileList();
    g_tileCount = count;
    // The frame's combat lights lead its list and so every tile's
    // (FrameLights.hpp MergeCombatFrameLights): combatLights shades them, and
    // neither the classic budget nor the physical model sees them.
    int head = 0;
    int nCombat = int(combatFrame.x);
    while (head < count && tileEntry(head) <= nCombat) ++head;
    g_combatHead = head;
    int classic = count - head;
    if (litNight.x > 0.5) {
        classic = 0;
        for (int k = 0; k < kMaxTileLights; ++k) {
            if (k >= count) break;
            if (k < head) continue;
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
            if (k < head) continue;
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
        if (k < head) continue;
        int id = tileEntry(k);
        if (physicalKind(id)) continue;
        float w = classic > kClassicBudget ? clamp(0.5 + (lightImportance(id) - mid) / band, 0.0, 1.0) : 1.0;
        if (w > 0.0) sum += w * tiledLight(id, n, kd, ka);
    }
    return sum * litLightCount.z;
}

// The combat shadow mask at this fragment: last frame's mask, where this
// point was on last frame's screen, read once per fragment (one bilinear tap
// for all four channels). Outside that screen, or behind that camera, nothing
// is shadowed. g_combatMaskInside tells debug view 11 where the tap missed.
vec4 g_combatMaskVis = vec4(1.0);
bool g_combatMaskRead = false;
bool g_combatMaskInside = false;

float combatShadowVis(int channel) {
    if (!g_combatMaskRead) {
        g_combatMaskRead = true;
        float z = dot(combatPrevZ.xyz, vWorldRel) + combatPrevZ.w;
        if (z > 0.05) {
            vec2 uv = vec2(dot(combatPrevU.xyz, vWorldRel) + combatPrevU.w, dot(combatPrevV.xyz, vWorldRel) + combatPrevV.w) / z;
            if (all(greaterThanEqual(uv, vec2(0.0))) && all(lessThanEqual(uv, vec2(1.0)))) {
                g_combatMaskVis = textureLod(combatMask, uv, 0.0);
                g_combatMaskInside = true;
            }
        }
    }
    float v = channel == 1 ? g_combatMaskVis.x : (channel == 2 ? g_combatMaskVis.y : (channel == 3 ? g_combatMaskVis.z : g_combatMaskVis.w));
    return mix(1.0, v, combatFrame.y);
}

// How much of a combat light's ball is above a surface's horizon, as a share
// of its full irradiance (CombatLights.hpp CombatBallCosine, the same math):
// c = cos to the ball's centre, the ball of radius `core` `d` metres off.
// cos while the ball is wholly above the horizon, (1 + cos) / 2 at its surface
// and inside it, the horizon-clipped sphere light (Lagarde and de Rousiers
// 2014) between: flat ground far from a ball low over it takes the little
// the ball's top shows it, so the plain behind a burst or a fire is no band
// along the horizon.
float combatBallCosine(float c, float core, float d) {
    float s = d > core ? core / d : 1.0;
    float s2 = s * s;
    if (c * c > s2 || s2 <= 0.0) return max(c, 0.0);
    if (s2 >= 1.0) return 0.5 * (1.0 + c);
    float sinT = sqrt(max(1.0 - c * c, 1e-12));
    float x = sqrt(1.0 / s2 - 1.0);
    float y = clamp(-x * c / sinT, -1.0, 1.0);
    float sy = sinT * sqrt(max(1.0 - y * y, 0.0));
    float e = (c * acos(y) - x * sy) * s2 + atan(sy / x);
    return max(e, 0.0) / (3.14159265 * s2);
}

// Combat light at this fragment (CombatLights.hpp): the muzzle flashes,
// bursts, fires, tracers, flares and missile motors at the head of the tile's
// list, in linear HDR (outside the classic clamp): E = I * window / (1 + d^2 /
// r0^2), a ball of radius r0 that lights a surface by the part of it above
// that surface's horizon (combatBallCosine: it still reaches a surface it
// sits low over and wraps around one it is inside, a flash at the muzzle
// lighting the shooter's arm), the shadowed ones dimmed by the mask, and a
// flash highlight on glossy materials (g_combatSpec). Leaf cards take the
// crown's wrapped terminator, as for the sun. No combat light without the
// tiled list.
vec3 g_combatSpec = vec3(0.0);

vec3 combatLights(vec3 n, vec3 nLight, bool foliage, vec3 v) {
    g_combatSpec = vec3(0.0);
    vec3 sum = vec3(0.0);
    if (litLightCount.y < 0.5) return sum;
    float power = litMatSpecular.w;
    for (int k = 0; k < kMaxTileLights; ++k) {
        if (k >= g_combatHead) break;
        int at = (tileEntry(k) - 1) * kTexelsPerLight;
        vec4 pos = frameLights[at];
        vec3 radiance = frameLights[at + 1].rgb;
        vec4 ext = frameLights[at + 2]; // x window end (m), y shadow channel + 1
        vec3 toLight = pos.xyz - vWorldRel;
        float d2 = dot(toLight, toLight);
        float r2 = ext.x * ext.x;
        if (d2 >= r2) continue;
        float x = d2 / r2;
        float win = 1.0 - x * x;
        win *= win;
        float e = win / (1.0 + d2 / max(pos.w * pos.w, 1e-4));
        float invD = inversesqrt(max(d2, 1e-6));
        vec3 l = toLight * invD;
        float nl = foliage ? clamp((dot(nLight, l) + 0.5) / 1.5, 0.0, 1.0)
                           : combatBallCosine(dot(n, l), pos.w, d2 * invD);
        // (xp-nightlight) The ground's bounce: the ball's light on the ground
        // the surface faces, by the part of the ball above that ground.
        if (litXpNight.y > 0.0 && xpBounceSetup(foliage ? nLight : n)) {
            vec3 toG = pos.xyz - g_xpBounceFoot;
            float g2 = dot(toG, toG);
            if (g2 < r2) {
                float xg = g2 / r2;
                float wg = 1.0 - xg * xg;
                float invG = inversesqrt(max(g2, 1e-6));
                float eg = wg * wg / (1.0 + g2 / max(pos.w * pos.w, 1e-4)) *
                           combatBallCosine(toG.y * invG, pos.w, g2 * invG);
                // A quarter: the ground beside a burning object is partly in
                // that object's shadow, which nothing here knows (at full
                // strength a burning hull read pale, as if lit directly, and
                // at half it still lost its shape).
                sum += radiance * g_groundTint * (kXpCombatBounce * eg * g_xpBounceView);
            }
        }
        if (nl <= 0.0) continue;
        if (ext.y > 0.5) e *= combatShadowVis(int(ext.y + 0.5));
        sum += radiance * (e * nl);
        if (power > 0.0) {
            vec3 h = normalize(l + v);
            float p = max(power, 1.0);
            g_combatSpec += radiance * (e * pow(max(dot(n, h), 0.0), p) * ((p + 8.0) / 28.0) * smoothstep(0.0, 0.15, nl));
        }
    }
    return sum;
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

// (xp-nightlight) The rooms of a house, lit at night, on a static object
// (kind 3). Three kinds of surface take part:
//   - glass (SurfaceMaterials glass and window rules, surfClass.x 3) on a
//     wall glows where its texture is not light and not warm (the pane;
//     white or wooden frames stay);
//   - a texture whose name names a window or a window's black fill
//     (surfClass.w 1: okna, okno_ram, okenice, black_sum, ...; the stock
//     data paints most windows into the facade or fills them with a black
//     texture) glows on a wall only where it is dark and cool, or pure
//     black (xpFacadePane: the pane; plaster, beams and stone stay as they
//     are);
//   - an inside surface (surfClass.w 2: _in, stena_in, strop, podlaha, ...;
//     the stock houses have rooms behind their window holes) is lit by the
//     room's lamp: its own colour times that light, so the walls, ceiling
//     and floor seen through a window read as a lit room.
// A pane is lit by a hash of its building (its origin, hashed by the vertex
// shader into vLitMode 1 to 1.4), the side it faces and its storey, a room
// by its building and storey: about kXpWindowLitShare of them, most in warm lamp light,
// some in a softer warm, a few in a television's cool colour, at a level from
// the night's own ambient (the sky, moon and key light on an up-facing
// surface), so the metered night keeps it in proportion. Returns linear scene
// light before the exposure; `cover` is how far it makes a blending pane
// opaque.
const float kXpWindowLevel = 4.0;
const float kXpRoomLevel = 12.0;
const float kXpWindowLitShare = 0.4;
float g_xpGlowShare = 0.0;

uint xpHash(uint x) {
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}

// A painted window pane on a facade: dark and cool (the stock facades
// paint their panes a dark blue grey, a reflection of the sky, while dark
// beams, stains and stone are neutral or warm), or a pure black fill.
float xpFacadePane(vec3 albedo) {
    float lum = dot(albedo, kLuma);
    float mx = max(albedo.r, max(albedo.g, albedo.b));
    float cool = (albedo.b - albedo.r) / max(mx, 1e-3);
    float dark = 1.0 - smoothstep(0.10, 0.20, lum);
    return max(dark * smoothstep(0.03, 0.12, cool), 1.0 - smoothstep(0.002, 0.006, lum));
}

vec3 xpWindowGlow(vec3 albedo, vec3 n, out float cover) {
    cover = 0.0;
    if (litXpNight.x <= 0.0) return vec3(0.0);
    float nightRef = dot(litZenith.rgb + litMoon.rgb + litSun.rgb, kLuma);
    int objectKind = int(surfClass.y + 0.5);
    // A wall drawn without an object kind (a proxy's section, kind 0): the
    // debug map paints it orange; window-any (surfClass.w 3) counts it.
    if (objectKind == 0 && abs(n.y) < 0.6 && litXpNight.w > 0.5) return vec3(1.0, 0.45, 0.0) * (20.0 * nightRef);
    if (objectKind != 3 && !(objectKind == 0 && surfClass.w > 2.5)) return vec3(0.0);
    bool glass = int(surfClass.x + 0.5) == 3;
    bool room = surfClass.w > 1.5 && surfClass.w < 2.5;
    bool named = (surfClass.w > 0.5 && surfClass.w < 1.5) || surfClass.w > 2.5;
    float building = clamp((vLitMode - 1.0) / 0.4, 0.0, 1.0);
    uint side = room ? 7u : (abs(n.x) > abs(n.z) ? (n.x > 0.0 ? 0u : 1u) : (n.z > 0.0 ? 2u : 3u));
    // The storey (3 m from the ground under the fragment) joins the hash, so
    // the floors of one side are lit apart.
    uint storey = g_groundHeight < -1e5 ? 0u
                                        : uint(clamp(floor((vWorldRel.y + litCamPos.y - g_groundHeight + 0.3) / 3.0), 0.0, 15.0));
    uint h = xpHash(uint(building * 1048575.0) * 8u + side + 1u + storey * 0x9e3779b9u);
    float lit = float(h & 0xffffu) / 65535.0;
    // POSEIDON_XP_NIGHTDEBUG (litXpNight.w): every candidate surface in a
    // flat colour (rooms cyan, named windows magenta, glass yellow); every
    // other static surface as red = its building's hash, green = its side
    // is lit, blue = its albedo passes the facade pane test (xpFacadePane).
    if (litXpNight.w > 0.5) {
        if (room || named || glass)
            return (room ? vec3(0.0, 1.0, 1.0) : (named ? vec3(1.0, 0.0, 1.0) : vec3(1.0, 1.0, 0.0))) * (20.0 * nightRef);
        return vec3(building, lit < kXpWindowLitShare ? 1.0 : 0.0, xpFacadePane(albedo)) * (6.0 * nightRef);
    }
    if (!glass && !named && !room) return vec3(0.0);
    if (!room && abs(n.y) > 0.6) return vec3(0.0);
    if (lit >= kXpWindowLitShare) return vec3(0.0);
    float kind = float(h >> 16) / 65535.0;
    vec3 tint = kind < 0.7 ? vec3(1.0, 0.56, 0.26) : (kind < 0.92 ? vec3(1.0, 0.72, 0.46) : vec3(0.55, 0.7, 1.0));
    tint /= dot(tint, kLuma);
    float level = mix(0.55, 1.25, fract(kind * 13.37)) * (kind < 0.92 ? 1.0 : 0.45);
    if (room) return albedo * tint * (kXpRoomLevel * level * nightRef * litXpNight.x);
    float lum = dot(albedo, kLuma);
    float mx = max(albedo.r, max(albedo.g, albedo.b));
    float warm = (albedo.r - albedo.b) / max(mx, 1e-3);
    float pane = glass ? (1.0 - smoothstep(0.3, 0.6, lum)) * (1.0 - smoothstep(0.12, 0.3, warm)) : xpFacadePane(albedo);
    if (pane <= 0.0) return vec3(0.0);
    cover = pane;
    return tint * (kXpWindowLevel * level * pane * nightRef * litXpNight.x);
}

// (xp-nightlight) The same for a static object's pre-lit section (display
// colour `c`, texture colour `tex`), which has no world position: under
// window-any (surfClass.w 3) its dark, cool texels glow in a
// warm room light, lit or dark by a hash of the texture coordinates (about
// one window of a facade atlas per cell), in display terms. The debug map
// paints every such section orange.
vec3 xpPrelitWindow(vec3 c, vec3 tex) {
    if (litXpNight.w > 0.5) return mix(c, vec3(1.0, 0.45, 0.0), 0.7);
    if (surfClass.w < 2.5) return c;
    float pane = xpFacadePane(toLinear(tex));
    if (pane <= 0.0) return c;
    uvec2 cell = uvec2(ivec2(floor(vUV0 * 6.0)) + ivec2(4096));
    uint h = xpHash(cell.x * 0x9e3779b9u + cell.y + 17u);
    if (float(h & 0xffffu) / 65535.0 >= kXpWindowLitShare) return c;
    float kind = float(h >> 16) / 65535.0;
    vec3 tint = kind < 0.7 ? vec3(1.0, 0.62, 0.32) : (kind < 0.92 ? vec3(1.0, 0.78, 0.55) : vec3(0.6, 0.75, 1.0));
    float level = mix(0.18, 0.32, fract(kind * 13.37)) * (kind < 0.92 ? 1.0 : 0.6) * min(litXpNight.x, 2.0);
    return mix(c, tint * level, pane);
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
