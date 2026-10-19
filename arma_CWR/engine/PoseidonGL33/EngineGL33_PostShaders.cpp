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

// Bloom downsample: four bilinear taps (a 4x4 box). The first level linearises
// the display-space image and keeps only what is close to white, with a soft
// knee: the sky as a whole and sunlit walls stay out (no haze halo along the
// skyline), the sun, its halo, glints and sharp highlights bloom.
static const char s_psPostBloomDownGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D srcTex;
uniform vec2 srcTexel;
uniform vec4 bloomParams; // x threshold (linear), y knee, z 1 = first level
out vec4 fragColor;
void main() {
    vec3 c = texture(srcTex, vUV + srcTexel * vec2(-1.0, -1.0)).rgb;
    c += texture(srcTex, vUV + srcTexel * vec2(1.0, -1.0)).rgb;
    c += texture(srcTex, vUV + srcTexel * vec2(-1.0, 1.0)).rgb;
    c += texture(srcTex, vUV + srcTexel * vec2(1.0, 1.0)).rgb;
    c *= 0.25;
    if (bloomParams.z > 0.5) {
        c = pow(max(c, vec3(0.0)), vec3(2.2));
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
static const char s_psPostShaftsGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D depthTex;
uniform sampler2D colorTex;
uniform vec4 sunScreen; // xy sun position (uv), z on-screen weight, w aspect (width/height)
uniform float noiseShift; // TAA: this frame's shift of the march start (0 = the fixed pattern)
out vec4 fragColor;
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
        vec3 s = pow(texture(colorTex, c).rgb, vec3(2.2));
        sum += s * sky * decay;
        decay *= 0.965;
    }
    float d = length(toSun * vec2(sunScreen.w, 1.0));
    float radial = exp(-d * 2.2);
    fragColor = vec4(sum / float(kSteps) * radial * sunScreen.z, 1.0);
}
)";

static const char s_psPostCompositeGLSL[] = R"(#version 330 core
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
out vec4 fragColor;

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

void main() {
    vec4 c = texture(colorTex, vUV);
    float ao = texture(aoTex, vUV).r;
    if (debugView.x > 0.5) { fragColor = vec4(vec3(ao), 1.0); return; }
    // Take the occluded part of the sky light out of the radiance. c.a is one
    // minus the pixel's sky light share (1 for pre-lit and coverage pixels).
    float share = 1.0 - clamp(c.a, 0.0, 1.0);
    float keep = max(1.0 - share * (1.0 - ao) * compParams.x, 0.0);
    vec3 lin = pow(clamp(c.rgb, 0.0, 1.0), vec3(2.2)) * keep;
    // Glow adds like light onto a screen: 1 - (1 - base)(1 - glow), with the
    // glow's own energy rolled off. Identity where there is no glow, never
    // above 1, so the tonemapped frame keeps its whites and the sky its colour.
    vec3 shaft = texture(shaftTex, vUV).rgb;
    if (max(shaft.r, max(shaft.g, shaft.b)) > 0.0) shaft *= shaftInscatter();
    vec3 glow = texture(bloomTex, vUV).rgb * compParams.y + shaft * shaftColor.rgb * compParams.z;
    lin += (1.0 - lin) * (1.0 - exp(-max(glow, vec3(0.0))));
    vec3 col = pow(lin, vec3(1.0 / 2.2));
    // Dither only what the composite changed: elsewhere the frame passes
    // through exactly (the 2.2 round trip is exact on 8-bit values).
    if (compParams.w > 0.5 && (keep < 0.9999 || max(glow.r, max(glow.g, glow.b)) > 1e-5)) {
        float n = fract(52.9829189 * fract(dot(gl_FragCoord.xy, vec2(0.06711056, 0.00583715))));
        float m = fract(52.9829189 * fract(dot(gl_FragCoord.yx + 17.0, vec2(0.06711056, 0.00583715))));
        col += (n + m - 1.0) / 255.0; // triangular dither, +-1 LSB
    }
    fragColor = vec4(col, 1.0);
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

// TAA resolve (the Temporal stage, step A5; Graphics/Remaster/TemporalAA.hpp).
// This frame was drawn with a sub-pixel jitter; the history is the resolved
// frame before it, with each pixel's view depth in alpha. Per pixel:
//   - rebuild this frame at the pixel centre without the jitter: Gaussian
//     weights exp(-4 d^2) (sigma 0.35 px) over the 3x3 neighbourhood centred
//     on the jitter offset. The converged image is the scene filtered by this
//     kernel: Karis's wider exp(-2.29 d^2) left the hero view 15% softer than
//     without TAA, this one 5% (A5 sweep);
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

// Luma sharpen (the Sharpen stage with TAA off, step B11, the sharpen switch):
// contrast-adaptive sharpening (AMD FidelityFX CAS, 3x3 form) of the
// composited frame, applied to brightness only (the research's full-colour CAS
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
    const char* pieces[7];
};
const PostShaderDef kPostShaders[] = {
    {"vsPost", {s_vsPostGLSL}},
    {"psPostAo", {s_psPostAoInputs, s_glslPostCommon, s_psPostAoMain}},
    {"psPostAoBlur", {s_psPostAoBlurGLSL}},
    {"psPostBloomDown", {s_psPostBloomDownGLSL}},
    {"psPostBloomUp", {s_psPostBloomUpGLSL}},
    {"psPostShafts", {s_psPostShaftsGLSL}},
    {"psPostComposite", {s_psPostCompositeGLSL}},
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
      Poseidon::gl33lighting::kRemasterSkyGLSL, Poseidon::gl33lighting::kPostAirCommonGLSL,
      Poseidon::gl33lighting::kPostAirApplyMainGLSL}},
    // The screen-space reflections (EngineGL33_PostSsrShaders.cpp). The trace
    // pastes the LitConstants block, the sky piece and the puddles piece of the
    // fragment prelude (the wet weather's puddles, the same as the forward pass's).
    {"psPostSsrTag", {Poseidon::gl33lighting::kPostSsrTagGLSL}},
    {"psPostSsrShare", {Poseidon::gl33lighting::kPostSsrShareGLSL}},
    {"psPostSsrTrace",
     {Poseidon::gl33lighting::kPostSsrTraceInputsGLSL, Poseidon::gl33lighting::kLitConstantsBlockGLSL,
      Poseidon::gl33lighting::kRemasterSkyGLSL, Poseidon::gl33lighting::kRemasterPuddlesGLSL,
      Poseidon::gl33lighting::kPostSsrTraceMainGLSL}},
    {"psPostSsrBlur", {Poseidon::gl33lighting::kPostSsrBlurGLSL}},
    {"psPostSsrComposite",
     {Poseidon::gl33lighting::kPostSsrCompositeInputsGLSL, s_glslPostCommon,
      Poseidon::gl33lighting::kPostSsrCompositeMainGLSL}},
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
