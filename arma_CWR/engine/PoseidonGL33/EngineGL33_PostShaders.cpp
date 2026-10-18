// Remastered lighting: the post-pass shaders. EngineGL33_PostFX.cpp runs the
// passes; this file holds their GLSL and no GL code, so the unit tests compile
// the very sources the engine builds. Each shader is assembled from the raw
// strings below by the kPostShaders table and read through PostShaderName and
// PostShaderSource (EngineGL33_Lighting.hpp). s_glslPostCommon holds the depth
// reconstruction the passes share; a pass that adds a shader adds its row to
// the table.

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
    float noise = fract(52.9829189 * fract(dot(gl_FragCoord.xy, vec2(0.06711056, 0.00583715))));
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
out vec4 fragColor;
void main() {
    if (sunScreen.z <= 0.0) { fragColor = vec4(0.0); return; }
    const int kSteps = 40;
    vec2 toSun = sunScreen.xy - vUV;
    vec2 stepUV = toSun / float(kSteps) * 0.85;
    float noise = fract(52.9829189 * fract(dot(gl_FragCoord.xy, vec2(0.06711056, 0.00583715))));
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

// Every post shader, assembled from the raw strings above: these pieces in this
// order, in PostShader order. The lighting lab's glsl_extract.py reads this
// table to assemble the same sources.
struct PostShaderDef
{
    const char* name; // override file stem
    const char* pieces[3];
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
