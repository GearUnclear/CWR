// Remastered lighting: the post-pass shaders. EngineGL33_PostFX.cpp runs the
// passes; this file holds their GLSL and no GL code, so the unit tests compile
// the very sources the engine builds. Each shader is assembled from the raw
// strings below by the kPostShaders table and read through PostShaderName and
// PostShaderSource (EngineGL33_Lighting.hpp). s_glslPostCommon holds the depth
// reconstruction the passes share; a pass that adds a shader adds its row to
// the table. A row may name pieces of other GL-free files: the sky table's
// (EngineGL33_PostSkyTableShaders.cpp), the air pass's
// (EngineGL33_PostAirShaders.cpp) and the remastered prelude's LitConstants
// block, terrain and clouds piece and sky piece, which the air pass pastes.

#include <PoseidonGL33/EngineGL33_Lighting.hpp>

#include <array>
#include <iterator>
#include <string>

namespace
{

static const char s_vsPostGLSL[] = R"(#version 330 core
out vec2 vUV;
void main() {
    // Fullscreen triangle from gl_VertexID; no vertex buffer needed.
    vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    vUV = p;
    gl_Position = vec4(p * 2.0 - 1.0, 0.0, 1.0);
}
)";

// Depth reconstruction shared by the post passes that read the world depth: the
// view-space depth in metres at a uv and the view-space position of a uv at a
// depth. The pass declares the inputs these read, under these names: depthTex,
// projInfo (x = _33, y = _43, z = 1/_11, w = 1/_22), depthMap (ndcZ = depth *
// x + y) and worldRect (the 3D viewport in uv), as psPostAo and
// psPostLinearDepth do.
static const char s_glslPostCommon[] = R"(
float viewZ(vec2 uv) {
    float d = texture(depthTex, uv).r;
    return projInfo.y / ((d * depthMap.x + depthMap.y) - projInfo.x);
}

vec3 viewPos(vec2 uv, float z) {
    vec2 ndc = (uv - worldRect.xy) / worldRect.zw * 2.0 - 1.0;
    return vec3(ndc.x * z * projInfo.z, ndc.y * z * projInfo.w, z);
}
)";

// Half-resolution ambient occlusion. Output r = visibility (1 = open), g = view
// depth in metres for the bilateral blur. Its inputs, then s_glslPostCommon,
// then its main.
static const char s_psPostAoInputs[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D depthTex;
uniform vec4 projInfo;  // x = _33, y = _43 (viewZ = y / (ndcZ - x)), z = 1/_11, w = 1/_22
uniform vec4 depthMap;  // ndcZ = depth * x + y
uniform vec4 aoParams;  // x radius (m), y strength, z fade start (m), w fade end (m)
uniform vec4 screen;    // xy full-res size, zw 1/size
uniform vec4 worldRect; // xy origin, zw size of the 3D viewport in uv (pillarbox crop)
uniform float noiseShift; // TAA: this frame's shift of the sample pattern (0 = the fixed pattern)
out vec4 fragColor;
)";

static const char s_psPostAoMain[] = R"(
void main() {
    float d0 = texture(depthTex, vUV).r;
    if (d0 >= 0.999999) { fragColor = vec4(1.0, 1e6, 0.0, 1.0); return; } // sky
    float z = viewZ(vUV);
    vec3 p = viewPos(vUV, z);

    // Normal from depth: on each axis take the neighbour with the smaller step,
    // so silhouettes do not bend the normal.
    vec2 tx = vec2(screen.z, 0.0);
    vec2 ty = vec2(0.0, screen.w);
    vec3 pr = viewPos(vUV + tx, viewZ(vUV + tx));
    vec3 pl = viewPos(vUV - tx, viewZ(vUV - tx));
    vec3 pu = viewPos(vUV + ty, viewZ(vUV + ty));
    vec3 pd = viewPos(vUV - ty, viewZ(vUV - ty));
    vec3 dx = abs(pr.z - p.z) < abs(p.z - pl.z) ? pr - p : p - pl;
    vec3 dy = abs(pu.z - p.z) < abs(p.z - pd.z) ? pu - p : p - pd;
    vec3 n = normalize(cross(dx, dy));
    if (dot(n, p) > 0.0) n = -n; // face the camera

    float radius = aoParams.x;
    vec2 radiusUV = vec2(radius / projInfo.z, radius / projInfo.w) * (0.5 / z) * worldRect.zw;
    // Interleaved gradient noise turns the spiral per pixel; under TAA it also
    // turns per frame, and the history averages the patterns.
    float noise = fract(fract(52.9829189 * fract(dot(gl_FragCoord.xy, vec2(0.06711056, 0.00583715)))) + noiseShift);
    const int kTaps = 12;
    float occlusion = 0.0;
    for (int i = 0; i < kTaps; ++i) {
        float t = (float(i) + 0.5 + noise * 0.5) / float(kTaps);
        float angle = (float(i) * 2.39996 + noise * 6.2831853); // golden-angle spiral
        vec2 suv = vUV + vec2(cos(angle), sin(angle)) * radiusUV * t;
        vec3 q = viewPos(suv, viewZ(suv));
        vec3 v = q - p;
        float vv = dot(v, v);
        float falloff = max(1.0 - vv / (radius * radius), 0.0);
        occlusion += max(dot(v, n) - 0.002 * z - 0.03, 0.0) / (vv + 0.05) * falloff;
    }
    float visibility = max(1.0 - aoParams.y * occlusion * (2.0 / float(kTaps)), 0.0);
    visibility = mix(visibility, 1.0, smoothstep(aoParams.z, aoParams.w, z));
    fragColor = vec4(visibility, z, 0.0, 1.0);
}
)";

// Depth-aware separable blur of the occlusion (run horizontally, then vertically).
static const char s_psPostAoBlurGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D aoTex;
uniform vec2 stepUV;
out vec4 fragColor;
void main() {
    vec2 c = texture(aoTex, vUV).rg;
    float sum = c.r;
    float wsum = 1.0;
    for (int i = -4; i <= 4; ++i) {
        if (i == 0) continue;
        vec2 s = texture(aoTex, vUV + stepUV * float(i)).rg;
        float w = exp(-float(i * i) / 8.0) * max(0.0, 1.0 - abs(s.g - c.g) / (0.04 * c.g + 0.15));
        sum += s.r * w;
        wsum += w;
    }
    fragColor = vec4(sum / wsum, c.g, 0.0, 1.0);
}
)";

// The photoreal camera's decoding (photoLight; Graphics/Remaster/PhotoCamera.hpp
// CameraDecode): with it on, the world's passes stored pre-exposed light on
// the camera's encoding (the lit shaders' camFold); camDecode unfolds it, hue
// kept. Every post pass that reads the frame's colour pastes
// gl33lighting::kCameraCodecGLSL.

// Bloom downsample: four bilinear taps (a 4x4 box). The first level linearises
// the display-space image and keeps only what is close to white, with a soft
// knee: the sky as a whole and sunlit walls stay out (no haze halo along the
// skyline), the sun, its halo, glints and sharp highlights bloom. Under the
// photoreal camera (camParams.x) the first level unfolds the stored scene light
// and takes it at the camera's exposure (camParams.y), so the threshold is the
// light past the curve's shoulder.
static const char s_psPostBloomDownHead[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D srcTex;
uniform vec2 srcTexel;
uniform vec4 bloomParams; // x threshold (linear), y knee, z 1 = first level
uniform vec4 camParams;   // x 1 = the photoreal camera, y its exposure (linear)
out vec4 fragColor;
)";

static const char s_psPostBloomDownMain[] = R"(
vec3 bloomTap(vec2 uv) {
    vec3 c = texture(srcTex, uv).rgb;
    // The camera's bloom: light past the shoulder, capped so the sun's disk
    // spreads a glare, not a flood.
    return (bloomParams.z > 0.5 && camParams.x > 0.5) ? min(camDecode(c) * camParams.y, vec3(8.0)) : c;
}
void main() {
    vec3 c = bloomTap(vUV + srcTexel * vec2(-1.0, -1.0));
    c += bloomTap(vUV + srcTexel * vec2(1.0, -1.0));
    c += bloomTap(vUV + srcTexel * vec2(-1.0, 1.0));
    c += bloomTap(vUV + srcTexel * vec2(1.0, 1.0));
    c *= 0.25;
    if (bloomParams.z > 0.5) {
        if (camParams.x < 0.5) c = pow(max(c, vec3(0.0)), vec3(2.2));
        float peak = max(c.r, max(c.g, c.b));
        float knee = bloomParams.y;
        float soft = clamp(peak - bloomParams.x + knee, 0.0, 2.0 * knee);
        soft = soft * soft / (4.0 * knee + 1e-4);
        c *= max(soft, peak - bloomParams.x) / max(peak, 1e-4);
    }
    fragColor = vec4(c, 1.0);
}
)";

// Bloom upsample: 3x3 tent of the smaller level, added onto the larger one.
static const char s_psPostBloomUpGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D srcTex;
uniform vec2 srcTexel;
out vec4 fragColor;
void main() {
    vec3 c = texture(srcTex, vUV).rgb * 4.0;
    c += (texture(srcTex, vUV + vec2(srcTexel.x, 0.0)).rgb + texture(srcTex, vUV - vec2(srcTexel.x, 0.0)).rgb +
          texture(srcTex, vUV + vec2(0.0, srcTexel.y)).rgb + texture(srcTex, vUV - vec2(0.0, srcTexel.y)).rgb) * 2.0;
    c += texture(srcTex, vUV + srcTexel).rgb + texture(srcTex, vUV - srcTexel).rgb +
         texture(srcTex, vUV + vec2(srcTexel.x, -srcTexel.y)).rgb + texture(srcTex, vUV + vec2(-srcTexel.x, srcTexel.y)).rgb;
    fragColor = vec4(c / 16.0, 1.0);
}
)";

// Sun shafts: march from each pixel toward the sun's screen position and gather
// the sky that is visible (depth at the far plane) along the way. Geometry in
// front of the sun - tree crowns, roofs, ridges - cuts the beams.
static const char s_psPostShaftsHead[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D depthTex;
uniform sampler2D colorTex;
uniform vec4 sunScreen; // xy sun position (uv), z on-screen weight, w aspect (width/height)
uniform float noiseShift; // TAA: this frame's shift of the march start (0 = the fixed pattern)
uniform vec4 camParams;   // x 1 = the photoreal camera: the frame holds scene light (camDecode)
out vec4 fragColor;
)";

static const char s_psPostShaftsMain[] = R"(
void main() {
    if (sunScreen.z <= 0.0) { fragColor = vec4(0.0); return; }
    const int kSteps = 40;
    vec2 toSun = sunScreen.xy - vUV;
    vec2 stepUV = toSun / float(kSteps) * 0.85;
    float noise = fract(fract(52.9829189 * fract(dot(gl_FragCoord.xy, vec2(0.06711056, 0.00583715)))) + noiseShift);
    vec2 uv = vUV + stepUV * noise;
    float decay = 1.0;
    vec3 sum = vec3(0.0);
    for (int i = 0; i < kSteps; ++i) {
        uv += stepUV;
        vec2 c = clamp(uv, vec2(0.001), vec2(0.999));
        float sky = texture(depthTex, c).r >= 0.999999 ? 1.0 : 0.0;
        vec3 t = texture(colorTex, c).rgb;
        // Under the camera the sky near the sun is far brighter than white:
        // the shafts gather it capped, so they stay rays, not a white flood.
        vec3 s = camParams.x > 0.5 ? min(camDecode(t), vec3(1.5)) : pow(t, vec3(2.2));
        sum += s * sky * decay;
        decay *= 0.965;
    }
    float d = length(toSun * vec2(sunScreen.w, 1.0));
    float radial = exp(-d * 2.2);
    fragColor = vec4(sum / float(kSteps) * radial * sunScreen.z, 1.0);
}
)";

static const char s_psPostCompositeHead[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D colorTex;
uniform sampler2D aoTex;
uniform sampler2D bloomTex;
uniform sampler2D shaftTex;
uniform sampler2D depthTex;
uniform vec4 compParams; // x AO strength, y bloom strength, z shaft strength, w dither (0/1)
uniform vec4 shaftColor; // rgb sun tint
uniform vec4 projInfo;   // as psPostAo
uniform vec4 depthMap;   // ndcZ = depth * x + y; z 1 / shaft scattering length (1/m)
uniform vec4 worldRect;  // as psPostAo
uniform vec4 debugView;  // x: 1 = show occlusion
// Combat light scattered in the air (the combatLights switch;
// CombatLights.hpp): the strongest combat lights, in view space.
uniform vec4 scatterParams; // x light count (0 = none), y scattering per metre times the exposure, z sky distance (m)
uniform vec4 scatterPos[8]; // xyz view space (m), w core radius r0
uniform vec4 scatterRgb[8]; // rgb linear radiance at the core, w window end (m)
uniform vec4 scatterMed[8]; // x radius of the lit smoke and dust around the light (m)
// Surfaces and ambient light (the surfaces switch, SurfaceLight.hpp): the
// surface target the world shaders wrote (unit 6: r the share the occlusion
// may darken, g the square root of the albedo's luminance, b the sun's share)
// replaces the frame alpha's share, the half-resolution horizon occlusion is
// upsampled by depth and corrected for multiple bounces.
uniform sampler2D surfTex;
uniform vec4 surfParams; // x 1 = the surface target holds the shares, y occlusion power, z contact shadows, w multi-bounce
uniform vec4 aoSize;     // xy the occlusion's size, zw 1 / size
uniform vec4 sunView;    // xyz toward the sun in view space, w this frame's turn of the march (TAA)
uniform vec4 surfParams2; // x the occlusion power's multiple on static architecture (the target's alpha code 1),
                          // y 1 = fogVeilTex holds the surface's share of each pixel's light after the fog
uniform sampler2D fogVeilTex; // r: the surface's share of the light after the fog (EngineGL33_PostFog.cpp, FogState::veilTex)
// The photoreal camera (photoLight; Graphics/Remaster/PhotoCamera.hpp):
uniform vec4 camParams;  // x 1 = on, y exposure (linear), z curve (0 hue-keeping, 1 per channel, 2 ACES,
                         // 3 Neutral), w vignette
uniform vec4 camCurve;   // the filmic curve y = x^a / (x^(a d) b + c): a, d, b, c
uniform vec4 camColor0;  // rows of the white balance times the scene saturation
uniform vec4 camColor1;
uniform vec4 camColor2;
uniform vec4 camGolden;  // rgb the golden hour's warm gain (1 = none), laid on the light that is not blue
uniform vec4 camGlow;    // x bloom gain, y shaft gain, z aspect (w / h), w the rods' warmth fade width
uniform vec4 camNight;   // x the night's rod shift, y its luminance, z the curve's white (scene light), w stops past
                         // it to white
uniform vec4 camRod;     // rgb the rods' cool grey, w the warmth from which warm light keeps its colour
uniform vec4 camShade;   // rgb the shade's cool balance at a low sun (unit luminance), w its strength
uniform vec4 camDark;    // x the dark tones' saturation (relative), y z the scene light it fades out over, w 1 = a
                         // vehicle interior (pre-lit: the camera's interior grade)
out vec4 fragColor;
)";

static const char s_psPostCompositeMain[] = R"(
float compositeViewZ(vec2 uv) {
    float d = textureLod(depthTex, uv, 0.0).r;
    if (d >= 0.999999) return 1e6;
    return projInfo.y / ((d * depthMap.x + depthMap.y) - projInfo.x);
}

// The half-resolution occlusion at this full-resolution pixel: its four
// nearest texels (clamped to the target), bilinear weights times how well each
// texel's depth matches the pixel's (within about 1% of the depth), so a near
// edge does not bleed its occlusion onto the far side: a crown's onto the
// grass behind it.
float upsampleOcclusion(vec2 uv, float z) {
    vec2 f = uv * aoSize.xy - 0.5;
    vec2 i0 = floor(f);
    vec2 fr = f - i0;
    float sum = 0.0;
    float wsum = 0.0;
    float nearest = 1.0;
    float best = 1e9;
    for (int k = 0; k < 4; ++k) {
        vec2 o = vec2(float(k & 1), float(k >> 1));
        vec2 ti = clamp(i0 + o, vec2(0.0), aoSize.xy - 1.0);
        vec2 s = textureLod(aoTex, (ti + 0.5) * aoSize.zw, 0.0).rg;
        float wb = mix(1.0 - fr.x, fr.x, o.x) * mix(1.0 - fr.y, fr.y, o.y);
        float dz = abs(s.g - z);
        float wd = exp(-dz / (0.008 * z + 0.04));
        sum += s.r * wb * wd;
        wsum += wb * wd;
        if (dz < best) { best = dz; nearest = s.r; }
    }
    return wsum > 1e-4 ? sum / wsum : nearest;
}

// Contact shadows: a short march toward the sun through the depth buffer
// finds the small casters the shadow map is too coarse for (a boot on the
// ground, a wheel on its hub, a sandbag's lip). 1 = lit. The march is a few
// centimetres to a metre long with the distance, and a sample counts as a
// caster only within a thickness behind it, so a surface far behind the ray
// does not shadow it.
float contactShadow(vec2 uv, float z) {
    if (z > 50.0) return 1.0;
    vec2 ndc = (uv - worldRect.xy) / worldRect.zw * 2.0 - 1.0;
    vec3 p = vec3(ndc.x * z * projInfo.z, ndc.y * z * projInfo.w, z);
    float len = clamp(0.03 * z, 0.12, 1.2);
    float thick = 0.25 + 0.02 * z;
    float noise = fract(fract(52.9829189 * fract(dot(gl_FragCoord.xy, vec2(0.06711056, 0.00583715)))) + sunView.w);
    const int kSteps = 8;
    float shadow = 0.0;
    for (int i = 0; i < kSteps; ++i) {
        float t = (float(i) + noise) / float(kSteps);
        vec3 q = p + sunView.xyz * (len * t);
        if (q.z <= 0.05) break;
        vec2 qn = vec2(q.x / (q.z * projInfo.z), q.y / (q.z * projInfo.w));
        vec2 quv = worldRect.xy + (qn * 0.5 + 0.5) * worldRect.zw;
        vec2 qr = (quv - worldRect.xy) / worldRect.zw;
        if (qr.x <= 0.0 || qr.y <= 0.0 || qr.x >= 1.0 || qr.y >= 1.0) break; // off the 3D view: open
        float zs = compositeViewZ(quv);
        float dz = q.z - zs;
        float bias = 0.015 + 0.0015 * q.z;
        if (dz > bias && dz < thick) {
            shadow = max(shadow, 1.0 - t * t);
        }
    }
    return 1.0 - shadow * (1.0 - smoothstep(30.0, 50.0, z));
}

// Jimenez et al. 2016: the light that bounces between the occluders before it
// arrives brightens the occlusion of a light surface.
float occlusionMultiBounce(float v, float albedo) {
    float a = 2.0404 * albedo - 0.3324;
    float b = -4.7951 * albedo + 0.6417;
    float c = 2.7552 * albedo + 0.6903;
    return max(v, ((v * a + b) * v + c) * v);
}

// Single scattering of the combat lights, integrated in closed form along the
// pixel's view ray through a ball of lit air around each light (its smoke and
// dust; a fire's spreads thinner and taller), up to the depth buffer: the glow
// around a flash stops at whatever stands in front of it. Nothing without a
// combat light.
vec3 combatScatter() {
    int count = int(scatterParams.x);
    if (count <= 0) return vec3(0.0);
    float d = textureLod(depthTex, vUV, 0.0).r;
    vec2 ndc = (vUV - worldRect.xy) / worldRect.zw * 2.0 - 1.0;
    vec3 dir = vec3(ndc.x * projInfo.z, ndc.y * projInfo.w, 1.0);
    float invLen = inversesqrt(dot(dir, dir));
    dir *= invLen;
    float far = scatterParams.z;
    if (d < 0.999999) far = (projInfo.y / ((d * depthMap.x + depthMap.y) - projInfo.x)) / invLen;
    vec3 sum = vec3(0.0);
    for (int i = 0; i < 8; ++i) {
        if (i >= count) break;
        vec3 L = scatterPos[i].xyz;
        float r0 = scatterPos[i].w;
        float b = dot(L, dir);
        float h2 = max(dot(L, L) - b * b, 0.0);
        float medium = min(scatterMed[i].x, scatterRgb[i].w);
        float medium2 = medium * medium;
        if (h2 >= medium2) continue;
        float chord = sqrt(medium2 - h2);
        float H = sqrt(h2 + r0 * r0);
        float t0 = max(b - chord, 0.0);
        float t1 = min(far, b + chord);
        if (t1 <= t0) continue;
        float rim = 1.0 - h2 / medium2;
        sum += scatterRgb[i].rgb * (r0 * r0 * (atan((t1 - b) / H) - atan((t0 - b) / H)) / H * rim * rim * rim);
    }
    return sum * scatterParams.y;
}

// The shaft pass gathers the sky toward the sun, which is the light the air
// scatters toward the eye along an endless ray. A surface ends the ray: only
// the air in front of it scatters, 1 - exp(-distance / length) of the sky's
// share (LightRig ShaftInscatter). Without this a backlit vehicle 10 m away
// took the full sky glow and turned into a grey cut-out.
float shaftInscatter() {
    float d = textureLod(depthTex, vUV, 0.0).r;
    if (d >= 0.999999) return 1.0; // sky, as the shaft pass sees it
    float z = projInfo.y / ((d * depthMap.x + depthMap.y) - projInfo.x);
    vec2 ndc = (vUV - worldRect.xy) / worldRect.zw * 2.0 - 1.0;
    float dist = z * length(vec3(ndc * projInfo.zw, 1.0));
    return 1.0 - exp(-max(dist, 0.0) * depthMap.z);
}

// The camera's tone curve (camParams.z): the filmic curve on the peak channel
// with the channel ratios kept until the output nears white (hue-keeping), the
// same curve per channel, the ACES fit (Narkowicz) or Khronos PBR Neutral.
float camCurveY(float x) {
    x = max(x, 0.0);
    return clamp(pow(x, camCurve.x) / (pow(x, camCurve.x * camCurve.y) * camCurve.z + camCurve.w), 0.0, 1.0);
}
vec3 camTone(vec3 x) {
    int kind = int(camParams.z + 0.5);
    if (kind == 1) return vec3(camCurveY(x.r), camCurveY(x.g), camCurveY(x.b));
    if (kind == 2) {
        x *= 0.6;
        return clamp((x * (2.51 * x + 0.03)) / (x * (2.43 * x + 0.59) + 0.14), 0.0, 1.0);
    }
    if (kind == 3) {
        float lo = min(x.r, min(x.g, x.b));
        x -= lo < 0.08 ? lo - 6.25 * lo * lo : 0.04;
        float pk = max(x.r, max(x.g, x.b));
        if (pk < 0.76) return x;
        float newPeak = 1.0 - 0.0576 / (pk - 0.52);
        x *= newPeak / pk;
        return mix(x, vec3(newPeak), 1.0 - 1.0 / (0.15 * (pk - newPeak) + 1.0));
    }
    float peak = max(x.r, max(x.g, x.b));
    if (peak <= 0.0) return vec3(0.0);
    float y = camCurveY(peak);
    vec3 ratio = x / peak;
    // To white late: near the curve's top (kCrosstalk) and with the light
    // past its white (kWhiteExcessStops stops past it is white). Saturated
    // light less (kChromaKeep): a flame rolls off in hue to a yellow-orange
    // core, a white sun still clips.
    float excess = clamp(log2(max(peak / camNight.z, 1.0)) / camNight.w, 0.0, 1.0);
    float chroma = 1.0 - min(ratio.r, min(ratio.g, ratio.b));
    float whiten = max(pow(y, 8.0), excess) * (1.0 - 0.8 * chroma * chroma); // kCrosstalk, kChromaKeep
    // A channel the light holds less of goes to white later (kHueRoll, look
    // fix "burst"): a bright orange rolls off to yellow-orange, not cream.
    vec3 w = pow(vec3(max(whiten, 1e-6)), 1.0 + 3.0 * (1.0 - ratio)); // kHueRoll
    return mix(ratio, vec3(1.0), w) * y;
}

void main() {
    vec4 c = texture(colorTex, vUV);
    float ao = texture(aoTex, vUV).r;
    // c.a is one minus the pixel's sky light share (1 for pre-lit and coverage
    // pixels); the camera's cool shade reads it in both paths.
    float share = 1.0 - clamp(c.a, 0.0, 1.0);
    float keep;
    if (surfParams.x > 0.5) {
        // The surface target's share, the occlusion upsampled by depth, its
        // strength as a power and the multi-bounce correction by the albedo.
        vec4 s = textureLod(surfTex, vUV, 0.0);
        // The volumetric fog veils the surface: the occlusion and the contact
        // shadow take the surface's share of the light after the fog (its own
        // light through it), not the air's light in front of it (the world's
        // shares hold only the classic fog's edge). (Without it a fogged
        // treeline 300 m out in rain kept its full occlusion over the fog's
        // light: dark scalloped rims at the foot of every crown.)
        float veil = surfParams2.y > 0.5 ? textureLod(fogVeilTex, vUV, 0.0).r : 1.0;
        s.r *= veil;
        s.b *= veil;
        float zc = compositeViewZ(vUV);
        ao = upsampleOcclusion(vUV, zc);
        // The target's alpha: 0.5 the occlusion as set, 1 static architecture
        // (eaves, door openings, the foot of a wall take it stronger).
        float power = surfParams.y * mix(1.0, surfParams2.x, smoothstep(0.5, 1.0, s.a));
        ao = pow(clamp(ao, 0.0, 1.0), power);
        if (surfParams.w > 0.5) ao = occlusionMultiBounce(ao, s.g * s.g);
        // A floor: light bounces into a pit or under a hull from the lit
        // ground around it, so the occluded sky light keeps about a third
        // (without it the underbody of a Jeep at a low sun went to 0).
        ao = max(ao, 0.35);
        // The sun's share loses what a contact shadow hides.
        float contact = (surfParams.z > 0.0 && s.b > 0.02) ? contactShadow(vUV, zc) : 1.0;
        contact = mix(1.0, contact, clamp(surfParams.z, 0.0, 1.0));
        if (debugView.x > 0.5) { fragColor = vec4(vec3(ao * contact), 1.0); return; }
        keep = max(1.0 - clamp(s.r, 0.0, 1.0) * (1.0 - ao) - clamp(s.b, 0.0, 1.0) * (1.0 - contact), 0.0);
    } else {
        if (debugView.x > 0.5) { fragColor = vec4(vec3(ao), 1.0); return; }
        // Take the occluded part of the sky light out of the radiance.
        keep = max(1.0 - share * (1.0 - ao) * compParams.x, 0.0);
    }
    vec3 shaft = texture(shaftTex, vUV).rgb;
    if (max(shaft.r, max(shaft.g, shaft.b)) > 0.0) shaft *= shaftInscatter();
    float n = fract(52.9829189 * fract(dot(gl_FragCoord.xy, vec2(0.06711056, 0.00583715))));
    float m = fract(52.9829189 * fract(dot(gl_FragCoord.yx + 17.0, vec2(0.06711056, 0.00583715))));
    if (camParams.x > 0.5) {
        // The camera's dither: white noise from a hash of the pixel and this
        // frame's turn (sunView.w), triangular, half an LSB: no fixed pattern
        // (the gradient-noise lattice drew a fine diagonal hatch in the sky
        // once the curve stretched it), and the TAA averages it out.
        uvec2 hp = uvec2(gl_FragCoord.xy) + uvec2(uint(fract(sunView.w * 7.31) * 4096.0), 0u);
        uint h = hp.x * 1973u + hp.y * 9277u + uint(fract(sunView.w) * 65536.0) * 26699u;
        h = (h ^ (h >> 15)) * 0x2c1b3c6du;
        h = (h ^ (h >> 12)) * 0x297a2d39u;
        h ^= h >> 15;
        n = float(h & 0xffffu) / 65535.0;
        m = float(h >> 16) / 65535.0;
        // The camera: the stored scene light, its occlusion, the glare of
        // light past the shoulder and the shafts, at the metered exposure;
        // white balance and saturation; the curve; the lens's vignette; a
        // dither everywhere (the curve moves every value).
        vec3 x = camDecode(c.rgb) * keep * camParams.y;
        x += texture(bloomTex, vUV).rgb * camGlow.x;
        x += shaft * shaftColor.rgb * (camGlow.y * camParams.y);
        // Combat light scattered in the air, as light at the exposure.
        x += combatScatter();
        x = max(vec3(dot(camColor0.xyz, x), dot(camColor1.xyz, x), dot(camColor2.xyz, x)), vec3(0.0));
        // The golden hour's warm balance (camGolden) is the low sun's light:
        // what it shows takes it, the blue sky and the sky-lit blue do not
        // (PhotoCamera.hpp GoldenWarmShare: none at a warmth (r - b) / peak
        // of -0.7, all from 0.4, a wide ramp so a sky's gradient from the
        // sun's glow to its blue stays smooth). Over the whole frame it
        // turned the clear sky teal-grey.
        if (camGolden.r != 1.0 || camGolden.b != 1.0) {
            float pk = max(x.r, max(x.g, x.b));
            float warmth = pk > 0.0 ? (x.r - x.b) / pk : 0.0;
            x *= mix(vec3(1.0), camGolden.rgb, smoothstep(-0.7, 0.4, warmth)); // kGoldenShareFrom, kGoldenShareTo
        }
        // At a low sun the camera, on daylight balance, sees the shade lit by
        // the blue sky: what the sky lights more than the sun (the world's
        // sky light share, `share`) leans cool (camShade), what the sun
        // lights keeps its amber (a pine crown stays green-dark, warm only at
        // its sunlit tips; pre-lit draws and the sky are left alone).
        if (camShade.w > 0.0) x = mix(x, x * camShade.rgb, camShade.w * smoothstep(0.2, 0.6, share));
        // Dark tones carry less colour than the mid tones (camDark): the
        // hue-keeping curve keeps a dark olive's full linear ratios, which
        // read as a candied mustard in a pine crown's shade.
        if (camDark.x < 1.0) {
            float l = dot(x, vec3(0.2126, 0.7152, 0.0722));
            float s = mix(camDark.x, 1.0, smoothstep(camDark.y, camDark.z, l));
            x = max(mix(vec3(l), x, s), vec3(0.0));
        }
        // The night's rods: dim light toward a cool grey of its brightness;
        // lamp and fire pools, far brighter, keep their colour, and so does
        // warm light however dim (warmth (r - b) / peak from camRod.w over
        // the width camGlow.w): the fire's smoke is warm through, not orange
        // particles among grey ones.
        if (camNight.x > 0.0) {
            float l = dot(x, vec3(0.2126, 0.7152, 0.0722));
            float pk = max(x.r, max(x.g, x.b));
            float warm = pk > 0.0 ? (x.r - x.b) / pk : 0.0;
            float rod = camNight.x * (1.0 - smoothstep(0.25 * camNight.y, camNight.y, l)) *
                        (1.0 - smoothstep(camRod.w, camRod.w + camGlow.w, warm));
            vec3 tint = camRod.rgb / dot(camRod.rgb, vec3(0.2126, 0.7152, 0.0722));
            x = mix(x, l * tint, rod);
        }
        // The lens's vignette, on the light before the curve: the corners
        // darken, light far past white (the sun) still clips.
        vec2 q = (vUV - 0.5) * vec2(camGlow.z, 1.0);
        // A vehicle interior (camDark.w): its pre-lit crew compartment is the
        // original's flat vertex light, which the meter and the occlusion
        // barely move. The camera grades it as it would a dim interior: half
        // a stop down, a deeper toe and a stronger vignette.
        float vig = camParams.w;
        if (camDark.w > 0.5) {
            x *= 0.7;
            float l = dot(x, vec3(0.2126, 0.7152, 0.0722));
            x *= mix(0.72, 1.0, smoothstep(0.02, 0.3, l));
            vig = min(camParams.w * 2.2, 0.5);
        }
        x *= 1.0 - vig * smoothstep(0.1, 1.1, dot(q, q) * 2.2);
        vec3 y = camTone(x);
        vec3 col = pow(clamp(y, 0.0, 1.0), vec3(1.0 / 2.2));
        col += 0.5 * (n + m - 1.0) / 255.0;
        fragColor = vec4(col, 1.0);
        return;
    }
    vec3 lin = pow(clamp(c.rgb, 0.0, 1.0), vec3(2.2)) * keep;
    // Glow adds like light onto a screen: 1 - (1 - base)(1 - glow), with the
    // glow's own energy rolled off. Identity where there is no glow, never
    // above 1, so the tonemapped frame keeps its whites and the sky its colour.
    vec3 glow = texture(bloomTex, vUV).rgb * compParams.y + shaft * shaftColor.rgb * compParams.z;
    glow += combatScatter();
    lin += (1.0 - lin) * (1.0 - exp(-max(glow, vec3(0.0))));
    vec3 col = pow(lin, vec3(1.0 / 2.2));
    // Dither only what the composite changed: elsewhere the frame passes
    // through exactly (the 2.2 round trip is exact on 8-bit values).
    if (compParams.w > 0.5 && (keep < 0.9999 || max(glow.r, max(glow.g, glow.b)) > 1e-5)) {
        col += (n + m - 1.0) / 255.0; // triangular dither, +-1 LSB
    }
    fragColor = vec4(col, 1.0);
}
)";

// The photoreal camera's meter (photoLight; PhotoCamera.hpp Meter): the
// resolved frame into kMeterWidth x kMeterHeight texels, each the weighted
// mean log2 luminance of the scene light (camDecode) over a 4 x 4 grid of
// bilinear taps across its block, its weight (centre weighted, sky taps
// counting meterParams.x), the block's brightest tap and its brightest sky
// tap (-32 when it shows no sky). A block
// that is black through (an optic's mask, pillarbox bars) weighs nothing, so
// the camera meters the scene inside it. Read back by the CPU, which builds
// the histogram.
// The meter reads the sky at its plain roll-off (PhotoCamera.hpp
// SkyMeterLight): toward the sun the dome's roll-off rises past white
// (camSkyRolloff, kSkySunCap) so the sun's glow burns, and the guards that keep
// the sky under white closed the camera down for that burn. Each sky tap's peak
// is taken back through the roll-off toward the sun along its own view ray
// (meterProj, meterRect, meterSun) and rolled off with the plain cap (meterSky:
// knee, cap, the cap toward the sun, the glow's radius), so the meter sees the
// sky round the sun as it sees the rest of the sky, and the frame keeps its
// burn.
static const char s_psPostMeterHead[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D colorTex;
uniform sampler2D depthTex;
uniform vec4 meterParams; // x the sky's weight, y 1 / the frame's pre-exposure, zw meter size
uniform vec4 meterSky;    // the sky's roll-off: x knee, y cap, z the cap toward the sun, w the glow's radius (rad)
uniform vec4 meterSun;    // xyz the unit direction to the sun in view space (z forward), w 1: take the sun's cap out
uniform vec4 meterProj;   // x 1 / proj._11, y 1 / proj._22: the view ray is (ndc.x x, ndc.y y, 1)
uniform vec4 meterRect;   // the 3D viewport in uv (x, y, w, h)
out vec4 fragColor;
)";

static const char s_psPostMeterMain[] = R"(
// The factor that takes a sky tap's pre-exposed light (the dome's roll-off
// toward the sun along this tap's view ray) to the plain roll-off of the same
// light (PhotoCamera.cpp SkyMeterLight): the roll-off with cap C is
// k + r (1 - exp(-(m - k) / r)), r = C - k; it is inverted with the cap toward
// the sun and rolled off again with the plain cap. Light at or past the cap
// toward the sun (the disc, the air's glow over it) reads as the plain cap.
float meterSkyLight(vec3 pre, vec2 uv) {
    float o = max(pre.r, max(pre.g, pre.b));
    float k = meterSky.x;
    if (meterSun.w < 0.5 || o <= k) return 1.0;
    vec2 ndc = (uv - meterRect.xy) / max(meterRect.zw, vec2(1e-6)) * 2.0 - 1.0;
    vec3 ray = normalize(vec3(ndc.x * meterProj.x, ndc.y * meterProj.y, 1.0));
    float c = clamp(dot(ray, meterSun.xyz), -1.0, 1.0);
    float glow = 1.0 / (1.0 + 2.0 * (1.0 - c) / (meterSky.w * meterSky.w));
    float rs = meterSky.y + (meterSky.z - meterSky.y) * glow - k;
    float rp = meterSky.y - k;
    float m = k - rs * log(max(1.0 - (o - k) / rs, 1e-6));
    return (k + rp * (1.0 - exp(-(m - k) / rp))) / o;
}

void main() {
    vec2 cell = floor(gl_FragCoord.xy);
    float sum = 0.0, wsum = 0.0, hi = -32.0, lo = 32.0, skyHi = -32.0;
    for (int j = 0; j < 4; ++j) {
        for (int i = 0; i < 4; ++i) {
            vec2 uv = (cell + (vec2(float(i), float(j)) + 0.5) / 4.0) / meterParams.zw;
            vec3 t = textureLod(colorTex, uv, 0.0).rgb;
            vec3 pre = camDecode(t);
            bool sky = textureLod(depthTex, uv, 0.0).r >= 0.999999;
            if (sky) pre *= meterSkyLight(pre, uv);
            vec3 lin = pre * meterParams.y;
            float l = log2(max(dot(lin, vec3(0.2126, 0.7152, 0.0722)), 1e-5));
            float w = sky ? meterParams.x : 1.0;
            sum += l * w;
            wsum += w;
            hi = max(hi, l);
            lo = min(lo, l);
            if (sky) skyHi = max(skyHi, l);
        }
    }
    vec2 q = (gl_FragCoord.xy / meterParams.zw - 0.5) * vec2(1.0, 0.8);
    float centre = 0.3 + 0.7 * exp(-dot(q, q) / (2.0 * 0.3 * 0.3));
    if (hi < -15.0) centre = 0.0; // black through (log2 of the 1e-5 floor is -16.6)
    fragColor = vec4(sum / max(wsum, 1e-6), wsum / 16.0 * centre, hi, skyHi);
}
)";

// Half-resolution linear depth (the LinearDepth stage), for the stages that
// trace or upsample at half resolution. Output r = view depth in metres of one
// of the four full-resolution pixels under the texel, 1e6 for the sky. Its
// inputs, then s_glslPostCommon, then its main.
static const char s_psPostLinearDepthInputs[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D depthTex;
uniform vec4 projInfo;  // x = _33, y = _43 (viewZ = y / (ndcZ - x)), z = 1/_11, w = 1/_22
uniform vec4 depthMap;  // ndcZ = depth * x + y
uniform vec4 screen;    // xy full-res size, zw 1/size
uniform vec4 worldRect; // xy origin, zw size of the 3D viewport in uv (read by viewPos)
out vec4 fragColor;
)";

// On a checkerboard, even texels keep the nearest of their four pixels and odd
// texels the farthest, so along a silhouette the half-resolution buffer holds
// both the edge and what lies behind it, and a depth-aware upsample finds a tap
// at either depth (a thin branch against the sky keeps its own). The four reads
// land on exact full-resolution texel centres; on an odd frame size the last
// row or column repeats the edge pixel (clamp to edge).
static const char s_psPostLinearDepthMain[] = R"(
void main() {
    ivec2 ij = ivec2(gl_FragCoord.xy);
    float zNear = 1e6;
    float zFar = 0.0;
    for (int k = 0; k < 4; ++k) {
        vec2 uv = (vec2(ij * 2 + ivec2(k & 1, k >> 1)) + 0.5) * screen.zw;
        float d = texture(depthTex, uv).r;
        float z = d >= 0.999999 ? 1e6 : viewZ(uv); // sky as psPostAo sees it
        zNear = min(zNear, z);
        zFar = max(zFar, z);
    }
    fragColor = vec4(((ij.x + ij.y) & 1) == 0 ? zNear : zFar, 0.0, 0.0, 1.0);
}
)";

// Debug view 5 (the DebugView stage): the half-resolution linear depth over the
// frame, on a log scale from white at 1 m to black at 8 km, the sky blue. The
// texels stay square (nearest filtering) and the checkerboard of nearest and
// farthest samples shows along silhouettes.
static const char s_psPostDebugDepthGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D linDepthTex; // r = view depth in metres, 1e6 = sky
out vec4 fragColor;
void main() {
    float z = texture(linDepthTex, vUV).r;
    if (z >= 1e5) { fragColor = vec4(0.25, 0.45, 0.85, 1.0); return; }
    float g = 1.0 - clamp(log2(max(z, 1.0)) / 13.0, 0.0, 1.0);
    fragColor = vec4(vec3(g), 1.0);
}
)";

// TAA resolve (the Temporal stage; Graphics/Remaster/TemporalAA.hpp).
// This frame was drawn with a sub-pixel jitter; the history is the resolved
// frame before it, with each pixel's view depth in alpha. Per pixel:
//   - rebuild this frame at the pixel centre without the jitter: Gaussian
//     weights exp(-4 d^2) (sigma 0.35 px) over the 3x3 neighbourhood centred
//     on the jitter offset. The converged image is the scene filtered by this
//     kernel: Karis's wider exp(-2.29 d^2) left a test view 15% softer than
//     without TAA, this one 5%;
//   - reproject: the nearest depth of the 3x3 (a moving silhouette keeps the
//     motion of the surface in front), unjittered to the view position, moved
//     into the previous view with the camera motion (reproj rows, built in
//     double precision on the CPU) and projected with the previous projection;
//   - drop the history where the surface it holds is not the one reprojected
//     (its stored depth range around that spot misses the reprojected depth by
//     more than 3% + 0.15 m: a disocclusion, a vehicle that moved) or where it
//     falls outside the 3D viewport;
//   - read the history with a 9-tap Catmull-Rom (keeps detail under motion)
//     and clip it into the box of this frame's 3x3 colours in YCoCg (mean +-
//     one standard deviation, inside the min and max), so a colour the pixel
//     no longer has cannot trail behind;
//   - blend at alpha (this frame's share), raised with fast motion, and with
//     luma weights against single bright samples;
//   - the particle mask: where the translucent objects drawn after the opaque
//     world (smoke, fire, muzzle flashes, glass, rain) changed the pixel, this
//     frame takes up to reactive.z of the blend and the luma weights step
//     aside, so a flash keeps its brightness and smoke does not smear.
// The frame is display space (the remastered shaders tonemap per fragment),
// which is the tonemapped space a resolve wants; the history is half float.
static const char s_psPostTaaInputs[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D curTex;    // this frame after the composite (display space)
uniform sampler2D depthTex;  // this frame's world depth
uniform sampler2D histTex;   // rgb the resolved frame before, a its view depth (m)
uniform sampler2D worldTex;  // this frame's world as drawn (before the post effects)
uniform sampler2D opaqueTex; // the same before the translucent objects
uniform vec4 projInfo;  // x = _33, y = _43 (viewZ = y / (ndcZ - x)), z = 1/_11, w = 1/_22
uniform vec4 depthMap;  // ndcZ = depth * x + y
uniform vec4 worldRect; // xy origin, zw size of the 3D viewport in uv
uniform vec4 screen;    // xy size, zw 1/size
uniform vec4 jitter;    // xy this frame's jitter (pixels), z 1 = the history is usable, w alpha
uniform vec4 reproj0;   // previous view position = (dot(row.xyz, p) + row.w) per row
uniform vec4 reproj1;
uniform vec4 reproj2;
uniform vec4 prevProj;  // xy _11 and _22 of the previous frame
uniform vec4 reactive;  // x low, y high threshold, z most alpha, w 1 = opaqueTex holds this frame
out vec4 fragColor;
)";

static const char s_psPostTaaMain[] = R"(
vec3 toYCoCg(vec3 c) { return vec3(0.25 * c.r + 0.5 * c.g + 0.25 * c.b, 0.5 * c.r - 0.5 * c.b, -0.25 * c.r + 0.5 * c.g - 0.25 * c.b); }
vec3 fromYCoCg(vec3 c) { return vec3(c.x + c.y - c.z, c.x + c.z, c.x - c.y - c.z); }

vec3 clipToBox(vec3 bmin, vec3 bmax, vec3 q) {
    vec3 c = 0.5 * (bmax + bmin);
    vec3 e = 0.5 * (bmax - bmin) + 1e-4;
    vec3 v = q - c;
    vec3 a = abs(v / e);
    float m = max(a.x, max(a.y, a.z));
    return m > 1.0 ? c + v / m : q;
}

// Catmull-Rom from nine bilinear taps.
vec3 sampleHistory(vec2 uv) {
    vec2 sp = uv * screen.xy;
    vec2 t1 = floor(sp - 0.5) + 0.5;
    vec2 f = sp - t1;
    vec2 w0 = f * (-0.5 + f * (1.0 - 0.5 * f));
    vec2 w1 = 1.0 + f * f * (-2.5 + 1.5 * f);
    vec2 w2 = f * (0.5 + f * (2.0 - 1.5 * f));
    vec2 w3 = f * f * (-0.5 + 0.5 * f);
    vec2 w12 = w1 + w2;
    vec2 t0 = (t1 - 1.0) * screen.zw;
    vec2 t3 = (t1 + 2.0) * screen.zw;
    vec2 t12 = (t1 + w2 / w12) * screen.zw;
    vec3 r = texture(histTex, vec2(t0.x, t0.y)).rgb * w0.x * w0.y;
    r += texture(histTex, vec2(t12.x, t0.y)).rgb * w12.x * w0.y;
    r += texture(histTex, vec2(t3.x, t0.y)).rgb * w3.x * w0.y;
    r += texture(histTex, vec2(t0.x, t12.y)).rgb * w0.x * w12.y;
    r += texture(histTex, vec2(t12.x, t12.y)).rgb * w12.x * w12.y;
    r += texture(histTex, vec2(t3.x, t12.y)).rgb * w3.x * w12.y;
    r += texture(histTex, vec2(t0.x, t3.y)).rgb * w0.x * w3.y;
    r += texture(histTex, vec2(t12.x, t3.y)).rgb * w12.x * w3.y;
    r += texture(histTex, vec2(t3.x, t3.y)).rgb * w3.x * w3.y;
    return max(r, vec3(0.0));
}

float viewDepth(float d) { return projInfo.y / ((d * depthMap.x + depthMap.y) - projInfo.x); }

void main() {
    ivec2 ip = ivec2(gl_FragCoord.xy);
    ivec2 lim = ivec2(screen.xy) - 1;
    vec3 m1 = vec3(0.0), m2 = vec3(0.0);
    vec3 cmin = vec3(1e9), cmax = vec3(-1e9);
    vec3 cur = vec3(0.0);
    float wsum = 0.0;
    float closest = 2.0;
    ivec2 closestOff = ivec2(0);
    for (int y = -1; y <= 1; ++y) {
        for (int x = -1; x <= 1; ++x) {
            ivec2 q = clamp(ip + ivec2(x, y), ivec2(0), lim);
            vec3 c = texelFetch(curTex, q, 0).rgb;
            vec3 yc = toYCoCg(c);
            m1 += yc;
            m2 += yc * yc;
            cmin = min(cmin, yc);
            cmax = max(cmax, yc);
            vec2 d = vec2(float(x), float(y)) - jitter.xy;
            float w = exp(-4.0 * dot(d, d));
            cur += c * w;
            wsum += w;
            float dz = texelFetch(depthTex, q, 0).r;
            if (dz < closest) { closest = dz; closestOff = ivec2(x, y); }
        }
    }
    cur /= wsum;

    // Where the surface at the nearest neighbour was one frame ago.
    vec2 uvU = (vec2(ip + closestOff) + 0.5 - jitter.xy) * screen.zw; // that surface without the jitter
    vec2 ndc = (uvU - worldRect.xy) / worldRect.zw * 2.0 - 1.0;
    float z = viewDepth(closest);
    vec3 p = vec3(ndc.x * z * projInfo.z, ndc.y * z * projInfo.w, z);
    vec3 pp = vec3(dot(reproj0.xyz, p) + reproj0.w, dot(reproj1.xyz, p) + reproj1.w, dot(reproj2.xyz, p) + reproj2.w);
    vec2 prevNdc = pp.xy * prevProj.xy / max(pp.z, 1e-6);
    vec2 motion = worldRect.xy + (prevNdc * 0.5 + 0.5) * worldRect.zw - uvU;
    vec2 histUV = (vec2(ip) + 0.5) * screen.zw + motion;
    bool valid = jitter.z > 0.5 && pp.z > 0.0 && all(greaterThanEqual(histUV, worldRect.xy)) &&
                 all(lessThanEqual(histUV, worldRect.xy + worldRect.zw));

    // The history held another surface there: start again from this frame.
    // The 3x3 range keeps the jitter on leaf edges from tripping it; the sky
    // (no depth to compare) relies on the colour clip.
    if (valid && closest < 0.999999) {
        ivec2 hq = ivec2(floor(histUV * screen.xy));
        float zmin = 1e9, zmax = 0.0;
        for (int y = -1; y <= 1; ++y) {
            for (int x = -1; x <= 1; ++x) {
                float zq = texelFetch(histTex, clamp(hq + ivec2(x, y), ivec2(0), lim), 0).a;
                zmin = min(zmin, zq);
                zmax = max(zmax, zq);
            }
        }
        float tol = 0.03 * pp.z + 0.15;
        valid = pp.z >= zmin - tol && pp.z <= zmax + tol;
    }

    vec3 mu = m1 / 9.0;
    vec3 sigma = sqrt(max(m2 / 9.0 - mu * mu, vec3(0.0)));
    vec3 hist = valid ? sampleHistory(histUV) : cur;
    hist = fromYCoCg(clipToBox(max(mu - sigma, cmin), min(mu + sigma, cmax), toYCoCg(hist)));

    float alpha = valid ? jitter.w : 1.0;
    // Fast motion leans on this frame a little more (less smear).
    alpha = min(alpha + 0.1 * clamp(length(motion * screen.xy) / 8.0, 0.0, 1.0), 1.0);
    // The particle mask.
    float r = 0.0;
    if (reactive.w > 0.5) {
        vec3 dw = abs(texelFetch(worldTex, ip, 0).rgb - texelFetch(opaqueTex, ip, 0).rgb);
        r = smoothstep(reactive.x, reactive.y, max(dw.r, max(dw.g, dw.b)));
        alpha = max(alpha, r * reactive.z);
    }
    // Luma weights: an isolated bright sample cannot flicker the result.
    float lc = dot(cur, vec3(0.2126, 0.7152, 0.0722)) * (1.0 - r);
    float lh = dot(hist, vec3(0.2126, 0.7152, 0.0722)) * (1.0 - r);
    float wc = alpha / (1.0 + lc);
    float wh = (1.0 - alpha) / (1.0 + lh);
    vec3 res = (cur * wc + hist * wh) / max(wc + wh, 1e-5);
    // This pixel's own view depth for the next frame's test (half float).
    float d0 = texelFetch(depthTex, ip, 0).r;
    fragColor = vec4(max(res, vec3(0.0)), min(viewDepth(d0), 65000.0));
}
)";

// TAA sharpen (the Sharpen stage): the resolved frame back into the frame
// target, every sample, with a bounded unsharp mask against the softness of
// the accumulation (its overshoot stays within a quarter of the local cross's
// range beyond it: crisper than a hard clamp, no halos) and a +-1 LSB
// triangular dither, since the history is half float and the target 8 bits.
static const char s_psPostTaaSharpenGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D srcTex;  // the resolved frame
uniform vec4 sharpen;      // x strength, yz size (pixels)
out vec4 fragColor;
void main() {
    ivec2 ip = ivec2(gl_FragCoord.xy);
    ivec2 lim = ivec2(sharpen.yz) - 1;
    vec3 c = texelFetch(srcTex, ip, 0).rgb;
    vec3 n = texelFetch(srcTex, clamp(ip + ivec2(0, 1), ivec2(0), lim), 0).rgb;
    vec3 s = texelFetch(srcTex, clamp(ip - ivec2(0, 1), ivec2(0), lim), 0).rgb;
    vec3 e = texelFetch(srcTex, clamp(ip + ivec2(1, 0), ivec2(0), lim), 0).rgb;
    vec3 w = texelFetch(srcTex, clamp(ip - ivec2(1, 0), ivec2(0), lim), 0).rgb;
    vec3 mn = min(c, min(min(n, s), min(e, w)));
    vec3 mx = max(c, max(max(n, s), max(e, w)));
    vec3 margin = 0.25 * (mx - mn);
    vec3 o = clamp(c + sharpen.x * (c - 0.25 * (n + s + e + w)), mn - margin, mx + margin);
    float a = fract(52.9829189 * fract(dot(gl_FragCoord.xy, vec2(0.06711056, 0.00583715))));
    float b = fract(52.9829189 * fract(dot(gl_FragCoord.yx + 17.0, vec2(0.06711056, 0.00583715))));
    o += (a + b - 1.0) / 255.0;
    fragColor = vec4(clamp(o, 0.0, 1.0), 1.0);
}
)";

// Luma sharpen (the Sharpen stage with TAA off, the sharpen switch):
// contrast-adaptive sharpening (AMD FidelityFX CAS, 3x3 form) of the
// composited frame, applied to brightness only (a full-colour CAS
// turned the coloured texel noise of pale ground into speckle), into every
// sample of the frame target. Strength 0 to 1 sets the CAS peak from -1/8 to
// -1/5; the backend skips the stage at strength 0.
static const char s_psPostLumaSharpenGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D srcTex; // the composited frame
uniform vec4 sharpen;     // x strength 0..1, yz size (pixels)
out vec4 fragColor;
vec3 tap(ivec2 ip, ivec2 o, ivec2 lim) { return texelFetch(srcTex, clamp(ip + o, ivec2(0), lim), 0).rgb; }
void main() {
    ivec2 ip = ivec2(gl_FragCoord.xy);
    ivec2 lim = ivec2(sharpen.yz) - 1;
    vec3 a = tap(ip, ivec2(-1, -1), lim);
    vec3 b = tap(ip, ivec2(0, -1), lim);
    vec3 c = tap(ip, ivec2(1, -1), lim);
    vec3 d = tap(ip, ivec2(-1, 0), lim);
    vec3 e = texelFetch(srcTex, ip, 0).rgb;
    vec3 f = tap(ip, ivec2(1, 0), lim);
    vec3 g = tap(ip, ivec2(-1, 1), lim);
    vec3 h = tap(ip, ivec2(0, 1), lim);
    vec3 i = tap(ip, ivec2(1, 1), lim);
    // Soft minimum and maximum of the cross plus the corners.
    vec3 mn = min(min(min(d, e), min(f, b)), h);
    mn += min(mn, min(min(a, c), min(g, i)));
    vec3 mx = max(max(max(d, e), max(f, b)), h);
    mx += max(mx, max(max(a, c), max(g, i)));
    // Less sharpening where the neighbourhood is near black or white.
    vec3 amp = sqrt(clamp(min(mn, 2.0 - mx) / max(mx, vec3(1e-5)), 0.0, 1.0));
    vec3 w = amp * (-1.0 / mix(8.0, 5.0, clamp(sharpen.x, 0.0, 1.0)));
    vec3 sh = clamp((b * w + d * w + f * w + h * w + e) / (1.0 + 4.0 * w), 0.0, 1.0);
    // Brightness only: the sharpened luma onto the pixel's own colour.
    const vec3 kLuma = vec3(0.2126, 0.7152, 0.0722);
    fragColor = vec4(clamp(e + (dot(sh, kLuma) - dot(e, kLuma)), 0.0, 1.0), 1.0);
}
)";

// Every post shader, assembled from the raw strings above: these pieces in this
// order, in PostShader order. The lighting lab's glsl_extract.py reads this
// table to assemble the same sources.
struct PostShaderDef
{
    const char* name; // override file stem
    const char* pieces[8];
};
const PostShaderDef kPostShaders[] = {
    {"vsPost", {s_vsPostGLSL}},
    {"psPostAo", {s_psPostAoInputs, s_glslPostCommon, s_psPostAoMain}},
    {"psPostAoBlur", {s_psPostAoBlurGLSL}},
    {"psPostBloomDown", {s_psPostBloomDownHead, Poseidon::gl33lighting::kCameraCodecGLSL, s_psPostBloomDownMain}},
    {"psPostBloomUp", {s_psPostBloomUpGLSL}},
    {"psPostShafts", {s_psPostShaftsHead, Poseidon::gl33lighting::kCameraCodecGLSL, s_psPostShaftsMain}},
    {"psPostComposite", {s_psPostCompositeHead, Poseidon::gl33lighting::kCameraCodecGLSL, s_psPostCompositeMain}},
    {"psPostLinearDepth", {s_psPostLinearDepthInputs, s_glslPostCommon, s_psPostLinearDepthMain}},
    {"psPostDebugDepth", {s_psPostDebugDepthGLSL}},
    {"psPostTaa", {s_psPostTaaInputs, s_psPostTaaMain}},
    {"psPostTaaSharpen", {s_psPostTaaSharpenGLSL}},
    {"psSkyView",
     {Poseidon::gl33lighting::kPostSkyTableHeaderGLSL, Poseidon::gl33lighting::kPostSkyTableModelGLSL,
      Poseidon::gl33lighting::kPostSkyViewMainGLSL}},
    {"psSkyViewMean",
     {Poseidon::gl33lighting::kPostSkyTableHeaderGLSL, Poseidon::gl33lighting::kPostSkyViewMeanMainGLSL}},
    {"psPostLumaSharpen", {s_psPostLumaSharpenGLSL}},
    {"psCanopySun", {Poseidon::gl33lighting::kPostCanopySunGLSL}},
    {"psPostAirMarch",
     {Poseidon::gl33lighting::kPostAirMarchInputsGLSL, Poseidon::gl33lighting::kLitConstantsBlockGLSL,
      Poseidon::gl33lighting::kRemasterTerrainCloudsGLSL, Poseidon::gl33lighting::kPostAirCommonGLSL,
      Poseidon::gl33lighting::kPostAirMarchHelpersGLSL, Poseidon::gl33lighting::kPostAirMarchMainGLSL}},
    {"psPostAirBlur", {Poseidon::gl33lighting::kPostAirBlurGLSL}},
    {"psPostAirMean", {Poseidon::gl33lighting::kPostAirMeanGLSL}},
    {"psPostAirApply",
     {Poseidon::gl33lighting::kPostAirApplyInputsGLSL, Poseidon::gl33lighting::kLitConstantsBlockGLSL,
      Poseidon::gl33lighting::kCameraCodecGLSL, Poseidon::gl33lighting::kRemasterSkyGLSL,
      Poseidon::gl33lighting::kPostAirCommonGLSL, Poseidon::gl33lighting::kPostAirApplyMainGLSL}},
    // The screen-space reflections (EngineGL33_PostSsrShaders.cpp). The trace
    // pastes the LitConstants block, the sky piece and the puddles piece of the
    // fragment prelude (the wet weather's puddles, the same as the forward pass's).
    {"psPostSsrTag", {Poseidon::gl33lighting::kPostSsrTagGLSL}},
    {"psPostSsrShare", {Poseidon::gl33lighting::kPostSsrShareGLSL}},
    {"psPostSsrTrace",
     {Poseidon::gl33lighting::kPostSsrTraceInputsGLSL, Poseidon::gl33lighting::kLitConstantsBlockGLSL,
      Poseidon::gl33lighting::kCameraCodecGLSL, Poseidon::gl33lighting::kRemasterSkyGLSL,
      Poseidon::gl33lighting::kRemasterPuddlesGLSL, Poseidon::gl33lighting::kPostSsrTraceMainGLSL}},
    {"psPostSsrBlur", {Poseidon::gl33lighting::kPostSsrBlurGLSL}},
    {"psPostSsrComposite",
     {Poseidon::gl33lighting::kPostSsrCompositeInputsGLSL, s_glslPostCommon, Poseidon::gl33lighting::kCameraCodecGLSL,
      Poseidon::gl33lighting::kPostSsrCompositeMainGLSL}},
    {"psCombatMask", {Poseidon::gl33lighting::kPostCombatMaskGLSL}},
    {"psCombatMaskBlur", {Poseidon::gl33lighting::kPostCombatMaskBlurGLSL}},
    {"psPostGtao",
     {Poseidon::gl33lighting::kPostGtaoInputsGLSL, s_glslPostCommon, Poseidon::gl33lighting::kPostGtaoMainGLSL}},
    {"psPostGtaoAccum", {Poseidon::gl33lighting::kPostGtaoAccumGLSL}},
    {"psPostMeter", {s_psPostMeterHead, Poseidon::gl33lighting::kCameraCodecGLSL, s_psPostMeterMain}},
    {"psFogInject",
     {Poseidon::gl33lighting::kPostFogInjectHeadGLSL, Poseidon::gl33lighting::kPostFogSharedInputsGLSL,
      Poseidon::gl33lighting::kLitConstantsBlockGLSL, Poseidon::gl33lighting::kRemasterTerrainCloudsGLSL,
      Poseidon::gl33lighting::kPostFogSkyInputsGLSL, Poseidon::gl33lighting::kRemasterSkyGLSL,
      Poseidon::gl33lighting::kPostFogMediumGLSL, Poseidon::gl33lighting::kPostFogInjectMainGLSL}},
    {"psFogIntegrate", {Poseidon::gl33lighting::kPostFogIntegrateGLSL}},
    {"psFogNear",
     {Poseidon::gl33lighting::kPostFogNearHeadGLSL, Poseidon::gl33lighting::kPostFogSharedInputsGLSL,
      Poseidon::gl33lighting::kLitConstantsBlockGLSL, Poseidon::gl33lighting::kRemasterTerrainCloudsGLSL,
      Poseidon::gl33lighting::kPostFogMediumGLSL, Poseidon::gl33lighting::kPostFogNearMainGLSL}},
    {"psFogApply",
     {Poseidon::gl33lighting::kPostFogApplyGLSL, Poseidon::gl33lighting::kLitConstantsBlockGLSL,
      Poseidon::gl33lighting::kCameraCodecGLSL, Poseidon::gl33lighting::kPostFogApplyMainGLSL}},
    {"psFogRing",
     {Poseidon::gl33lighting::kPostFogRingGLSL, Poseidon::gl33lighting::kCameraCodecGLSL,
      Poseidon::gl33lighting::kPostFogRingMainGLSL}},
    {"psFogGlow",
     {Poseidon::gl33lighting::kPostFogGlowGLSL, Poseidon::gl33lighting::kCameraCodecGLSL,
      Poseidon::gl33lighting::kPostFogGlowMainGLSL}},
};
constexpr int kPostShaderCount = static_cast<int>(Poseidon::gl33lighting::PostShader::Count);
static_assert(std::size(kPostShaders) == static_cast<size_t>(kPostShaderCount), "one entry per PostShader");

} // namespace

namespace Poseidon::gl33lighting
{

const char* PostShaderName(PostShader shader)
{
    const int i = static_cast<int>(shader);
    return i >= 0 && i < kPostShaderCount ? kPostShaders[i].name : nullptr;
}

const char* PostShaderSource(PostShader shader)
{
    // Joined once, on first use (thread-safe static).
    static const std::array<std::string, kPostShaderCount> sources = []
    {
        std::array<std::string, kPostShaderCount> joined;
        for (int i = 0; i < kPostShaderCount; ++i)
            for (const char* piece : kPostShaders[i].pieces)
                if (piece)
                    joined[i] += piece;
        return joined;
    }();
    const int i = static_cast<int>(shader);
    return i >= 0 && i < kPostShaderCount ? sources[i].c_str() : nullptr;
}

} // namespace Poseidon::gl33lighting
