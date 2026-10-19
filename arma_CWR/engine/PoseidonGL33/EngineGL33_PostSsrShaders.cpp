// Remastered lighting: the screen-space reflections' post shaders (feature
// switch ssr, INTEGRATION-PLAN.md step B6; the math they share with the
// engine and the tests is Graphics/Remaster/ScreenReflections.hpp, the passes
// EngineGL33_PostSsr.cpp). No GL code: the unit tests compile these sources,
// assembled by the kPostShaders table of EngineGL33_PostShaders.cpp.
//
//   psPostSsrTag        a constant where the stencil test passes: the tag
//                       readout adds each tag bit into an R8 target;
//   psPostSsrShare      the share of water and of wettable ground per cell of
//                       a small grid, read back for the frame gate;
//   psPostSsrTrace      one reflection ray per half-resolution texel over the
//                       half-resolution linear depth, its result a multiply
//                       and an add for the frame's linear colour, and the
//                       width of its cone at what it reflects (the spread);
//                       pastes the LitConstants block and the sky piece (the
//                       sea's own sky reflection, skyReflection);
//   psPostSsrBlur       the trace gathered sideways over each texel's spread,
//                       so a rough reflection has no hard outline;
//   psPostSsrComposite  the full-resolution frame times the multiply plus the
//                       add, taken from the four half-resolution texels around
//                       each pixel that reflect for the same surface at a
//                       similar depth.

#include <PoseidonGL33/EngineGL33_Lighting.hpp>

namespace Poseidon::gl33lighting
{

const char kPostSsrTagGLSL[] = R"(#version 330 core
uniform vec4 tagValue; // the tag bit (value / 255) the readout adds where the stencil test passes
out vec4 fragColor;
void main() {
    fragColor = tagValue;
}
)";

const char kPostSsrShareGLSL[] = R"(#version 330 core
// The share of each grid cell's pixels that show water, and the ground the
// rain wets (terrain and roads), from the tags (a covered pixel counts as
// neither). kSsrShareTaps x kSsrShareTaps taps per cell.
uniform sampler2D tagTex; // R8: the stencil tag / 255
uniform vec4 grid;        // xy frame size (pixels), zw cells
out vec4 fragColor;
const int kSsrShareTaps = 6;
void main() {
    ivec2 cell = ivec2(gl_FragCoord.xy);
    vec2 cellSize = grid.xy / grid.zw;
    ivec2 lim = ivec2(grid.xy) - 1;
    float water = 0.0;
    float ground = 0.0;
    for (int y = 0; y < kSsrShareTaps; ++y) {
        for (int x = 0; x < kSsrShareTaps; ++x) {
            vec2 p = (vec2(cell) + (vec2(x, y) + 0.5) / float(kSsrShareTaps)) * cellSize;
            int tg = int(texelFetch(tagTex, clamp(ivec2(p), ivec2(0), lim), 0).r * 255.0 + 0.5);
            if ((tg & 0x20) != 0) continue;
            int s = (tg >> 6) & 3;
            water += s == 2 ? 1.0 : 0.0;
            ground += (s == 1 || s == 3) ? 1.0 : 0.0;
        }
    }
    fragColor = vec4(water, ground, 0.0, 1.0) / vec4(float(kSsrShareTaps * kSsrShareTaps));
}
)";

// The trace's inputs. It reads the rig (LitConstants, pasted after this) and
// the sky piece's skyReflection, which also reads PSConstants.fogColor (here a
// plain uniform with the same value) and the prelude's colour helpers (copied
// below, the same text: a test compares them), and whose dome function, never
// called here, names tex0, vUV0, vWorldRel and g_groundTint.
const char kPostSsrTraceInputsGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D linDepthTex;   // unit 0: half-resolution view depth (m), 1e6 = sky
uniform sampler2D tagTex;        // unit 2: full-resolution stencil tags (R8: value / 255)
uniform sampler2D mipTex;        // unit 3: the resolved frame with mips (display), the reflections' blur
uniform sampler2D terrainHeight; // unit 4: r = terrain height (m)
uniform sampler2D depthTex;      // unit 5: the resolved depth (the ray's origin and the surface normal)
uniform sampler2D skyLut;        // unit 8: the sky table (skyReflection)
uniform sampler2D groundField;   // unit 12: the land's own colour per terrain vertex (the puddles piece's classes)
uniform vec4 projInfo;   // x = _33, y = _43 (viewZ = y / (ndcZ - x)), z = 1/_11, w = 1/_22
uniform vec4 depthMap;   // ndcZ = depth * x + y
uniform vec4 worldRect;  // the 3D viewport in uv
uniform vec4 screen;     // xy full-resolution size, zw half-resolution size
uniform mat3 w2v;        // world -> view
uniform mat3 v2w;        // view -> world
uniform vec4 camWorld;   // xyz camera position (world, m), w sea level (m)
uniform vec4 fogParam;   // x fog start (m), y 1 / fog range: the world's (BeginPass), as its vertex shader fogs
uniform vec4 fogColor;   // the world's fog colour (display), as PSConstants.fogColor
uniform vec4 ssrParams;  // x ssrWater, y ground wetness 0..1, z sea clock (s), w mip levels of mipTex
uniform vec4 ssrFrame;   // x march start turn (per frame under TAA), y 1 = the reflections alone (debug view), z far plane (m), w 1 = the sea program draws the sea
uniform vec4 ssrWetMode; // x 1 = the wet weather shades the wet ground (no darkening here; its puddles), y its puddle level
layout(location = 0) out vec4 fragColor;
layout(location = 1) out float fragSpread; // the rough blur's spread (half-resolution texels)

// What the sky piece's dome function names (it is never called here).
uniform sampler2D tex0;
uniform sampler2D skyDensity;
vec2 vUV0 = vec2(0.0);
vec3 vWorldRel = vec3(0.0);
vec3 g_groundTint = vec3(1.0);
// The lit sky clouds' density lookups (B10) live in the terrain and clouds
// piece, which the trace does not paste; the dome shading that calls them is
// unused here.
float skyCloudDensity(vec4 m) { return 0.0; }
vec2 skyDomeUv(vec3 w) { return vec2(0.0); }

const vec3 kLuma = vec3(0.2126, 0.7152, 0.0722);

vec3 toLinear(vec3 c) { return pow(max(c, vec3(0.0)), vec3(2.2)); }
vec3 toDisplay(vec3 c) { return pow(max(c, vec3(0.0)), vec3(1.0 / 2.2)); }

// Khronos PBR Neutral without its toe (the toe would black out classic-level
// nights). Identity below 0.8, soft shoulder, hue kept, highlights whiten.
vec3 tonemapNeutral(vec3 c) {
    float peak = max(c.r, max(c.g, c.b));
    if (peak < 0.8) return c;
    const float d = 0.2;
    float newPeak = 1.0 - d * d / (peak + d - 0.8);
    c *= newPeak / peak;
    float g = 1.0 - 1.0 / (0.15 * (peak - newPeak) + 1.0);
    return mix(c, vec3(newPeak), g);
}
)";

const char kPostSsrTraceMainGLSL[] = R"(
// Under the photoreal camera (litCamera.x) the frame holds pre-exposed light
// on the camera's encoding: the trace reads it decoded, and takes the sky
// (scene light) at the frame's pre-exposure, so its reflection is in the
// frame's own light.
vec3 frameLin(vec3 c) { return litCamera.x > 0.5 ? camDecode(c) : toLinear(c); }
vec3 skyLin(vec3 disp) { return toLinear(disp) * (litCamera.x > 0.5 ? litCamera.w : 1.0); }
// ---- The march (ScreenReflections.hpp; SsrMarchRow mirrors it) ----
const int kSsrSteps = 48;
const float kSsrStride = 1.5;
const float kSsrStartSkip = 1.5;
const int kSsrRefine = 5;
const float kSsrThickBase = 0.4;
const float kSsrThickPerM = 0.04;
const float kSsrBehindNear = 3.0;
const float kSsrUnderside = 0.35; // the hidden side of what a ray slipped behind, against its lit front
const float kSsrHiddenFrom = 1.0; // steps behind surfaces from which a sky end is trusted less ...
const float kSsrHiddenTo = 4.0;   // ... and not at all
const float kSsrSkyDepth = 100000.0;
const float kSsrSelfBase = 0.03;  // a candidate this close to the start's plane (m) ...
const float kSsrSelfPerM = 0.004; // ... plus this per metre of its depth is the ray's own surface
const float kSsrCoverSeeFrom = 0.04; // a covered panel this close in colour to what is next to it is see-through ...
const float kSsrCoverSeeTo = 0.12;   // ... and this far, opaque
const float kSsrCoverLod = 1.0;      // the mip level the two colours are compared at
const float kSsrStreakMaxPx = 72.0;  // a streak's half length at most (pixels)
const int kSsrStreakTaps = 12;       // its taps on each side at most
const float kSsrEdgeFade = 0.06;
const float kSsrWaterSwapMax = 0.95;
const float kSsrWaterRatioMax = 4.0;
const float kSsrSeaBlurLod = 3.0; // the sea's colour around a pixel: 8 x 8 pixels
const float kSsrBlurMaxSpread = 24.0;
const float kSsrWaterRipple = 0.35; // the ripples' slope (ScreenReflections.hpp)
const int kSsrSkySearch = 4;        // steps up the frame to its open sky (skyEstimate)
// Wet ground.
const float kSsrWetRoadDark = 0.8;
const float kSsrWetGroundDark = 0.8;
const float kSsrWetGrassDark = 0.92;
const float kSsrPuddleDark = 0.55;
const float kSsrPuddleRoad = 0.61;
const float kSsrPuddleGround = 0.67;
const float kSsrPuddleFadeStart = 150.0;
const float kSsrPuddleFadeEnd = 400.0;
// The sea program's mean sky weight (s_psWaterRemasterMain): its far fade.
const float kSeaFarSkyStart = 1000.0;
const float kSeaFarSkyEnd = 3000.0;
const float kSeaFarSkyFloor = 0.5;

// Half-resolution texel position <-> uv: texel i covers full-resolution
// pixels 2i and 2i + 1, as the linear depth lays them out.
vec2 uvOfHalf(vec2 s) { return s * 2.0 / screen.xy; }
vec2 halfOfUv(vec2 uv) { return uv * screen.xy * 0.5; }
vec2 uvOfView(vec3 p) {
    vec2 ndc = vec2(p.x / (projInfo.z * p.z), p.y / (projInfo.w * p.z));
    return worldRect.xy + (ndc * 0.5 + 0.5) * worldRect.zw;
}
vec3 viewOfUv(vec2 uv, float z) {
    vec2 ndc = (uv - worldRect.xy) / worldRect.zw * 2.0 - 1.0;
    return vec3(ndc.x * z * projInfo.z, ndc.y * z * projInfo.w, z);
}
float sceneZ(vec2 s) { return texelFetch(linDepthTex, clamp(ivec2(s), ivec2(0), ivec2(screen.zw) - 1), 0).r; }
int tagAt(vec2 uv) {
    ivec2 p = clamp(ivec2(uv * screen.xy), ivec2(0), ivec2(screen.xy) - 1);
    return int(texelFetch(tagTex, p, 0).r * 255.0 + 0.5);
}
bool inFrame(vec2 uv) {
    return all(greaterThan(uv, worldRect.xy + 0.002)) && all(lessThan(uv, worldRect.xy + worldRect.zw - 0.002));
}
float edgeFade(vec2 uv) {
    vec2 e = min(uv - worldRect.xy, worldRect.xy + worldRect.zw - uv) / worldRect.zw;
    return smoothstep(0.0, kSsrEdgeFade, min(e.x, e.y));
}

// The binary refine between two march positions: where the ray crosses the
// surface in front of it.
float ssrRefine(float a, float b, vec2 s0, vec2 dS, float k0, float k1) {
    for (int j = 0; j < kSsrRefine; ++j) {
        float m = 0.5 * (a + b);
        if (1.0 / mix(k0, k1, m) >= sceneZ(s0 + dS * m)) b = m; else a = m;
    }
    return b;
}

// One ray from the view position p0 along the unit view direction r, linear in
// the half-resolution screen with a perspective-correct depth. Returns
// 0 left the frame, 1 met a surface (at hitS, travel metres away), 2 ended on
// open sky inside the frame, 3 slipped behind a surface right after its start
// (hitS is where, travel how far: the hidden side of that surface is what the
// ray would see). `hidden` counts the steps the ray spent behind a surface: what
// it found after them may be what that surface hid. A surface the ray meets on
// water (water never reflects itself) does not stop it, nor does its own
// surface: `plane` (w 1) is the start's plane in view space, and a candidate
// that lies on it is the half-resolution depth putting that surface in front
// of a grazing ray (SsrSelfTolerance). A covered pixel (a blended panel: glass,
// a canvas, a bush card) does not stop it either; the first one it meets is
// `cover` (xy where, z how far, w 1, or 2 once the march reached an uncovered
// texel after it, `afterS`): the caller decides how much of what lies beyond
// that panel hides (coverOpacity); w 3 when that texel stands in front of the
// ray (the panel is set in something solid: a window in a canvas).
int ssrMarch(vec3 p0, vec3 r, vec4 plane, float jitter, out vec2 hitS, out float travel, out float hidden,
             out vec4 cover, out vec2 afterS) {
    travel = 0.0;
    hidden = 0.0;
    cover = vec4(0.0);
    afterS = vec2(0.0);
    float len = ssrFrame.z * 0.98;
    if (r.z < 0.0) len = min(len, (p0.z - 0.3) / -r.z);
    vec3 p1 = p0 + r * len;
    vec2 s0 = halfOfUv(uvOfView(p0));
    vec2 s1 = halfOfUv(uvOfView(p1));
    float k0 = 1.0 / p0.z, k1 = 1.0 / p1.z;
    vec2 lo = halfOfUv(worldRect.xy) + 0.5;
    vec2 hi = halfOfUv(worldRect.xy + worldRect.zw) - 0.5;
    vec2 dS = s1 - s0;
    float tcx = 1.0, tcy = 1.0;
    if (dS.x > 1e-4) tcx = (hi.x - s0.x) / dS.x; else if (dS.x < -1e-4) tcx = (lo.x - s0.x) / dS.x;
    if (dS.y > 1e-4) tcy = (hi.y - s0.y) / dS.y; else if (dS.y < -1e-4) tcy = (lo.y - s0.y) / dS.y;
    float tc = clamp(min(tcx, tcy), 0.0, 1.0);
    float span = max(abs(dS.x), abs(dS.y));
    int n = int(clamp(span * tc / kSsrStride, 4.0, float(kSsrSteps)));
    float dt = tc / float(n);
    float startT = kSsrStartSkip / max(span, 1e-3);
    float tPrev = 0.0;
    float zPrev = p0.z;
    bool behind = false;
    vec2 behindS = s0;
    float behindTr = 0.0;
    for (int i = 0; i < kSsrSteps; ++i) {
        if (i >= n) break;
        float t = min(dt * (float(i) + jitter) + startT, tc);
        vec2 s = s0 + dS * t;
        float z = 1.0 / mix(k0, k1, t);
        float sz = sceneZ(s);
        float thick = kSsrThickBase + kSsrThickPerM * sz;
        bool cand = max(zPrev, z) >= sz && min(zPrev, z) <= sz + thick;
        int tg = (cand || cover.w == 1.0) ? tagAt(uvOfHalf(s)) : 0;
        bool covered = (tg & 0x20) != 0;
        if (cover.w == 1.0 && !covered) { afterS = s; cover.w = sz < kSsrSkyDepth && min(zPrev, z) > sz + thick ? 3.0 : 2.0; }
        float tr = abs(r.z) > 1e-3 ? abs((z - p0.z) / r.z) : len * t;
        if (cand) {
            if ((tg & 0xC0) == 0x80) cand = false;
            else if (plane.w > 0.5 && dot(plane.xyz, viewOfUv(uvOfHalf(s), sz) - p0) < kSsrSelfBase + kSsrSelfPerM * sz) cand = false;
            else if (covered) {
                if (cover.w == 0.0) {
                    float tk = ssrRefine(tPrev, t, s0, dS, k0, k1);
                    float zk = 1.0 / mix(k0, k1, tk);
                    cover = vec4(s0 + dS * tk, abs(r.z) > 1e-3 ? abs((zk - p0.z) / r.z) : len * tk, 1.0);
                }
                cand = false;
            }
        }
        bool occluded = !cand && sz < kSsrSkyDepth && min(zPrev, z) > sz + thick;
        hidden += occluded ? 1.0 : 0.0;
        if (occluded && !behind && tr < kSsrBehindNear) {
            behind = true;
            behindS = s;
            behindTr = tr;
        }
        if (cand) {
            float b = ssrRefine(tPrev, t, s0, dS, k0, k1);
            float zb = 1.0 / mix(k0, k1, b);
            travel = abs(r.z) > 1e-3 ? abs((zb - p0.z) / r.z) : len * b;
            hitS = s0 + dS * b;
            return 1;
        }
        tPrev = t;
        zPrev = z;
    }
    hitS = s0 + dS * tc;
    if (behind) {
        hitS = behindS;
        travel = behindTr;
        return 3;
    }
    if (sceneZ(hitS) >= kSsrSkyDepth) { travel = 1e9; return 2; }
    return 0;
}

float hash12(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}
vec2 hash22(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * vec3(0.1031, 0.1030, 0.0973));
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.xx + p3.yz) * p3.zy);
}
float vnoise(vec2 p) {
    vec2 i = floor(p), f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash12(i), hash12(i + vec2(1, 0)), u.x), mix(hash12(i + vec2(0, 1)), hash12(i + vec2(1, 1)), u.x), u.y);
}
float gnoise(vec2 p) {
    vec2 i = floor(p), f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    float a = dot(hash22(i) * 2.0 - 1.0, f);
    float b = dot(hash22(i + vec2(1, 0)) * 2.0 - 1.0, f - vec2(1, 0));
    float c = dot(hash22(i + vec2(0, 1)) * 2.0 - 1.0, f - vec2(0, 1));
    float d = dot(hash22(i + vec2(1, 1)) * 2.0 - 1.0, f - vec2(1, 1));
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

// Wind ripples on the reflection: four octaves of drifting gradient noise, each
// rotated so no grid shows, their slope kSsrWaterRipple of the research's
// (its calm sea: the sea program's own waves already break the reflection up).
// Octaves shorter than the pixel's footprint leave the normal and blur the
// reflection instead (lostVar).
vec3 rippleNormal(vec2 xz, float t, float foot, out float lostVar) {
    const float L[4] = float[](11.0, 4.3, 1.7, 0.65);
    const float S[4] = float[](0.07, 0.065, 0.06, 0.05);
    const float A[4] = float[](0.3, 1.9, 4.1, 5.6);
    vec2 g = vec2(0.0);
    lostVar = 0.0;
    for (int i = 0; i < 4; ++i) {
        float keep = clamp(L[i] / (3.0 * foot) - 0.5, 0.0, 1.0);
        float s = S[i] * kSsrWaterRipple;
        lostVar += 0.5 * s * s * (1.0 - keep * keep);
        if (keep <= 0.0) continue;
        float ca = cos(A[i]), sa = sin(A[i]);
        mat2 rot = mat2(ca, sa, -sa, ca);
        float speed = sqrt(9.81 * L[i] / 6.2831853); // deep-water phase speed (m/s)
        vec2 q = rot * xz / L[i] + vec2(t * speed / L[i], 0.0);
        const float e = 0.08;
        float h0 = gnoise(q);
        vec2 grad = vec2(gnoise(q + vec2(e, 0.0)) - h0, gnoise(q + vec2(0.0, e)) - h0) / e;
        g += (transpose(rot) * grad) * (s * keep * 0.8);
    }
    return normalize(vec3(-g.x, 1.0, -g.y));
}

// The view depth (m) of a full-resolution pixel, 1e6 for the sky.
float fullZ(ivec2 q) {
    float d = texelFetch(depthTex, clamp(q, ivec2(0), ivec2(screen.xy) - 1), 0).r;
    return d >= 0.999999 ? 1e6 : projInfo.y / ((d * depthMap.x + depthMap.y) - projInfo.x);
}
vec3 viewOfPixel(ivec2 q) { return viewOfUv((vec2(q) + 0.5) / screen.xy, fullZ(q)); }

// The view-space normal from the full-resolution depth (the half-resolution
// one alternates its nearest and farthest samples): on each axis the neighbour
// with the smaller step, so silhouettes do not bend it.
vec3 depthNormal(ivec2 q, vec3 p) {
    vec3 pr = viewOfPixel(q + ivec2(1, 0));
    vec3 pl = viewOfPixel(q - ivec2(1, 0));
    vec3 pu = viewOfPixel(q + ivec2(0, 1));
    vec3 pd = viewOfPixel(q - ivec2(0, 1));
    vec3 dx = abs(pr.z - p.z) < abs(p.z - pl.z) ? pr - p : p - pl;
    vec3 dy = abs(pu.z - p.z) < abs(p.z - pd.z) ? pu - p : p - pd;
    vec3 n = normalize(cross(dx, dy));
    if (dot(n, p) > 0.0) n = -n;
    return n;
}

// Taps along the screen vertical (a reflection in wavy or wet ground stretches
// toward the viewer) over pxV pixels each way (kSsrStreakMaxPx at most), the
// mip level from the horizontal spread (pixels). The taps stand no farther
// apart than that level's footprint (SsrStreakTaps): more taps where the
// footprint is small (water at a grazing angle), a coarser level past
// kSsrStreakTaps a side. No gaps between them, so no ladder of copies.
vec3 streakSample(vec2 uv, float pxV, float pxH) {
    float maxLod = max(ssrParams.w - 1.0, 0.0);
    float spanPx = min(pxV, kSsrStreakMaxPx);
    float n = clamp(ceil(spanPx / max(pxH, 1.0)), 1.0, float(kSsrStreakTaps));
    float stepPx = spanPx / n;
    float lod = clamp(log2(max(max(pxH, stepPx), 1.0)), 0.0, maxLod);
    vec2 lo = worldRect.xy;
    vec2 hi = worldRect.xy + worldRect.zw;
    vec3 acc = frameLin(textureLod(mipTex, uv, lod).rgb);
    float wsum = 1.0;
    for (int k = 1; k <= kSsrStreakTaps; ++k) {
        if (float(k) > n) break;
        float x = float(k) / n;
        float w = exp(-2.0 * x * x);
        vec2 o = vec2(0.0, float(k) * stepPx / screen.y);
        acc += (frameLin(textureLod(mipTex, clamp(uv + o, lo, hi), lod).rgb) +
                frameLin(textureLod(mipTex, clamp(uv - o, lo, hi), lod).rgb)) * w;
        wsum += 2.0 * w;
    }
    return acc / wsum;
}

// How much of what the ray found beyond the covered panel it met first hides
// (SsrCoverOpacity): none without a panel, or when the ray slipped behind a
// surface before it; all when the march never left the panel, or when the
// first uncovered texel after it stands in front of the ray (the panel is set
// in something solid, a window in a vehicle's canvas: no sky through it);
// otherwise by how far the panel's colour is from that texel, the background
// next to it (a transparent part of a card shows that background: the ray
// passes, no box under a bush; a sign or a card's leaves do not).
float coverOpacity(vec4 cover, vec2 afterS, int kind, float travel) {
    if (cover.w < 0.5 || (kind == 3 && cover.z > travel)) return 0.0;
    if (cover.w < 1.5 || cover.w > 2.5) return 1.0;
    vec3 d = abs(textureLod(mipTex, uvOfHalf(cover.xy), kSsrCoverLod).rgb -
                 textureLod(mipTex, uvOfHalf(afterS), kSsrCoverLod).rgb);
    return smoothstep(kSsrCoverSeeFrom, kSsrCoverSeeTo, max(d.r, max(d.g, d.b)));
}

// The sky a ray that left the frame upward sees: the open sky of the frame in
// its direction (at the frame's edge when it points out of the frame, or the
// first open sky above where it points when a hill stands there: the sky a ray
// that ends on open sky in the frame takes, so the two agree), else the sky
// table's sky (or the fog colour toward the sun without it).
vec3 skyEstimate(vec3 dirW, float lod) {
    vec3 up = normalize(vec3(dirW.x, max(dirW.y, 0.01), dirW.z));
    vec3 s = skyLin(skyReflection(up));
    vec3 dV = w2v * dirW;
    if (dV.z > 0.02) {
        vec2 ui = uvOfView(dV);
        vec2 uc = clamp(ui, worldRect.xy + 0.004, worldRect.xy + worldRect.zw - 0.004);
        float top = worldRect.y + worldRect.w - 0.004;
        for (int k = 0; k <= kSsrSkySearch; ++k) {
            vec2 uk = vec2(uc.x, mix(uc.y, top, float(k) / float(kSsrSkySearch)));
            if (sceneZ(halfOfUv(uk)) >= kSsrSkyDepth) {
                float pull = length((ui - uk) / worldRect.zw);
                s = mix(s, frameLin(textureLod(mipTex, uk, lod).rgb), exp(-1.5 * pull));
                break;
            }
        }
    }
    return s;
}

// The rough blur's spread (SsrBlurSpread): half the horizontal spread of the
// reflection in full-resolution pixels, in half-resolution texels.
float ssrBlurSpread(float pxH) { return clamp(0.5 * pxH, 0.0, kSsrBlurMaxSpread); }

// Schlick with a roughness cap (rough surfaces never reach mirror reflectance
// at grazing).
float fresnelRough(float nv, float f0, float rough) {
    return f0 + (max(1.0 - rough, f0) - f0) * pow(1.0 - clamp(nv, 0.0, 1.0), 5.0);
}

// The result, and the surface it is for (1 terrain, 2 water, 3 road), packed as
// a = m + 4 surface: on the ground the frame's linear colour times m plus add,
// on water times add (a multiplier per channel).
vec4 packResult(vec3 add, float m, int surface) {
    return vec4(add, clamp(m, 0.0, 3.99) + 4.0 * float(surface));
}

// What water at relW (world, from the camera; view depth z) shows of what the
// ray met at hitS, travel metres away: the mirror image moved by the ripples
// (the rippled direction rP re-projected at that distance, kept where it lands
// on something that is not water and not in front of the sea), stretched by the
// ripple slope finer than a pixel (sigma, rad). conf is its edge fade, pxH its
// horizontal spread (pixels).
vec3 waterImage(vec2 hitS, float travel, vec3 relW, vec3 rP, float z, float sigma, float vy, float pixAngle,
                out float conf, out float pxH) {
    vec2 uvH = uvOfHalf(hitS);
    vec2 uvP = uvH;
    vec3 hv = w2v * (relW + rP * travel);
    if (hv.z > 0.1) uvP = uvOfView(hv);
    int tp = tagAt(uvP);
    bool okP = inFrame(uvP) && (tp & 0xC0) != 0x80 && sceneZ(halfOfUv(uvP)) > z * 0.98;
    vec2 uvS = okP ? uvP : uvH;
    float hz = max(sceneZ(halfOfUv(uvS)), 0.1);
    float px = sigma * travel / (hz * pixAngle);
    pxH = px * sqrt(max(vy, 0.02)) * 2.0;
    conf = edgeFade(uvS);
    return streakSample(uvS, px, pxH);
}

void main() {
    ivec2 ij = ivec2(gl_FragCoord.xy);
    fragSpread = 0.0;
    // The surface this texel reflects for: its four pixels' tags, water before
    // roads before terrain; a covered pixel does not count.
    ivec2 lim = ivec2(screen.xy) - 1;
    int surface = 0;
    int rank = 0;
    ivec2 pick = min(ij * 2, lim);
    for (int k = 0; k < 4; ++k) {
        ivec2 q = min(ij * 2 + ivec2(k & 1, k >> 1), lim);
        int tg = int(texelFetch(tagTex, q, 0).r * 255.0 + 0.5);
        if ((tg & 0x20) != 0) continue;
        int s = (tg >> 6) & 3;
        int r = s == 2 ? 3 : (s == 3 ? 2 : s);
        if (r > rank) { rank = r; surface = s; pick = q; }
    }
    float wet = clamp(ssrParams.y, 0.0, 1.0);
    // The wet weather (wetWeather) shades the wet ground itself: its puddles
    // can outlast the wet surfaces.
    bool wetWeather = ssrWetMode.x > 0.5;
    bool water = surface == 2 && ssrFrame.w > 0.5;
    bool ground = (surface == 1 || surface == 3) && (wet > 0.0 || (wetWeather && ssrWetMode.y > 0.0));
    // The ray starts on that pixel of the surface, at its full-resolution depth.
    float z = fullZ(pick);
    bool reflOnly = ssrFrame.y > 0.5;
    if (!(water || ground) || z >= kSsrSkyDepth) {
        fragColor = packResult(vec3(surface == 2 && !reflOnly ? 1.0 : 0.0), reflOnly ? 0.0 : 1.0, surface);
        return;
    }
    vec2 uvC = (vec2(pick) + 0.5) / screen.xy;
    vec3 p = viewOfUv(uvC, z);
    vec3 relW = v2w * p;
    vec3 posW = relW + camWorld.xyz;
    vec3 vW = normalize(-relW);
    float dist = length(relW);
    float pixAngle = 2.0 * projInfo.w / screen.y; // radians per full-resolution pixel
    // The world vertex shader's fog factor: a fully fogged pixel is the fog
    // colour, and the reflection leaves it as it is.
    float fogT = clamp(1.0 - (dist - fogParam.x) * fogParam.y, 0.0, 1.0);
    float jitter = fract(fract(52.9829189 * fract(dot(vec2(ij), vec2(0.06711056, 0.00583715)))) + ssrFrame.x);
    float maxLod = max(ssrParams.w - 1.0, 0.0);
    vec3 c0 = textureLod(mipTex, uvC, 0.0).rgb;
    vec2 hitS;
    float travel;
    vec3 add = vec3(0.0);
    float m = 1.0;
    vec3 refl = vec3(0.0);

    if (water) {
        // One mirror trace with the flat sea, then the ripples move the mirror
        // image: the rippled direction is re-projected at the traced distance
        // (no per-pixel re-trace, so no holes), and the ripple slope finer
        // than a pixel stretches it vertically.
        float foot = dist * pixAngle / sqrt(max(vW.y, 0.01));
        float lostVar;
        vec3 nWave = rippleNormal(posW.xz, ssrParams.z, foot, lostVar);
        vec3 rM = vec3(-vW.x, max(vW.y, 0.002), -vW.z);
        float hidden;
        vec4 cover;
        vec2 afterS;
        // No plane test: a ray from water never stops on water.
        int kind = ssrMarch(p, normalize(w2v * rM), vec4(0.0), jitter, hitS, travel, hidden, cover, afterS);
        vec3 rP = reflect(-vW, nWave);
        rP.y = max(rP.y, 0.35 * rM.y + 0.0005);
        rP = normalize(rP);
        float sigma = 2.0 * sqrt(lostVar) + 0.003; // angular spread (rad)
        vec3 R = vec3(0.0);
        float conf = 0.0;
        float pxH = 0.0;
        if (kind == 1) {
            R = waterImage(hitS, travel, relW, rP, z, sigma, vW.y, pixAngle, conf, pxH);
        } else if (kind == 3) {
            // Under a hull: the water there sees the hull's unlit underside.
            R = frameLin(textureLod(mipTex, uvOfHalf(hitS), 1.0).rgb) * kSsrUnderside;
            conf = 1.0 - smoothstep(0.5 * kSsrBehindNear, kSsrBehindNear, travel);
        }
        // A covered panel the ray met first (a boat's canvas or glass) lies
        // over what it found beyond, as much as it hides.
        float a = coverOpacity(cover, afterS, kind, travel);
        if (a > 0.0) {
            float confK, pxK;
            vec3 RK = waterImage(cover.xy, cover.z, relW, rP, z, sigma, vW.y, pixAngle, confK, pxK);
            float wK = a * confK;
            float c = wK + (1.0 - wK) * conf;
            R = (wK * RK + (1.0 - wK) * conf * R) / max(c, 1e-4);
            conf = c;
            pxH = mix(pxH, pxK, wK);
        }
        fragSpread = ssrBlurSpread(pxH);
        // The sea program's mean sky weight and its sky (s_psWaterRemasterMain)
        // here, and the share of the sea's light that sky gives around the
        // pixel (its blurred colour): the light the reflection swaps for its own.
        float Ff = 0.02 + 0.98 * pow(1.0 - clamp(vW.y, 0.0, 1.0), 5.0);
        float skyMean = litWater0.z * mix(kSeaFarSkyFloor, 1.0, 1.0 - smoothstep(kSeaFarSkyStart, kSeaFarSkyEnd, dist));
        vec3 rS = normalize(vec3(rP.x, max(rP.y, 0.01), rP.z));
        vec3 S = skyLin(mix(skyReflection(rS), fogColor.rgb, litWater0.w)) * mix(1.0, 0.82, smoothstep(0.15, 0.9, rS.y));
        float seaLum = dot(frameLin(textureLod(mipTex, uvC, kSsrSeaBlurLod).rgb), kLuma);
        float share = clamp(skyMean * Ff * dot(S, kLuma) / max(seaLum, 1e-4), 0.0, 1.0);
        // Shallow water shows its floor, not a mirror.
        float depthFade = 1.0;
        if (litTerrain.x > 0.5) {
            float gh = textureLod(terrainHeight, (posW.xz * litTerrain.y + 0.5) * litTerrain.z, 0.0).r;
            depthFade = smoothstep(0.0, 1.2, camWorld.w - gh);
        }
        // SsrWaterComposite: lin' = lin (keep + swap R / S), per channel.
        float fade = clamp(conf * fogT * depthFade, 0.0, 1.0);
        float x = min(share * max(ssrParams.x, 0.0), kSsrWaterSwapMax);
        vec3 ratio = clamp(R / max(S, vec3(1e-4)), 0.0, kSsrWaterRatioMax);
        add = vec3(1.0 - fade * x) + fade * x * ratio; // the water's multiplier
        m = 1.0;
        refl = R * conf;
    } else {
        vec3 nV = depthNormal(pick, p); // view space: the plane the ray starts on
        vec3 nD = normalize(v2w * nV);
        bool road = surface == 3;
        // Puddles: value noise with the fine octaves faded below the pixel
        // footprint, only on level ground. Roads pool water, bare ground a
        // little, grass none.
        float footT = dist * pixAngle / max(vW.y, 0.05);
        float o2 = mix(0.5, vnoise(posW.xz / 1.3 + 17.0), clamp(1.3 / (3.0 * footT), 0.0, 1.0));
        float o3 = mix(0.5, vnoise(posW.xz / 0.45 + 41.0), clamp(0.45 / (3.0 * footT), 0.0, 1.0));
        float pn = vnoise(posW.xz / 3.5) * 0.55 + o2 * 0.3 + o3 * 0.15;
        // Far away puddles shrink below the pixel and the ground is one wet sheen.
        float near = 1.0 - smoothstep(kSsrPuddleFadeStart, kSsrPuddleFadeEnd, dist);
        float flatness = smoothstep(0.85, 0.98, nD.y);
        // The land's grass share from the ground field (groundFieldAt, the
        // forward pass's own classes): continuous across the landscape's
        // squares, where the frame's colour jumped at every square whose
        // texture differed a little from its neighbour's. The frame's colour
        // only while the field is not built.
        vec4 field = road ? vec4(0.0) : groundFieldAt(posW.xz);
        bool fieldOn = field.a > 0.5;
        float grass = road ? 0.0 : (fieldOn ? wetVegetation(field.rgb) : smoothstep(0.01, 0.05, c0.g - max(c0.r, c0.b) + 0.01));
        float th = road ? kSsrPuddleRoad : kSsrPuddleGround;
        float puddle = wet * flatness * smoothstep(th, th + 0.035, pn + 0.08 * (wet - 1.0)) * (1.0 - grass) * near;
        if (wetWeather) {
            // The wet weather's puddles (wetPuddle, the forward pass's own).
            // Without the field, a grass test that holds under warm lamps,
            // which turn lit grass from green to amber: green near red and
            // well above blue.
            if (!road && !fieldOn) {
                float gr = c0.g / max(c0.r, 1e-3);
                float gb = c0.g / max(c0.b, 1e-3);
                grass = max(grass, smoothstep(0.84, 0.92, gr) * smoothstep(1.1, 1.3, gb));
            }
            puddle = wetPuddle(posW, footT, road ? 1.0 : 0.0, grass, ssrWetMode.y).x;
        }
        vec3 nW = normalize(mix(nD, vec3(0.0, 1.0, 0.0), mix(0.6, 1.0, puddle)));
        // Rain rings on the puddles.
        float rip = 0.03 * puddle * (vnoise(posW.xz * 4.0 + ssrParams.z * 1.3) - 0.5);
        nW = normalize(nW + vec3(rip, 0.0, -rip));
        float rough = mix(road ? 0.30 : mix(0.45, 0.75, grass), 0.02, puddle);
        float cone = mix(road ? 0.10 : 0.22, 0.008, puddle);
        // With the wet weather on, only its puddles mirror: the wet ground
        // around them is dark and rough (the forward pass gives it at most a
        // dim, broad glint near a light), so a road or a square never shows a
        // reflected lamp, wall or sky outside its standing water.
        float weight = wetWeather ? puddle : mix(road ? 0.65 * wet : 0.5 * wet * (1.0 - 0.85 * grass), 1.0, puddle);
        vec3 rW = reflect(-vW, nW);
        if (rW.y < 0.002) rW = normalize(vec3(rW.x, 0.002, rW.z));
        float hidden;
        vec4 cover;
        vec2 afterS;
        int kind = ssrMarch(p, normalize(w2v * rW), vec4(nV, 1.0), jitter, hitS, travel, hidden, cover, afterS);
        // Sky found after a stretch behind a surface may be what that surface
        // hid (a soldier in front of a tent): trusted less, so no pale column.
        float trust = 1.0 - smoothstep(kSsrHiddenFrom, kSsrHiddenTo, hidden);
        vec3 R = vec3(0.0);
        float conf = 0.0;
        // The reflection's horizontal spread (pixels): the cone's width at what
        // the ray met, or its angle alone for the sky and a ray out of the frame.
        float pxH = 0.7 * cone / pixAngle;
        if (kind == 1 || kind == 2) {
            vec2 uvH = uvOfHalf(hitS);
            conf = edgeFade(uvH) * (kind == 2 ? trust : 1.0);
            float hz = kind == 2 ? 1.0 : max(sceneZ(hitS), 0.1);
            float px = kind == 2 ? cone / pixAngle : cone * travel / (hz * pixAngle);
            R = streakSample(uvH, px * 2.0, px * 0.7);
            pxH = px * 0.7;
        } else if (kind == 3) {
            // Under a vehicle, behind a kerb: the hidden, unlit side of what the
            // ray went behind (taking the ground's own colour drew pale boxes),
            // fading out where that information ends.
            R = frameLin(textureLod(mipTex, uvOfHalf(hitS), 1.0).rgb) * kSsrUnderside;
            conf = 1.0 - smoothstep(0.5 * kSsrBehindNear, kSsrBehindNear, travel);
            pxH = 0.7 * cone * travel / (max(sceneZ(hitS), 0.1) * pixAngle);
        }
        // Where the reflection is unknown (the ray left the frame, or near its
        // edges) only a ray going up falls back to the sky; the others, and a
        // ray behind a surface, give no reflection, so no pale band at the
        // frame's sides.
        float upward = smoothstep(0.05, 0.25, rW.y) * trust * (kind == 3 ? 0.0 : 1.0);
        vec3 sky = skyEstimate(rW, clamp(log2(max(cone / pixAngle, 1.0)) + 1.0, 0.0, maxLod));
        float use = conf + (1.0 - conf) * upward;
        vec3 Rmix = conf * R + (1.0 - conf) * upward * sky;
        // A covered panel the ray met first (a vehicle's canvas, glass, a bush
        // card) lies over what it found beyond, as much as it hides.
        float a = coverOpacity(cover, afterS, kind, travel);
        if (a > 0.0) {
            vec2 uvK = uvOfHalf(cover.xy);
            float wK = a * edgeFade(uvK);
            float pxK = cone * cover.z / (max(sceneZ(cover.xy), 0.1) * pixAngle);
            Rmix = mix(Rmix, streakSample(uvK, pxK * 2.0, pxK * 0.7), wK);
            use = mix(use, 1.0, wK);
            pxH = mix(pxH, pxK * 0.7, wK);
        }
        fragSpread = ssrBlurSpread(pxH);
        float F = fresnelRough(dot(nW, vW), 0.02, rough);
        float specW = F * weight;
        // Wet ground darkens (water fills the pores); a puddle is a dark mirror.
        // With the wet weather on the forward pass has darkened both already.
        float darken = mix(1.0, road ? kSsrWetRoadDark : mix(kSsrWetGroundDark, kSsrWetGrassDark, grass), wet);
        float base = wetWeather ? 1.0 : darken * mix(1.0, kSsrPuddleDark, puddle);
        m = 1.0 - fogT + fogT * base * (1.0 - specW * use);
        add = fogT * specW * Rmix;
        refl = use > 0.0 ? Rmix / use : vec3(0.0);
    }
    if (reflOnly) {
        fragColor = packResult(refl, 0.0, surface);
        return;
    }
    fragColor = packResult(add, m, surface);
}
)";

const char kPostSsrBlurGLSL[] = R"(#version 330 core
// The rough blur (ScreenReflections.hpp; SsrBlurRow mirrors it): a horizontal
// gather over each texel's spread, on the same surface at a similar depth,
// from taps whose own spread is as wide (a puddle's sharp mirror does not
// spill into the rough road around it). A tap per texel of the spread
// (SsrBlurTaps), so the gather never steps over texels.
uniform sampler2D traceTex;    // unit 0: half res: rgb add (linear), a = m + 4 surface
uniform sampler2D spreadTex;   // unit 2: half res: the spread (half-resolution texels)
uniform sampler2D linDepthTex; // unit 3: half-resolution view depth (m)
uniform vec4 screen;           // xy full-resolution size, zw half-resolution size
out vec4 fragColor;
const int kSsrBlurMaxTaps = 24;
const float kSsrBlurMaxSpread = 24.0;
const float kSsrBlurMinSpread = 0.75;
float blurDepthWeight(float tz, float z) {
    float dz = (tz - z) / (0.03 * z + 0.1);
    return 1.0 / (1.0 + dz * dz);
}
void main() {
    ivec2 ij = ivec2(gl_FragCoord.xy);
    vec4 c = texelFetch(traceTex, ij, 0);
    int s = int(floor(c.a * 0.25));
    float r = min(texelFetch(spreadTex, ij, 0).r, kSsrBlurMaxSpread);
    if (s == 0 || r < kSsrBlurMinSpread) {
        fragColor = c;
        return;
    }
    float z = texelFetch(linDepthTex, ij, 0).r;
    vec3 add = c.rgb;
    float m = c.a - 4.0 * float(s);
    float wsum = 1.0;
    int last = int(screen.z) - 1;
    float n = clamp(ceil(r), 1.0, float(kSsrBlurMaxTaps));
    for (int k = 1; k <= kSsrBlurMaxTaps; ++k) {
        if (float(k) > n) break;
        float o = r * float(k) / n;
        float x = float(k) / n;
        float wk = exp(-2.0 * x * x);
        for (int side = -1; side <= 1; side += 2) {
            ivec2 q = ivec2(clamp(ij.x + int(floor(float(side) * o + 0.5)), 0, last), ij.y);
            vec4 t = texelFetch(traceTex, q, 0);
            if (int(floor(t.a * 0.25)) != s) continue;
            float ts = clamp(texelFetch(spreadTex, q, 0).r / r, 0.0, 1.0);
            float w = wk * blurDepthWeight(texelFetch(linDepthTex, q, 0).r, z) * ts * ts;
            add += t.rgb * w;
            m += (t.a - 4.0 * float(s)) * w;
            wsum += w;
        }
    }
    fragColor = vec4(add / wsum, m / wsum + 4.0 * float(s));
}
)";

// The composite: its inputs, then the shared depth reconstruction
// (s_glslPostCommon, viewZ), then its main.
const char kPostSsrCompositeInputsGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D colorTex;    // unit 0: the resolved frame (display; alpha the sky light share)
uniform vec4 ssrCamera;        // x 1 = the photoreal camera: the frame holds pre-exposed light (camDecode, camFold)
uniform sampler2D tagTex;      // unit 2: the stencil tags (R8: value / 255)
uniform sampler2D traceTex;    // unit 3: half res: rgb add (linear), a = m + 4 surface
uniform sampler2D linDepthTex; // unit 4: half-resolution view depth (m)
uniform sampler2D depthTex;    // unit 5: the resolved depth
uniform vec4 projInfo;         // x = _33, y = _43, z = 1/_11, w = 1/_22
uniform vec4 depthMap;         // ndcZ = depth * x + y
uniform vec4 worldRect;        // the 3D viewport in uv
uniform vec4 screen;           // xy full-resolution size, zw half-resolution size
uniform vec4 debugView;        // x 1 = the tags, y 1 = the reflections alone
out vec4 fragColor;
)";

const char kPostSsrCompositeMainGLSL[] = R"(
vec3 toLinearC(vec3 c) { return pow(max(c, vec3(0.0)), vec3(2.2)); }
vec3 toDisplayC(vec3 c) { return pow(max(c, vec3(0.0)), vec3(1.0 / 2.2)); }

void main() {
    ivec2 ip = ivec2(gl_FragCoord.xy);
    vec4 c = texelFetch(colorTex, ip, 0);
    int tg = int(texelFetch(tagTex, ip, 0).r * 255.0 + 0.5);
    int surface = (tg & 0x20) != 0 ? 0 : (tg >> 6) & 3;
    if (debugView.x > 0.5) {
        // The tags' debug view (9): water blue, terrain green, roads orange, covered magenta.
        vec3 col = surface == 2 ? vec3(0.15, 0.4, 1.0) : (surface == 1 ? vec3(0.2, 0.85, 0.2) : vec3(1.0, 0.55, 0.1));
        if ((tg & 0x20) != 0) col = vec3(1.0, 0.1, 0.9);
        bool tagged = (tg & 0xE0) != 0;
        fragColor = vec4(tagged ? mix(c.rgb * 0.4, col, 0.7) : c.rgb * 0.4, c.a);
        return;
    }
    if (surface == 0) {
        fragColor = debugView.y > 0.5 ? vec4(0.0, 0.0, 0.0, c.a) : c;
        return;
    }
    float z = viewZ((vec2(ip) + 0.5) / screen.xy);
    // The four half-resolution texels around the pixel, bilinear, kept where
    // they reflect for the same surface at a similar depth.
    vec2 hp = (vec2(ip) + 0.5) * 0.5 - 0.5;
    ivec2 i0 = ivec2(floor(hp));
    vec2 fr = hp - vec2(i0);
    ivec2 hlim = ivec2(screen.zw) - 1;
    vec3 add = vec3(0.0);
    float m = 0.0;
    float wsum = 0.0;
    for (int k = 0; k < 4; ++k) {
        ivec2 o = ivec2(k & 1, k >> 1);
        ivec2 q = clamp(i0 + o, ivec2(0), hlim);
        vec4 t = texelFetch(traceTex, q, 0);
        int ts = int(floor(t.a * 0.25));
        if (ts != surface) continue;
        float tz = texelFetch(linDepthTex, q, 0).r;
        float dz = (tz - z) / (0.03 * z + 0.1);
        float w = (o.x == 1 ? fr.x : 1.0 - fr.x) * (o.y == 1 ? fr.y : 1.0 - fr.y) + 1e-3;
        w /= 1.0 + dz * dz;
        add += t.rgb * w;
        m += (t.a - 4.0 * float(ts)) * w;
        wsum += w;
    }
    if (wsum < 1e-4) {
        fragColor = debugView.y > 0.5 ? vec4(0.0, 0.0, 0.0, c.a) : c;
        return;
    }
    add /= wsum;
    m /= wsum;
    if (debugView.y > 0.5) {
        fragColor = vec4(toDisplayC(add), c.a);
        return;
    }
    // Water: a multiplier per channel; the ground: a multiply and an add.
    // Under the photoreal camera in the frame's own (pre-exposed) light.
    bool cam = ssrCamera.x > 0.5;
    vec3 base = cam ? camDecode(c.rgb) : toLinearC(c.rgb);
    vec3 lin = surface == 2 ? base * add : base * m + add;
    fragColor = vec4(cam ? camFold(lin) : toDisplayC(max(lin, vec3(0.0))), c.a);
}
)";

} // namespace Poseidon::gl33lighting
