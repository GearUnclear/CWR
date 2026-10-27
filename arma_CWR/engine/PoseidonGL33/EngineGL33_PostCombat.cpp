// Remastered lighting: combat light (the
// combatLights and combatShadows switches; Graphics/Rendering/Lighting/
// CombatLights.hpp).
//
// The game spawns and moves its combat lights whatever the lighting setting.
// While their gate is on, this file:
//   - puts them into the frame's tiled light list (GatherCombatLights, from
//     UpdateLightTiles) as HDR entries at the head of the list, the strongest
//     at the camera first, at most twelve, never cutting a fire, a flare or a
//     missile motor (KeepCombatLights), and picks up to four of them for
//     shadows with hysteresis (UpdateShadowSelection); the classic lights of
//     the same sources then light nothing (UpdateLightTiles,
//     Object::DrawDecal);
//   - builds the shadow mask at the start of the world (UpdateCombatMask): a
//     half-resolution trace toward each shadowed light through the previous
//     frame's linear depth (the LinearDepth post stage), then a depth-aware
//     blur across and down, into an RGBA8 target on kCombatMaskUnit; the
//     forward shaders map their fragment into that screen through the rows
//     of the FrameLights block (UploadCombatFrame, CombatMaskRows) and take
//     one bilinear tap;
//   - hands the composite the strongest lights for their scattering in the air
//     (SetCombatScatterUniforms), and UploadLitFrameConstants the flash's
//     exposure response.
// Off (the switches, their gates, classic lighting, night vision, a portrait
// or a 3D preview), none of it runs and the classic lights light the frame as
// before.

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/EngineGL33_PostPass.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <Poseidon/Graphics/Core/GLBlendState.hpp>
#include <Poseidon/Graphics/Core/GLDepthStencilState.hpp>
#include <Poseidon/Graphics/Core/GLPipelineState.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

using namespace Poseidon::gl33lighting;
using namespace Poseidon::gl33post;
namespace rm = Poseidon::remaster;
namespace fl = Poseidon::lighting;

namespace
{

// The mask's march (CombatLights.hpp): steps per
// shadowed light, the thickness an occluder is taken to have, and how far
// toward the light the march goes (a flare 80 m up would spread the steps too
// thin to catch a man beside the receiver).
constexpr float kMaskSteps = 24.0f;
constexpr float kMaskThickness = 1.5f;
constexpr float kMaskMarch = 25.0f;
// Scattering per metre of the smoky air around a combat light (the phase
// function folded in), times the combatScatter knob; the in-scatter of a ray
// that meets no surface ends this far out.
constexpr float kScatterPerMetre = 0.004f;
constexpr float kScatterSkyDistance = 300.0f;
constexpr int kMaxScatterLights = 8;
// A fire's share of the in-scatter (a flash's is 1): a faint halo round the
// flames (ScatterMedium).
constexpr float kFireScatterShare = 0.25f;

GLuint CompileCombatShader(GLenum type, const char* src, const char* name)
{
    std::string overrideSource;
    if (LoadShaderOverride(name, overrideSource))
        src = overrideSource.c_str();
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        LOG_ERROR(Graphics, "GL33 combat mask: shader compile error [{}]: {}", name, log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

GLuint LinkCombatProgram(GLuint vs, PostShader shader)
{
    GLuint fs = CompileCombatShader(GL_FRAGMENT_SHADER, PostShaderSource(shader), PostShaderName(shader));
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
        LOG_ERROR(Graphics, "GL33 combat mask: program link error [{}]: {}", PostShaderName(shader), log);
        glDeleteProgram(p);
        return 0;
    }
    return p;
}

void DeleteTex(unsigned int& t)
{
    if (t)
    {
        Poseidon::GL33Bind::OnTexDeleted(t);
        glDeleteTextures(1, &t);
    }
    t = 0;
}

// A fire's light on surfaces under the photoreal camera comes from a body of
// flames this wide (m), not from the 0.45 m ball the smoke reach is measured
// in (from that ball the stones round a camp fire went past white to a
// cream slab).
constexpr float kFireSurfaceBody = 1.3f;
// A shell burst's light on surfaces ends this many of its ball's radii out
// (20 m lit the plain behind a burst into a band along the horizon). The ball
// is half the fireball's radius (a 120 mm HE shell's 1.1 m), and three of them
// lit a flat bright disc with an edge; the light falls off as the inverse
// square from a ball 1.3 m up and fades out smoothly by 10 balls (5 fireball
// radii, 11 m for the shell), and lasts only as long as the gas glows.
constexpr float kBurstSurfaceReachCores = 10.0f;
// ... and never past this (m): a fuel fireball's 3.2 m ball lit a 32 m strip
// along the horizon. Its surface gain under the camera (0.3 left
// the ground within a grenade's reach one flat, saturated ellipse).
constexpr float kBurstSurfaceReachMax = 10.0f;
constexpr float kBurstSurfaceGain = 0.12f;
// A fuel fireball's ball sits a third of its 13 m height up and its ground lit
// from eye height drew an orange line along the horizon brighter than the
// fireball: its surface gain.
constexpr float kFuelSurfaceGain = 0.04f;

// How thick the lit smoke and dust around a light is (m): muzzle gas, the
// dust and smoke of a blast, the hot air round a fire's flames. A fire's own
// smoke is lit particle by particle (Object::DrawDecal), its base glowing; a
// 4.4 m ball of lit air round the embers drew a round glow over a burning
// wreck where its black column stands, and a large halo round a camp fire.
float ScatterMedium(fl::CombatLightKind kind, float core)
{
    switch (kind)
    {
        case fl::CombatLightKind::Fire:
            return 2.0f * core + 0.8f;
        case fl::CombatLightKind::Muzzle:
            return 2.0f * core + 0.3f;
        case fl::CombatLightKind::Tracer:
        case fl::CombatLightKind::Motor:
            return 0.0f;
        case fl::CombatLightKind::Explosion:
            // The lit air of a detonation is its own fireball
            // (no dust has risen within its few frames of flash); a 2 core +
            // 1.5 m ball drew a soft globe of light by night; half its ball.
            return 0.5f * core;
        default:
            return 2.0f * core + 1.5f;
    }
}

} // namespace

void EngineGL33::InitCombatResources()
{
    CombatState& c = _combat;
    // Unit 14 always holds a complete texture (a shader must never
    // sample an incomplete texture): white, nothing
    // shadowed, until the first mask is built.
    GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
    const unsigned char white[4] = {255, 255, 255, 255};
    glGenTextures(1, &c.standInTex);
    glBindTexture(GL_TEXTURE_2D, c.standInTex);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    GL33Bind::Invalidate();
    GL33Bind::Tex2D(kCombatMaskUnit, c.standInTex);
    GL33Bind::ActiveUnit(0);
    c.maskBuilt = false;
    c.maskFrame = 0xffffffffu;
    c.depthValid = false;
    c.uploadedValid = false;
    c.active = false;
}

void EngineGL33::DeinitCombatResources()
{
    CombatState& c = _combat;
    for (int i = 0; i < 2; ++i)
    {
        DeleteTex(c.maskTex[i]);
        if (c.maskFbo[i])
            glDeleteFramebuffers(1, &c.maskFbo[i]);
        c.maskFbo[i] = 0;
    }
    DeleteTex(c.standInTex);
    for (unsigned int* p : {&c.progMask, &c.progBlur})
    {
        if (*p)
            glDeleteProgram(*p);
        *p = 0;
    }
    if (c.vao)
    {
        GL33Bind::OnVaoDeleted(c.vao);
        glDeleteVertexArrays(1, &c.vao);
    }
    c.vao = 0;
    c.maskW = c.maskH = 0;
    c.programsTried = false;
    c.unavailable = false;
    c.maskBuilt = false;
    c.maskFrame = 0xffffffffu;
    c.depthValid = false;
    c.uploadedValid = false;
    c.active = false;
}

void EngineGL33::BindCombatMask()
{
    const CombatState& c = _combat;
    const bool mask = c.maskBuilt && c.maskFrame == GetFrameCounter() && c.maskTex[0];
    GL33Bind::Tex2D(kCombatMaskUnit, mask ? c.maskTex[0] : c.standInTex);
}

void EngineGL33::GatherCombatLights(const fl::TileView& view, std::vector<fl::FrameLight>& out)
{
    CombatState& c = _combat;
    out.clear();
    const fl::CombatLightSet& set = fl::GCombatLights();
    const float* cam = _frameState.cameraPos;
    struct Candidate
    {
        fl::FrameLight light;
        const fl::CombatLightRecord* record;
        float score;
    };
    std::vector<Candidate> cands;
    float flash = 0.0f;
    for (const fl::CombatLightRecord& r : set.Records())
    {
        const fl::CombatLightSample& s = r.sample;
        if (s.intensity <= 1e-5f)
            continue;
        fl::FrameLight f;
        f.kind = r.kind == fl::CombatLightKind::Fire ? fl::FrameLightKind::Fire : fl::FrameLightKind::Combat;
        f.hdr = true;
        for (int i = 0; i < 3; ++i)
        {
            f.pos[i] = r.pos[i] - cam[i];
            f.diffuse[i] = s.rgb[i] * s.intensity;
        }
        f.start = s.core;
        f.range = s.range;
        // Under the photoreal camera the dusk and
        // night exposure lifts every combat light several stops over the
        // classic frame's: a rifleman's flash left a flat yellow-white
        // ellipse on the ground under every man, and a shell lit a white
        // band across the plain. A muzzle flash lights a metre or two round
        // the muzzle, dim, and is gone; a burst's light ends within 40 m;
        // fires and the fireball after the flash take a deeper orange (the
        // fire's light read olive on grass).
        if (_camera.frameOn)
        {
            float gain = 1.0f;
            float warmG = 1.0f, warmB = 1.0f;
            switch (r.kind)
            {
                case fl::CombatLightKind::Muzzle:
                    gain = 0.05f;
                    f.range = std::min(f.range, 2.5f);
                    warmG = 0.86f;
                    warmB = 0.62f;
                    break;
                case fl::CombatLightKind::Tracer:
                    gain = 0.5f;
                    break;
                case fl::CombatLightKind::Explosion:
                case fl::CombatLightKind::Cannon:
                    // (Seen from eye height the lit ground round a burst 60 m
                    // out is a thin strip along the horizon.)
                    gain = r.kind != fl::CombatLightKind::Explosion ? 0.3f
                           : r.detonation && r.det.deflagration     ? kFuelSurfaceGain
                                                                    : kBurstSurfaceGain;
                    // A young burst lit the plain 20 m
                    // round into a ruler-straight band along the horizon,
                    // brighter than its own fireball. Its light ends within
                    // kBurstSurfaceReachCores of its ball, under the cloud.
                    f.range = std::min(f.range, r.kind == fl::CombatLightKind::Explosion
                                                    ? std::min(kBurstSurfaceReachCores * f.start, kBurstSurfaceReachMax)
                                                    : 20.0f);
                    warmG = 0.8f;
                    warmB = 0.56f;
                    break;
                case fl::CombatLightKind::Fire:
                    // (A burning wreck's fire at its roof lit the plain behind
                    // it out to 120 m: a strip along the horizon at dusk.)
                    // A wood fire's light is near 2000 K;
                    // at (0.74, 0.48) the stones round a camp fire went to a
                    // cream slab past white and the lit grass stayed
                    // grey-green (red over green 5.8 levels).
                    f.range = std::min(f.range, 20.0f);
                    warmG = 0.6f;
                    warmB = 0.3f;
                    // The flames are a body, not a point: the stones round
                    // them take a flame's light (1 / (body^2 + d^2) at the
                    // same power); from 3 m out the pool loses under a quarter stop.
                    if (f.start < kFireSurfaceBody)
                    {
                        gain *= (f.start * f.start) / (kFireSurfaceBody * kFireSurfaceBody);
                        f.start = kFireSurfaceBody;
                    }
                    break;
                default:
                    break;
            }
            // A steady light reads against the twilight
            // (CombatTwilight): the burning wreck's fire drew a warm line
            // along the dusk horizon behind it.
            if (fl::CombatLightIsSteady(r.kind))
                gain *= std::min(std::max(set.Tuning().twilight, 0.0f), 1.0f);
            // The warmth at the same luminance.
            const float lum0 = 0.2126f * f.diffuse[0] + 0.7152f * f.diffuse[1] + 0.0722f * f.diffuse[2];
            f.diffuse[1] *= warmG;
            f.diffuse[2] *= warmB;
            const float lum1 = 0.2126f * f.diffuse[0] + 0.7152f * f.diffuse[1] + 0.0722f * f.diffuse[2];
            const float k = lum1 > 1e-9f ? gain * lum0 / lum1 : gain;
            for (float& v : f.diffuse)
                v *= k;
        }
        const float d2 = f.pos[0] * f.pos[0] + f.pos[1] * f.pos[1] + f.pos[2] * f.pos[2];
        // The flash the camera itself is in (the exposure response).
        if (fl::CombatLightOpensExposure(r.kind))
            flash += fl::CombatIrradiance(s, d2);
        // A light whose reach is off screen lights nothing on screen.
        float rect[4];
        if (!fl::LightScreenRect(f, view, rect))
            continue;
        cands.push_back({f, &r, fl::ShadowScore(s, d2)});
    }
    std::stable_sort(cands.begin(), cands.end(),
                     [](const Candidate& a, const Candidate& b) { return a.score > b.score; });
    // Past the list's twelve: the steady lights stay (their classic lights
    // light nothing now), the weakest bursts and tracers go.
    if (static_cast<int>(cands.size()) > fl::kMaxCombatFrameLights)
    {
        std::vector<fl::CombatLightKind> kinds(cands.size());
        for (size_t i = 0; i < cands.size(); ++i)
            kinds[i] = cands[i].record->kind;
        const std::vector<int> keep =
            fl::KeepCombatLights(kinds.data(), static_cast<int>(kinds.size()), fl::kMaxCombatFrameLights);
        std::vector<Candidate> kept;
        kept.reserve(keep.size());
        for (int i : keep)
            kept.push_back(cands[static_cast<size_t>(i)]);
        cands.swap(kept);
    }
    // The simulation time since the last frame of combat light: the flash
    // response and the shadow hysteresis age by it (a paused game holds both).
    const double now = set.SimTime();
    const float dt = static_cast<float>(std::max(now - c.selectionTime, 0.0));
    c.selectionTime = now;
    // The night of this frame's evaluation (CombatNight: the photoreal
    // camera's own under it).
    c.flashHeld = fl::FlashHeldIrradiance(c.flashHeld, flash, dt);
    c.flashGain = fl::FlashExposureGain(c.flashHeld, c.night);

    // The shadowed lights: tracers and missile motors cast none (too small to
    // matter); the rest compete for the four channels with hysteresis.
    const bool shadows = _featureGates[rm::Feature::CombatShadows].on;
    std::vector<fl::ShadowCandidate> sc(cands.size());
    for (size_t i = 0; i < cands.size(); ++i)
    {
        const fl::CombatLightKind k = cands[i].record->kind;
        const bool casts = k != fl::CombatLightKind::Tracer && k != fl::CombatLightKind::Motor;
        sc[i] = {cands[i].record->id, casts ? cands[i].score : 0.0f};
    }
    std::vector<int> channel(cands.size(), -1);
    if (shadows)
        fl::UpdateShadowSelection(c.selection, sc.data(), static_cast<int>(sc.size()), dt, fl::kMaxShadowedCombatLights,
                                  channel.data());
    else
        c.selection = fl::ShadowSelection{};
    c.shadowed = 0;
    for (float& v : c.maskLight)
        v = 0.0f;
    for (float& v : c.maskRange)
        v = 0.0f;
    const fl::CombatMaskCamera& dc = c.depthCamera;
    for (size_t i = 0; i < cands.size(); ++i)
    {
        Candidate& cd = cands[i];
        cd.light.shadowSlot = channel[i];
        const int ch = channel[i];
        if (ch < 0)
            continue;
        // The light in the view of the depth the mask traces through.
        const fl::CombatLightRecord& r = *cd.record;
        const double rel[3] = {r.pos[0] - dc.pos[0], r.pos[1] - dc.pos[1], r.pos[2] - dc.pos[2]};
        for (int row = 0; row < 3; ++row)
            c.maskLight[ch * 4 + row] =
                static_cast<float>(dc.rot[row][0] * rel[0] + dc.rot[row][1] * rel[1] + dc.rot[row][2] * rel[2]);
        c.maskLight[ch * 4 + 3] = r.sample.core;
        c.maskRange[ch] = r.sample.range;
        c.shadowed = std::max(c.shadowed, ch + 1);
    }

    // The scatter list: the strongest at the camera, in this frame's view.
    c.scatterCount = 0;
    const float gain = std::max(set.Tuning().scatter, 0.0f);
    for (const Candidate& cd : cands)
    {
        if (c.scatterCount >= kMaxScatterLights || gain <= 0.0f)
            break;
        const fl::CombatLightRecord& r = *cd.record;
        const float medium = ScatterMedium(r.kind, r.sample.core);
        if (medium <= 0.0f)
            continue;
        const int i = c.scatterCount++;
        for (int row = 0; row < 3; ++row)
            c.scatterPos[i * 4 + row] = view.rot[row][0] * cd.light.pos[0] + view.rot[row][1] * cd.light.pos[1] +
                                        view.rot[row][2] * cd.light.pos[2];
        c.scatterPos[i * 4 + 3] = r.sample.core;
        const float k = r.kind == fl::CombatLightKind::Fire ? kFireScatterShare : 1.0f;
        for (int ch = 0; ch < 3; ++ch)
            c.scatterRgb[i * 4 + ch] = cd.light.diffuse[ch] * k;
        c.scatterRgb[i * 4 + 3] = r.sample.range;
        c.scatterMed[i * 4 + 0] = medium;
    }

    out.reserve(cands.size());
    for (const Candidate& cd : cands)
        out.push_back(cd.light);
}

void EngineGL33::UploadCombatFrame()
{
    CombatState& c = _combat;
    if (!_lightTiles.dataBuf)
        return;
    float data[16] = {};
    data[0] = c.active ? static_cast<float>(c.count) : 0.0f;
    const bool mask = c.active && c.maskBuilt && c.maskFrame == GetFrameCounter();
    data[1] = mask ? 1.0f : 0.0f;
    if (mask)
    {
        const double cur[3] = {_frameState.cameraPos[0], _frameState.cameraPos[1], _frameState.cameraPos[2]};
        fl::CombatMaskRows(c.depthCamera, cur, c.rows);
        for (int r = 0; r < 3; ++r)
            for (int k = 0; k < 4; ++k)
                data[4 + r * 4 + k] = c.rows[r][k];
    }
    if (c.uploadedValid && std::equal(std::begin(data), std::end(data), std::begin(c.uploaded)))
        return;
    glBindBuffer(GL_UNIFORM_BUFFER, _lightTiles.dataBuf);
    glBufferSubData(GL_UNIFORM_BUFFER,
                    static_cast<GLintptr>(fl::kMaxFrameLights * fl::kTexelsPerLight * 4 * sizeof(float)), sizeof(data),
                    data);
    std::copy(std::begin(data), std::end(data), std::begin(c.uploaded));
    c.uploadedValid = true;
}

void EngineGL33::UpdateCombatMask()
{
    CombatState& c = _combat;
    const std::uint32_t frame = GetFrameCounter();
    if (c.maskFrame == frame)
        return;
    c.maskFrame = frame;
    c.maskBuilt = false;
    if (!c.active || c.shadowed <= 0 || !_featureGates[rm::Feature::CombatShadows].on || c.unavailable)
        return;
    // The previous frame's linear depth, at most a frame older (a frame
    // without a post pass leaves none).
    if (!c.depthValid || frame - c.depthFrame > 2u || !_post.linDepthTex || !_post.vao)
        return;
    const int hw = fl::CombatMaskSize(c.depthCamera.targetW), hh = fl::CombatMaskSize(c.depthCamera.targetH);
    if (hw <= 0 || hh <= 0 || hw != (_post.w + 1) / 2 || hh != (_post.h + 1) / 2)
        return;
    if (!c.programsTried)
    {
        c.programsTried = true;
        GLuint vs = CompileCombatShader(GL_VERTEX_SHADER, PostShaderSource(PostShader::Vertex),
                                        PostShaderName(PostShader::Vertex));
        c.progMask = LinkCombatProgram(vs, PostShader::CombatMask);
        c.progBlur = LinkCombatProgram(vs, PostShader::CombatMaskBlur);
        if (vs)
            glDeleteShader(vs);
        glGenVertexArrays(1, &c.vao);
        if (!c.progMask || !c.progBlur)
        {
            LOG_ERROR(Graphics, "GL33 combat mask: programs failed to build; combat light casts no shadow");
            c.unavailable = true;
            return;
        }
    }
    if (c.maskW != hw || c.maskH != hh)
    {
        for (int i = 0; i < 2; ++i)
        {
            DeleteTex(c.maskTex[i]);
            if (c.maskFbo[i])
                glDeleteFramebuffers(1, &c.maskFbo[i]);
            c.maskFbo[i] = 0;
        }
        GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
        bool complete = true;
        for (int i = 0; i < 2; ++i)
        {
            glGenTextures(1, &c.maskTex[i]);
            glBindTexture(GL_TEXTURE_2D, c.maskTex[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, hw, hh, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
            glGenFramebuffers(1, &c.maskFbo[i]);
            glBindFramebuffer(GL_FRAMEBUFFER, c.maskFbo[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, c.maskTex[i], 0);
            complete = complete && glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        GL33Bind::Invalidate();
        BindFrameRenderTarget();
        if (!complete)
        {
            LOG_WARN(Graphics, "GL33 combat mask: targets {}x{} incomplete; combat light casts no shadow", hw, hh);
            c.unavailable = true;
            return;
        }
        c.maskW = hw;
        c.maskH = hh;
    }

    GpuMark(Poseidon::gpu::Mark::CombatMaskBegin);
    // What the passes change that the next draw does not set again (as the
    // sky table's passes): the viewport, the target, cull, scissor, stencil
    // and depth clamp; depth, blend, the program and the vertex array come
    // back through the pipeline and bind caches, invalidated at the end.
    GLint viewport[4] = {0, 0, 0, 0};
    glGetIntegerv(GL_VIEWPORT, viewport);
    const bool cull = glIsEnabled(GL_CULL_FACE) == GL_TRUE;
    const bool scissor = glIsEnabled(GL_SCISSOR_TEST) == GL_TRUE;
    const bool stencil = glIsEnabled(GL_STENCIL_TEST) == GL_TRUE;
    const bool depthClamp = glIsEnabled(GL_DEPTH_CLAMP) == GL_TRUE;
    Poseidon::render::depthstencil::Disabled(/*hasStencil*/ false);
    glDisable(GL_STENCIL_TEST);
    Poseidon::render::blend::Opaque();
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    Poseidon::render::pipeline::DisableDepthClamp();
    GL33Bind::Vao(c.vao);
    // The passes read with texelFetch, no sampler objects: unit 0 (whose
    // sampler belongs to the pipeline, restored below) and the mask's unit.
    glBindSampler(0, 0);
    glBindSampler(kCombatMaskUnit, 0);

    const fl::CombatMaskCamera& dc = c.depthCamera;
    // The trace into maskTex[0], from the linear depth on unit 0.
    glUseProgram(c.progMask);
    SetInt(c.progMask, "linDepthTex", 0);
    Set4(c.progMask, "maskProj", 1.0f / dc.p11, 1.0f / dc.p22, dc.p11, dc.p22);
    Set4(c.progMask, "maskRect", dc.rect[0], dc.rect[1], dc.rect[2], dc.rect[3]);
    Set4(c.progMask, "maskSize", static_cast<float>(dc.targetW), static_cast<float>(dc.targetH), static_cast<float>(hw),
         static_cast<float>(hh));
    const GLint lightLoc = glGetUniformLocation(c.progMask, "maskLight");
    if (lightLoc >= 0)
        glUniform4fv(lightLoc, fl::kMaxShadowedCombatLights, c.maskLight);
    Set4(c.progMask, "maskRange", c.maskRange[0], c.maskRange[1], c.maskRange[2], c.maskRange[3]);
    // Under TAA the noise turns every frame and the history averages it.
    const float turn = TaaWantedThisFrame()
                           ? static_cast<float>(std::fmod(0.6180339887 * static_cast<double>(_taa.sequence), 1.0))
                           : 0.0f;
    Set4(c.progMask, "maskParams", kMaskSteps, kMaskThickness, turn, kMaskMarch);
    GL33Bind::Tex2D(0, _post.linDepthTex);
    DrawTo(c.maskFbo[0], hw, hh);
    // The blur across, then down, cut at depth steps.
    glUseProgram(c.progBlur);
    SetInt(c.progBlur, "maskTex", 0);
    SetInt(c.progBlur, "linDepthTex", kCombatMaskUnit);
    GL33Bind::Tex2D(kCombatMaskUnit, _post.linDepthTex);
    GL33Bind::Tex2D(0, c.maskTex[0]);
    Set4(c.progBlur, "blurDir", 1.0f, 0.0f, static_cast<float>(hw), static_cast<float>(hh));
    DrawTo(c.maskFbo[1], hw, hh);
    GL33Bind::Tex2D(0, c.maskTex[1]);
    Set4(c.progBlur, "blurDir", 0.0f, 1.0f, static_cast<float>(hw), static_cast<float>(hh));
    DrawTo(c.maskFbo[0], hw, hh);

    GL33Bind::Tex2D(0, 0);
    glBindSampler(0, _samplerObjects[0]);
    glUseProgram(0);
    BindFrameRenderTarget();
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    if (cull)
        glEnable(GL_CULL_FACE);
    if (scissor)
        glEnable(GL_SCISSOR_TEST);
    if (stencil)
        glEnable(GL_STENCIL_TEST);
    if (depthClamp)
        Poseidon::render::pipeline::EnableDepthClamp();
    _pixelShaderSel = PSNone;
    GL33Bind::Invalidate();
    InvalidatePipelineCache();
    c.maskBuilt = true;
    BindCombatMask();
    GL33Bind::ActiveUnit(0);
    GpuMark(Poseidon::gpu::Mark::CombatMaskEnd);
}

void EngineGL33::SaveCombatMaskCamera(int targetW, int targetH, const float rect[4])
{
    CombatState& c = _combat;
    Camera* camera = GScene ? GScene->GetCamera() : nullptr;
    if (!camera)
    {
        c.depthValid = false;
        return;
    }
    fl::CombatMaskCamera& dc = c.depthCamera;
    // The camera the world was drawn from, as the light list takes it
    // (UpdateLightTiles): the view rotation and the camera-relative origin.
    const Matrix4& toView = camera->InverseScaled();
    const Vector3 axes[3] = {Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1)};
    for (int col = 0; col < 3; ++col)
    {
        const Vector3 v = toView.Rotate(axes[col]);
        dc.rot[0][col] = v.X();
        dc.rot[1][col] = v.Y();
        dc.rot[2][col] = v.Z();
    }
    for (int i = 0; i < 3; ++i)
        dc.pos[i] = _frameState.cameraPos[i];
    dc.p11 = _frameState.projection._11;
    dc.p22 = _frameState.projection._22;
    for (int i = 0; i < 4; ++i)
        dc.rect[i] = rect[i];
    dc.targetW = targetW;
    dc.targetH = targetH;
    c.depthFrame = GetFrameCounter();
    c.depthValid = true;
}

void EngineGL33::SetCombatScatterUniforms(unsigned int program, float exposure)
{
    // exposure 0: this frame's world had no combat light.
    const CombatState& c = _combat;
    const int n = exposure > 0.0f ? c.scatterCount : 0;
    const float gain = std::max(Poseidon::lighting::GCombatLights().Tuning().scatter, 0.0f);
    Set4(program, "scatterParams", static_cast<float>(n), kScatterPerMetre * gain * exposure, kScatterSkyDistance,
         0.0f);
    if (n <= 0)
        return;
    const GLint pos = glGetUniformLocation(program, "scatterPos");
    if (pos >= 0)
        glUniform4fv(pos, kMaxScatterLights, c.scatterPos);
    const GLint rgb = glGetUniformLocation(program, "scatterRgb");
    if (rgb >= 0)
        glUniform4fv(rgb, kMaxScatterLights, c.scatterRgb);
    const GLint med = glGetUniformLocation(program, "scatterMed");
    if (med >= 0)
        glUniform4fv(med, kMaxScatterLights, c.scatterMed);
}
