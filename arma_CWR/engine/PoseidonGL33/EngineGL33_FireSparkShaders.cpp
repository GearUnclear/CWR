// The fire spread's sparks (POSEIDON_XP_FIRESPREAD; World/Simulation/
// FireSparks.hpp): their shaders. EngineGL33_FireSparks.cpp draws them; kept
// in a file of their own (no EngineGL33 members here) so the shader tests can
// read them alone.
//
// One instance a spark, a quad built on screen round the streak a camera's
// shutter (kSparkShutter) sees it draw: from where it was half the shutter
// ago to where it will be half the shutter on, at least a pixel's blur
// across. Its light is a glowing body's (a black body at its temperature,
// its luminance anchored on Planck's slope: kSparkNitsRef at kSparkRefK) over
// its own projected area, spread over the streak as a Gaussian of at least
// kSparkMinSigmaPx and normalized, so a far spark keeps its light as it
// shrinks under a pixel and a fast one dims as it smears, as on film. The
// light is only ever added (the linear flame target's TransmittanceAccumulate
// with a = 1, or added into the coded frame without the target): nothing
// darkens what is behind a spark, so by day a spark against the sky is a
// faint bright speck or nothing, never a dark dot. Its depth goes into the
// flame target's depth weight as a flame's does, so the merge cuts a spark
// behind a trunk at the full resolution and the fog takes it at its depth.

#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <string>

namespace Poseidon::gl33lighting
{

namespace
{

const char kFireSparkVertexGLSL[] = R"(
// The fire spread's sparks (EngineGL33_FireSparks.cpp): a quad a spark round
// its streak, built in the target's pixels (4 vertices, a triangle strip).
uniform mat4 sparkView;   // the world's view (rotation only: positions are camera-relative)
uniform mat4 sparkProj;   // the world's projection
uniform vec4 sparkCam;    // xy the TAA jitter (NDC), z fog start (m), w fog 1 / range (0 = no fog)
uniform vec4 sparkTarget; // xy the target's size (pixels), z the shutter (s), w the blur's least sigma (pixels)
uniform vec4 sparkGlare;  // x at night the least size a far spark is drawn (m; 0 by day), from y to z metres away

layout(location = 0) in vec4 aPos;   // xyz the spark, camera-relative (m), w its size across (m)
layout(location = 1) in vec4 aVel;   // xyz its velocity (m/s), w its glow's temperature (K)
layout(location = 2) in vec4 aLight; // x its light's weight, y the area it is spread over (a multiple of its own, >= 1)

noperspective out vec2 vLocal; // the pixel from the streak's start: along it, across it (pixels)
flat out vec4 vStreak;         // x the streak's length (pixels), y the blur's sigma (pixels), z its light (the
                               // area its body covers over the profile's integral), w its view depth (m)
flat out float vKelvin;
flat out float vFogKeep;       // the share of it the classic fog keeps (1 = no fog)

const float kSparkNearW = 0.05;   // a streak end nearer than this (m) is cut there
const float kSparkReach = 3.0;    // the quad reaches this many sigmas past the streak

void main() {
    vec3 half_ = aVel.xyz * (0.5 * sparkTarget.z);
    vec4 c0 = sparkProj * (sparkView * vec4(aPos.xyz - half_, 1.0));
    vec4 c1 = sparkProj * (sparkView * vec4(aPos.xyz + half_, 1.0));
    // behind the eye: none; one end behind it: cut at the near distance
    if (c0.w < kSparkNearW && c1.w < kSparkNearW) {
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
        vLocal = vec2(0.0);
        vStreak = vec4(0.0);
        vKelvin = 0.0;
        vFogKeep = 0.0;
        return;
    }
    if (c0.w < kSparkNearW) c0 = mix(c0, c1, (kSparkNearW - c0.w) / (c1.w - c0.w));
    if (c1.w < kSparkNearW) c1 = mix(c1, c0, (kSparkNearW - c1.w) / (c0.w - c1.w));
    vec2 s0 = (c0.xy / c0.w * 0.5 + 0.5) * sparkTarget.xy;
    vec2 s1 = (c1.xy / c1.w * 0.5 + 0.5) * sparkTarget.xy;
    float wMid = 0.5 * (c0.w + c1.w);
    // its body's radius on screen (pixels: at night a far one at least the
    // glare's size), and the blur's (its LOD area spread, at least the least sigma)
    float pxPerM = sparkProj[1][1] * 0.5 * sparkTarget.y / max(wMid, kSparkNearW);
    float dist = length(aPos.xyz);
    float size = max(aPos.w, sparkGlare.x * smoothstep(sparkGlare.y, sparkGlare.z, dist));
    float r = 0.5 * size * pxPerM;
    float sigma = max(0.6 * r * sqrt(max(aLight.y, 1.0)), sparkTarget.w);
    vec2 d = s1 - s0;
    float len = length(d);
    vec2 dir = len > 1e-4 ? d / len : vec2(1.0, 0.0);
    vec2 across = vec2(-dir.y, dir.x);
    float u = (gl_VertexID & 1) == 0 ? -kSparkReach * sigma : len + kSparkReach * sigma;
    float v = (gl_VertexID & 2) == 0 ? -kSparkReach * sigma : kSparkReach * sigma;
    vec2 sp = s0 + dir * u + across * v;
    vec4 cMid = mix(c0, c1, 0.5);
    gl_Position = vec4(sp / sparkTarget.xy * 2.0 - 1.0 + sparkCam.xy, clamp(cMid.z / cMid.w, -1.0, 1.0), 1.0);
    vLocal = vec2(u, v);
    // the light: its body's area over the Gaussian streak's integral
    float area = 3.14159265 * r * r;
    float integral = len * 2.50662827 * sigma + 6.28318531 * sigma * sigma;
    vStreak = vec4(len, sigma, max(aLight.x, 0.0) * area / integral, wMid);
    vKelvin = aVel.w;
    float nearKeep = smoothstep(0.25, 0.8, dist);
    vFogKeep = (sparkCam.w > 0.0 ? clamp(1.0 - (dist - sparkCam.z) * sparkCam.w, 0.0, 1.0) : 1.0) * nearKeep;
}
)";

const char kFireSparkFragmentGLSL[] = R"(
uniform sampler2D sparkDepth; // the world's depth (the flames' copy), read while sparkDepthMap.y is not 0
uniform vec4 sparkDepthMap;   // x the projection's _33, y its _43 (view z = y / (ndc z - x)), ndc z = depth z + w
// x 0: into the linear flame target (rgb the light added at the frame's
// pre-exposure; the post pass merges it), 1: straight into the frame, coded;
// y the depth copy's texels per target pixel across (2: the linear target is
// half the frame's size); z the flames' own exposure over the camera's (log2:
// the sparks are exposed as the flames are: EngineGL33::FireFlameExposureLog2,
// fix 1).
uniform vec4 sparkOut;

noperspective in vec2 vLocal;
flat in vec4 vStreak;
flat in float vKelvin;
flat in float vFogKeep;

layout(location = 0) out vec4 fragColor;
// Into the linear target, its depth target (as the flames': r the view
// depth (km) times its weight, g the weight); straight into the frame, the
// surface target, untouched.
layout(location = 1) out vec4 fragSurf;

// ---- Calibration ----
// Scene light of 1 cd/m2 (LightRig.cpp ComputePhysicalRig: a white surface
// under the reference sun is 1.0, about 32000 cd/m2; the flames' same scale).
const float kSparkNitToScene = 1.0 / 32000.0;
// A glowing body's luminance (cd/m2) by its temperature K: kSparkNitsRef at
// kSparkRefK, falling as exp(kSparkSlopeK (1 / kSparkRefK - 1 / K)) (Planck's
// slope in the visible, c2 over 0.58 um), times its emissivity: a 1 cm ember
// at 1300 K about 0.03 cd, a flaming fragment at 1550 K several times that
// (the consult: 0.01 to 0.05 cd, and about 0.5 for a flaming fragment's own
// flame), a dying one at 1050 K a dull red glow.
const float kSparkRefK = 1500.0;
const float kSparkNitsRef = 7400.0;
const float kSparkSlopeK = 24800.0;
const float kSparkEmissivity = 0.9;
const float kSparkGlow = 1.0;          // the light's scale (1: as calibrated)
const float kSparkSoftDepth = 0.15;    // it fades out over this much in front of what is behind it (m)
const float kSparkDepthLight = 4.0;    // the depth weight: this times its light (scene light), at most 1
const vec3 kSparkLuma = vec3(0.2126, 0.7152, 0.0722);

// The linear colour of a black body at unit luminance (CombatLights.cpp
// BlackBodyRgb's fit, as the flames' flameBlackBody).
vec3 sparkBlackBodyColour(float k) {
    float t = clamp(k, 1000.0, 3000.0);
    vec3 c = vec3(1.0, max(0.155 + (t - 1500.0) * 0.000207, 0.05),
                  0.15 * pow(clamp((t - 1900.0) / 1100.0, 0.0, 1.0), 1.5));
    return c / dot(c, kSparkLuma);
}

float sparkViewZ(float d) {
    return sparkDepthMap.y / ((d * sparkDepthMap.z + sparkDepthMap.w) - sparkDepthMap.x);
}

void main() {
    float len = vStreak.x, sigma = vStreak.y;
    // the pixel's distance from the streak (a capsule), and its Gaussian
    float du = vLocal.x < 0.0 ? vLocal.x : (vLocal.x > len ? vLocal.x - len : 0.0);
    float d2 = du * du + vLocal.y * vLocal.y;
    float g = exp(-0.5 * d2 / (sigma * sigma));
    if (g < 0.01) discard;
    // its depth test against the world's (half resolution: the farthest of
    // the four texels under the pixel, so it shows wherever any of them does;
    // the merge cuts it at the full resolution)
    float soft = 1.0;
    if (sparkDepthMap.y != 0.0) {
        float d;
        if (sparkOut.y > 1.5) {
            ivec2 p = ivec2(gl_FragCoord.xy) * 2;
            ivec2 lim = textureSize(sparkDepth, 0) - 1;
            d = max(max(texelFetch(sparkDepth, min(p, lim), 0).r, texelFetch(sparkDepth, min(p + ivec2(1, 0), lim), 0).r),
                    max(texelFetch(sparkDepth, min(p + ivec2(0, 1), lim), 0).r,
                        texelFetch(sparkDepth, min(p + ivec2(1, 1), lim), 0).r));
        } else {
            d = texelFetch(sparkDepth, ivec2(gl_FragCoord.xy), 0).r;
        }
        soft = smoothstep(0.0, kSparkSoftDepth, sparkViewZ(d) - vStreak.w);
    }
    float fog = clamp(min(vFogKeep * (1.0 + litHaze.x), 1.0), 0.0, 1.0);
    float keep = g * soft * fog * vStreak.z;
    if (keep <= 0.0) discard;
    float nits = kSparkNitsRef * kSparkEmissivity * exp(kSparkSlopeK * (1.0 / kSparkRefK - 1.0 / max(vKelvin, 500.0)));
    float scale = litZenith.w * (litCamera.x > 0.5 ? max(litCamera.w, 0.0) : 1.0);
    vec3 light = sparkBlackBodyColour(vKelvin) * (nits * kSparkNitToScene * kSparkGlow * scale * keep * exp2(sparkOut.z));
    if (max(light.r, max(light.g, light.b)) < 1e-6) discard;
    if (sparkOut.x > 0.5) {
        vec3 code = litCamera.x > 0.5 ? camFold(light) : pow(light, vec3(1.0 / 2.2));
        fragColor = vec4(code, 0.0);
        fragSurf = vec4(0.0);
    } else {
        fragColor = vec4(light, 1.0);
        float w = min(kSparkDepthLight * dot(light, kSparkLuma) / max(scale, 1e-6), 1.0);
        float z = sparkDepthMap.y != 0.0 ? vStreak.w : 0.0;
        fragSurf = vec4(w * z * 0.001, w, 0.0, 1.0);
    }
}
)";

} // namespace

const char* FireSparkVertexSource()
{
    static const std::string s = std::string("#version 330 core\n") + kFireSparkVertexGLSL;
    return s.c_str();
}

const char* FireSparkFragmentSource()
{
    static const std::string s =
        std::string("#version 330 core\n") + LitConstantsBlock() + kCameraCodecGLSL + kFireSparkFragmentGLSL;
    return s.c_str();
}

} // namespace Poseidon::gl33lighting
