// The Graphics page's Sun Glare option (first the experiment lane
// xp-atmosphere on graphics-experimental): the low sun's glare under the
// photoreal camera. Off by default. SetLookOptions sets its strength from the
// option (1) or from POSEIDON_XP_SUNGLARE when that is set (0 off, another
// number that strength, Graphics/Remaster/LookOptions.hpp).
//
// The sun disc (camSunDisc) and the sky's roll-off toward the sun burn a small
// white spot with a soft warm rim; the bloom chain reaches a few dozen pixels.
// A camera or an eye looking into a low sun sees a much larger glare: a bright
// core a degree or two across and a long faint halo (the lens's and the eye's
// point spread function, scattered light falling off about as 1 / angle^2).
// The glare is the PSF of the light that actually reaches the camera from the
// disc, so it collapses when a crown, a ridge or a cloud covers the sun and
// never draws a solid white ball:
//   - a 1x1 pass (psXpSunGlareVis) taps the frame (scene light, after the air)
//     at 24 points over the projected disc and measures the share of them that
//     still burns (decoded scene light between kVisLo and kVisHi), smoothed
//     over about four frames (a ping-pong pair: leaves crossing the sun do not
//     strobe);
//   - the composite adds, on the light before the curve (after the meter, so
//     the exposure does not change), a radially symmetric glare round the
//     sun's screen point: a Gaussian core plus a Lorentzian halo, in the
//     sun's colour (the core toward white), times the visible share, the
//     strength, a low-sun weight (full up to 15 degrees, 0.35 from 40) and the
//     day (none at night or with the sun under the horizon).
// Cost: one 1x1 draw of 24 taps and a few ALU per pixel in the composite, and
// nothing at all with the glare off (no pass runs and the composite's branch
// is skipped on a zero gain).

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/EngineGL33_PostPass.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>

#include <algorithm>
#include <cmath>
#include <string>

using namespace Poseidon::gl33post;

namespace
{

// The visibility pass: the post vertex shader's vUV, the frame as the
// composite reads it, last frame's share. Output r: the smoothed share, g: this
// frame's share.
const char kXpSunGlareVisHead[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D colorTex; // unit 0: the frame (camera encoded scene light)
uniform sampler2D prevTex;  // unit 2: last frame's share (1x1)
uniform vec4 visSun;        // xy the sun's uv, zw the disc's radius in uv (x, y)
uniform vec4 visParams;     // x history keep, y 1 = the history is valid, z low, w high (decoded scene light)
uniform vec4 visRect;       // the 3D viewport in uv (x, y, w, h)
out vec4 fragColor;
)";

const char kXpSunGlareVisMain[] = R"(
void main() {
    const int kTaps = 24;
    float sum = 0.0;
    for (int i = 0; i < kTaps; ++i) {
        // A golden-angle spiral over the disc, out to 0.9 of its radius (its
        // anti-aliased rim is half dark).
        float r = 0.9 * sqrt((float(i) + 0.5) / float(kTaps));
        float a = float(i) * 2.39996323;
        vec2 uv = visSun.xy + vec2(cos(a), sin(a)) * r * visSun.zw;
        vec2 q = (uv - visRect.xy) / max(visRect.zw, vec2(1e-6));
        if (q.x < 0.0 || q.y < 0.0 || q.x > 1.0 || q.y > 1.0) continue;
        vec3 c = camDecode(textureLod(colorTex, uv, 0.0).rgb);
        float l = max(c.r, max(c.g, c.b));
        sum += smoothstep(visParams.z, visParams.w, l);
    }
    float v = sum / float(kTaps);
    float prev = texelFetch(prevTex, ivec2(0, 0), 0).r;
    // Without a history the target's first contents are never read (a new
    // texture's are undefined).
    float smoothed = visParams.y > 0.5 ? mix(v, prev, visParams.x) : v;
    fragColor = vec4(smoothed, v, 0.0, 1.0);
}
)";

// Decoded scene light (pre-exposed) the disc's taps must reach to count: the
// disc is drawn at kSunDiscLevel (12) and the sky round it rolls off toward
// kSkySunCap (10); a crown, a ridge or a cloud's shaded side stays far under.
constexpr float kVisLo = 3.0f;
constexpr float kVisHi = 8.0f;
// The share of last frame's visibility kept each frame (about four frames).
constexpr float kVisKeep = 0.75f;

// The PSF (radians): the core's Gaussian width and level, the halo's
// half-width and level, on the light at the exposure (the composite's x, where
// the sky round a low sun sits at about 1 to 10). The soft end of the round-1
// bracket (stills r1 to r3): a stronger core (1.5 degrees x 6, 0.8 x 2.5)
// clips into a flat pale ball with a visible rim.
constexpr float kCoreRad = 0.0105f; // 0.6 degrees
constexpr float kCoreLevel = 1.5f;
constexpr float kHaloRad = 0.0524f; // 3 degrees
constexpr float kHaloLevel = 0.4f;
// Low-sun weight: full up to kFullDeg, kHighKeep from kFadeDeg up.
constexpr float kFullDeg = 15.0f;
constexpr float kFadeDeg = 40.0f;
constexpr float kHighKeep = 0.35f;

} // namespace

void EngineGL33::DestroyXpSunGlare()
{
    PostTargets& p = _post;
    for (int i = 0; i < 2; ++i)
    {
        if (p.xpGlareFbo[i])
            glDeleteFramebuffers(1, &p.xpGlareFbo[i]);
        if (p.xpGlareTex[i])
            glDeleteTextures(1, &p.xpGlareTex[i]);
        p.xpGlareFbo[i] = 0;
        p.xpGlareTex[i] = 0;
    }
    if (p.progXpGlareVis)
        glDeleteProgram(p.progXpGlareVis);
    p.progXpGlareVis = 0;
    p.xpGlareTried = false;
    p.xpGlareHistory = false;
}

EngineGL33::XpSunGlareFrame EngineGL33::PostXpSunGlare(const PostFrame& f)
{
    XpSunGlareFrame out;
    const float gain = _sunGlareGain;
    PostTargets& p = _post;
    if (gain <= 0.0f || !f.camera || !f.sunInFront || !GScene)
    {
        p.xpGlareHistory = false;
        return out;
    }
    if (!p.xpGlareTried)
    {
        p.xpGlareTried = true;
        using Poseidon::gl33lighting::PostShader;
        using Poseidon::gl33lighting::PostShaderSource;
        const char* vsSrc = PostShaderSource(PostShader::Vertex);
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, &vsSrc, nullptr);
        glCompileShader(vs);
        std::string fsSrc =
            std::string(kXpSunGlareVisHead) + Poseidon::gl33lighting::kCameraCodecGLSL + kXpSunGlareVisMain;
        const char* fsPtr = fsSrc.c_str();
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, &fsPtr, nullptr);
        glCompileShader(fs);
        GLint okV = 0, okF = 0;
        glGetShaderiv(vs, GL_COMPILE_STATUS, &okV);
        glGetShaderiv(fs, GL_COMPILE_STATUS, &okF);
        if (okV && okF)
        {
            GLuint prog = glCreateProgram();
            glAttachShader(prog, vs);
            glAttachShader(prog, fs);
            glLinkProgram(prog);
            GLint okL = 0;
            glGetProgramiv(prog, GL_LINK_STATUS, &okL);
            if (okL)
                p.progXpGlareVis = prog;
            else
                glDeleteProgram(prog);
        }
        if (!p.progXpGlareVis)
        {
            char log[1024] = {};
            glGetShaderInfoLog(fs, sizeof(log), nullptr, log);
            LOG_ERROR(Graphics, "GL33 xp sun glare: visibility program failed: {}", log);
        }
        glDeleteShader(vs);
        glDeleteShader(fs);
        // The targets are made on unit 0, which the pass binds again below
        // (any other unit may hold a world texture).
        glActiveTexture(GL_TEXTURE0);
        for (int i = 0; i < 2; ++i)
        {
            glGenTextures(1, &p.xpGlareTex[i]);
            glBindTexture(GL_TEXTURE_2D, p.xpGlareTex[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 1, 1, 0, GL_RGBA, GL_HALF_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
            glGenFramebuffers(1, &p.xpGlareFbo[i]);
            glBindFramebuffer(GL_FRAMEBUFFER, p.xpGlareFbo[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, p.xpGlareTex[i], 0);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                LOG_ERROR(Graphics, "GL33 xp sun glare: 1x1 target incomplete");
                if (p.progXpGlareVis)
                    glDeleteProgram(p.progXpGlareVis);
                p.progXpGlareVis = 0;
            }
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        GL33Bind::Invalidate();
        p.xpGlareHistory = false;
    }
    if (!p.progXpGlareVis)
        return out;

    LightSun* sun = GScene->MainLight();
    if (!sun)
        return out;
    const Vector3 toSun = -sun->SunDirection();
    const float elevDeg = std::asin(std::max(-1.0f, std::min(1.0f, toSun.Y()))) * 57.2957795f;
    const float night = std::min(std::max(sun->NightEffect(), 0.0f), 1.0f);
    // Full up to kFullDeg, kHighKeep from kFadeDeg; none with the sun's disc
    // under the horizon (from -0.5 to 0.5 degrees) or at night.
    float t = (elevDeg - kFullDeg) / (kFadeDeg - kFullDeg);
    t = std::min(std::max(t, 0.0f), 1.0f);
    t = t * t * (3.0f - 2.0f * t);
    float up = (elevDeg + 0.5f) / 1.0f;
    up = std::min(std::max(up, 0.0f), 1.0f);
    const float weight = (1.0f - (1.0f - kHighKeep) * t) * up * (1.0f - night);
    if (weight <= 0.0f)
    {
        p.xpGlareHistory = false;
        return out;
    }

    // The disc's radius in uv: tan(radius) times the projection's scale, half
    // the ndc range per uv, the 3D viewport's share of the target.
    const float tanR = std::tan(0.45f * 0.0174532925f);
    const float ru = tanR * (1.0f / std::max(f.projInfo[2], 1e-6f)) * 0.5f * f.rect[2];
    const float rv = tanR * (1.0f / std::max(f.projInfo[3], 1e-6f)) * 0.5f * f.rect[3];

    const int cur = p.xpGlareCur ^ 1;
    glUseProgram(p.progXpGlareVis);
    SetInt(p.progXpGlareVis, "colorTex", 0);
    SetInt(p.progXpGlareVis, "prevTex", 2);
    BindTex(0, PostSceneTex(f));
    BindTex(2, p.xpGlareTex[p.xpGlareCur]);
    Set4(p.progXpGlareVis, "visSun", f.sunU, f.sunV, ru, rv);
    Set4(p.progXpGlareVis, "visParams", kVisKeep, p.xpGlareHistory ? 1.0f : 0.0f, kVisLo, kVisHi);
    Set4(p.progXpGlareVis, "visRect", f.rect[0], f.rect[1], f.rect[2], f.rect[3]);
    DrawTo(p.xpGlareFbo[cur], 1, 1);
    p.xpGlareCur = cur;
    p.xpGlareHistory = true;

    // The sun's colour, by its brightest channel.
    const Color& tint = f.shaftTint;
    const float pk = std::max(1e-4f, std::max(tint.R(), std::max(tint.G(), tint.B())));
    out.on = true;
    out.tex = p.xpGlareTex[cur];
    // The sun's view ray at z = 1 (the composite builds each pixel's the same
    // way from projInfo and worldRect and takes the exact angle between them).
    const float ndcX = ((f.sunU - f.rect[0]) / std::max(f.rect[2], 1e-6f)) * 2.0f - 1.0f;
    const float ndcY = ((f.sunV - f.rect[1]) / std::max(f.rect[3], 1e-6f)) * 2.0f - 1.0f;
    out.sunX = ndcX * f.projInfo[2];
    out.sunY = ndcY * f.projInfo[3];
    out.gain = gain * weight;
    out.coreRad = kCoreRad;
    out.coreLevel = kCoreLevel;
    out.haloRad = kHaloRad;
    out.haloLevel = kHaloLevel;
    out.r = tint.R() / pk;
    out.g = tint.G() / pk;
    out.b = tint.B() / pk;
    return out;
}
