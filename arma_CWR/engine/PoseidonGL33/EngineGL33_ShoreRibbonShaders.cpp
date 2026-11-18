// The shore ribbons' shaders (EngineGL33_ShoreRibbons.cpp draws them; a file of
// their own so the shader tests compile them without the backend).

#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <string>

namespace
{

static const char s_vsRibbonMain[] = R"(
layout(std140) uniform VSConstants {
    mat4 proj;
    mat4 view;
    mat4 world;
    vec4 sunDir;
    vec4 ambient;
    vec4 diffuse;
    vec4 emissive;
    vec4 fogParam;      // {start, invRange, enabled, 0}
    vec4 camPos;
    vec4 specular;
    vec4 specEn;
    vec4 sunEn;
    vec4 vpScale;       // zw TAA jitter (NDC)
};

uniform sampler2D terrainHeight;  // unit 4: r = height (m)
uniform sampler2D ribbonPoints;   // unit 17: x, z, sea normal angle, slope (negative: a piece's last point)
uniform vec4 ribbonTime;          // x time (s), y drawn sea level (m), z fade start (m), w fade end (m)
uniform vec4 ribbonGrid;          // x points per texture row

out vec4 vColor;
out vec4 vSpecColor;
out vec2 vUV0;
out vec2 vUV1;
out float vFogTC;
out vec3 vWorldRel;
out vec3 vNormalW;
out vec4 vCrown;
flat out float vLitMode;
out vec4 vRibbon;   // x foam, y wet, z metres up the beach from the waterline, w 0
out vec2 vFoamUV;   // the foam pattern's place (m): it travels with the wave

const int kSlots = 2;
const int kRows = 6;
const int kVertsPerSlot = (kRows - 1) * 6;
const int kVertsPerPoint = kSlots * kVertsPerSlot;

// One ribbon's cycle (s); two ribbons a point, so a wave every half of it.
const float kPeriod = 17.0;
// The cycle's stages (fractions): the line rolls in, breaks, runs up, slides
// back, fades; then nothing until the next cycle.
const float kWA = 0.20;
const float kWB = 0.27;
const float kWR = 0.40;
const float kWD = 0.62;
const float kWF = 0.76;
const float kRunUp = 0.30;     // m: the swash's vertical reach on an average wave
const float kRollIn = 0.95;    // m of depth where the line starts to show
const float kBreak = 0.42;     // m of depth where it breaks
const float kWet = 0.20;       // the wet band's darkening
const float kLift = 0.03;      // m over the sea or the sand
const float kSearch = 24.0;    // m: how far along its normal a point looks for the drawn waterline

// The ground's height under world xz on the landscape's triangles (the shore
// water's xpFloorHeight); deep water off the map.
float ribbonFloor(vec2 xz) {
    vec2 g = xz * litTerrain.y;
    float last = 1.0 / litTerrain.z - 1.0;
    if (g.x < 0.0 || g.y < 0.0 || g.x >= last || g.y >= last) return -1.0e4;
    vec2 i = floor(g);
    vec2 f = g - i;
    ivec2 p = ivec2(i);
    float h00 = texelFetch(terrainHeight, p, 0).r;
    float h10 = texelFetch(terrainHeight, p + ivec2(1, 0), 0).r;
    float h01 = texelFetch(terrainHeight, p + ivec2(0, 1), 0).r;
    if (f.x + f.y <= 1.0) return h00 + f.x * (h10 - h00) + f.y * (h01 - h00);
    float h11 = texelFetch(terrainHeight, p + ivec2(1, 1), 0).r;
    return h11 + (1.0 - f.x) * (h01 - h11) + (1.0 - f.y) * (h10 - h11);
}

float ribbonHash(vec2 p) {
    p = fract(p * vec2(0.1031, 0.1030));
    p += dot(p, p.yx + 33.33);
    return fract((p.x + p.y) * p.x);
}

float ribbonNoise(vec2 p) {
    vec2 i = floor(p), f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(ribbonHash(i), ribbonHash(i + vec2(1, 0)), u.x),
               mix(ribbonHash(i + vec2(0, 1)), ribbonHash(i + vec2(1, 1)), u.x), u.y);
}

vec4 ribbonPoint(int i) {
    int w = int(ribbonGrid.x + 0.5);
    return texelFetch(ribbonPoints, ivec2(i % w, i / w), 0);
}

void collapse() {
    gl_Position = vec4(0.0, 0.0, 2.0, 1.0);
    vColor = vec4(0.0);
    vSpecColor = vec4(0.0);
    vUV0 = vec2(0.0);
    vUV1 = vec2(0.0);
    vFogTC = 1.0;
    vWorldRel = vec3(0.0);
    vNormalW = vec3(0.0, 1.0, 0.0);
    vCrown = vec4(0.0);
    vLitMode = 1.0;
    vRibbon = vec4(0.0);
    vFoamUV = vec2(0.0);
}

void main() {
    int id = gl_VertexID;
    int pi = id / kVertsPerPoint;
    int loc = id - pi * kVertsPerPoint;
    int slot = loc / kVertsPerSlot;
    int q = loc - slot * kVertsPerSlot;
    int quad = q / 6;
    int corner = q - quad * 6;
    // Two triangles a quad: (0,0) (1,0) (1,1) and (0,0) (1,1) (0,1), as (along, across).
    int u = (corner == 1 || corner == 2 || corner == 4) ? 1 : 0;
    int row = quad + ((corner == 2 || corner == 4 || corner == 5) ? 1 : 0);

    // Everything that drops geometry is decided on the segment's first point,
    // which both of its ends read: a segment is drawn whole or not at all (a
    // vertex dropped alone would stretch its triangles across the screen).
    vec4 a = ribbonPoint(pi);
    if (a.w < 0.0) { collapse(); return; } // a piece's last point starts no segment
    vec2 cam = litCamPos.xz;
    if (length(a.xy - cam) > ribbonTime.w + 60.0) { collapse(); return; }
    vec4 P = u == 1 ? ribbonPoint(pi + 1) : a;
    vec2 base = P.xy;
    vec2 nS = vec2(cos(P.z), sin(P.z)); // toward the sea
    float slope = abs(P.w);
    float S = ribbonTime.y;

    // The waterline at the drawn sea level along the sea normal, at most
    // kSearch metres away: bracket it, then halve the bracket and finish on
    // the line between its ends. Without one (a flat, a lagoon behind the
    // beach) the point stays and shows nothing; nor does one that moved far.
    float h0 = ribbonFloor(base);
    float reach0 = clamp(abs(h0 - S) / max(slope, 0.01) + 1.0, 1.0, kSearch * 0.25);
    float tL = 0.0, tS = 0.0, hL = h0, hS = h0;
    bool found = false;
    if (h0 > S) {
        tS = reach0;
        for (int i = 0; i < 3; ++i) {
            hS = ribbonFloor(base + nS * tS);
            if (hS <= S) { found = true; break; }
            tL = tS; hL = hS; tS = min(tS * 2.0, kSearch);
        }
    } else {
        tL = -reach0;
        for (int i = 0; i < 3; ++i) {
            hL = ribbonFloor(base + nS * tL);
            if (hL > S) { found = true; break; }
            tS = tL; hS = hL; tL = max(tL * 2.0, -kSearch);
        }
    }
    float tW = 0.0;
    if (found) {
        for (int i = 0; i < 6; ++i) {
            float tm = 0.5 * (tL + tS);
            float hm = ribbonFloor(base + nS * tm);
            if (hm > S) { tL = tm; hL = hm; } else { tS = tm; hS = hm; }
        }
        tW = tL + (tS - tL) * clamp((hL - S) / max(hL - hS, 1e-5), 0.0, 1.0);
    }
    float shown = found ? 1.0 - smoothstep(0.4 * kSearch, kSearch, abs(tW)) : 0.0;
    vec2 W = base + nS * tW;

    // The wave's phase here: a swell direction (the breaks run along the
    // beach), smooth noise that drifts slowly (the local period wanders a few
    // percent), and the second slot about half a cycle later (so the waves
    // come 6 to 11 s apart, never evenly for long).
    float t = ribbonTime.x;
    float ph = t / kPeriod + dot(W, vec2(0.0042, 0.0026)) +
               1.6 * ribbonNoise(W * 0.006 + vec2(t * 0.0017, 3.7)) +
               0.45 * ribbonNoise(W * 0.022 + vec2(11.0, -t * 0.0031)) +
               float(slot) * (0.5 + 0.3 * (ribbonNoise(W * 0.008 + vec2(-t * 0.004, 21.0)) - 0.5));
    float wave = floor(ph);
    float w = ph - wave;
    // This wave's strength, and its reach along the coast.
    float seed = ribbonHash(vec2(wave * 0.731, float(slot) * 3.1 + 0.37));
    float amp = mix(0.45, 1.05, seed) * mix(0.75, 1.2, ribbonNoise(W * 0.03 + vec2(wave * 7.31, wave * 3.17)));
    float s = clamp(slope, 0.012, 0.9);
    float R = clamp(kRunUp * amp / s, 0.5, 15.0);   // m up the beach
    float D = clamp(kRollIn * amp / s, 3.0, 26.0);  // m out where the line shows
    float Db = clamp(kBreak * amp / s, 1.2, 12.0);  // m out where it breaks

    // F the front, cw the crest's width behind it, B the back edge; fc and fb
    // the crest's and the trail's foam; reach the furthest point run up; lace
    // the line left there; wet the band the swash wetted.
    float F, cw, B, fc, fb, reach, lace = 0.0;
    if (w < kWA) {
        float p = w / kWA;
        F = mix(-D, -Db, p * (1.5 - 0.5 * p));
        cw = mix(0.25, 0.5, p);
        B = F - mix(0.6, 1.2, p);
        fc = 0.34 * amp * smoothstep(0.0, 0.6, p);
        fb = 0.0;
        reach = F;
    } else if (w < kWB) {
        float p = (w - kWA) / (kWB - kWA);
        F = mix(-Db, 0.0, p);
        cw = mix(0.5, 0.95, p);
        B = F - mix(1.2, 3.6, p);
        fc = mix(0.34 * amp, 1.0, smoothstep(0.0, 0.7, p));
        fb = mix(0.05, 0.55, p);
        reach = F;
    } else if (w < kWR) {
        float p = (w - kWB) / (kWR - kWB);
        float e = 1.0 - (1.0 - p) * (1.0 - p);
        F = R * e;
        cw = mix(0.95, 0.45, p);
        B = mix(-3.6, -0.7, e);
        fc = mix(1.0, 0.8, p);
        fb = mix(0.55, 0.4, p);
        reach = F;
    } else if (w < kWD) {
        float p = (w - kWR) / (kWD - kWR);
        float e = p * p * (3.0 - 2.0 * p);
        F = mix(R, 0.1 * R, e);
        cw = mix(0.45, 0.3, p);
        B = mix(-0.7, -2.2, e);
        fc = mix(0.8, 0.18, p);
        fb = mix(0.4, 0.15, p);
        reach = R;
        lace = 0.5 * (1.0 - smoothstep(0.1, 1.0, p));
    } else {
        float p = clamp((w - kWD) / (kWF - kWD), 0.0, 1.0);
        F = mix(0.1 * R, -0.4, p);
        cw = 0.3;
        B = mix(-2.2, -3.0, p);
        fc = 0.18 * (1.0 - p);
        fb = 0.15 * (1.0 - p);
        reach = R;
    }
    float wet = w < kWB ? 0.0 : kWet * (1.0 - smoothstep(kWR + 0.05, 0.98, w));
    B = min(B, F - cw - 0.2);
    // The rows, back to front: back edge, trail, crest, front, lace, reach.
    float c;
    float foam;
    if (row == 0) { c = B; foam = 0.0; }
    else if (row == 1) { c = mix(B, F - cw, 0.5); foam = fb; }
    else if (row == 2) { c = F - cw; foam = fc; }
    else if (row == 3) { c = F; foam = 0.0; }
    else if (row == 4) { c = max(reach - 0.35, F); foam = lace; }
    else { c = max(reach, F); foam = 0.0; wet = 0.0; }

    vec2 xz = W - nS * c;
    float ground = ribbonFloor(xz);
    // Out at sea a row over land (a spit, a sand bar) and up the beach a row
    // over water show no foam.
    if ((c < -0.5 && ground > S + 0.05) || (c > 0.5 && ground < S - 0.05)) foam = 0.0;
    vec3 world = vec3(xz.x, max(ground, S) + kLift, xz.y);
    vec3 rel = world - litCamPos.xyz;
    float dist = length(rel);
    float fade = (1.0 - smoothstep(ribbonTime.z, ribbonTime.w, dist)) * shown;
    // Pulled toward the camera along the view ray: the same pixel, a nearer depth.
    float pull = min(0.03 + 0.0016 * dist, 0.5 * dist);
    vec3 relZ = rel * (1.0 - pull / max(dist, 1e-3));
    gl_Position = proj * (view * vec4(relZ, 1.0));
    if (vpScale.z != 0.0 || vpScale.w != 0.0) gl_Position.xy += vpScale.zw * gl_Position.w;

    vColor = vec4(1.0);
    vSpecColor = vec4(0.0);
    vUV0 = vec2(0.0);
    vUV1 = vec2(0.0);
    float fogFactor = clamp(1.0 - (length(rel - camPos.xyz) - fogParam.x) * fogParam.y, 0.0, 1.0);
    vFogTC = (fogParam.z > 0.5) ? fogFactor : 1.0;
    vWorldRel = rel;
    vNormalW = vec3(0.0, 1.0, 0.0);
    vCrown = vec4(0.0);
    vLitMode = 1.0;
    vRibbon = vec4(foam * fade, wet * fade, c, 0.0);
    vFoamUV = xz + nS * F + vec2(31.7, 17.3) * seed;
}
)";

static const char s_psRibbonMain[] = R"(
// The shore ribbons (EngineGL33_ShoreRibbons.cpp): foam broken by a pattern
// that travels with the wave, lit as the sea's foam is (a heap of bubbles
// takes a low sun by a wrapped cosine), fogged as the lit world, blended
// premultiplied: the foam over, the wet band darkening the sand.
in vec4 vRibbon;
in vec2 vFoamUV;
uniform vec4 ribbonTime;

const float kRibbonAlbedo = 0.82;
const float kRibbonWrap = 1.0;
const float kRibbonGlow = 0.8; // the foam lit through toward the sun

void main() {
    float foot = length(fwidth(vWorldRel.xz));
    float foamEnv = vRibbon.x;
    float wetEnv = vRibbon.y * smoothstep(0.05, 0.6, vRibbon.z);
    fragSurf = vec4(0.0);
    if (foamEnv < 0.004 && wetEnv < 0.004) discard;
    float t = ribbonTime.x;
    vec2 p = vFoamUV;
    // White water: blobs of cover that a strong envelope fills solid; the
    // lace: the web of bubble lines a thinning sheet leaves (the ridges of a
    // finer noise). Detail below the pixel's footprint turns into its mean.
    float b = puddleNoise(p * 0.55 + vec2(t * 0.03, 0.0)) * 0.6 + puddleNoise(p * 1.7 + vec2(13.0, -t * 0.06)) * 0.4;
    float r1 = 1.0 - abs(2.0 * puddleNoise(p * 2.6 + vec2(41.0, t * 0.05)) - 1.0);
    float r2 = 1.0 - abs(2.0 * puddleNoise(p * 6.1 + vec2(7.0, -t * 0.09)) - 1.0);
    float kR1 = clamp(0.38 / (3.0 * foot), 0.0, 1.0);
    float kR2 = clamp(0.16 / (3.0 * foot), 0.0, 1.0);
    float lace = mix(0.5, r1, kR1) * 0.6 + mix(0.5, r2, kR2) * 0.4;
    float th = 1.0 - foamEnv;
    float solid = smoothstep(th - 0.12, th + 0.12, b);
    float web = smoothstep(0.6, 0.86, lace) * smoothstep(0.02, 0.35, foamEnv);
    float foam = max(solid, web * 0.85) * min(foamEnv * 1.6, 1.0);
    // Bubble grain in the white water.
    float grain = mix(1.0, 0.84 + 0.16 * puddleNoise(p * 13.0 + 3.0), clamp(0.08 / (3.0 * foot), 0.0, 1.0));

    vec3 posAbs = vWorldRel + litCamPos.xyz;
    vec3 nUp = vec3(0.0, 1.0, 0.0);
    vec3 l = litSunDir.xyz;
    float sunOn = litSunDir.w;
    float ly = max(l.y, 0.0);
    float skyVisibility;
    float sunVis = shadowMapVisibility(nUp, ly, 0.0, 0.0) * terrainSunVisibility(posAbs, skyVisibility) *
                   cloudSunVisibility(posAbs);
    float wrap = (ly + kRibbonWrap) / (1.0 + kRibbonWrap) * smoothstep(0.0, 0.04, l.y);
    vec3 kd = litMatDiffuse.rgb;
    vec3 ka = litMatAmbient.rgb;
    // Bubbles pass light on: looking toward the sun, the foam glows (a low
    // sun behind the surf lights its edge).
    float toward = pow(max(dot(normalize(vWorldRel), l), 0.0), 6.0) * smoothstep(0.0, 0.04, l.y);
    vec3 light = litSun.rgb * kd * ((wrap + kRibbonGlow * toward) * sunVis * sunOn) +
                 ka * skyIrradiance(nUp) * (skyVisibility * sunOn) + kd * litMoon.rgb * (max(litMoonDir.y, 0.0) * sunOn);
    vec3 rgb = litDisplay(light * (kRibbonAlbedo * grain * litZenith.w));
    // The fog of the lit world (finishFragment's).
    vec3 fogC = fogColorToward(vWorldRel);
    if (litAtmo.z > 0.0 && vFogTC < 1.0) fogC = atmoFogColor(vWorldRel, fogC);
    float keep = min(vFogTC * (1.0 + litHaze.x), 1.0);
    rgb = mix(fogC, rgb, keep);
    if (litCamera.x > 0.5) rgb = camEncode(toLinear(rgb));
    float wet = wetEnv * clamp(keep, 0.0, 1.0);
    fragColor = vec4(rgb * foam, foam + wet * (1.0 - foam));
}
)";

} // namespace

namespace Poseidon::gl33lighting
{
std::string ShoreRibbonVertexSource()
{
    return std::string("#version 330 core\n") + LitConstantsBlock() + s_vsRibbonMain;
}

std::string ShoreRibbonFragmentSource()
{
    return std::string(RemasterFragmentCommon()) + s_psRibbonMain;
}
} // namespace Poseidon::gl33lighting
