// Remastered lighting: the volumetric fog's shaders (the Air post stage, the
// look lane "fog"; EngineGL33_PostFog.cpp runs them, and
// Graphics/Remaster/VolumetricFog.hpp holds the medium, the slice layout and
// the C++ mirrors). No GL code, so the unit tests compile the sources the
// engine builds; kPostShaders (EngineGL33_PostShaders.cpp) assembles them from
// these pieces and from three pieces of the remastered prelude: the
// LitConstants block, the terrain and clouds piece and the sky piece, so the
// air reads the same rig, cloud field, horizon map, canopy shadows and sky
// table as the shading.
//
// Four passes:
//   - psFogInject: a froxel volume (cells of 8 to 16 pixels, 48 to 96
//     exponential depth slices out to the classic fog end), eight slices per
//     draw (eight colour attachments, each a slice of the 3D target): per
//     cell the medium's extinction and the light it scatters toward the eye.
//     The air's light along a column is the sky's own colour just above the
//     land in that azimuth (measured each frame: the horizon ring), so far
//     ground melts into the sky behind it and far ridges take its colour; the
//     share of it that is scattered sunlight goes dark where the cell is in
//     the shade of the shadow cascades, the canopy, the terrain horizon or the
//     cloud field (the shadows of ridges in the haze), and the mist and fog
//     above a cell take light out of its sun. Each frame samples a jittered
//     point of each cell and blends it with the last frame's volume
//     reprojected to that point, so the history averages the jitter;
//   - psFogIntegrate: front to back along each column, the light scattered
//     toward the eye and the transmittance from the eye to the far face of
//     each cell (energy-conserving per slice);
//   - psFogNear: half resolution, per pixel, what the cells are too coarse
//     for: the sun's forward lobe in the mist, fog and humid ground air of the
//     first kFogBeamReach metres, marched with the sun's visibility at every
//     step (shafts between crowns and trunks with their own edges), and the
//     lamps', headlights' and fires' light in the air in closed form (the
//     integral of I / (r^2 + 1 m^2) along the ray, the low beam's pattern and
//     the phase over six points of its angle), so a lamp gets a compact halo,
//     a headlight lights a cone of fog ahead of it and the light stops at the
//     surface in front;
//   - psFogApply: full resolution, each pixel's depth looked up in the volume,
//     C' = C T + S in linear light before the world's shoulder (the input's
//     highlights expanded back first), plus the near-field light upsampled by
//     depth. The sky takes the whole volume where the weather fog, the mist
//     and the humid air fill it (and their part beyond the far plane), only
//     the horizon's share of the clear haze and air (the dome holds the clear
//     atmosphere), so stars fade with the fog and a lamp glows in the sky.

#include <PoseidonGL33/EngineGL33_Lighting.hpp>

namespace Poseidon::gl33lighting
{

// The injection's own inputs and outputs (its head: the #version line).
const char kPostFogInjectHeadGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler3D fogHistory; // unit 0: the last frame's injection (rgb in-scatter, a extinction; per km)
uniform vec4 fogGrid;      // xy cells, z slices, w the first slice of this draw
uniform vec4 fogDepth;     // x far (m), y k, z 1 / (exp(k) - 1), w this frame's depth jitter in a slice (0..1)
uniform vec4 fogFrame;     // xy this frame's jitter in a cell (cells), z this frame's share (1 = no history), w time (s)
uniform vec4 prevRow0;     // camera-relative world to the previous frame's view space, by rows
uniform vec4 prevRow1;
uniform vec4 prevRow2;
uniform vec4 prevProj;     // x _11, y _22, z far (m), w k of the previous frame's volume
layout(location = 0) out vec4 fog0;
layout(location = 1) out vec4 fog1;
layout(location = 2) out vec4 fog2;
layout(location = 3) out vec4 fog3;
layout(location = 4) out vec4 fog4;
layout(location = 5) out vec4 fog5;
layout(location = 6) out vec4 fog6;
layout(location = 7) out vec4 fog7;
)";

// What the injection and the near-field pass share: the world's maps, the
// shadow constants under their PSConstants names (the terrain and clouds
// piece's canopySunVisibility reads them), the camera and the medium
// (VolumetricFog.hpp FogMedium). The LitConstants block and the terrain and
// clouds piece follow it; the piece reads terrainHorizon, terrainHeight,
// cloudNoise, skyDensity and canopySun, and vWorldRel (a global here).
const char kPostFogSharedInputsGLSL[] = R"(
uniform sampler2DArrayShadow shadowMap; // unit 2: the sun's cascade array, comparison sampler
uniform sampler2DArray terrainHorizon;  // unit 3: horizon angles
uniform sampler2D terrainHeight;        // unit 4: r = ground height (m)
uniform sampler2D cloudNoise;           // unit 5: cloud-shadow noise (also the mist banks)
uniform sampler2D skyLut;               // unit 8: the sky table (or its stand-in)
uniform sampler2D skyDensity;           // unit 9: the weather sky's cloud density (as the world bound it)
uniform sampler2D fogFloor;             // unit 12: r = the fog floor (m), g = the mist layer's depth (m)
uniform sampler2D canopySun;            // unit 15: the canopy's sun-visibility map (as the world bound it)
uniform vec4 shadowCtl;     // x 1 = a shadow map this frame
uniform mat4 cascadeVP[4];  // the depth pass's cascades, camera-relative
uniform vec4 cascadeSplits;
uniform vec4 cascadeCtl;    // x count, y fade range, z bias base, w omni count
uniform vec4 camFwd;        // the depth pass's camera forward
vec3 vWorldRel = vec3(0.0); // the camera-relative point canopySunVisibility reads
uniform vec4 fogTerrain;    // x 1 = a fog floor map, y 1 / its cell size (1/m), z 1 / its size
uniform vec4 projInfo;      // z = 1/_11, w = 1/_22
uniform mat3 viewToWorld;   // view-space direction to world (camera-relative)
uniform vec4 shadowDelta;   // xyz camera move since the depth pass
uniform vec4 fogHaze;       // x haze (1/m at sea level), y its scale height (m), z Rayleigh scale, w the sun's forward lobe
uniform vec4 fogWeather;    // x weather fog (1/m at the floor), y its scale height (m), z sky-through, w night
uniform vec4 fogGround;     // x humid ground air (1/m at the ground), y its scale height above the ground (m), z dust (1/m at 0 m), w its scale height (m)
uniform vec4 fogMist;       // x valley mist (1/m in the layer), y layer scale, z fall above it (m), w banks
uniform vec4 fogLightK;     // x the sun's light in the air, y the sky's, z Mie g, w lights
uniform vec4 fogWind;       // xy the mist banks' drift (m), z the clear air's near thinning ends (m), w its share at the eye
uniform vec4 fogNightAir;   // rgb the night air's light (linear), w its weight (the night)
uniform sampler2D fogRing;  // unit 13: the horizon ring (kPostFogRingGLSL)
)";

// What the sky piece reads besides the rig: the classic fog colour, the
// colour helpers of the prelude's inputs, and the dome's texture names (its
// dome and cloud-sheet functions are compiled, never called here).
const char kPostFogSkyInputsGLSL[] = R"(
uniform vec4 fogColor;     // the classic fog colour (display), PSConstants c0 in the world programs
uniform sampler2D tex0;    // unused: the sky piece's dome functions name it
vec2 vUV0 = vec2(0.0);     // unused: the sky piece's dome functions name it
const vec3 kLuma = vec3(0.2126, 0.7152, 0.0722);
vec3 toLinear(vec3 c) { return pow(max(c, vec3(0.0)), vec3(2.2)); }
vec3 toDisplay(vec3 c) { return pow(max(c, vec3(0.0)), vec3(1.0 / 2.2)); }
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

// The medium and the sun's visibility in the air (the injection and the
// near-field pass).
const char kPostFogMediumGLSL[] = R"(
const float kFogPi = 3.14159265;
const float kFogIso = 1.0 / (4.0 * 3.14159265);
const vec3 kFogLuma = vec3(0.2126, 0.7152, 0.0722);
const vec3 kRayleighSea = vec3(5.802e-6, 13.558e-6, 33.1e-6); // Atmosphere.hpp kRayleighScattering
const float kRayleighHeight = 8000.0;
// VolumetricFog.hpp kMistNearKeep, kMistNearFrom, kMistNearTo, kHazeNear*,
// kGroundAirTop, kGroundAirFade.
const float kMistNearKeep = 0.15;
const float kHazeNearFrom = 200.0; // the end and the share at the eye are FogMedium's (fogWind.zw)
const float kMistNearFrom = 20.0;
const float kMistNearTo = 300.0;
const float kGroundAirTop = 20.0;
const float kGroundAirFade = 30.0;
// Dust's light: warm (the fine sand takes the blue out), luminance about 1.
const vec3 kDustTint = vec3(1.10, 1.0, 0.82);
// VolumetricFog.hpp kDustNearKeep, kDustNearFrom, kDustNearTo.
const float kDustNearKeep = 0.3;
const float kDustNearFrom = 100.0;
const float kDustNearTo = 1500.0;

// Henyey-Greenstein, normalised over the sphere (isotropic = 1 / 4 pi).
float fogHG(float c, float g) {
    float g2 = g * g;
    return (1.0 - g2) / (4.0 * kFogPi * pow(max(1.0 + g2 - 2.0 * g * c, 1e-4), 1.5));
}
float fogMiePhase(float c) { return mix(fogHG(c, fogLightK.z), fogHG(c, -0.3), 0.12); }

// The medium at a point: the clear haze, the Rayleigh air (per channel), the
// "wet" media near the ground (humid air, weather fog, mist; their sum, the
// dense ones alone), and the optical depth of the mist and fog straight
// above it (what they take out of the sun on the way down).
float g_fogGround = 0.0;  // the ground under the last point (m)
float g_fogAbove = 0.0;   // the point's height over it (m)
struct FogMediumCell {
    float haze;
    float hazeLobe; // the haze the sun's forward lobe lights (a lighter near thinning: the glow over the water)
    vec3 ray;
    float dust;
    float wet;
    float dense;
    float tauUp;
};

FogMediumCell fogMediumAt(vec3 rel) {
    vec3 p = rel + litCamPos.xyz;
    float ground = 0.0, floorY = 0.0, layer = 0.0;
    if (fogTerrain.x > 0.5) {
        vec2 uv = (p.xz * fogTerrain.y + 0.5) * fogTerrain.z;
        ground = texture(terrainHeight, uv).r;
        vec2 fl = texture(fogFloor, uv).rg;
        floorY = fl.r;
        layer = fl.g;
    }
    float hAbove = max(p.y - ground, 0.0);
    g_fogGround = ground;
    g_fogAbove = hAbove;
    // The eye's own surroundings stay legible: inside the mist and the humid
    // air their density near the eye is a small share, and the clear haze
    // and air keep kHazeNearKeep of theirs there (the eye reads its own
    // surroundings in full contrast; the depth is in the distance).
    float dist = length(rel);
    float nearKeep = mix(kMistNearKeep, 1.0, smoothstep(kMistNearFrom, kMistNearTo, dist));
    float hazeKeep = mix(fogWind.w, 1.0, smoothstep(kHazeNearFrom, fogWind.z, dist));
    FogMediumCell c;
    float haze0 = fogHaze.x * exp(-max(p.y, -50.0) / fogHaze.y);
    c.haze = haze0 * hazeKeep;
    c.hazeLobe = haze0 * mix(0.1, 1.0, smoothstep(30.0, 800.0, dist));
    c.ray = kRayleighSea * (fogHaze.z * exp(-max(p.y, 0.0) / kRayleighHeight) * hazeKeep);
    // Dust hangs low and close: it keeps more near the eye than the haze.
    c.dust = fogGround.z * exp(-hAbove / max(fogGround.w, 1.0)) *
             mix(kDustNearKeep, 1.0, smoothstep(kDustNearFrom, kDustNearTo, dist));
    float overFloor = max(p.y - floorY, 0.0);
    // The humid air pools in the low ground: thin on the slopes, gone above
    // the mist layer's reach (a hilltop stands in clear air).
    float gTop = floorY + layer * fogMist.y + kGroundAirTop;
    float sGround = fogGround.x * exp(-hAbove / max(fogGround.y, 1.0) - overFloor / 80.0) * nearKeep *
                    (1.0 - smoothstep(gTop, gTop + kGroundAirFade, p.y));
    float wExp = exp(-overFloor / max(fogWeather.y, 1.0));
    float sWeather = fogWeather.x * wExp;
    float sMist = 0.0;
    float mistTau = 0.0;
    float depth = layer * fogMist.y;
    if (fogMist.x > 0.0 && depth > 0.5) {
        // The layer's top rises and falls with two drifting noise octaves; the
        // banks thicken and thin it.
        vec2 q = p.xz + fogWind.xy;
        float n1 = texture(cloudNoise, q * (1.0 / 1100.0) + vec2(0.37, 0.11)).r;
        float n2 = texture(cloudNoise, q * (1.0 / 310.0) + vec2(0.71, 0.53) + p.y * 0.002).r;
        float top = floorY + depth * mix(0.6, 1.35, n1) + (n2 - 0.5) * 0.35 * depth;
        float above = p.y - top;
        float banks = mix(1.0, 1.7 * smoothstep(0.2, 0.85, n1 * 0.6 + n2 * 0.4), fogMist.w);
        float shape = above <= 0.0 ? 1.0 : exp(-above / fogMist.z);
        sMist = fogMist.x * shape * banks * nearKeep;
        mistTau = fogMist.x * banks * (max(-above, 0.0) + fogMist.z * min(shape, 1.0));
    }
    c.wet = sGround + sWeather + sMist;
    c.dense = sWeather + sMist;
    c.tauUp = mistTau + fogWeather.x * fogWeather.y * wExp;
    return c;
}

// One hardware 2x2 comparison of the cascade that holds a camera-relative
// point (the shading's tier selection and far fade, without its cross-fade).
float fogShadowMap(vec3 rel) {
    if (shadowCtl.x < 0.5) return 1.0;
    vec3 r = rel + shadowDelta.xyz;
    int nC = int(cascadeCtl.x);
    int omniN = int(cascadeCtl.w);
    float ed = dot(r, camFwd.xyz);
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
            float lit = texture(shadowMap, vec4(suv, float(i), sc.z - bias));
            float fade = clamp((cascadeSplits[nC - 1] - ed) / max(cascadeCtl.y, 0.001), 0.0, 1.0);
            return 1.0 - (1.0 - lit) * fade;
        }
    }
    return 1.0;
}

// Hills between a point hAbove metres over the ground and the sun: the horizon
// stored for the ground below, seen from that height (as from an occluder
// about 900 m away).
float fogTerrainSun(vec3 p, float hAbove) {
    if (litTerrain.x < 0.5 || litTerrainLift.z <= 0.0) return 1.0;
    vec2 g = p.xz * litTerrain.y;
    float last = 1.0 / litTerrain.z - 1.0;
    if (g.x < 0.0 || g.y < 0.0 || g.x > last || g.y > last) return 1.0;
    vec2 uv = (g + 0.5) * litTerrain.z;
    float h0 = dot(texture(terrainHorizon, vec3(uv, litTerrainSun.x)), litTerrainSel0);
    float h1 = dot(texture(terrainHorizon, vec3(uv, litTerrainSun.y)), litTerrainSel1);
    float horizonDeg = mix(-5.0, 45.0, mix(h0, h1, litTerrainSun.z));
    float lift = degrees(atan(max(hAbove, 0.0) / 900.0));
    float soft = litTerrainSun.w + 0.5;
    float vis = smoothstep(horizonDeg - soft, horizonDeg + soft, litSun.w + lift);
    return mix(1.0, vis, litTerrainLift.z);
}

// The cloud field's shadow on a point in the air (cloudSunVisibility without
// the world pass's flag: the dome's own clouds or the noise field).
float fogCloudSun(vec3 p) {
    if (litCloud.w < 0.5) return 1.0;
    if (litSky.y > 0.5) return mix(1.0, litCloud.y, domeCloudCover(p - litCamPos.xyz));
    float up = max(litSunDir.y, 0.12);
    vec2 q = p.xz + litSunDir.xz / up * (litCloudOffset.z - p.y);
    float n = texture(cloudNoise, (q + litCloudOffset.xy) * litCloud.z).r;
    float cover = clamp(litCloud.x, 0.0, 1.0);
    float edge = litCloudOffset.w * clamp(4.0 * cover * (1.0 - cover), 0.0, 1.0);
    float covered = edge > 1e-4 ? smoothstep(1.0 - cover - edge, 1.0 - cover + edge, n) : step(1.0 - cover, n);
    return mix(1.0, litCloud.y, covered);
}

// The sun's visibility at a camera-relative point (after fogMediumAt there):
// shadow cascades, canopy, hills and clouds.
float fogSunVisibility(vec3 rel) {
    vec3 p = rel + litCamPos.xyz;
    vWorldRel = rel;
    g_groundHeight = g_fogGround;
    return fogShadowMap(rel) * canopySunVisibility(p) * fogTerrainSun(p, g_fogAbove) * fogCloudSun(p);
}

// The sun's radiance as the air takes it (linear, exposed), 0 under the
// horizon.
vec3 fogSunRadiance() {
    return litSun.rgb * (litZenith.w * litSunDir.w * smoothstep(-0.02, 0.03, litSunDir.y));
}
)";

const char kPostFogInjectMainGLSL[] = R"(
float fogSliceDepth(float t) {
    return fogDepth.y < 1e-3 ? fogDepth.x * t : fogDepth.x * (exp(fogDepth.y * t) - 1.0) * fogDepth.z;
}

// This column's light in the air (main sets them once per column): the sky's
// own colour just above the land in this azimuth (the horizon ring, or the
// sky table's aerial perspective with its glow toward the sun where the ring
// has not seen the sky), so far ground melts into the sky behind it, and the
// share of it the sun gives (single scattering of the sun against the sky's
// light along this direction; the rest is the sky's). A cell in the shade of
// a tree, a ridge or a cloud keeps the sky's share.
vec3 g_air = vec3(0.0);
float g_sunShare = 0.0;
float g_sinSun = 1.0;
// The sun's forward lobe along this column (the glow toward a low sun): the
// air's luminance in the sun's tint times the lobe's excess over the
// isotropic level, on the clear haze and the dust, in the sun's shade gone.
vec3 g_lobe = vec3(0.0);

// One cell's medium and light at a camera-relative point: rgb the light it
// scatters toward the eye per metre, a its extinction per metre, both per km.
vec4 fogCell(vec3 rel) {
    FogMediumCell c = fogMediumAt(rel);
    float sMie = c.haze + c.dust + c.wet;
    float vis = 1.0;
    float tSun = 1.0;
    if (g_sunShare > 0.0 || g_lobe.g > 0.0) {
        vis = fogSunVisibility(rel);
        tSun = exp(-c.tauUp / g_sinSun);
    }
    // Air in the shade keeps a third of its light at least: the sky and the
    // light scattered more than once (a ridge's shadow in the haze stays
    // grey, not black).
    float share = min(g_sunShare, 0.65);
    float lit = (1.0 - share) + share * vis * tSun;
    // Dense mist and fog glow in their own shade: light scattered more than
    // once keeps a share of the sun there. The light is the sky's own colour
    // for every part (its blue is already the air's): with the volume's grey
    // extinction the far end converges to exactly that colour, never bluer
    // than the sky behind it.
    float sAir = dot(c.ray, kFogLuma);
    vec3 inscatter = g_air * ((c.haze + c.wet + sAir) * lit + c.dense * 0.35 * g_sunShare * tSun * (1.0 - vis)) +
                     dot(g_air, kFogLuma) * kDustTint * (c.dust * lit) + g_lobe * ((c.hazeLobe + c.dust) * vis * tSun);
    return vec4(inscatter, sMie + sAir) * 1000.0;
}

// The cell of slice s on this column: this frame's jittered point, blended with
// the last frame's volume at the same point.
vec4 fogSlice(float s, vec3 dirZ) {
    float z = fogSliceDepth((s + fogDepth.w) / fogGrid.z);
    vec3 rel = dirZ * z;
    vec4 cur = fogCell(rel);
    if (fogFrame.z < 1.0) {
        vec3 pv = vec3(dot(prevRow0.xyz, rel) + prevRow0.w, dot(prevRow1.xyz, rel) + prevRow1.w,
                       dot(prevRow2.xyz, rel) + prevRow2.w);
        if (pv.z > 0.05) {
            vec2 puv = pv.xy * prevProj.xy / pv.z * 0.5 + 0.5;
            float pt = prevProj.w < 1e-3 ? pv.z / prevProj.z
                                         : log(1.0 + pv.z * (exp(prevProj.w) - 1.0) / prevProj.z) / prevProj.w;
            if (puv.x >= 0.0 && puv.x <= 1.0 && puv.y >= 0.0 && puv.y <= 1.0 && pt <= 1.0)
                cur = mix(texture(fogHistory, vec3(puv, pt)), cur, fogFrame.z);
        }
    }
    return cur;
}

void main() {
    vec2 ndc = (gl_FragCoord.xy + fogFrame.xy) / fogGrid.xy * 2.0 - 1.0;
    vec3 dirZ = viewToWorld * vec3(ndc.x * projInfo.z, ndc.y * projInfo.w, 1.0); // per metre of view depth
    vec3 d = normalize(dirZ);
    // The air's light along this column: the colour the forward fog fades
    // to, the sky's horizon in this azimuth (fogColorToward, and the sky
    // table's aerial perspective while it draws), in linear light; at night
    // the night sky's own light (fogNightAir).
    vec3 classic = fogColorToward(d);
    vec3 disp = litAtmo.z > 0.0 ? atmoFogColor(d, classic) : classic;
    // Under the photoreal camera the model's light is scene light: the frame's
    // pre-exposure puts it in the frame's units (the ring below is measured
    // from the frame and is in them already).
    float camPre = litCamera.x > 0.5 ? litCamera.w : 1.0;
    g_air = mix(toLinear(disp), fogNightAir.rgb, fogNightAir.w) * camPre;
    // Where the ring has seen the sky in this azimuth, the sky's own colour:
    // far ground converges to exactly the sky behind it, day and night.
    // Nine taps over 22 degrees of azimuth: the sky's broad colour, not the
    // gaps between crowns.
    float az = atan(d.x, d.z) * (1.0 / 6.2831853);
    vec4 ring = vec4(0.0);
    for (int k = -4; k <= 4; ++k) {
        float w = 5.0 - abs(float(k));
        vec4 a = texture(fogRing, vec2(az + float(k) * (2.0 / 256.0), 0.5));
        ring += w * vec4(a.rgb * a.a, a.a);
    }
    ring /= 25.0;
    ring.rgb /= max(ring.a, 1e-4);
    // (Not at night: the lamps' heads and flares above the roofs would read
    // as sky there; the night sky is the model's dark colour.)
    g_air = mix(g_air, ring.rgb, clamp(ring.a, 0.0, 1.0) * (1.0 - fogNightAir.w)) * fogLightK.y;

    // The sun's share of it: the sun's single scattering (Mie phase) against
    // the sky's light (the rig's, a white surface under the sky).
    float exposure = litZenith.w * litSunDir.w;
    float sunLum = dot(fogSunRadiance(), kFogLuma);
    float skyLum = dot(0.5 * (litHorizon.rgb + litZenith.rgb), kFogLuma) * exposure;
    float sunAir = kFogPi * fogMiePhase(dot(d, litSunDir.xyz)) * sunLum;
    float k = max(fogLightK.x, 0.0);
    // The air's light is mostly scattered sunlight under a clear sky (the
    // horizon is bright because of it), more so toward the sun; under an
    // overcast it is the sky's diffuse light: shadowed air darkens in clear
    // weather (the shadows of ridges in the haze), barely under cloud.
    float sunUp = smoothstep(-0.02, 0.03, litSunDir.y);
    float physShare = sunAir / max(sunAir + skyLum, 1e-6);
    g_sunShare = sunUp * mix(0.15, 1.0, clamp(fogWeather.z, 0.0, 1.0)) * mix(0.6, 0.95, physShare) * min(k, 1.0);
    g_air *= 1.0 + max(k - 1.0, 0.0) * g_sunShare; // fogSun above 1: more of the sun in the air
    g_sinSun = max(litSunDir.y, 0.03);
    // The forward lobe: HG(0.6)'s excess over isotropic (2.3 toward the sun,
    // 0.6 at 30 degrees, none past 55) times the air's own luminance, in the
    // sun's colour, more of it under a clear sky.
    vec3 sunRad = fogSunRadiance();
    float sunL = dot(sunRad, kFogLuma);
    if (fogHaze.w > 0.0 && sunL > 1e-6) {
        float shape = kFogPi * max(fogHG(dot(d, litSunDir.xyz), 0.6) - kFogIso, 0.0);
        g_lobe = (sunRad / sunL) * (dot(g_air, kFogLuma) * fogHaze.w * shape *
                                    mix(0.15, 1.0, clamp(fogWeather.z, 0.0, 1.0)) * (1.0 - fogNightAir.w));
    }

    float base = fogGrid.w;
    fog0 = fogSlice(base + 0.0, dirZ);
    fog1 = fogSlice(base + 1.0, dirZ);
    fog2 = fogSlice(base + 2.0, dirZ);
    fog3 = fogSlice(base + 3.0, dirZ);
    fog4 = fogSlice(base + 4.0, dirZ);
    fog5 = fogSlice(base + 5.0, dirZ);
    fog6 = fogSlice(base + 6.0, dirZ);
    fog7 = fogSlice(base + 7.0, dirZ);
}
)";

// The integration: front to back along each column of the injected volume,
// eight slices of the integrated volume per draw (every draw walks from the
// eye: 36 reads per cell on average at 64 slices).
const char kPostFogIntegrateGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler3D fogInject; // unit 0: rgb in-scatter, a extinction (per km)
uniform vec4 fogGrid;        // xy cells, z slices, w the first slice of this draw
uniform vec4 fogDepth;       // x far (m), y k, z 1 / (exp(k) - 1)
uniform vec4 projInfo;       // z = 1/_11, w = 1/_22
layout(location = 0) out vec4 fog0;
layout(location = 1) out vec4 fog1;
layout(location = 2) out vec4 fog2;
layout(location = 3) out vec4 fog3;
layout(location = 4) out vec4 fog4;
layout(location = 5) out vec4 fog5;
layout(location = 6) out vec4 fog6;
layout(location = 7) out vec4 fog7;

float fogSliceDepth(float t) {
    return fogDepth.y < 1e-3 ? fogDepth.x * t : fogDepth.x * (exp(fogDepth.y * t) - 1.0) * fogDepth.z;
}

vec3 g_S = vec3(0.0); // light scattered toward the eye so far
float g_T = 1.0;      // transmittance from the eye
float g_len = 1.0;    // ray metres per metre of view depth on this column

void fogStep(ivec2 cell, int s) {
    float dt = (fogSliceDepth(float(s + 1) / fogGrid.z) - fogSliceDepth(float(s) / fogGrid.z)) * g_len;
    vec4 v = texelFetch(fogInject, ivec3(cell, s), 0) * 0.001;
    float st = max(v.a, 0.0);
    float x = st * dt;
    float tr = exp(-x);
    // The slice's own scattering, attenuated inside it: (1 - e^-x) / sigma.
    float through = x < 1e-3 ? dt * (1.0 - 0.5 * x) : (1.0 - tr) / st;
    g_S += g_T * max(v.rgb, vec3(0.0)) * through;
    g_T *= tr;
}

void main() {
    ivec2 cell = ivec2(gl_FragCoord.xy);
    vec2 ndc = (vec2(cell) + 0.5) / fogGrid.xy * 2.0 - 1.0;
    g_len = length(vec3(ndc.x * projInfo.z, ndc.y * projInfo.w, 1.0));
    int base = int(fogGrid.w);
    for (int s = 0; s < 128; ++s) {
        if (s >= base) break;
        fogStep(cell, s);
    }
    fogStep(cell, base + 0); fog0 = vec4(g_S, g_T);
    fogStep(cell, base + 1); fog1 = vec4(g_S, g_T);
    fogStep(cell, base + 2); fog2 = vec4(g_S, g_T);
    fogStep(cell, base + 3); fog3 = vec4(g_S, g_T);
    fogStep(cell, base + 4); fog4 = vec4(g_S, g_T);
    fogStep(cell, base + 5); fog5 = vec4(g_S, g_T);
    fogStep(cell, base + 6); fog6 = vec4(g_S, g_T);
    fogStep(cell, base + 7); fog7 = vec4(g_S, g_T);
}
)";

// The near-field pass's own inputs (its head: the #version line).
const char kPostFogNearHeadGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D linDepth;   // unit 0: half-resolution view depth (m; the sky 1e6)
uniform vec4 worldRect;       // the 3D viewport in uv
uniform vec4 fogNearGrid;     // xy this target's size, z march steps, w this frame's noise offset
uniform vec4 fogNearSun;      // rgb the beams' gain (times the sun's radiance), w 1 = march the beams
uniform vec4 fogNearCtl;      // x beam reach (m), y lights, z the far end for the sky (m)
uniform vec4 fogLightPos[16]; // xyz camera-relative, w reach in the air (m)
uniform vec4 fogLightCol[16]; // rgb intensity (linear, I in E = I / r^2), w 1 = a low beam
uniform vec4 fogLightDir[16]; // xyz the low beam's axis, w the air's scattering at the light (1/m)
out vec4 fragColor;
)";

// psFogNear's main: the sun's beams and the lamps' light in the air, per
// pixel at half resolution, linear (exposed like the shading).
const char kPostFogNearMainGLSL[] = R"(
// FrameLights.hpp's low beam (kLowBeam*; the shading's lowBeamPattern).
const float kAirBeamCutLow = -0.005;
const float kAirBeamCutHigh = 0.025;
const float kAirBeamHotBand = -0.03;
const float kAirBeamHotWidth = 0.05;
const float kAirBeamFloor = 0.02;
const float kAirBeamFloorEnd = -0.6;
const float kAirBeamFloorStart = -0.1;
const float kAirBeamCoreWidth = 0.25;
const float kAirBeamSpill = 0.15;
const float kAirBeamSpillWidth = 0.6;
// The headlight glass glows a little every way (its halo seen from outside
// the beam), a share of the peak.
const float kAirBulbGlow = 0.012;
// A light is not a point: the integral of I / (r^2 + kAirSoft) along the ray.
const float kAirSoft = 1.0;

float fogLowBeam(vec3 L, vec3 beam) {
    float c = dot(L, beam);
    if (c <= 0.0) return 0.0;
    vec3 right = vec3(beam.z, 0.0, -beam.x) * inversesqrt(max(beam.x * beam.x + beam.z * beam.z, 1e-12));
    vec3 u = cross(beam, right);
    float ah = atan(dot(L, right), c);
    float av = atan(dot(L, u), c);
    float cut = 1.0 - smoothstep(kAirBeamCutLow, kAirBeamCutHigh, av);
    float hv = (av - kAirBeamHotBand) / kAirBeamHotWidth;
    float vert = exp(-hv * hv) + kAirBeamFloor * smoothstep(kAirBeamFloorEnd, kAirBeamFloorStart, av);
    float h1 = ah / kAirBeamCoreWidth;
    float h2 = ah / kAirBeamSpillWidth;
    float horiz = exp(-h1 * h1) + kAirBeamSpill * exp(-h2 * h2);
    return vert * horiz * cut;
}

// One light's light scattered toward the eye along the ray d from the eye to
// tEnd: sigma * I(dir) * phase * e^-(path) / (r^2 + kAirSoft) integrated in
// the angle u seen from the light (s = b + h tan u, ds / (h^2 + (s - b)^2) =
// du / h), six points over the part of the ray inside the light's reach.
vec3 fogLampAir(int i, vec3 d, float tEnd) {
    vec3 L = fogLightPos[i].xyz;
    float reach = fogLightPos[i].w;
    float b = dot(L, d);
    vec3 perp = L - d * b;
    float h2raw = dot(perp, perp);
    float disc = reach * reach - h2raw;
    if (disc <= 0.0) return vec3(0.0);
    float sq = sqrt(disc);
    float s0 = max(b - sq, 0.0);
    float s1 = min(b + sq, tEnd);
    if (s1 <= s0) return vec3(0.0);
    float h = sqrt(h2raw + kAirSoft);
    float u0 = atan((s0 - b) / h);
    float u1 = atan((s1 - b) / h);
    bool beam = fogLightCol[i].w > 0.5;
    float sig = fogLightDir[i].w;
    float sum = 0.0;
    for (int k = 0; k < 6; ++k) {
        float u = mix(u0, u1, (float(k) + 0.5) / 6.0);
        float s = b + h * tan(u);
        vec3 v = d * s - L;
        float r = length(v);
        vec3 w = v / max(r, 1e-4); // the light's travel from the lamp
        // Scattered from w toward the eye (-d): forward-peaked.
        // Fog droplets scatter strongly forward: a lamp gets a tight aureole
        // and a broader glow round its head and little light every way (the
        // street's sky stays dark away from it); a headlight, seen mostly
        // from in front of its beam, the milder phase that keeps its car
        // readable.
        float cw = dot(w, -d);
        float ph = beam ? mix(fogHG(cw, 0.6), kFogIso, 0.3)
                        : 0.55 * fogHG(cw, 0.85) + 0.4 * fogHG(cw, 0.6) + 0.05 * kFogIso;
        float I = beam ? fogLowBeam(w, fogLightDir[i].xyz) + kAirBulbGlow : 1.0;
        float window = 1.0 - smoothstep(0.1 * reach * reach, reach * reach, r * r);
        sum += I * ph * window * exp(-sig * (s + r));
    }
    return fogLightCol[i].rgb * (sig * sum * (u1 - u0) / (6.0 * h));
}

void main() {
    float z = texture(linDepth, vUV).r;
    vec2 cuv = (vUV - worldRect.xy) / worldRect.zw;
    vec2 ndc = cuv * 2.0 - 1.0;
    vec3 dirZ = viewToWorld * vec3(ndc.x * projInfo.z, ndc.y * projInfo.w, 1.0);
    float len = length(dirZ);
    vec3 d = dirZ / len;
    float tEnd = z >= 1e5 ? fogNearCtl.z * len : z * len;
    vec3 S = vec3(0.0);
    // The sun's forward lobe in the wet air of the first metres: marched with
    // the sun's visibility at every step (more steps near the eye), the start
    // turned every frame by an interleaved-gradient offset that the TAA
    // history averages.
    if (fogNearSun.w > 0.5) {
        float tMax = min(tEnd, fogNearCtl.x);
        float n = fogNearGrid.z;
        float j = fract(52.9829189 * fract(dot(gl_FragCoord.xy + fogNearGrid.w * vec2(5.588238, 5.588238),
                                                 vec2(0.06711056, 0.00583715))));
        float T = 1.0;
        float acc = 0.0;
        float accAll = 0.0;
        float sinSun = max(litSunDir.y, 0.03);
        for (int i = 0; i < 32; ++i) {
            if (float(i) >= n) break;
            float a = (float(i) + j) / n;
            float t = tMax * a * a;
            float dt = tMax * (2.0 * a + 1.0 / n) / n;
            vec3 rel = d * t;
            FogMediumCell c = fogMediumAt(rel);
            if (c.wet > 1e-6) {
                float vis = fogSunVisibility(rel);
                float w = c.wet * exp(-c.tauUp / sinSun) * T * dt;
                acc += w * vis;
                accAll += w;
            }
            T *= exp(-(c.haze + c.dust + c.wet + c.ray.g) * dt);
        }
        float lobe = kFogPi * max(fogMiePhase(dot(d, litSunDir.xyz)) - kFogIso, 0.0);
        // The shafts read against the shade around them: the share of the
        // ray's wet air that sees the sun is lifted (to the power 0.4), so a
        // ray lit for a fifth of its way in a wood reads 2.6 times as bright,
        // while open air (all of it lit) and air all in shade keep theirs.
        float lit = accAll > 1e-9 ? acc / accAll : 0.0;
        vec3 beam = fogSunRadiance() * fogNearSun.rgb * (lobe * accAll * pow(lit, 0.4));
        // Rolled off toward the sun's own glow, so the lobe never whitens the
        // sky around the disc into a blob while the shafts stay linear.
        float bl = dot(beam, kFogLuma);
        S += beam / (1.0 + bl / 0.45);
    }
    // Lamps, headlights and fires.
    int lights = int(fogNearCtl.y);
    for (int i = 0; i < 16; ++i) {
        if (i >= lights) break;
        S += fogLampAir(i, d, tEnd);
    }
    // Under the photoreal camera the sun's and the lamps' light is scene
    // light: the frame's pre-exposure puts it in the frame's units.
    if (litCamera.x > 0.5) S *= litCamera.w;
    fragColor = vec4(S, 1.0);
}
)";

// The horizon ring: the sky's own colour just above the land in every
// azimuth, measured from the resolved frame (before the fog) and kept for
// the azimuths out of view. One row of kFogRingBins texels, u = the azimuth
// from north (clockwise) over a full turn: the sky a little above the
// silhouette (the lowest sky in that azimuth, the limit far ground melts
// into); rgb linear, a how sure (1 measured this frame, decaying while out of
// view; 0 never seen: the model's colour then).
const char kPostFogRingGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D colorTex;  // unit 0: the resolved frame (display)
uniform sampler2D ringPrev;  // unit 3: last frame's ring
uniform sampler2D depthTex;  // unit 5: the world depth
uniform vec4 ringView0;      // camera-relative world to view space, by rows
uniform vec4 ringView1;
uniform vec4 ringView2;
uniform vec4 ringProj;       // x _11, y _22, z 1 = start again (a camera jump), w this frame's share
uniform vec4 worldRect;      // the 3D viewport in uv
uniform vec4 ringCtl;        // x bins, y the decay per frame out of view, z how grey the sky is (0..1)
uniform vec4 ringSun;        // xyz the direction to the sun (world)
uniform vec4 fogCam;         // x 1 = the photoreal camera: the frame holds scene light (camDecode)
out vec4 fragColor;
)";

// The ring's main (after its inputs and the camera's codec).
const char kPostFogRingMainGLSL[] = R"(

// The frame's uv of a world direction, w 0 when it is off screen.
vec3 ringProject(vec3 d) {
    vec3 v = vec3(dot(ringView0.xyz, d), dot(ringView1.xyz, d), dot(ringView2.xyz, d));
    if (v.z < 0.05) return vec3(0.0);
    vec2 ndc = vec2(v.x * ringProj.x, v.y * ringProj.y) / v.z;
    if (abs(ndc.x) > 0.985 || abs(ndc.y) > 0.985) return vec3(0.0);
    return vec3(worldRect.xy + (ndc * 0.5 + 0.5) * worldRect.zw, 1.0);
}

vec3 ringDir(float az, float el) {
    return vec3(sin(az) * cos(el), sin(el), cos(az) * cos(el));
}

void main() {
    float az = (gl_FragCoord.x / ringCtl.x) * 6.2831853;
    vec4 prev = texelFetch(ringPrev, ivec2(gl_FragCoord.xy), 0);
    if (ringProj.z > 0.5) prev = vec4(0.0);
    // From 3 degrees under the horizon upward: the first sky in this azimuth.
    float found = -100.0;
    for (int i = 0; i < 70; ++i) {
        float el = radians(-3.0 + 0.5 * float(i));
        vec3 p = ringProject(ringDir(az, el));
        if (p.z < 0.5) continue;
        if (textureLod(depthTex, p.xy, 0.0).r >= 0.999999) {
            found = el;
            break;
        }
    }
    vec4 outc = vec4(prev.rgb, prev.a * ringCtl.y);
    if (found > -99.0) {
        // Six samples from two degrees up (the band above the land, the same
        // height in every azimuth), none within five degrees of the sun, the
        // mean of the darker half: the sky's colour, not the sun's disc and
        // glare, a star or a bright gap between crowns.
        // Under a grey sky (ringCtl.z) the samples start just above the land
        // and stay low: the weather's fill along the horizon is the air's
        // colour, and the darker deck above it made the rain's air a dark
        // slate that far hills faded into (look director round 2).
        float base = mix(max(found, radians(2.0)), max(found, 0.0) + radians(0.3), ringCtl.z);
        float stepDeg = mix(1.0, 0.4, ringCtl.z);
        vec4 s[3];
        for (int k = 0; k < 3; ++k) s[k] = vec4(0.0, 0.0, 0.0, 1e9);
        int nS = 0;
        vec3 sumAll = vec3(0.0);
        for (int k = 0; k < 6; ++k) {
            vec3 dd = ringDir(az, base + radians(0.6 * stepDeg + stepDeg * float(k)));
            if (dot(dd, ringSun.xyz) > 0.9962) continue;
            vec3 p = ringProject(dd);
            if (p.z < 0.5 || textureLod(depthTex, p.xy, 0.0).r < 0.999999) continue;
            vec3 t = textureLod(colorTex, p.xy, 0.0).rgb;
            vec3 c = fogCam.x > 0.5 ? camDecode(t) : pow(clamp(t, 0.0, 1.0), vec3(2.2));
            vec4 v = vec4(c, dot(c, vec3(0.2126, 0.7152, 0.0722)));
            sumAll += c;
            // Keep the three darkest, s[0] darkest.
            for (int j = 0; j < 3; ++j) {
                if (v.a < s[j].a) { vec4 t = s[j]; s[j] = v; v = t; }
            }
            ++nS;
        }
        vec3 best = vec3(-1.0);
        if (nS > 0) {
            int m = min(nS, 3);
            vec3 sum = s[0].rgb;
            if (m > 1) sum += s[1].rgb;
            if (m > 2) sum += s[2].rgb;
            best = mix(sum / float(m), sumAll / float(nS), ringCtl.z);
        }
        if (best.r >= 0.0)
            outc = vec4(prev.a > 0.0 ? mix(prev.rgb, best, ringProj.w) : best, 1.0);
    }
    fragColor = outc;
}
)";

// The night glow (psFogGlow): the frame's light blurred at a quarter and a
// sixteenth of the resolution, in linear light with the clipped highlights
// expanded back (a lamp's head is far brighter than white), alpha the
// luminance-weighted view depth of that light. The apply lays it into the
// fog in front of each pixel (to the nearer of the pixel and the light), so
// lamps, headlights, fires and the walls they light glow in the fog at night
// in their own colours and the sky away from them stays dark. Four modes:
// 0 the frame down to a quarter (a 4x4 mean), 1 a glow target down by four,
// 2 a nine-tap Gaussian along glowDir, 3 a quarter target plus a sixteenth
// one upsampled.
const char kPostFogGlowGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D srcTex;    // unit 0: the resolved frame (mode 0, display) or a glow target
uniform sampler2D src2Tex;   // unit 3: the sixteenth glow (mode 3)
uniform sampler2D depthTex;  // unit 5: the world depth (mode 0)
uniform vec4 glowCtl;        // x mode, y the far end (m), zw the source's texel size
uniform vec4 glowDir;        // xy the blur step (uv), z the sixteenth's share (mode 3)
uniform vec4 projInfo;       // x = _33, y = _43
uniform vec4 depthMap;       // ndcZ = depth * x + y
uniform vec4 fogCam;         // x 1 = the photoreal camera: the frame holds scene light (camDecode)
out vec4 fragColor;
)";

// The night glow's main (after its inputs and the camera's codec).
const char kPostFogGlowMainGLSL[] = R"(
vec3 glowExpand(vec3 lin) {
    float peak = max(lin.r, max(lin.g, lin.b));
    if (peak <= 0.8) return lin;
    float hdr = 0.6 + 0.04 / (1.0 - min(peak, 0.99));
    return lin * (hdr / peak);
}

vec4 glowFrame(vec2 uv) {
    vec3 t = textureLod(srcTex, uv, 0.0).rgb;
    vec3 c = fogCam.x > 0.5 ? camDecode(t) : glowExpand(pow(clamp(t, 0.0, 1.0), vec3(2.2)));
    float dz = textureLod(depthTex, uv, 0.0).r;
    float z = dz >= 0.999999 ? glowCtl.y : min(projInfo.y / ((dz * depthMap.x + depthMap.y) - projInfo.x), glowCtl.y);
    return vec4(c, dot(c, vec3(0.2126, 0.7152, 0.0722)) * z);
}

void main() {
    int mode = int(glowCtl.x + 0.5);
    vec2 t = glowCtl.zw;
    if (mode <= 1) {
        // Four bilinear taps, each the mean of a 2x2 block: a 4x4 mean.
        vec4 s = vec4(0.0);
        for (int k = 0; k < 4; ++k) {
            vec2 o = vec2(float(k & 1) * 2.0 - 1.0, float(k >> 1) * 2.0 - 1.0) * t;
            s += mode == 0 ? glowFrame(vUV + o) : textureLod(srcTex, vUV + o, 0.0);
        }
        fragColor = s * 0.25;
    } else if (mode == 2) {
        const float w[5] = float[5](0.2270270, 0.1945946, 0.1216216, 0.0540541, 0.0162162);
        vec4 s = textureLod(srcTex, vUV, 0.0) * w[0];
        for (int k = 1; k < 5; ++k) {
            vec2 o = glowDir.xy * float(k);
            s += (textureLod(srcTex, vUV + o, 0.0) + textureLod(srcTex, vUV - o, 0.0)) * w[k];
        }
        fragColor = s;
    } else {
        fragColor = mix(textureLod(srcTex, vUV, 0.0), textureLod(src2Tex, vUV, 0.0), glowDir.z);
    }
}
)";

// The apply: full resolution, the frame after the occlusion pass's inputs.
// Each pixel's view depth (the sky at the volume's far end) is looked up in
// the integrated volume (texel s holds the light and transmittance up to the
// far face of slice s, so the lookup sits half a texel back) and the pixel's
// linear colour, its highlights expanded back through the world's shoulder,
// becomes C T + S, plus the near-field light and the night glow; then the
// shoulder again. Alpha keeps the sky-light share the composite's occlusion
// reads, minus what the fog veils.
//
// A pixel in a small gap of nearer geometry (alpha-tested foliage, a trunk
// row, a fence) takes the fog of the geometry round it, not of the deeper
// scene showing through: the gap is fogged as the surface it is a hole in.
// The sky under the horizon (past the world's far end, where the dome's flat
// fill shows) is a far sea or plain at the distance the ray reaches sea
// level, darker than the sky and broken into patches, the clear haze laid
// over it up to there: brighter and paler toward the horizon line.
const char kPostFogApplyGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D colorTex;  // unit 0: the resolved frame (display space; alpha 1 - sky light share)
uniform sampler2D glowTex;   // unit 1: the night glow (psFogGlow; linear rgb, a = luminance x depth)
uniform sampler3D fogVolume; // unit 2: rgb light scattered toward the eye, a transmittance
uniform sampler2D nearTex;   // unit 3: the near-field light (half resolution, linear)
uniform sampler2D linDepth;  // unit 4: half-resolution view depth (m; the sky 1e6)
uniform sampler2D depthTex;  // unit 5: the world depth
uniform vec4 projInfo;       // x = _33, y = _43 (viewZ = y / (ndcZ - x)), z = 1/_11, w = 1/_22
uniform vec4 depthMap;       // ndcZ = depth * x + y
uniform vec4 worldRect;      // the 3D viewport in uv
uniform vec4 fogGrid;        // xy cells, z slices
uniform vec4 fogDepth;       // x far (m), y k
uniform vec4 fogApply;       // x 1 = dither, y 1 = show the fog's light alone (debug), z the sky's fade (sin of elevation), w 1 = near-field light
uniform vec4 fogNearSize;    // xy the near target's size, z 1 = an optic overlay over the world this frame,
                             // w 1 = worldTex holds the world before it
uniform sampler2D worldTex;  // unit 6: the world before this frame's screen overlay (SnapshotWorldBeforeOverlay)
uniform vec4 fogSkyEye;      // x the eye's height (m), y the fog floor under it (m), z the wet air at the eye (1/m), w its depth (m)
uniform vec4 fogSkyWeather;  // x weather fog (1/m at the floor), y its scale height (m)
uniform vec4 fogSkyClear;    // x haze (1/m at sea level), y its scale height, z Rayleigh (green, 1/m at sea level), w dust (1/m at 0 m)
uniform vec4 fogSkyDust;     // x dust's scale height (m), yz the eye's world x, z (m), w the ground under the eye (m)
uniform vec4 fogGlow;        // x the night glow's gain (0 = none)
uniform vec4 fogSkyLobe;     // x the sun's forward lobe in the clear air past the far end (0 = none)
uniform mat3 viewToWorld;    // view-space direction to world (camera-relative)
out vec4 fragColor;
)";

// The apply's main (after its inputs and the LitConstants block: the sun's
// direction and colour for the glow past the far end).
const char kPostFogApplyMainGLSL[] = R"(
const vec3 kApplyLuma = vec3(0.2126, 0.7152, 0.0722);
const vec3 kApplyDustTint = vec3(1.10, 1.0, 0.82); // kDustTint
// Past the far end the clear air goes on this far along a low ray (m).
const float kApplyBeyond = 40000.0;
// The sun's glow past the far end fades above the horizon with
// exp(-sin(elevation) / this): 2.9 degrees.
const float kApplyLobeFade = 0.05;

// The world's per-fragment shoulder above 0.8 (tonemapNeutral's peak curve,
// without its whitening, which the frame already holds) and its inverse: the
// frame arrives through it, the fog is composited before it, so a pixel the
// fog leaves alone comes out as it went in and a bright sky or sun behind
// thin fog keeps its range. The inverse stops at a peak of 0.97 (a clipped
// sun path on the sea is not eight times white behind the mist); what the
// cap takes off is given back in proportion to what the fog leaves.
vec3 fogShoulder(vec3 c) {
    float peak = max(c.r, max(c.g, c.b));
    if (peak < 0.8) return c;
    return c * ((1.0 - 0.04 / (peak - 0.6)) / peak);
}
vec3 fogExpand(vec3 lin) {
    float peak = max(lin.r, max(lin.g, lin.b));
    if (peak <= 0.8) return lin;
    float hdr = 0.6 + 0.04 / (1.0 - min(peak, 0.97));
    return lin * (hdr / peak);
}

// Optical depth of an exponential layer along a ray from the eye: sigma0 at
// the eye, scale height H, sine of the elevation s, length L.
float fogLayerTau(float sigma0, float H, float s, float L) {
    float x = clamp(L * s / H, -30.0, 30.0);
    if (abs(x) < 1e-3) return sigma0 * L;
    return sigma0 * H * (1.0 - exp(-x)) / s;
}

float fogViewZ(vec2 uv) {
    float d = textureLod(depthTex, uv, 0.0).r;
    return d >= 0.999999 ? 1e6 : projInfo.y / ((d * depthMap.x + depthMap.y) - projInfo.x);
}

// A gap in nearer geometry: the further of the two nearest depths on either
// side, across and down (a morphological closing over 4.5 px each way: only
// the small holes of alpha-tested foliage and trunk rows; a wider window
// stamped square notches of near depth onto fogged silhouettes), when both
// sides are markedly nearer than the pixel; else the pixel's own depth.
float fogGapZ(float z) {
    vec2 px = 1.0 / vec2(textureSize(depthTex, 0));
    float l = 1e7, r = 1e7, u = 1e7, dn = 1e7;
    for (int i = 0; i < 3; ++i) {
        float o = 1.5 + 1.5 * float(i);
        l = min(l, fogViewZ(vUV - vec2(o * px.x, 0.0)));
        r = min(r, fogViewZ(vUV + vec2(o * px.x, 0.0)));
        u = min(u, fogViewZ(vUV + vec2(0.0, o * px.y)));
        dn = min(dn, fogViewZ(vUV - vec2(0.0, o * px.y)));
    }
    float zc = min(max(l, r), max(u, dn));
    return zc < 0.8 * z ? zc : z;
}

// The volume at a view depth: S and T (nearer than the first slice's far
// face, a share of it).
vec4 fogAt(vec2 cuv, float z) {
    float t = fogDepth.y < 1e-3 ? z / fogDepth.x : log(1.0 + max(z, 0.0) * (exp(fogDepth.y) - 1.0) / fogDepth.x) / fogDepth.y;
    float f = clamp(t, 0.0, 1.0) * fogGrid.z;
    vec4 v = textureLod(fogVolume, vec3(cuv, (f - 0.5) / fogGrid.z), 0.0);
    float k0 = clamp(f, 0.0, 1.0);
    return vec4(max(v.rgb, vec3(0.0)) * k0, mix(1.0, clamp(v.a, 0.0, 1.0), k0));
}

// The near-field light at this pixel: the four half-resolution texels around
// it, weighted by how close their depth is to the pixel's.
vec3 fogNear(float z) {
    vec2 hp = vUV * fogNearSize.xy - 0.5;
    vec2 f = fract(hp);
    vec2 base = (floor(hp) + 0.5) / fogNearSize.xy;
    vec2 px = 1.0 / fogNearSize.xy;
    vec3 sum = vec3(0.0);
    float wsum = 0.0;
    vec3 best = vec3(0.0);
    float bestDz = 1e9;
    for (int k = 0; k < 4; ++k) {
        vec2 o = vec2(float(k & 1), float(k >> 1));
        vec2 uv = base + o * px;
        float zt = textureLod(linDepth, uv, 0.0).r;
        vec3 s = textureLod(nearTex, uv, 0.0).rgb;
        float wb = (o.x > 0.5 ? f.x : 1.0 - f.x) * (o.y > 0.5 ? f.y : 1.0 - f.y);
        float dz = abs(min(zt, 1e5) - min(z, 1e5));
        float wz = 1.0 / (1.0 + dz / (0.05 * min(z, 1e5) + 0.3));
        sum += s * wb * wz;
        wsum += wb * wz;
        if (dz < bestDz) { bestDz = dz; best = s; }
    }
    return wsum > 1e-3 ? sum / wsum : best;
}

// Value noise (a hash lattice, smooth cells) for the far surface's patches.
float fogHash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
float fogNoise(vec2 p) {
    vec2 i = floor(p), f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(fogHash(i), fogHash(i + vec2(1.0, 0.0)), f.x),
               mix(fogHash(i + vec2(0.0, 1.0)), fogHash(i + vec2(1.0, 1.0)), f.x), f.y);
}

void main() {
    vec4 cOver = texture(colorTex, vUV);
    // An optic overlay (drawn over the world without depth): with the world
    // as it was before it (worldTex), the fog applies to that world and the
    // overlay's darkening is laid back over the result (a rifle's blurred
    // iron sight keeps its soft edge); without it, the overlay's black mask
    // and reticle stay as drawn.
    bool snap = fogNearSize.w > 0.5;
    vec4 c = snap ? texture(worldTex, vUV) : cOver;
    if (!snap && fogNearSize.z > 0.5 && max(c.r, max(c.g, c.b)) < 0.012) {
        fragColor = c;
        return;
    }
    float zRaw = fogViewZ(vUV);
    float zg = fogGapZ(zRaw);
    bool sky = zg >= 1e5;
    float z = sky ? fogDepth.x : zg;
    vec2 cuv = (vUV - worldRect.xy) / worldRect.zw;
    vec4 v = fogAt(cuv, z);
    vec3 S = v.rgb;
    float T = v.a;
    // The air's own colour at this depth (what a long column of it shows).
    vec3 airAt = v.rgb / max(1.0 - v.a, 1e-3);
    bool airValid = 1.0 - v.a > 1e-3;
    // Under the photoreal camera (litCamera.x) the frame holds pre-exposed
    // scene light on the camera's encoding: the fog works in that light (the
    // model's sky and sun, scene light, take the frame's pre-exposure
    // litCamera.w in the passes before) and stores the result the same way,
    // with no shoulder of its own (the camera's curve comes after).
    bool cam = litCamera.x > 0.5;
    vec3 lin0 = cam ? camDecode(c.rgb) : pow(clamp(c.rgb, 0.0, 1.0), vec3(2.2));
    vec3 base = lin0;
    // The sky (and the far horizon polygon): the dome already holds the clear
    // atmosphere above the horizon, so it takes the clear haze and air only
    // near the horizon (fading over a few degrees above it); the weather fog,
    // the mist and the humid air are not in it, so it takes their share whole
    // and what of them lies beyond the far plane. Stars and the moon fade with
    // them, and the sky behind fogged roofs is never darker than the air.
    if (sky || z > 0.95 * fogDepth.x) {
        vec2 ndc = cuv * 2.0 - 1.0;
        vec3 dw = normalize(viewToWorld * vec3(ndc.x * projInfo.z, ndc.y * projInfo.w, 1.0));
        float L = fogDepth.x * length(vec3(ndc.x * projInfo.z, ndc.y * projInfo.w, 1.0));
        float s = dw.y;
        float y0 = fogSkyEye.x;
        float hd = max(fogSkyDust.x, 1.0);
        float tauW = fogLayerTau(fogSkyWeather.x * exp(-max(y0 - fogSkyEye.y, 0.0) / max(fogSkyWeather.y, 1.0)),
                                 max(fogSkyWeather.y, 1.0), s, L);
        float tauLow = fogLayerTau(fogSkyEye.z, max(fogSkyEye.w, 1.0), s, L);
        float tauClear = fogLayerTau(fogSkyClear.x * exp(-max(y0, -50.0) / fogSkyClear.y), fogSkyClear.y, s, L) +
                         fogLayerTau(fogSkyClear.w * exp(-max(y0 - fogSkyDust.w, 0.0) / hd), hd, s, L) +
                         fogLayerTau(fogSkyClear.z * exp(-max(y0, 0.0) / 8000.0), 8000.0, s, L);
        float wet = (tauW + tauLow) / max(tauW + tauLow + tauClear, 1e-9);
        float w = mix(exp(-max(s, 0.0) / fogApply.z), 1.0, wet);
        vec3 airC = S / max(1.0 - T, 1e-3);
        float airL = 1.0 - T > 1e-3 ? dot(airC, kApplyLuma) : dot(lin0, kApplyLuma);
        // The clear air past the far end, out to sea level under the horizon
        // or kApplyBeyond over it: the dome holds its grey and blue, not the
        // sun's forward glow nor the dust, which are laid on here.
        float yE = y0 + s * L;
        // The sky under the horizon and the far horizon polygon at the far
        // plane under it (the world's flat fill past its end).
        bool under = s < 0.0 && (sky || z > 0.98 * fogDepth.x);
        float Lb = kApplyBeyond;
        float n = 0.5;
        if (under) {
            float Lg = clamp(max(y0, 2.0) / max(-s, 1e-5), L, 2e5);
            Lb = Lg - L;
            vec2 hit = fogSkyDust.yz + dw.xz * Lg;
            n = 0.6 * fogNoise(hit * (1.0 / 1800.0)) + 0.4 * fogNoise(hit * (1.0 / 520.0) + 7.3);
        }
        float tauHb = fogLayerTau(fogSkyClear.x * exp(-max(yE, -50.0) / fogSkyClear.y), fogSkyClear.y, s, Lb);
        float tauDb = fogLayerTau(fogSkyClear.w * exp(-max(yE - fogSkyDust.w, 0.0) / hd), hd, s, Lb);
        if (under) {
            // Under the horizon: a far surface where the ray meets sea level,
            // darker than the dome's fill and in patches, fading into the
            // fill (the horizon's colour) with the haze, the air and the
            // weather fog from the far end out to it, so the band brightens
            // and pales toward the horizon line instead of standing as one
            // flat colour, and meets the sky above it without a seam.
            float yMid = 0.5 * max(yE, 0.0);
            float sig = fogSkyClear.x * exp(-yMid / fogSkyClear.y) + fogSkyClear.z * exp(-yMid / 8000.0) +
                        fogSkyWeather.x * exp(-max(yMid - fogSkyEye.y, 0.0) / max(fogSkyWeather.y, 1.0));
            // (The air over the open sea, far from the land's dust and
            // moisture, is a third as thick; patches of it thicker and
            // thinner give the band its structure.)
            float rest = 1.0 - exp(-sig * Lb * mix(0.5, 0.9, n));
            // Only under a clear sky: in rain or fog the air past the far end
            // is the fill's own grey (a darker plate there read as a wall).
            float wetSky = clamp(fogSkyWeather.x * 2000.0, 0.0, 1.0);
            // (look director round 2: a lighter surface and thicker air over
            // it: the darker band read as a murky brown strip under a dawn
            // sky; the far air at the horizon is as bright as the sky.)
            base = lin0 * mix(mix(vec3(0.62, 0.66, 0.72), vec3(0.80, 0.83, 0.88), n), vec3(1.0), max(rest, wetSky));
        }
        if (!(under)) {
            S *= w;
            T = mix(1.0, T, w);
            // Beyond the far plane the weather fog goes on: the rest of its
            // layer along the ray, in the colour of the air this column
            // already holds.
            float yEnd = y0 + L * s;
            float sigEnd = fogSkyWeather.x * exp(-max(yEnd - fogSkyEye.y, 0.0) / max(fogSkyWeather.y, 1.0));
            float tauB = sigEnd * fogSkyWeather.y / max(s, 0.03);
            float rest = 1.0 - exp(-tauB);
            S += T * airC * rest * wet;
            T *= 1.0 - rest * wet;
        }
        if ((sky || under) && fogSkyLobe.x > 0.0) {
            vec3 sunRad = litSun.rgb * smoothstep(-0.02, 0.03, litSunDir.y);
            float sunL = dot(sunRad, kApplyLuma);
            if (sunL > 1e-6) {
                float g2 = 0.49;
                float ph = (1.0 - g2) / (12.566371 * pow(max(1.0 + g2 - 1.4 * dot(dw, litSunDir.xyz), 1e-4), 1.5));
                float shape = 3.14159265 * max(ph - 0.0795775, 0.0);
                // Only in the low air: the dome holds its own aureole above
                // (the glow lies along the horizon, not round the disc).
                float low = exp(-max(s, 0.0) / kApplyLobeFade);
                S += T * (sunRad / sunL) * (airL * fogSkyLobe.x * shape * low * (1.0 - exp(-(tauHb + tauDb))));
            }
        }
        if ((sky || under) && tauDb > 0.0) {
            float rd = 1.0 - exp(-tauDb);
            S += T * airL * kApplyDustTint * mix(vec3(1.0), base / max(dot(base, kApplyLuma), 1e-4), 0.5) * rd;
            T *= 1.0 - rd;
        }
    }
    // In rain and weather fog the dome's own fill under and along the horizon
    // already is the weather's air (the engine draws it in the fog colour):
    // the volume's darker air laid over it read as a grey wall beyond the
    // land. The sky keeps its fill there; the land still fogs.
    if (sky) {
        float wetSky = clamp(fogSkyWeather.x * 2000.0, 0.0, 1.0);
        S *= 1.0 - wetSky;
        T = mix(T, 1.0, wetSky);
        // The deck above the fill sinks into the weather's air toward the
        // horizon (its height fade, about 5 degrees): the fill's flat band
        // ended in a hard edge under the clouds.
        if (wetSky > 0.0 && airValid) {
            vec2 ndcW = cuv * 2.0 - 1.0;
            float sW = normalize(viewToWorld * vec3(ndcW.x * projInfo.z, ndcW.y * projInfo.w, 1.0)).y;
            float hb = exp(-max(sW, 0.0) / 0.09);
            base = mix(base, airAt, 0.9 * hb * wetSky);
        }
    }
    if (fogApply.w > 0.5) S += fogNear(sky ? 1e6 : z);
    // The night glow: the blurred light of the frame, laid into the fog in
    // front of the nearer of this pixel and the light's own depth.
    if (fogGlow.x > 0.0) {
        vec4 g = textureLod(glowTex, vUV, 0.0);
        float lg = dot(g.rgb, kApplyLuma);
        if (lg > 1e-6) {
            float zs = min(g.a / lg, z);
            S += g.rgb * ((1.0 - fogAt(cuv, zs).a) * fogGlow.x);
        }
    }
    // The highlights expanded back as far as the fog leaves them; the cap's
    // share given back where the fog leaves the pixel (identity unfogged).
    vec3 col;
    if (cam) {
        vec3 outLin = fogApply.y > 0.5 ? S : base * T + S;
        if (snap) {
            // The overlay's darkening (its share of the world it left), and
            // whatever it drew brighter than the world (a reticle, the
            // target brackets) added back as drawn.
            vec3 over = camDecode(cOver.rgb);
            outLin = outLin * clamp(over / max(lin0, vec3(1e-5)), 0.0, 1.0) + max(over - lin0, vec3(0.0));
        }
        col = camFold(outLin);
    } else {
        vec3 e = fogExpand(base);
        vec3 lin = mix(base, e, T);
        vec3 outLin = fogApply.y > 0.5 ? S : fogShoulder(lin * T + S) + (base - fogShoulder(e)) * T;
        col = pow(max(outLin, vec3(0.0)), vec3(1.0 / 2.2));
    }
    if (fogApply.x > 0.5) {
        float n = fract(52.9829189 * fract(dot(gl_FragCoord.xy, vec2(0.06711056, 0.00583715))));
        float m = fract(52.9829189 * fract(dot(gl_FragCoord.yx + 17.0, vec2(0.06711056, 0.00583715))));
        col += (n + m - 1.0) / 255.0; // triangular dither, +-1 LSB
    }
    fragColor = vec4(clamp(col, 0.0, 1.0), 1.0 - (1.0 - clamp(c.a, 0.0, 1.0)) * T);
}
)";

} // namespace Poseidon::gl33lighting
