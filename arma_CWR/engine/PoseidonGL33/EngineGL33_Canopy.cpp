// Long tree shadows (feature switch `canopyShadows`, INTEGRATION-PLAN.md
// section 5 B9, research track hfrt).
//
// At a low sun every tree, bush, forest block and building casts its long
// shadow, also beyond the sun shadow maps' few hundred metres
// (Graphics/Remaster/Canopy.hpp). Three parts here:
//
//   - the canopy texture (unit 10): the heightfield of the landscape objects
//     the world builds with the terrain horizon map (TerrainLighting.cpp),
//     uploaded whole when it arrives, and only the rectangles a destroyed tree
//     or building changed after that;
//   - the sun-visibility map (unit 15): a world-space map around the camera,
//     built on the GPU in one fullscreen pass (psCanopySun,
//     EngineGL33_PostCanopyShaders.cpp) that cone-traces the canopy toward the
//     sun from every texel, before the world, and only when the sun has moved
//     kCanopySunRebuildDeg, the camera has left the middle of the map or the
//     canopy changed (CanopySunNeedsRebuild);
//   - the Canopy slot of LitConstants, which the world programs read
//     (canopySunVisibility in the remastered prelude).
//
// Off (the switch, its gate outside a low clear sun, classic lighting, a
// portrait, a 3D preview after the world, night vision) nothing is built and
// the slot is zero, so the world programs read nothing and draw the frame
// before the feature.

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <Poseidon/Graphics/Core/GLBlendState.hpp>
#include <Poseidon/Graphics/Core/GLDepthStencilState.hpp>
#include <Poseidon/Graphics/Core/GLPipelineState.hpp>
#include <Poseidon/Graphics/Remaster/Canopy.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/Graphics/Remaster/TerrainHorizon.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

using namespace Poseidon::gl33lighting;
namespace rm = Poseidon::remaster;

namespace
{

struct CanopyState
{
    // What the world handed over (it changes in place: TerrainCanopyChanged).
    std::shared_ptr<const rm::CanopyField> field;
    bool uploadWhole = false;
    std::vector<rm::CanopyRect> changed;

    // GL objects of the current context.
    GLuint fieldTex = 0;     // RG16 with every level (unit 10)
    GLuint fieldStandIn = 0; // 1x1 RG16, no canopy (unit 10 until the first upload)
    std::uint32_t uploadedVersion = 0;
    GLuint sunTex = 0, sunFbo = 0; // the sun-visibility map, RGBA8 (unit 15)
    GLuint sunStandIn = 0;         // 1x1 RGBA8, all lit (unit 15 until the first build)
    GLuint prog = 0, vao = 0;
    bool programTried = false;
    bool unavailable = false;

    // The map in sunTex.
    rm::CanopySunKey built;
    unsigned builds = 0;
};

CanopyState s_canopy;

constexpr float kDeg = 57.29578f;

GLuint CompileCanopyShader(GLenum type, const char* src, const char* name)
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
        LOG_ERROR(Graphics, "GL33 canopy: shader compile error [{}]: {}", name, log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

GLuint LinkCanopyProgram()
{
    GLuint vs =
        CompileCanopyShader(GL_VERTEX_SHADER, PostShaderSource(PostShader::Vertex), PostShaderName(PostShader::Vertex));
    GLuint fs = CompileCanopyShader(GL_FRAGMENT_SHADER, PostShaderSource(PostShader::CanopySun),
                                    PostShaderName(PostShader::CanopySun));
    if (!vs || !fs)
    {
        if (vs)
            glDeleteShader(vs);
        if (fs)
            glDeleteShader(fs);
        return 0;
    }
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    glDeleteShader(vs);
    glDeleteShader(fs);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetProgramInfoLog(p, sizeof(log), nullptr, log);
        LOG_ERROR(Graphics, "GL33 canopy: program link error [{}]: {}", PostShaderName(PostShader::CanopySun), log);
        glDeleteProgram(p);
        return 0;
    }
    return p;
}

GLuint MakeTexture(GLenum internalFormat, GLenum format, GLenum type, int w, int h, const void* data, bool linear)
{
    GLuint t = 0;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, type, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    return t;
}

void DeleteTexture(GLuint& t)
{
    if (t)
    {
        Poseidon::GL33Bind::OnTexDeleted(t);
        glDeleteTextures(1, &t);
    }
    t = 0;
}

void BindUnit(int unit, GLuint tex)
{
    Poseidon::GL33Bind::Tex2D(unit, tex);
}

void SetUniform4(GLuint prog, const char* name, float a, float b, float c, float d)
{
    const GLint loc = glGetUniformLocation(prog, name);
    if (loc >= 0)
        glUniform4f(loc, a, b, c, d);
}

void SetSampler(GLuint prog, const char* name, int unit)
{
    const GLint loc = glGetUniformLocation(prog, name);
    if (loc >= 0)
        glUniform1i(loc, unit);
}

} // namespace

void EngineGL33::InitCanopy()
{
    // Units 10 and 15 always hold a complete texture of the right kind (the F3
    // rule): stand-ins until the first canopy and the first map.
    GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    const unsigned short none[2] = {0, 0};
    s_canopy.fieldStandIn = MakeTexture(GL_RG16, GL_RG, GL_UNSIGNED_SHORT, 1, 1, none, false);
    const unsigned char lit[4] = {255, 255, 0, 8}; // ground and raised lit, no edge, softness 0.5 m
    s_canopy.sunStandIn = MakeTexture(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 1, 1, lit, false);
    glBindTexture(GL_TEXTURE_2D, 0);
    GL33Bind::Invalidate();
    BindUnit(kCanopyUnit, s_canopy.fieldStandIn);
    BindUnit(kCanopyVisUnit, s_canopy.sunStandIn);
    GL33Bind::ActiveUnit(0);
    // The canopy (if any) belongs to the landscape, not the context: upload it
    // again, and build a new map.
    s_canopy.uploadWhole = s_canopy.field != nullptr;
    s_canopy.changed.clear();
    s_canopy.built = rm::CanopySunKey{};
}

void EngineGL33::DeinitCanopy()
{
    // GL objects only: the canopy is the world's.
    DeleteTexture(s_canopy.fieldTex);
    DeleteTexture(s_canopy.fieldStandIn);
    DeleteTexture(s_canopy.sunTex);
    DeleteTexture(s_canopy.sunStandIn);
    if (s_canopy.sunFbo)
        glDeleteFramebuffers(1, &s_canopy.sunFbo);
    s_canopy.sunFbo = 0;
    if (s_canopy.prog)
        glDeleteProgram(s_canopy.prog);
    s_canopy.prog = 0;
    if (s_canopy.vao)
    {
        GL33Bind::OnVaoDeleted(s_canopy.vao);
        glDeleteVertexArrays(1, &s_canopy.vao);
    }
    s_canopy.vao = 0;
    s_canopy.programTried = false;
    s_canopy.unavailable = false;
    s_canopy.uploadedVersion = 0;
    s_canopy.built = rm::CanopySunKey{};
}

void EngineGL33::SetTerrainCanopy(std::shared_ptr<const rm::CanopyField> canopy)
{
    s_canopy.field = std::move(canopy);
    s_canopy.uploadWhole = true;
    s_canopy.changed.clear();
    s_canopy.built = rm::CanopySunKey{};
}

void EngineGL33::TerrainCanopyChanged(const rm::CanopyRect* rects, int count)
{
    if (!s_canopy.field || !rects)
        return;
    s_canopy.changed.insert(s_canopy.changed.end(), rects, rects + count);
}

void EngineGL33::UploadCanopyField()
{
    const rm::CanopyField* f = s_canopy.field.get();
    if (s_canopy.uploadWhole)
    {
        s_canopy.uploadWhole = false;
        s_canopy.changed.clear();
        DeleteTexture(s_canopy.fieldTex);
        s_canopy.uploadedVersion = 0;
        if (f && f->Valid())
        {
            GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
            glGenTextures(1, &s_canopy.fieldTex);
            glBindTexture(GL_TEXTURE_2D, s_canopy.fieldTex);
            for (int l = 0; l < f->Levels(); ++l)
            {
                const int s = f->LevelSize(l);
                glTexImage2D(GL_TEXTURE_2D, l, GL_RG16, s, s, 0, GL_RG, GL_UNSIGNED_SHORT,
                             f->levels[static_cast<size_t>(l)].data());
            }
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, f->Levels() - 1);
            glBindTexture(GL_TEXTURE_2D, 0);
            GL33Bind::Invalidate();
            s_canopy.uploadedVersion = f->version;
            LOG_INFO(Graphics, "GL33: canopy {}x{} at {} m ({} levels) uploaded", f->size, f->size, f->texel,
                     f->Levels());
        }
        BindUnit(kCanopyUnit, s_canopy.fieldTex ? s_canopy.fieldTex : s_canopy.fieldStandIn);
        GL33Bind::ActiveUnit(0);
        return;
    }
    if (s_canopy.changed.empty() || !f || !s_canopy.fieldTex)
        return;
    // A destroyed tree or building: the rectangles of every level it changed.
    GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
    glBindTexture(GL_TEXTURE_2D, s_canopy.fieldTex);
    for (const rm::CanopyRect& r : s_canopy.changed)
    {
        if (r.level < 0 || r.level >= f->Levels() || r.x1 <= r.x0 || r.z1 <= r.z0)
            continue;
        const int s = f->LevelSize(r.level);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, s);
        const std::uint16_t* src =
            f->levels[static_cast<size_t>(r.level)].data() + (static_cast<size_t>(r.z0) * s + r.x0) * 2;
        glTexSubImage2D(GL_TEXTURE_2D, r.level, r.x0, r.z0, r.x1 - r.x0, r.z1 - r.z0, GL_RG, GL_UNSIGNED_SHORT, src);
    }
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    GL33Bind::Invalidate();
    GL33Bind::ActiveUnit(0);
    s_canopy.changed.clear();
    s_canopy.uploadedVersion = f->version;
}

bool EngineGL33::BuildCanopySunGpu(const Vector3& toSun, float originX, float originZ, float texel)
{
    const rm::CanopyField* f = s_canopy.field.get();
    if (s_canopy.unavailable || !f || !s_canopy.fieldTex || !_terrainMap || !_terrainMap->Valid() || !_terrainHeightTex)
        return false;
    constexpr int n = rm::kCanopySunSize;
    if (!s_canopy.programTried)
    {
        s_canopy.programTried = true;
        s_canopy.prog = LinkCanopyProgram();
        glGenVertexArrays(1, &s_canopy.vao);
        GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
        s_canopy.sunTex = MakeTexture(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, n, n, nullptr, true);
        glBindTexture(GL_TEXTURE_2D, 0);
        GL33Bind::Invalidate();
        glGenFramebuffers(1, &s_canopy.sunFbo);
        glBindFramebuffer(GL_FRAMEBUFFER, s_canopy.sunFbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_canopy.sunTex, 0);
        const bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
        BindFrameRenderTarget();
        if (!s_canopy.prog || !complete)
        {
            LOG_ERROR(Graphics, "GL33 canopy: program or target failed; no long tree shadows");
            s_canopy.unavailable = true;
            return false;
        }
    }
    const rm::CanopySunRay ray = rm::CanopySunRayToward(toSun.X(), toSun.Y(), toSun.Z());

    // What the pass changes that the next draw does not set again (as the sky
    // table's passes): the viewport, the target, cull, scissor, stencil and
    // depth clamp; depth, blend, the program and the vertex array come back
    // through the pipeline and bind caches, invalidated at the end.
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
    GL33Bind::Vao(s_canopy.vao);

    glUseProgram(s_canopy.prog);
    SetSampler(s_canopy.prog, "canopyMap", kCanopyUnit);
    SetSampler(s_canopy.prog, "terrainHeight", kTerrainHeightUnit);
    const float extent = static_cast<float>(f->size) * f->texel;
    SetUniform4(s_canopy.prog, "canopyField", 1.0f / extent, f->texel, f->heightBase, f->heightRange);
    SetUniform4(s_canopy.prog, "terrainGrid", 1.0f / _terrainMap->cellSize,
                1.0f / static_cast<float>(_terrainMap->size), 0.0f, 0.0f);
    SetUniform4(s_canopy.prog, "sunRay", ray.dirX, ray.dirZ, ray.tanSun, ray.tanSoft);
    SetUniform4(s_canopy.prog, "sunMap", originX, originZ, texel, ray.length);
    BindUnit(kCanopyUnit, s_canopy.fieldTex);
    BindUnit(kTerrainHeightUnit, _terrainHeightTex);
    // The map is the target: nothing samples it meanwhile.
    BindUnit(kCanopyVisUnit, s_canopy.sunStandIn);
    glBindFramebuffer(GL_FRAMEBUFFER, s_canopy.sunFbo);
    glViewport(0, 0, n, n);
    glDrawArrays(GL_TRIANGLES, 0, 3);

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
    BindUnit(kCanopyVisUnit, s_canopy.sunTex);
    GL33Bind::ActiveUnit(0);
    ++s_canopy.builds;
    return true;
}

void EngineGL33::UploadLitCanopy(const Vector3& toSun, float sunElevationDeg, float skyThrough)
{
    // The canopy texture follows the world's canopy whether or not the
    // feature runs this frame, so it is current when the gate opens.
    if (s_canopy.uploadWhole || !s_canopy.changed.empty())
        UploadCanopyField();
    const rm::CanopyField* f = s_canopy.field.get();
    Camera* camera = GScene ? GScene->GetCamera() : nullptr;
    const bool live = _featureGates.computed && _featureGates[rm::Feature::CanopyShadows].on && _worldEffectsOn &&
                      !_portraitTargetSize && !_worldSceneEnded && camera && f && s_canopy.fieldTex &&
                      _terrainHeightTex && _terrainMap && _terrainMap->Valid();
    bool ready = false;
    if (live)
    {
        rm::CanopySunKey want;
        want.valid = true;
        Vector3 sun = toSun;
        sun.Normalize();
        want.sun[0] = sun.X();
        want.sun[1] = sun.Y();
        want.sun[2] = sun.Z();
        want.texel = rm::CanopySunTexelFor(GScene->GetFogMaxRange());
        want.canopyVersion = s_canopy.uploadedVersion;
        const Vector3 cam = camera->Position();
        rm::CanopySunOrigin(cam.X(), cam.Z(), want.texel, want.originX, want.originZ);
        ready = s_canopy.built.valid;
        // Only in the world's own passes (`live`): a portrait or a 3D preview
        // after the world never rebuilds the world's map.
        if (rm::CanopySunNeedsRebuild(s_canopy.built, want, cam.X(), cam.Z()))
        {
            GpuMark(Poseidon::gpu::Mark::CanopyBegin);
            if (BuildCanopySunGpu(toSun, want.originX, want.originZ, want.texel))
            {
                GpuMark(Poseidon::gpu::Mark::CanopyEnd);
                s_canopy.built = want;
                ready = true;
            }
        }
    }
    const rm::CanopySunKey& b = s_canopy.built;
    const rm::CanopyFrameConstants c = rm::ComputeCanopyFrameConstants(
        _lighting, _featureGates, live && ready, sunElevationDeg, skyThrough, b.originX, b.originZ, b.texel);
    SetLitSlot(LitConst::Canopy, c.canopy[0], c.canopy[1], c.canopy[2], c.canopy[3]);
}

void EngineGL33::SurfaceCanopyConstants(float out[4]) const
{
    out[0] = out[1] = out[2] = out[3] = 0.0f;
    const rm::CanopyField* f = s_canopy.field.get();
    if (!f || !f->Valid() || !s_canopy.fieldTex || s_canopy.uploadWhole)
        return;
    const float extent = static_cast<float>(f->size) * f->texel;
    if (!(extent > 0.0f))
        return;
    out[0] = f->heightBase;
    out[1] = f->heightRange;
    out[2] = 1.0f / extent;
    out[3] = 1.0f;
}

void EngineGL33::BindCanopy()
{
    BindUnit(kCanopyVisUnit, s_canopy.built.valid && s_canopy.sunTex ? s_canopy.sunTex : s_canopy.sunStandIn);
}

std::string EngineGL33::RemasterCanopyCheck(int texels)
{
    if (!_glContext)
        return "FAIL:no context";
    const rm::CanopyField* f = s_canopy.field.get();
    if (!f || !s_canopy.built.valid || !s_canopy.sunTex)
        return "FAIL:no sun-visibility map built (switch or gate off, or no canopy yet)";
    if (s_canopy.built.canopyVersion != f->version)
        return "FAIL:the canopy changed since the map was built";
    if (!_terrainMap || !_terrainMap->Valid())
        return "FAIL:no terrain map";
    constexpr int n = rm::kCanopySunSize;
    FlushQueues();
    std::vector<unsigned char> gpu(static_cast<size_t>(n) * n * 4);
    GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_canopy.sunTex);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, gpu.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    GL33Bind::Invalidate();
    GL33Bind::ActiveUnit(0);

    // The receiver's ground as the pass reads it: the terrain height texture,
    // bilinear on its grid (GL_LINEAR, clamp to edge).
    const rm::TerrainHorizonMap& tm = *_terrainMap;
    auto ground = [&tm](float x, float z)
    {
        const int s = tm.size;
        const float fx = std::clamp(x / tm.cellSize, 0.0f, static_cast<float>(s - 1));
        const float fz = std::clamp(z / tm.cellSize, 0.0f, static_cast<float>(s - 1));
        const int x0 = std::min(static_cast<int>(fx), s - 2), z0 = std::min(static_cast<int>(fz), s - 2);
        const float tx = fx - static_cast<float>(x0), tz = fz - static_cast<float>(z0);
        auto at = [&](int xx, int zz) { return tm.height[static_cast<size_t>(zz) * s + xx]; };
        return (at(x0, z0) * (1.0f - tx) + at(x0 + 1, z0) * tx) * (1.0f - tz) +
               (at(x0, z0 + 1) * (1.0f - tx) + at(x0 + 1, z0 + 1) * tx) * tz;
    };
    const rm::CanopySunKey& b = s_canopy.built;
    const rm::CanopySunRay ray = rm::CanopySunRayToward(b.sun[0], b.sun[1], b.sun[2]);
    const int count = std::max(16, std::min(texels, n * n));
    const int step = std::max(1, static_cast<int>(std::sqrt(static_cast<float>(n) * n / count)));
    float worst[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    float worstRead = 0.0f;
    int worstAt[2] = {0, 0};
    int checked = 0, shaded = 0;
    std::vector<float> reads;
    for (int y = step / 2; y < n; y += step)
    {
        for (int x = step / 2; x < n; x += step)
        {
            const float wx = b.originX + (static_cast<float>(x) + 0.5f) * b.texel;
            const float wz = b.originZ + (static_cast<float>(y) + 0.5f) * b.texel;
            const rm::CanopySunTexel cpu = rm::CanopySunAt(*f, wx, wz, ground(wx, wz), ray);
            const unsigned char* g = gpu.data() + (static_cast<size_t>(y) * n + x) * 4;
            rm::CanopySunTexel got;
            got.ground = g[0] / 255.0f;
            got.raisedLow = g[1] / 255.0f;
            got.edge = g[2] / 255.0f * rm::kCanopyEdgeRange;
            got.soft = g[3] / 255.0f * rm::kCanopySoftRange;
            const float want[4] = {cpu.ground, cpu.raisedLow, std::clamp(cpu.edge / rm::kCanopyEdgeRange, 0.0f, 1.0f),
                                   std::clamp(cpu.soft / rm::kCanopySoftRange, 0.0f, 1.0f)};
            for (int c = 0; c < 4; ++c)
            {
                const float err = std::fabs(g[c] / 255.0f - want[c]);
                if (!(err <= worst[c]))
                    worst[c] = std::isfinite(err) ? err : 1e9f;
            }
            // The mirror's texel as the map stores it (8 bits a channel), so
            // the reads compare the two traces, not the storage.
            auto store = [](float v) { return std::round(std::clamp(v, 0.0f, 1.0f) * 255.0f) / 255.0f; };
            rm::CanopySunTexel stored;
            stored.ground = store(want[0]);
            stored.raisedLow = store(want[1]);
            stored.edge = store(want[2]) * rm::kCanopyEdgeRange;
            stored.soft = store(want[3]) * rm::kCanopySoftRange;
            float texelRead = 0.0f;
            for (float hr : {0.0f, 1.0f, 3.0f, 8.0f, 15.0f})
            {
                const float err =
                    std::fabs(rm::CanopyReceiverVisibility(got, hr) - rm::CanopyReceiverVisibility(stored, hr));
                texelRead = std::isfinite(err) ? std::max(texelRead, err) : 1e9f;
                if (!(err <= worstRead))
                {
                    worstRead = std::isfinite(err) ? err : 1e9f;
                    worstAt[0] = x;
                    worstAt[1] = y;
                }
            }
            reads.push_back(texelRead);
            shaded += cpu.ground < 0.9f ? 1 : 0;
            ++checked;
        }
    }
    // The stored channels against the mirror: the GPU's filter weights keep 8
    // bits of the texel fraction, so beside a 20 m crown the canopy's top
    // moves by a few centimetres and a sample's occlusion by up to a few
    // hundredths; 0.03 is 8 storage steps. What a receiver reads (against the
    // mirror stored the same way) can differ by one storage step of the edge
    // (48 m / 255) on its steepest slope (1.5 / (2 x 2.06 m) at the softest
    // edge a sample sets), about 0.07, at the texels where the two land on
    // either side of a rounding; most texels agree far closer.
    std::sort(reads.begin(), reads.end());
    const float p99 = reads.empty() ? 0.0f : reads[static_cast<size_t>(0.99 * static_cast<double>(reads.size() - 1))];
    constexpr float kToleranceStored = 0.03f;
    constexpr float kToleranceMax = 0.08f;
    constexpr float kToleranceP99 = 0.035f;
    const bool ok = worst[0] <= kToleranceStored && worst[1] <= kToleranceStored && worst[2] <= kToleranceStored &&
                    worst[3] <= kToleranceStored && worstRead <= kToleranceMax && p99 <= kToleranceP99;
    char buf[448];
    std::snprintf(
        buf, sizeof(buf),
        "%s:texels=%d shaded=%d sun=%.2f texel=%.3f origin=%.0f,%.0f builds=%u abs_err ground=%.4f "
        "raised=%.4f edge=%.4f soft=%.4f read=%.4f@%d,%d read_p99=%.4f tolerance=%.3f/%.3f/%.3f",
        ok ? "OK" : "FAIL", checked, shaded, static_cast<double>(std::asin(std::clamp(b.sun[1], -1.0f, 1.0f)) * kDeg),
        static_cast<double>(b.texel), static_cast<double>(b.originX), static_cast<double>(b.originZ), s_canopy.builds,
        static_cast<double>(worst[0]), static_cast<double>(worst[1]), static_cast<double>(worst[2]),
        static_cast<double>(worst[3]), static_cast<double>(worstRead), worstAt[0], worstAt[1], static_cast<double>(p99),
        static_cast<double>(kToleranceStored), static_cast<double>(kToleranceMax), static_cast<double>(kToleranceP99));
    LOG_INFO(Graphics, "GL33 canopy check: {}", buf);
    return buf;
}
