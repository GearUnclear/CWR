// Remastered lighting: post pass at the world/HUD boundary.
//
// World::Draw calls Engine::EndWorldScene() after the 3D scene and before the
// map, HUD and dialogs. When the remastered lighting and its post effects are
// on, this resolves the frame target (colour + depth), then:
//   1. ambient occlusion (Alchemy/SAO style) at half resolution from the depth
//      buffer, blurred with a depth-aware separable filter;
//   2. a five-level bloom chain from the bright parts of the image;
//   3. sun shafts: a radial blur of the visible sky toward the sun;
//   4. a composite back into the frame target. Occlusion darkens only the sky
//      light: the remastered shaders store one minus each opaque pixel's sky
//      light share in the colour alpha (blended draws keep the destination
//      alpha), so a sunlit wall next to a corner keeps its sunlight and pre-lit
//      or alpha-to-coverage pixels (alpha 1) are left alone. Bloom and shafts
//      add in linear light with a soft screen-style add that can never clip,
//      and leave the frame exactly as it was where they are zero (the frame is
//      already tonemapped: no second curve). A dither hides banding.
// The HUD then draws on top, untouched.
//
// All GL state the passes touch is restored or the bind caches invalidated;
// texture unit 1 is never used (its sampler object belongs to ApplyPipeline).

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <Poseidon/Graphics/Core/GLBlendState.hpp>
#include <Poseidon/Graphics/Core/GLDepthStencilState.hpp>
#include <Poseidon/Graphics/Core/GLPipelineState.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>

#include <algorithm>
#include <cmath>

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

// Half-resolution ambient occlusion. Output r = visibility (1 = open), g = view
// depth in metres for the bilateral blur.
static const char s_psPostAoGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D depthTex;
uniform vec4 projInfo;  // x = _33, y = _43 (viewZ = y / (ndcZ - x)), z = 1/_11, w = 1/_22
uniform vec4 depthMap;  // ndcZ = depth * x + y
uniform vec4 aoParams;  // x radius (m), y strength, z fade start (m), w fade end (m)
uniform vec4 screen;    // xy full-res size, zw 1/size
uniform vec4 worldRect; // xy origin, zw size of the 3D viewport in uv (pillarbox crop)
out vec4 fragColor;

float viewZ(vec2 uv) {
    float d = texture(depthTex, uv).r;
    return projInfo.y / ((d * depthMap.x + depthMap.y) - projInfo.x);
}

vec3 viewPos(vec2 uv, float z) {
    vec2 ndc = (uv - worldRect.xy) / worldRect.zw * 2.0 - 1.0;
    return vec3(ndc.x * z * projInfo.z, ndc.y * z * projInfo.w, z);
}

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
uniform vec4 compParams; // x AO strength, y bloom strength, z shaft strength, w dither (0/1)
uniform vec4 shaftColor; // rgb sun tint
uniform vec4 debugView;  // x: 1 = show occlusion
out vec4 fragColor;

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
    vec3 glow = texture(bloomTex, vUV).rgb * compParams.y + texture(shaftTex, vUV).rgb * shaftColor.rgb * compParams.z;
    lin += (1.0 - lin) * (1.0 - exp(-max(glow, vec3(0.0))));
    vec3 col = pow(lin, vec3(1.0 / 2.2));
    if (compParams.w > 0.5) {
        float n = fract(52.9829189 * fract(dot(gl_FragCoord.xy, vec2(0.06711056, 0.00583715))));
        float m = fract(52.9829189 * fract(dot(gl_FragCoord.yx + 17.0, vec2(0.06711056, 0.00583715))));
        col += (n + m - 1.0) / 255.0; // triangular dither, +-1 LSB
    }
    fragColor = vec4(col, 1.0);
}
)";

// Post constants shared with the lab (tests/rendering/lighting_lab/postfx.py).
constexpr float kAoRadius = 1.2f;      // metres
constexpr float kAoFadeStart = 120.0f; // metres: AO fades out with distance
constexpr float kAoFadeEnd = 250.0f;
constexpr float kBloomThreshold = 0.82f; // linear peak where bloom starts
constexpr float kBloomKnee = 0.12f;

GLuint CompilePostShader(GLenum type, const char* src, const char* name)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        LOG_ERROR(Graphics, "GL33 post: shader compile error [{}]: {}", name, log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

GLuint LinkPostProgram(GLuint vs, const char* fsSrc, const char* name)
{
    GLuint fs = CompilePostShader(GL_FRAGMENT_SHADER, fsSrc, name);
    if (!vs || !fs)
    {
        if (fs)
            glDeleteShader(fs);
        return 0;
    }
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    glDeleteShader(fs);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetProgramInfoLog(p, sizeof(log), nullptr, log);
        LOG_ERROR(Graphics, "GL33 post: program link error [{}]: {}", name, log);
        glDeleteProgram(p);
        return 0;
    }
    return p;
}

GLuint MakeTexture(GLenum internalFormat, GLenum format, GLenum type, int w, int h, bool linear)
{
    GLuint t = 0;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, type, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    return t;
}

GLuint MakeFbo(GLenum attachment, GLuint tex)
{
    GLuint f = 0;
    glGenFramebuffers(1, &f);
    glBindFramebuffer(GL_FRAMEBUFFER, f);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, tex, 0);
    return f;
}

} // namespace

void EngineGL33::DestroyPostTargets()
{
    auto delTex = [](unsigned int& t)
    {
        if (t)
        {
            GL33Bind::OnTexDeleted(t);
            glDeleteTextures(1, &t);
        }
        t = 0;
    };
    auto delFbo = [](unsigned int& f)
    {
        if (f)
            glDeleteFramebuffers(1, &f);
        f = 0;
    };
    PostTargets& p = _post;
    delFbo(p.colorFbo);
    delFbo(p.depthFbo);
    delFbo(p.aoFbo[0]);
    delFbo(p.aoFbo[1]);
    delFbo(p.shaftFbo);
    for (int i = 0; i < PostTargets::kBloomLevels; ++i)
    {
        delFbo(p.bloomFbo[i]);
        delTex(p.bloomTex[i]);
    }
    delTex(p.colorTex);
    delTex(p.depthTex);
    delTex(p.aoTex[0]);
    delTex(p.aoTex[1]);
    delTex(p.shaftTex);
    p.w = p.h = 0;
}

void EngineGL33::DestroyPostPrograms()
{
    PostTargets& p = _post;
    for (unsigned int* prog :
         {&p.progAo, &p.progAoBlur, &p.progBloomDown, &p.progBloomUp, &p.progShafts, &p.progComposite})
    {
        if (*prog)
            glDeleteProgram(*prog);
        *prog = 0;
    }
    if (p.vao)
    {
        GL33Bind::OnVaoDeleted(p.vao);
        glDeleteVertexArrays(1, &p.vao);
    }
    p.vao = 0;
    p.programsTried = false;
}

bool EngineGL33::EnsurePostResources(int w, int h)
{
    PostTargets& p = _post;
    if (!p.programsTried)
    {
        p.programsTried = true;
        GLuint vs = CompilePostShader(GL_VERTEX_SHADER, s_vsPostGLSL, "vsPost");
        p.progAo = LinkPostProgram(vs, s_psPostAoGLSL, "psPostAo");
        p.progAoBlur = LinkPostProgram(vs, s_psPostAoBlurGLSL, "psPostAoBlur");
        p.progBloomDown = LinkPostProgram(vs, s_psPostBloomDownGLSL, "psPostBloomDown");
        p.progBloomUp = LinkPostProgram(vs, s_psPostBloomUpGLSL, "psPostBloomUp");
        p.progShafts = LinkPostProgram(vs, s_psPostShaftsGLSL, "psPostShafts");
        p.progComposite = LinkPostProgram(vs, s_psPostCompositeGLSL, "psPostComposite");
        if (vs)
            glDeleteShader(vs);
        glGenVertexArrays(1, &p.vao);
        if (!p.progAo || !p.progAoBlur || !p.progBloomDown || !p.progBloomUp || !p.progShafts || !p.progComposite)
        {
            LOG_ERROR(Graphics, "GL33 post: programs failed to build; remastered post effects disabled");
            p.unavailable = true;
        }
    }
    if (p.unavailable)
        return false;
    if (p.w == w && p.h == h && p.colorTex)
        return true;

    DestroyPostTargets();
    const int hw = (w + 1) / 2;
    const int hh = (h + 1) / 2;
    GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
    p.colorTex = MakeTexture(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, w, h, true);
    p.depthTex = MakeTexture(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, w, h, false);
    p.aoTex[0] = MakeTexture(GL_RG16F, GL_RG, GL_HALF_FLOAT, hw, hh, true);
    p.aoTex[1] = MakeTexture(GL_RG16F, GL_RG, GL_HALF_FLOAT, hw, hh, true);
    p.shaftTex = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, hw, hh, true);
    int bw = hw, bh = hh;
    for (int i = 0; i < PostTargets::kBloomLevels; ++i)
    {
        p.bloomW[i] = bw = bw > 1 ? bw : 1;
        p.bloomH[i] = bh = bh > 1 ? bh : 1;
        p.bloomTex[i] = MakeTexture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, bw, bh, true);
        bw = (bw + 1) / 2;
        bh = (bh + 1) / 2;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    GL33Bind::Invalidate();

    bool complete = true;
    auto check = [&complete]()
    {
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            complete = false;
    };
    p.colorFbo = MakeFbo(GL_COLOR_ATTACHMENT0, p.colorTex);
    check();
    p.depthFbo = MakeFbo(GL_DEPTH_STENCIL_ATTACHMENT, p.depthTex);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    check();
    for (int i = 0; i < 2; ++i)
    {
        p.aoFbo[i] = MakeFbo(GL_COLOR_ATTACHMENT0, p.aoTex[i]);
        check();
    }
    p.shaftFbo = MakeFbo(GL_COLOR_ATTACHMENT0, p.shaftTex);
    check();
    for (int i = 0; i < PostTargets::kBloomLevels; ++i)
    {
        p.bloomFbo[i] = MakeFbo(GL_COLOR_ATTACHMENT0, p.bloomTex[i]);
        check();
    }
    BindFrameRenderTarget();
    if (!complete)
    {
        LOG_WARN(Graphics, "GL33 post: targets {}x{} incomplete; remastered post effects disabled", w, h);
        DestroyPostTargets();
        p.unavailable = true;
        return false;
    }
    p.w = w;
    p.h = h;
    LOG_INFO(Graphics, "GL33 post: targets {}x{} (AO/shafts {}x{})", w, h, hw, hh);
    return true;
}

void EngineGL33::EndWorldScene()
{
    // Only the frame that actually drew the world through the remastered
    // shaders has a sky light share in its alpha and a world depth buffer.
    const bool worldDrawn = _worldPassOpen;
    // Queued world draws commit with the remastered programs before the HUD
    // and dialogs switch back to the classic set.
    if (_remasterPrograms)
        FlushAndFreeAllQueues(_queueNo);
    _worldPassOpen = false;
    _worldEffectsOn = false;
    _worldSceneEnded = true;
    UpdateRemasterPrograms();
    if (!worldDrawn || !_glContext || !_lightingActive || !_lighting.postEffects || !SSAAActive() ||
        _portraitTargetSize)
        return;

    FlushAndFreeAllQueues(_queueNo);
    int w = 0, h = 0;
    RenderTargetSize(w, h);
    if (w <= 0 || h <= 0 || !EnsurePostResources(w, h))
        return;
    PostTargets& p = _post;

    // Resolve the multisampled frame target: colour (with the ambient share in
    // alpha) and depth.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _ssaaFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, p.colorFbo);
    glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, p.depthFbo);
    glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);

    // Common state for the fullscreen passes.
    Poseidon::render::depthstencil::Disabled(/*hasStencil*/ false);
    glDisable(GL_STENCIL_TEST);
    Poseidon::render::blend::Opaque();
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    Poseidon::render::pipeline::DisableDepthClamp();
    glBindVertexArray(p.vao);
    for (int unit : {0, 2, 3, 4, 5})
        glBindSampler(static_cast<GLuint>(unit), 0);

    auto bindTex = [](int unit, GLuint tex)
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, tex);
    };
    auto drawTo = [](GLuint fbo, int vw, int vh)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, vw, vh);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    };
    auto u1i = [](GLuint prog, const char* name, int v)
    {
        const GLint loc = glGetUniformLocation(prog, name);
        if (loc >= 0)
            glUniform1i(loc, v);
    };
    auto u4f = [](GLuint prog, const char* name, float a, float b, float c, float d)
    {
        const GLint loc = glGetUniformLocation(prog, name);
        if (loc >= 0)
            glUniform4f(loc, a, b, c, d);
    };
    auto u2f = [](GLuint prog, const char* name, float a, float b)
    {
        const GLint loc = glGetUniformLocation(prog, name);
        if (loc >= 0)
            glUniform2f(loc, a, b);
    };

    const int hw = (w + 1) / 2;
    const int hh = (h + 1) / 2;
    const float aoStrength = _lighting.ambientOcclusion;
    const float bloomStrength = _lighting.bloom;

    // 1. Ambient occlusion + blur.
    const GfxMatrix& proj = _frameState.projection;
    // The 3D scene may fill only a sub-rectangle (aspect pillarbox); uv <-> NDC
    // goes through that rectangle.
    const Poseidon::AspectSettings& as = _aspectSettings;
    const bool fullRect =
        as.worldLeft <= 0.0f && as.worldTop <= 0.0f && as.worldRight >= 1.0f && as.worldBottom >= 1.0f;
    const float rectX = fullRect ? 0.0f : as.worldLeft;
    const float rectY = fullRect ? 0.0f : 1.0f - as.worldBottom;
    const float rectW = fullRect ? 1.0f : as.worldRight - as.worldLeft;
    const float rectH = fullRect ? 1.0f : as.worldBottom - as.worldTop;
    const float depthScale = GLAD_GL_ARB_clip_control ? 1.0f : 2.0f;
    const float depthBias = GLAD_GL_ARB_clip_control ? 0.0f : -1.0f;
    if (aoStrength > 0.0f)
    {
        glUseProgram(p.progAo);
        u1i(p.progAo, "depthTex", 0);
        u4f(p.progAo, "projInfo", proj._33, proj._43, 1.0f / proj._11, 1.0f / proj._22);
        u4f(p.progAo, "depthMap", depthScale, depthBias, 0.0f, 0.0f);
        u4f(p.progAo, "aoParams", kAoRadius, 1.0f, kAoFadeStart, kAoFadeEnd);
        u4f(p.progAo, "screen", static_cast<float>(w), static_cast<float>(h), 1.0f / w, 1.0f / h);
        u4f(p.progAo, "worldRect", rectX, rectY, rectW, rectH);
        bindTex(0, p.depthTex);
        drawTo(p.aoFbo[0], hw, hh);
        glUseProgram(p.progAoBlur);
        u1i(p.progAoBlur, "aoTex", 0);
        bindTex(0, p.aoTex[0]);
        u2f(p.progAoBlur, "stepUV", 1.0f / hw, 0.0f);
        drawTo(p.aoFbo[1], hw, hh);
        bindTex(0, p.aoTex[1]);
        u2f(p.progAoBlur, "stepUV", 0.0f, 1.0f / hh);
        drawTo(p.aoFbo[0], hw, hh);
    }

    // 2. Bloom chain.
    if (bloomStrength > 0.0f)
    {
        glUseProgram(p.progBloomDown);
        u1i(p.progBloomDown, "srcTex", 0);
        for (int i = 0; i < PostTargets::kBloomLevels; ++i)
        {
            const bool first = i == 0;
            const GLuint src = first ? p.colorTex : p.bloomTex[i - 1];
            const int sw = first ? w : p.bloomW[i - 1];
            const int sh = first ? h : p.bloomH[i - 1];
            bindTex(0, src);
            u2f(p.progBloomDown, "srcTexel", 1.0f / sw, 1.0f / sh);
            u4f(p.progBloomDown, "bloomParams", kBloomThreshold, kBloomKnee, first ? 1.0f : 0.0f, 0.0f);
            drawTo(p.bloomFbo[i], p.bloomW[i], p.bloomH[i]);
        }
        glUseProgram(p.progBloomUp);
        u1i(p.progBloomUp, "srcTex", 0);
        Poseidon::render::blend::Additive(); // (SRC_ALPHA, ONE) with alpha 1 = add
        for (int i = PostTargets::kBloomLevels - 1; i > 0; --i)
        {
            bindTex(0, p.bloomTex[i]);
            u2f(p.progBloomUp, "srcTexel", 1.0f / p.bloomW[i], 1.0f / p.bloomH[i]);
            drawTo(p.bloomFbo[i - 1], p.bloomW[i - 1], p.bloomH[i - 1]);
        }
        Poseidon::render::blend::Opaque();
    }

    // 3. Sun shafts, when the sun is in front of the camera and above the horizon.
    float shaftWeight = 0.0f;
    float sunU = 0.5f, sunV = 0.5f;
    Color shaftTint(1, 1, 1, 1);
    if (_lighting.sunShafts > 0.0f && GScene)
    {
        LightSun* sun = GScene->MainLight();
        Camera* cam = GScene->GetCamera();
        const Vector3 toSun = -sun->SunDirection();
        // Camera space: InverseScaled maps world directions into view space.
        const Vector3 v = cam->InverseScaled().Rotate(toSun);
        if (v.Z() > 0.05f && toSun.Y() > -0.02f)
        {
            const float ndcX = proj._11 * v.X() / v.Z();
            const float ndcY = proj._22 * v.Y() / v.Z();
            sunU = rectX + (ndcX * 0.5f + 0.5f) * rectW;
            sunV = rectY + (ndcY * 0.5f + 0.5f) * rectH;
            const float offX = std::fabs(ndcX) > 1.0f ? std::fabs(ndcX) - 1.0f : 0.0f;
            const float offY = std::fabs(ndcY) > 1.0f ? std::fabs(ndcY) - 1.0f : 0.0f;
            const float onScreen = std::exp(-3.0f * (offX + offY));
            const float lowSun = 0.55f + 0.45f * (1.0f - std::min(1.0f, toSun.Y() / 0.6f));
            Landscape* land = GScene->GetLandscape();
            const float clear = land ? land->SkyThrough() : 1.0f;
            shaftWeight = onScreen * lowSun * clear * (1.0f - sun->NightEffect());
            shaftTint = _lightRigSun;
        }
    }
    if (shaftWeight > 0.001f)
    {
        glUseProgram(p.progShafts);
        u1i(p.progShafts, "depthTex", 0);
        u1i(p.progShafts, "colorTex", 2);
        bindTex(0, p.depthTex);
        bindTex(2, p.colorTex);
        u4f(p.progShafts, "sunScreen", sunU, sunV, shaftWeight, static_cast<float>(w) / static_cast<float>(h));
        drawTo(p.shaftFbo, hw, hh);
    }

    // 4. Composite into the frame target (every sample gets the resolved value).
    glUseProgram(p.progComposite);
    u1i(p.progComposite, "colorTex", 0);
    u1i(p.progComposite, "aoTex", 2);
    u1i(p.progComposite, "bloomTex", 3);
    u1i(p.progComposite, "shaftTex", 4);
    bindTex(0, p.colorTex);
    bindTex(2, aoStrength > 0.0f ? p.aoTex[0] : p.whiteTex);
    bindTex(3, bloomStrength > 0.0f ? p.bloomTex[0] : p.blackTex);
    bindTex(4, shaftWeight > 0.001f ? p.shaftTex : p.blackTex);
    u4f(p.progComposite, "compParams", aoStrength, bloomStrength * 0.25f, _lighting.sunShafts * 0.6f, 1.0f);
    const float lum = std::max(1e-4f, 0.2126f * shaftTint.R() + 0.7152f * shaftTint.G() + 0.0722f * shaftTint.B());
    u4f(p.progComposite, "shaftColor", shaftTint.R() / lum, shaftTint.G() / lum, shaftTint.B() / lum, 1.0f);
    u4f(p.progComposite, "debugView", _lighting.debugView == 4 ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f);
    drawTo(_ssaaFbo, w, h);

    // Hand the GL state back: frame target bound with its full viewport, unit 0
    // sampler restored, caches invalidated so the next draw re-applies state.
    for (int unit : {0, 2, 3, 4, 5})
    {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindSampler(0, _samplerObjects[0]);
    glBindVertexArray(0);
    glUseProgram(0);
    BindFrameRenderTarget();
    _pixelShaderSel = PSNone;
    GL33Bind::Invalidate();
    InvalidatePipelineCache();
}
