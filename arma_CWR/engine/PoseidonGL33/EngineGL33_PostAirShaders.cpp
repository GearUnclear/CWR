// Remastered lighting: the air pass's shaders (the Air post stage, step B8;
// EngineGL33_PostAir.cpp runs them, Graphics/Remaster/AirMedium.hpp holds the
// medium and the C++ mirror of the optical-depth math). No GL code, so the
// unit tests compile the sources the engine builds; kPostShaders
// (EngineGL33_PostShaders.cpp) assembles them from these pieces and from two
// pieces of the remastered prelude: the LitConstants block and the terrain and
// clouds piece (cloudSunVisibility), so the air reads the same rig, cloud field
// and horizon map as the shading.
//
// Four passes:
//   - psPostAirMarch (half resolution, two targets): per pixel of the linear
//     depth, the sun's single scattering in the beam mist marched over the
//     first kBeamDistance metres with the sun visibility of the shadow map,
//     the terrain horizon and the cloud field (light shafts), and the same
//     scattering as if the whole ray saw the sun; the headlight cones; the
//     sun visibility of the haze's part of the ray (ridge and cloud shadows in
//     the haze); the valley mist's top and density at the surface the ray
//     ends on, and the view depth;
//   - psPostAirBlur: a depth-aware separable blur of the first target;
//   - psPostAirMean: the beams' depth-aware local mean (beam contrast);
//   - psPostAirApply (full resolution): the haze and valley mist in closed
//     form, C' = C T + S, toward the sky behind each pixel (the sky table's
//     colour, as the classic fog's), then the beams added like light on a
//     screen. Its output replaces the resolved frame for the stages after it.

#include <PoseidonGL33/EngineGL33_Lighting.hpp>

namespace Poseidon::gl33lighting
{

// The march's inputs. The LitConstants block and the terrain and clouds piece
// follow it; the piece reads terrainHorizon, terrainHeight, cloudNoise and,
// for the sky-matched cloud shadows (B10), skyDensity. Its long tree shadows
// (canopySunVisibility, B9) read the world's names below; the march never
// calls them.
const char kPostAirMarchInputsGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D linDepthTex;          // unit 0: half-resolution view depth (m), 1e6 = sky (PostLinearDepth)
uniform sampler2DArrayShadow shadowMap; // unit 2: the sun's cascade array, comparison sampler
uniform sampler2DArray terrainHorizon;  // unit 3: horizon angles
uniform sampler2D terrainHeight;        // unit 4: r = ground height (m)
uniform sampler2D cloudNoise;           // unit 5: cloud-shadow noise (also the mist banks)
uniform sampler2D skyDensity;           // unit 9: the weather sky's cloud density (the world's binding)
uniform sampler2D canopySun;            // unit 15: the canopy's sun-visibility map (unused here)
uniform vec4 projInfo;      // x = _33, y = _43, z = 1/_11, w = 1/_22
uniform vec4 worldRect;     // the 3D viewport in uv
uniform mat3 viewToWorld;   // view-space direction to world (camera-relative)
uniform mat4 cascadeVP[4];  // the depth pass's cascades, camera-relative
uniform vec4 cascadeSplits;
uniform vec4 cascadeCtl;    // x count, y fade range, z bias base, w omni count
uniform vec4 shadowCam;     // xyz camera forward of the depth pass, w 1 = a map this frame
uniform vec4 shadowDelta;   // xyz camera move since the depth pass
uniform vec4 airMedium;     // x haze (1/m at sea level), y valley mist (1/m), z beam mist at the ground (1/m), w its scale height (m)
uniform vec4 airMarch;      // x beam march length (m), y beam steps, z haze visibility steps (0 = none), w this frame's jitter shift
uniform vec4 airSun;        // x sun beams strength (0 = none), y beam air (1/m at sea level), z beam asymmetry g, w haze near distance (m)
uniform vec4 airSpots;      // x headlight count, y their march steps
uniform vec4 airSpotPos[4]; // xyz camera-relative, w start distance (m)
uniform vec4 airSpotDir[4]; // xyz beam direction, w luminance of the linear colour times the strength
layout(location = 0) out vec4 fragColor; // r sun beams, g headlight beams, b haze sun visibility, a sun beams as if all lit
layout(location = 1) out vec4 fragMist;  // r valley mist top (m), g its density factor, b view depth (m)
vec4 shadowCtl = vec4(0.0);              // canopySunVisibility's names (unused here)
vec4 camFwd = vec4(0.0, 0.0, 1.0, 0.0);
vec3 vWorldRel = vec3(0.0);
)";

// Shared by the march and the apply: the medium's closed forms (AirMedium.cpp
// mirrors them) and the phase function.
const char kPostAirCommonGLSL[] = R"(
const float kAirSeaFloor = -30.0;
const float kAirHazeScaleHeight = 350.0; // kHazeScaleHeight
const float kAirMistFall = 25.0;         // kValleyMistFall

// Henyey-Greenstein, normalised so that an isotropic medium gives 1.
float airHG(float c, float g) {
    float g2 = g * g;
    return (1.0 - g2) / pow(max(1.0 + g2 - 2.0 * g * c, 1e-4), 1.5);
}

// Optical length (m, at sea-level density) of a segment from height yA to yB,
// len metres long, through air of density exp(-y / H) (ExpOpticalLength).
float airExpLength(float yA, float yB, float len, float H) {
    float x = (yB - yA) / H;
    float f = abs(x) < 1e-3 ? 1.0 - 0.5 * x : (1.0 - exp(-x)) / x;
    return len * exp(-yA / H) * f;
}

// Optical length (m) of the segment through a layer of density
// min(1, exp(-(y - top) / fall)) (LayerOpticalLength).
float airLayerLength(float yA, float yB, float len, float top, float fall) {
    float lo = min(yA, yB), hi = max(yA, yB), dy = hi - lo;
    if (dy < 0.5) return len * min(1.0, exp(-(0.5 * (lo + hi) - top) / fall)); // level: the middle height
    float below = clamp((top - lo) / dy, 0.0, 1.0);
    float a = max(lo, top);
    float above = a < hi ? fall * (exp(-(a - top) / fall) - exp(-(hi - top) / fall)) / dy : 0.0;
    return len * (below + above);
}

// Interleaved gradient noise: the march's start per pixel.
float airIgn(vec2 p) { return fract(52.9829189 * fract(dot(p, vec2(0.06711056, 0.00583715)))); }
)";

// The march's helpers: the ground, the sun's visibility from a point in the
// air, the valley floor.
const char kPostAirMarchHelpersGLSL[] = R"(
// Ground height under a world xz, and whether the horizon map covers it.
float airGround(vec2 xz, out vec2 uv, out bool onMap) {
    onMap = false;
    uv = vec2(0.0);
    if (litTerrain.x < 0.5) return litCamPos.y - 1.75;
    vec2 g = xz * litTerrain.y;
    float last = 1.0 / litTerrain.z - 1.0;
    if (g.x < 0.0 || g.y < 0.0 || g.x > last || g.y > last) return 0.0;
    uv = (g + 0.5) * litTerrain.z;
    onMap = true;
    return texture(terrainHeight, uv).r;
}

// Hills between a point hAbove metres up and the sun: the horizon stored for
// the ground below, seen from that height (as from an occluder ~900 m away).
float airTerrainSun(vec2 uv, float hAbove) {
    float h0 = dot(texture(terrainHorizon, vec3(uv, litTerrainSun.x)), litTerrainSel0);
    float h1 = dot(texture(terrainHorizon, vec3(uv, litTerrainSun.y)), litTerrainSel1);
    float horizonDeg = mix(-5.0, 45.0, mix(h0, h1, litTerrainSun.z));
    float lift = degrees(atan(max(hAbove, 0.0) / 900.0));
    float soft = litTerrainSun.w + 0.5;
    float vis = smoothstep(horizonDeg - soft, horizonDeg + soft, litSun.w + lift);
    return mix(1.0, vis, litTerrainLift.z);
}

// One hardware 2x2 comparison of the cascade that holds a camera-relative
// point (the shading's tier selection, without its cross-fade).
float airShadowAt(vec3 rel) {
    if (shadowCam.w < 0.5) return 1.0;
    vec3 r = rel + shadowDelta.xyz;
    int nC = int(cascadeCtl.x);
    int omniN = int(cascadeCtl.w);
    float ed = dot(r, shadowCam.xyz);
    float d3 = length(r);
    for (int i = 0; i < 4; ++i) {
        if (i >= nC) break;
        float metric = (i < omniN) ? d3 : ed;
        if (metric <= cascadeSplits[i]) {
            vec4 cp = cascadeVP[i] * vec4(r, 1.0);
            vec3 sc = cp.xyz / cp.w;
            vec2 suv = sc.xy * 0.5 + 0.5;
            if (suv.x <= 0.0 || suv.x >= 1.0 || suv.y <= 0.0 || suv.y >= 1.0 || sc.z <= 0.0 || sc.z >= 1.0) return 1.0;
            float bias = cascadeCtl.z * float(i + 1) * float(i + 1);
            return texture(shadowMap, vec4(suv, float(i), sc.z - bias));
        }
    }
    return 1.0;
}

// The valley floor under xz: the lowest ground of the point and two rings of
// taps (8 at 400 m, 6 at 850 m), never below sea level, and how far the rings'
// mean ground stands above it (LocalValley): a plain or the open sea has no
// valley for the mist to pool in.
float airLocalLow(vec2 xz, out float depth) {
    vec2 uv;
    bool onMap;
    float m = airGround(xz, uv, onMap);
    float sum = 0.0;
    for (int k = 0; k < 8; ++k) {
        float a = float(k) * 0.785398;
        float h = airGround(xz + vec2(cos(a), sin(a)) * 400.0, uv, onMap);
        m = min(m, h);
        sum += h;
    }
    for (int k = 0; k < 6; ++k) {
        float a = float(k) * 1.047198 + 0.5;
        float h = airGround(xz + vec2(cos(a), sin(a)) * 850.0, uv, onMap);
        m = min(m, h);
        sum += h;
    }
    m = max(m, 0.0);
    depth = sum / 14.0 - m;
    return m;
}
)";

const char kPostAirMarchMainGLSL[] = R"(
void main() {
    float z = texelFetch(linDepthTex, ivec2(gl_FragCoord.xy), 0).r;
    bool sky = z >= 1e5;
    vec2 ndc = (vUV - worldRect.xy) / worldRect.zw * 2.0 - 1.0;
    vec3 wdir = viewToWorld * vec3(ndc.x * projInfo.z, ndc.y * projInfo.w, 1.0);
    float lenPerZ = length(wdir);
    wdir /= lenPerZ;
    float rayLen = sky ? 1e6 : z * lenPerZ;
    float jitter = fract(airIgn(gl_FragCoord.xy) + airMarch.w);
    vec2 uv;
    bool onMap;

    // The valley mist at the surface the ray ends on: its top above the
    // lowest ground nearby, varied by two noise banks (1700 m and 520 m).
    float top = -1e4;
    float mistDensity = 0.0;
    if (airMedium.y > 0.0 && !sky && rayLen > airSun.w) {
        vec2 xz = litCamPos.xz + wdir.xz * rayLen;
        float n1 = texture(cloudNoise, xz * (1.0 / 1700.0) + vec2(0.37, 0.11)).r;
        float n2 = texture(cloudNoise, xz * (1.0 / 520.0) + vec2(0.71, 0.53)).r;
        float depth;
        top = airLocalLow(xz, depth) + 40.0 * mix(0.45, 1.35, n1) + (n2 - 0.5) * 12.0;
        mistDensity = mix(0.35, 1.5, n1 * 0.7 + n2 * 0.3) * smoothstep(20.0, 60.0, depth);
    }

    // Sun beams: single scattering in the beam mist (the air plus a ground
    // mist of scale height airMedium.w), denser samples near the camera.
    // sunOpen is the same scattering as if the whole ray saw the sun: the
    // apply's contrast works on the share that does (sunBeams / sunOpen).
    float sunBeams = 0.0;
    float sunOpen = 0.0;
    if (airSun.x > 0.0) {
        float tEnd = min(rayLen, airMarch.x);
        int n = int(airMarch.y);
        float optical = 0.0;
        float sumLit = 0.0;
        float sumOpen = 0.0;
        for (int i = 0; i < 48; ++i) {
            if (i >= n) break;
            float x = (float(i) + jitter) / float(n);
            float t = tEnd * x * x;
            float dt = tEnd * 2.0 * max(x, 0.5 / float(n)) / float(n);
            vec3 rel = wdir * t;
            vec3 pa = rel + litCamPos.xyz;
            float hA = pa.y - airGround(pa.xz, uv, onMap);
            float sigma = airSun.y * exp(-max(pa.y, kAirSeaFloor) / kAirHazeScaleHeight) +
                          airMedium.z * exp(-max(hA, 0.0) / airMedium.w);
            float w = sigma * exp(-optical) * dt;
            optical += sigma * dt;
            float vis = airShadowAt(rel) * cloudSunVisibility(pa);
            if (onMap) vis *= airTerrainSun(uv, hA);
            sumLit += w * vis;
            sumOpen += w;
        }
        float c = dot(wdir, litSunDir.xyz);
        float phase = 0.85 * airHG(c, airSun.z) + 0.15 * airHG(c, -0.3);
        // E = pi * litSun (litSun is the radiance a white Lambert face sends
        // back); L = sigma * p * E with p = phase / (4 pi): a factor 1/4.
        sunBeams = phase * 0.25 * sumLit * airSun.x;
        sunOpen = phase * 0.25 * sumOpen * airSun.x;
    }

    // Headlight cones: a march through each cone over its reach (ten times
    // its start distance, at most 60 m), forward scattering toward the eye.
    float spots = 0.0;
    int ns = int(airSpots.x);
    int nk = int(airSpots.y);
    for (int i = 0; i < 4; ++i) {
        if (i >= ns) break;
        vec3 p = airSpotPos[i].xyz;
        float sp = clamp(airSpotPos[i].w, 0.5, 6.0);
        float reach = min(10.0 * sp, 60.0);
        float t0 = dot(p, wdir);
        float h2 = max(dot(p, p) - t0 * t0, 0.0);
        if (h2 > reach * reach) continue;
        float halfLen = sqrt(reach * reach - h2);
        float ta = max(0.0, t0 - halfLen);
        float tb = min(rayLen, t0 + halfLen);
        if (tb <= ta) continue;
        vec3 lightAbs = p + litCamPos.xyz;
        float sigmaL = airSun.y + airMedium.z * exp(-max(lightAbs.y - airGround(lightAbs.xz, uv, onMap), 0.0) / airMedium.w);
        float dts = (tb - ta) / float(nk);
        float integral = 0.0;
        for (int k = 0; k < 32; ++k) {
            if (k >= nk) break;
            float t = ta + (float(k) + jitter) * dts;
            vec3 fromLight = wdir * t - p;
            float d2 = max(dot(fromLight, fromLight), 1e-4);
            float inside = dot(fromLight, airSpotDir[i].xyz);
            if (inside <= 0.0) continue;
            float cone = smoothstep(0.9415, 0.9806, inside * inside / d2); // the shading's cone
            float fade = 1.0 - smoothstep(0.25 * reach * reach, reach * reach, d2);
            // The scattering angle is between the light's travel and the way
            // to the eye: looking into a headlight is the bright direction.
            float cf = -dot(fromLight * inversesqrt(d2), wdir);
            integral += cone * fade * sp * sp / max(d2, sp * sp) * airHG(cf, 0.5) * dts;
        }
        spots += airSpotDir[i].w * sigmaL * integral * 0.25;
    }

    // The haze's sun visibility: the density-weighted share of the haze's
    // part of the ray (from the near distance to the surface) that sees the
    // sun past the hills and the clouds.
    float vis = 1.0;
    int nv = int(airMarch.z);
    if (nv > 0 && !sky && rayLen > airSun.w) {
        float mist = airMedium.y * mistDensity;
        float ws = 1e-9;
        float vs = 0.0;
        for (int i = 0; i < 16; ++i) {
            if (i >= nv) break;
            float t = mix(airSun.w, rayLen, (float(i) + jitter) / float(nv));
            vec3 pa = litCamPos.xyz + wdir * t;
            float g = airGround(pa.xz, uv, onMap);
            float dens = airMedium.x * exp(-max(pa.y, kAirSeaFloor) / kAirHazeScaleHeight);
            if (mist > 0.0) dens += mist * min(1.0, exp(-(pa.y - top) / kAirMistFall));
            float s = cloudSunVisibility(pa);
            if (onMap) s *= airTerrainSun(uv, pa.y - g);
            ws += dens;
            vs += dens * s;
        }
        vis = vs / ws;
    }

    fragColor = vec4(sunBeams, spots, vis, sunOpen);
    fragMist = vec4(top, mistDensity, min(z, 6e4), 1.0);
}
)";

// Depth-aware separable blur of the march's first target (the view depth is
// the second target's b), run across and then down.
const char kPostAirBlurGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D airTex;
uniform sampler2D airDepthTex; // unit 3: the march's second target, b = view depth (m)
uniform vec2 stepUV;
out vec4 fragColor;
void main() {
    vec4 sum = texture(airTex, vUV);
    float zc = texture(airDepthTex, vUV).b;
    float wsum = 1.0;
    for (int i = -5; i <= 5; ++i) {
        if (i == 0) continue;
        vec2 uv = vUV + stepUV * float(i);
        float zs = texture(airDepthTex, uv).b;
        float w = exp(-float(i * i) / 12.0) * max(0.0, 1.0 - abs(zs - zc) / (0.08 * zc + 0.5));
        sum += texture(airTex, uv) * w;
        wsum += w;
    }
    fragColor = sum / wsum;
}
)";

// The beams' local mean (beam contrast): a disc of meanParams.x of the frame
// height, taking only samples at a similar view depth (sky with sky, a tree
// line with the tree line), so a silhouette against a bright glow gets no rim.
const char kPostAirMeanGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D airTex;
uniform sampler2D airDepthTex; // unit 3: the march's second target, b = view depth (m)
uniform vec4 meanParams; // x radius (uv of the frame height), y aspect (width / height)
out vec4 fragColor;      // the mean of airTex's r, g and a (sun beams, headlight beams, sun beams as if all lit)
void main() {
    float zc = texture(airDepthTex, vUV).b;
    vec3 mean = vec3(0.0);
    float wm = 0.0;
    for (int k = 0; k < 24; ++k) {
        float a = float(k) * 2.39996;
        float r = meanParams.x * sqrt((float(k) + 0.5) / 24.0);
        vec2 o = vec2(cos(a) * r / meanParams.y, sin(a) * r);
        vec2 uv = clamp(vUV + o, vec2(0.0), vec2(1.0));
        float wk = exp(-abs(texture(airDepthTex, uv).b - zc) / (0.25 * zc + 2.0));
        mean += texture(airTex, uv).rga * wk;
        wm += wk;
    }
    fragColor = vec4(mean / max(wm, 1e-4), 1.0);
}
)";

// The apply's inputs. The LitConstants block, the colour helpers, the sky
// piece of the prelude (the fog colour toward the sun and the sky table's
// aerial perspective: the haze fades toward the colour the classic fog does)
// and the common air math follow it. The sky piece also holds the dome's
// shading, which reads the names below; this pass never calls it.
const char kPostAirApplyInputsGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D colorTex;   // unit 0: the resolved frame (display; a = 1 - the sky light share)
uniform sampler2D airTex;     // unit 2: half resolution: r sun beams, g headlight beams, b haze sun visibility, a sun beams as if all lit
uniform sampler2D airMistTex; // unit 3: half resolution: r valley mist top (m), g its density factor, b view depth (m)
uniform sampler2D airMeanTex; // unit 4: half resolution: the local mean of airTex's r, g and a
uniform sampler2D depthTex;   // unit 5: the resolved depth
uniform sampler2D skyLut;     // unit 8: the sky table (the world's binding)
uniform vec4 fogColor;        // the classic fog colour (display)
uniform vec4 projInfo;        // x = _33, y = _43, z = 1/_11, w = 1/_22
uniform vec4 depthMap;        // ndcZ = depth * x + y
uniform vec4 worldRect;       // the 3D viewport in uv
uniform mat3 viewToWorld;     // view-space direction to world (camera-relative)
uniform vec4 airHalf;         // xy half-resolution size, z 1 = the march ran this frame, w 1 = the scattering alone (debug)
uniform vec4 airMedium;       // x haze (1/m at sea level), y valley mist (1/m), z haze near distance (m), w mist full from (m)
uniform vec4 airBeams;        // x 1 = beams, y high-pass share, z contrast gain, w 1 = dither
uniform vec4 airSunColor;     // rgb the sun's radiance times the exposure (linear)
uniform vec4 airSpotTint;     // rgb the headlights' mean chroma (linear, unit luminance)
uniform vec4 airFog;          // x the classic fog's start as the world drew it (m), y 1 / (end - start), z 1 = on
uniform sampler2D tex0;       // the dome shading's names (unused here)
uniform sampler2D skyDensity;
vec3 vWorldRel = vec3(0.0);
vec2 vUV0 = vec2(0.0);
vec3 g_groundTint = vec3(1.0);
// The lit sky clouds' density lookups (B10) live in the terrain and clouds
// piece, which the apply does not paste; the dome shading that calls them is
// unused here.
float skyCloudDensity(vec4 m) { return 0.0; }
vec2 skyDomeUv(vec3 w) { return vec2(0.0); }
out vec4 fragColor;

const vec3 kLuma = vec3(0.2126, 0.7152, 0.0722);
vec3 toLinear(vec3 c) { return pow(max(c, vec3(0.0)), vec3(2.2)); }
vec3 toDisplay(vec3 c) { return pow(max(c, vec3(0.0)), vec3(1.0 / 2.2)); }

// The prelude's tone curve (its inputs piece declares the world's varyings, so
// a post pass cannot paste it): Khronos PBR Neutral without its toe.
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

const char kPostAirApplyMainGLSL[] = R"(
const float kAirHazeShadowAmbient = 0.55; // kHazeShadowAmbient

// The sky the haze fades toward: the classic fog colour with its glow toward
// the sun and, under the sky table, the clear sky just above the horizon in
// this azimuth (finishFragment's fog colour for lit geometry).
vec3 airSkyLimit(vec3 rel) {
    vec3 c = fogColorToward(rel);
    if (litAtmo.z > 0.0) c = atmoFogColor(rel, c);
    return c;
}

void main() {
    ivec2 ip = ivec2(gl_FragCoord.xy);
    vec4 c = texelFetch(colorTex, ip, 0);
    float d = texelFetch(depthTex, ip, 0).r;
    bool sky = d >= 0.999999;
    float z = sky ? 6e4 : min(projInfo.y / ((d * depthMap.x + depthMap.y) - projInfo.x), 6e4);
    vec2 ndc = (vUV - worldRect.xy) / worldRect.zw * 2.0 - 1.0;
    vec3 rel = viewToWorld * vec3(ndc.x * projInfo.z, ndc.y * projInfo.w, 1.0) * z;
    float dist = length(rel);

    // Joint-bilateral upsample of the half-resolution targets: the four
    // nearest texels, bilinear weights times depth similarity. conf is 0 when
    // no tap matched this pixel's depth (a thin branch against the glow).
    vec4 air = vec4(0.0, 0.0, 1.0, 0.0);
    vec2 mist = vec2(-1e4, 0.0);
    vec3 mean = vec3(0.0);
    float conf = 1.0;
    if (airHalf.z > 0.5) {
        vec2 pos = vUV * airHalf.xy - 0.5;
        vec2 base = floor(pos);
        vec2 f = pos - base;
        float best = 1e9;
        float wsum = 0.0;
        vec4 sum = vec4(0.0);
        vec2 msum = vec2(0.0);
        vec3 esum = vec3(0.0);
        for (int j = 0; j < 2; ++j) {
            for (int i = 0; i < 2; ++i) {
                ivec2 tc = clamp(ivec2(base) + ivec2(i, j), ivec2(0), ivec2(airHalf.xy) - 1);
                vec3 m = texelFetch(airMistTex, tc, 0).rgb;
                float wb = (i == 0 ? 1.0 - f.x : f.x) * (j == 0 ? 1.0 - f.y : f.y);
                float dn = abs(m.b - z) / (0.05 * z + 0.3);
                best = min(best, dn);
                float w = wb / (1e-3 + dn) + 1e-6;
                sum += texelFetch(airTex, tc, 0) * w;
                msum += m.rg * w;
                if (airBeams.x > 0.5) esum += texelFetch(airMeanTex, tc, 0).rgb * w;
                wsum += w;
            }
        }
        air = sum / wsum;
        mist = msum / wsum;
        mean = esum / wsum;
        conf = 1.0 - smoothstep(0.5, 2.0, best);
    }

    vec3 lin = toLinear(c.rgb);
    vec3 added = vec3(0.0);

    // Haze and valley mist beyond the near distance, in closed form. The air
    // converges to the sky behind the pixel where it sees the sun and keeps a
    // dimmer zenith-coloured sky light in the shadow of a ridge or a cloud;
    // the mist is a bright multiple-scattering layer with a broad forward
    // lobe of the sun. vis is the density-weighted sun visibility along the
    // ray (the march's third channel).
    if (!sky && dist > airMedium.z && (airMedium.x > 0.0 || airMedium.y > 0.0)) {
        vec3 dir = rel / dist;
        float t0 = airMedium.z;
        float len = dist - t0;
        float yA = litCamPos.y + dir.y * t0;
        float yB = litCamPos.y + rel.y;
        vec3 tauAir = airMedium.x * vec3(0.85, 1.0, 1.2) *
                      airExpLength(max(yA, kAirSeaFloor), max(yB, kAirSeaFloor), len, kAirHazeScaleHeight);
        // The mist shows from the near distance and in full from airMedium.w
        // on: no cool lift over the mid field.
        float mistTau = airMedium.y * mist.y * airLayerLength(yA, yB, len, mist.x, kAirMistFall) *
                        smoothstep(t0, airMedium.w, dist);
        vec3 tau = tauAir + vec3(mistTau);
        vec3 T = exp(-tau);
        float vis = clamp(air.b, 0.0, 1.0);
        // The air, like the fog it replaces, stays on the sun (B3): under the
        // moon key litSunDir and litSun carry the moon, the sky's sun is set.
        float sunOn = smoothstep(-0.035, 0.0175, skySunDir().y) * litSunDir.w;
        vec3 skyLim = toLinear(airSkyLimit(rel));
        float skyLum = dot(skyLim, kLuma);
        vec3 zen = litZenith.rgb / max(dot(litZenith.rgb, kLuma), 1e-4);
        vec3 airAmb = mix(zen * (skyLum * kAirHazeShadowAmbient), skyLim, mix(1.0, vis, sunOn));
        vec3 sunL = skySunRadiance() * (litZenith.w * sunOn);
        float cs = dot(dir, skySunDir());
        vec3 mistAmb = mix(vec3(skyLum), skyLim, 0.6) * mix(0.62, 0.85, vis);
        vec3 mistSun = sunL * ((0.10 + 0.07 * airHG(cs, 0.5)) * vis);
        vec3 scat = (tauAir * airAmb + vec3(mistTau) * (mistAmb + mistSun)) / max(tau, vec3(1e-6)) * (1.0 - T);
        // The world drew this pixel with the classic fog's edge, f of it left
        // and the rest the sky limit. The haze goes on top as if that edge fog
        // came after it, mix(limit, C T + S, f): toward the view distance the
        // mist, too, dissolves into the sky (no hard line where the mist's far
        // edge meets the horizon). For the air alone S is limit (1 - T) and
        // this is C T + S.
        float fEdge = airFog.z > 0.5 ? clamp(1.0 - (dist - airFog.x) * airFog.y, 0.0, 1.0) : 1.0;
        scat = mix(skyLim * (1.0 - T), scat, fEdge);
        added += scat;
        lin = lin * T + scat;
    }

    // Beams: the local contrast of the scattering (what stands above its
    // depth-matched mean is boosted by gain - 1, the smooth glow loses up to
    // the high-pass share, never below zero), added like light on a screen.
    // For the sun the contrast works on the share of the scattering that sees
    // the sun (AirMedium.cpp's BeamShaft): a ray that only runs longer through
    // the ground mist, as over open ground toward the horizon, stands above
    // its mean in scattering but not in that share, so it gets no boost (no
    // band). The headlight cones are their own contrast.
    vec3 glow = vec3(0.0);
    if (airBeams.x > 0.5 && airHalf.z > 0.5) {
        vec2 shaft = vec2(clamp(air.r / max(air.a, 1e-7), 0.0, 1.0), air.g);
        vec2 local = vec2(clamp(mean.x / max(mean.z, 1e-7), 0.0, 1.0), mean.y);
        // No half-resolution tap matched this pixel's depth: the upsample
        // borrowed a neighbour's (the sky's) scattering. Clamp it to the mean
        // and give it no boost, so crowns against the glow get no bright rim.
        shaft = mix(min(shaft, local), shaft, conf);
        float boost = (airBeams.z - 1.0) * conf;
        vec2 floorAt = (1.0 - airBeams.y) * shaft;
        shaft = max(shaft - airBeams.y * local, floorAt) + boost * max(shaft - local, vec2(0.0));
        glow = airSunColor.rgb * (shaft.x * air.a) + airSpotTint.rgb * shaft.y;
        lin += (1.0 - lin) * (1.0 - exp(-max(glow, vec3(0.0))));
    }
    if (airHalf.w > 0.5) {
        // The scattering alone over black (debug view 8, kDebugViewAir).
        fragColor = vec4(toDisplay(min(added + (1.0 - exp(-max(glow, vec3(0.0)))), vec3(1.0))), 1.0);
        return;
    }
    vec3 col = toDisplay(clamp(lin, 0.0, 1.0));
    // Dither only what this pass changed; elsewhere the frame passes through
    // exactly (the 2.2 round trip is exact on 8-bit values).
    if (airBeams.w > 0.5 && max(max(added.r, added.g), max(added.b, max(glow.r, max(glow.g, glow.b)))) > 1e-5) {
        float n = fract(52.9829189 * fract(dot(gl_FragCoord.xy, vec2(0.06711056, 0.00583715))));
        float m = fract(52.9829189 * fract(dot(gl_FragCoord.yx + 17.0, vec2(0.06711056, 0.00583715))));
        col += (n + m - 1.0) / 255.0;
    }
    fragColor = vec4(col, c.a);
}
)";

} // namespace Poseidon::gl33lighting
