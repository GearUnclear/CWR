// Remastered sky: the sky table (feature switch `atmosphere`).
//
// A physically based clear sky (Graphics/Remaster/Atmosphere.hpp) lights the
// sky dome under the original sky texture's clouds and gives the fog of the
// world draws its colour (aerial perspective). Three parts:
//
//   - the transmittance and multiple-scattering tables, built on a worker
//     thread when the haze changes (about 100 ms; std::async, as the terrain
//     horizon map), uploaded as float textures;
//   - the sky-view table, built on the GPU in two fullscreen passes
//     (psSkyView, psSkyViewMean; EngineGL33_PostSkyTableShaders.cpp) whenever
//     the sun has moved kSkyTableRebuildDeg, before the dome draws. On the CPU
//     this was 8 to 12 ms per rebuild, a hitch every few seconds of play;
//   - the sky pass: Landscape::DrawSky and DrawClouds open it
//     (BeginRemasterSky / EndRemasterSky) around the pre-world draws of the
//     dome and the sky clouds. They come before the world's first 3D pass, so
//     they would draw through the classic programs; the pass opens the
//     remastered set for them as BeginPass does for the world, and the sky
//     mode (LitConstants litSkyFwd.w) tells the screen vertex shader which
//     draws are the dome.
//
// Off (the switch, the gate below -9 degrees of sun, classic lighting, a
// portrait, a 3D preview after the world, night vision) the pass does nothing,
// the sky slots stay zero and every draw is the one before the feature.

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <Poseidon/Graphics/Core/GLBlendState.hpp>
#include <Poseidon/Graphics/Core/GLDepthStencilState.hpp>
#include <Poseidon/Graphics/Core/GLPipelineState.hpp>
#include <Poseidon/Graphics/Remaster/Atmosphere.hpp>
#include <Poseidon/Graphics/Remaster/FeatureSwitches.hpp>
#include <Poseidon/Graphics/Remaster/PhotoCamera.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <future>
#include <string>
#include <vector>

using namespace Poseidon::gl33lighting;
namespace rm = Poseidon::remaster;

namespace
{

constexpr float kDeg = 57.29578f;

struct SkyTableState
{
    // The atmosphere tables, built on a worker thread when the haze changes.
    // The CPU copy also gives the sky clouds their sun colour and is what
    // triSkyTableCheck compares the GPU table with.
    std::future<rm::AtmosphereTables> pending;
    bool building = false;
    rm::AtmosphereTables tables;

    // GL objects of the current context.
    GLuint standIn = 0;  // 1x1 on unit 8 until the sky-view table is built
    GLuint transTex = 0; // transmittance (RGB32F, texelFetch)
    GLuint msTex = 0;    // multiple scattering (RGB32F, texelFetch)
    float uploadedHaze = -1.0f;
    GLuint scratchTex = 0, scratchFbo = 0; // regions 0 and 1 (pass 1)
    GLuint lutTex = 0, lutFbo = 0;         // the sky-view table the shaders read (pass 2)
    GLuint progView = 0, progMean = 0, vao = 0;
    bool programsTried = false;
    bool unavailable = false;

    // The sky-view table in lutTex.
    bool built = false;
    float builtSunElevRad = 0.0f;
    float builtHaze = -1.0f;
    unsigned builds = 0;

    // The open sky pass (BeginRemasterSky .. EndRemasterSky).
    bool open = false;
    int mode = 0;                // Poseidon::RemasterSky of the draws; 0 = the world's
    bool forcedPrograms = false; // the pass opened the remastered programs

    // The sky light's spherical harmonics (the surfaces lane, SurfaceLight.hpp),
    // projected from the CPU atmosphere tables on a worker thread whenever the
    // sun moved kSkyLightRebuildRad or the haze changed.
    std::future<rm::SkyLightSH> shPending;
    bool shBuilding = false;
    rm::SkyLightSH sh;
};

// The sun moves this far (radians, about 0.1 degree) before the sky light is
// projected again: the light changes far more slowly than the sky's own look.
constexpr float kSkyLightRebuildRad = 0.0017f;

SkyTableState s_sky;

float HazeOf(const rm::LightingTuning& t)
{
    // Under the photoreal camera's switch the air holds a real summer haze
    // (camHaze times the knob); the switch off keeps the reference air.
    const float camera = rm::FeatureOn(t.features, rm::Feature::PhotoLight) ? std::max(t.camHaze, 0.0f) : 1.0f;
    return std::max(0.05f, t.atmoHaze * camera);
}

float SunElevationRad(LightSun* sun)
{
    const float y = -sun->Direction().Y();
    return std::asin(y < -1.0f ? -1.0f : (y > 1.0f ? 1.0f : y));
}

rm::Rgb UnitLuminance(const rm::Rgb& c)
{
    const float l = rm::Luminance(c);
    return l > 1e-9f ? c * (1.0f / l) : rm::Rgb(1.0f, 1.0f, 1.0f);
}

GLuint CompileSkyShader(GLenum type, const char* src, const char* name)
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
        LOG_ERROR(Graphics, "GL33 sky table: shader compile error [{}]: {}", name, log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

GLuint LinkSkyProgram(GLuint vs, PostShader shader)
{
    GLuint fs = CompileSkyShader(GL_FRAGMENT_SHADER, PostShaderSource(shader), PostShaderName(shader));
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
        LOG_ERROR(Graphics, "GL33 sky table: program link error [{}]: {}", PostShaderName(shader), log);
        glDeleteProgram(p);
        return 0;
    }
    return p;
}

// A float texture on the bound unit: linear filtering for the table the
// shaders sample, nearest for the ones the passes read with texelFetch.
GLuint MakeFloatTexture(GLenum internalFormat, GLenum format, int w, int h, const float* data, bool linear)
{
    GLuint t = 0;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, linear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    return t;
}

GLuint MakeColorFbo(GLuint tex)
{
    GLuint f = 0;
    glGenFramebuffers(1, &f);
    glBindFramebuffer(GL_FRAMEBUFFER, f);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    return f;
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

void SetUniform4(GLuint prog, const char* name, float a, float b, float c, float d)
{
    const GLint loc = glGetUniformLocation(prog, name);
    if (loc >= 0)
        glUniform4f(loc, a, b, c, d);
}

// The model's numbers for both passes (Atmosphere.hpp), the haze applied to Mie.
void SetModelUniforms(GLuint prog, float sunElevationRad, float haze)
{
    namespace a = rm::atmo;
    glUseProgram(prog);
    SetUniform4(prog, "atmoRadii", a::kGroundRadius, a::kTopRadius, a::kRayleighScaleHeight, a::kMieScaleHeight);
    SetUniform4(prog, "atmoRayleigh", a::kRayleighScattering[0], a::kRayleighScattering[1], a::kRayleighScattering[2],
                a::kMieG);
    SetUniform4(prog, "atmoMie", a::kMieScattering * haze, a::kMieExtinction * haze, a::kOzoneCentre,
                a::kOzoneHalfWidth);
    SetUniform4(prog, "atmoOzone", a::kOzoneAbsorption[0], a::kOzoneAbsorption[1], a::kOzoneAbsorption[2], 0.0f);
    SetUniform4(prog, "atmoView", sunElevationRad, a::kGroundRadius + std::max(rm::kSkyViewerHeight, 1.0f),
                static_cast<float>(rm::kSkyViewSteps), static_cast<float>(rm::kSkyViewMeanTaps));
    const GLint sizes = glGetUniformLocation(prog, "atmoSizes");
    if (sizes >= 0)
        glUniform4i(sizes, rm::kTransmittanceWidth, rm::kTransmittanceHeight, rm::kMultiScatterSize, rm::kSkyViewSize);
    const GLint trans = glGetUniformLocation(prog, "transTex");
    if (trans >= 0)
        glUniform1i(trans, 0);
    const GLint ms = glGetUniformLocation(prog, "msTex");
    if (ms >= 0)
        glUniform1i(ms, kSkyLutUnit);
    const GLint view = glGetUniformLocation(prog, "viewTex");
    if (view >= 0)
        glUniform1i(view, 0);
}

} // namespace

void EngineGL33::InitSkyTable()
{
    // Unit 8 always holds a complete texture of the right kind (a shader must
    // never sample an incomplete texture): a stand-in until the first sky-view table is built.
    GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
    const float zero[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    s_sky.standIn = MakeFloatTexture(GL_RGBA32F, GL_RGBA, 1, 1, zero, false);
    glBindTexture(GL_TEXTURE_2D, 0);
    GL33Bind::Invalidate();
    GL33Bind::Tex2D(kSkyLutUnit, s_sky.standIn);
    GL33Bind::ActiveUnit(0);
}

void EngineGL33::DeinitSkyTable()
{
    // GL objects only: the atmosphere tables are CPU data and survive a
    // context loss (the next build re-uploads them).
    DeleteTexture(s_sky.standIn);
    DeleteTexture(s_sky.transTex);
    DeleteTexture(s_sky.msTex);
    DeleteTexture(s_sky.scratchTex);
    DeleteTexture(s_sky.lutTex);
    for (GLuint* f : {&s_sky.scratchFbo, &s_sky.lutFbo})
    {
        if (*f)
            glDeleteFramebuffers(1, f);
        *f = 0;
    }
    for (GLuint* p : {&s_sky.progView, &s_sky.progMean})
    {
        if (*p)
            glDeleteProgram(*p);
        *p = 0;
    }
    if (s_sky.vao)
    {
        GL33Bind::OnVaoDeleted(s_sky.vao);
        glDeleteVertexArrays(1, &s_sky.vao);
    }
    s_sky.vao = 0;
    s_sky.programsTried = false;
    s_sky.unavailable = false;
    s_sky.uploadedHaze = -1.0f;
    s_sky.built = false;
    s_sky.open = false;
    s_sky.mode = 0;
    s_sky.forcedPrograms = false;
}

bool EngineGL33::SkyTablesReady() const
{
    return s_sky.tables.Valid() && s_sky.tables.haze == HazeOf(_lighting);
}

bool EngineGL33::PollSkyTables()
{
    if (s_sky.building && s_sky.pending.valid() &&
        s_sky.pending.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
    {
        s_sky.tables = s_sky.pending.get();
        s_sky.building = false;
        LOG_INFO(Graphics, "GL33: sky table: atmosphere tables built for haze {:.2f}", s_sky.tables.haze);
    }
    if (SkyTablesReady())
        return true;
    if (!s_sky.building)
    {
        const float haze = HazeOf(_lighting);
        s_sky.building = true;
        s_sky.pending = std::async(std::launch::async, [haze] { return rm::BuildAtmosphereTables(haze); });
    }
    return false;
}

void EngineGL33::PollSkyLightSH()
{
    if (s_sky.shBuilding && s_sky.shPending.valid() &&
        s_sky.shPending.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
    {
        s_sky.sh = s_sky.shPending.get();
        s_sky.shBuilding = false;
    }
    // Off by default (surfSkyLight 0): nothing to project.
    if (s_sky.shBuilding || !GScene || !_featureGates.computed || !_featureGates[rm::Feature::Surfaces].on ||
        _lighting.surfSkyLight <= 0.0f)
        return;
    LightSun* sun = GScene->MainLight();
    if (!sun || !PollSkyTables())
        return;
    const float elev = SunElevationRad(sun);
    const bool stale = !s_sky.sh.valid || s_sky.sh.haze != s_sky.tables.haze ||
                       std::fabs(s_sky.sh.sunElevationRad - elev) > kSkyLightRebuildRad;
    if (!stale)
        return;
    s_sky.shBuilding = true;
    s_sky.shPending =
        std::async(std::launch::async, [tables = s_sky.tables, elev] { return rm::ProjectSkyLight(tables, elev); });
}

const rm::SkyLightSH& EngineGL33::SkyLightSHNow() const
{
    return s_sky.sh;
}

bool EngineGL33::PhysicalSkyFor(float sunElevationRad, rm::PhysicalSky& out)
{
    // The photoreal rig (photoLight): the clear sky's light from the same
    // atmosphere tables, integrated on the CPU when the sun has moved
    // kPhysicalSkyRebuildDeg (about a millisecond), and the reference once per
    // haze.
    if (!PollSkyTables())
        return false;
    CameraState& c = _camera;
    const float haze = s_sky.tables.haze;
    if (c.referenceHaze != haze)
    {
        const rm::PhysicalSky ref = rm::ComputePhysicalSky(s_sky.tables, rm::kPhysicalReferenceSunDeg / kDeg);
        c.referenceIlluminant = rm::PhysicalIlluminant(ref);
        c.referenceIrradiance = rm::Luminance(c.referenceIlluminant);
        c.referenceHaze = haze;
    }
    const float deg = sunElevationRad * kDeg;
    if (!c.physical.valid || c.physicalHaze != haze ||
        std::fabs(c.physical.sunElevationDeg - deg) >= rm::kPhysicalSkyRebuildDeg)
    {
        c.physical = rm::ComputePhysicalSky(s_sky.tables, sunElevationRad);
        c.physicalHaze = haze;
    }
    out = c.physical;
    out.referenceIrradiance = c.referenceIrradiance;
    out.referenceIlluminant = c.referenceIlluminant;
    return out.valid && out.referenceIrradiance > 0.0f;
}

bool EngineGL33::RemasterSkyPending() const
{
    return s_sky.building || SkyDensityPending();
}

void EngineGL33::BindSkyTable()
{
    GL33Bind::Tex2D(kSkyLutUnit, s_sky.built && s_sky.lutTex ? s_sky.lutTex : s_sky.standIn);
}

bool EngineGL33::BuildSkyViewTableGpu(float sunElevationRad)
{
    if (s_sky.unavailable || !SkyTablesReady())
        return false;
    constexpr int n = rm::kSkyViewSize;
    if (!s_sky.programsTried)
    {
        s_sky.programsTried = true;
        GLuint vs = CompileSkyShader(GL_VERTEX_SHADER, PostShaderSource(PostShader::Vertex),
                                     PostShaderName(PostShader::Vertex));
        s_sky.progView = LinkSkyProgram(vs, PostShader::SkyView);
        s_sky.progMean = LinkSkyProgram(vs, PostShader::SkyViewMean);
        if (vs)
            glDeleteShader(vs);
        glGenVertexArrays(1, &s_sky.vao);
        GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
        s_sky.scratchTex = MakeFloatTexture(GL_RGBA32F, GL_RGBA, 2 * n, n, nullptr, false);
        s_sky.lutTex = MakeFloatTexture(GL_RGBA32F, GL_RGBA, rm::kSkyViewRegions * n, n, nullptr, true);
        glBindTexture(GL_TEXTURE_2D, 0);
        GL33Bind::Invalidate();
        s_sky.scratchFbo = MakeColorFbo(s_sky.scratchTex);
        bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
        s_sky.lutFbo = MakeColorFbo(s_sky.lutTex);
        complete = complete && glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
        BindFrameRenderTarget();
        if (!s_sky.progView || !s_sky.progMean || !complete)
        {
            LOG_ERROR(Graphics, "GL33 sky table: programs or targets failed; the sky stays classic");
            s_sky.unavailable = true;
            return false;
        }
    }
    const float haze = s_sky.tables.haze;
    if (s_sky.uploadedHaze != haze)
    {
        DeleteTexture(s_sky.transTex);
        DeleteTexture(s_sky.msTex);
        GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
        s_sky.transTex = MakeFloatTexture(GL_RGB32F, GL_RGB, rm::kTransmittanceWidth, rm::kTransmittanceHeight,
                                          s_sky.tables.transmittance.data(), false);
        s_sky.msTex = MakeFloatTexture(GL_RGB32F, GL_RGB, rm::kMultiScatterSize, rm::kMultiScatterSize,
                                       s_sky.tables.multiScatter.data(), false);
        glBindTexture(GL_TEXTURE_2D, 0);
        GL33Bind::Invalidate();
        s_sky.uploadedHaze = haze;
    }

    // What the passes change that the next draw does not set again: the
    // viewport, the target, cull, scissor, stencil and depth clamp. Depth,
    // blend, the program and the vertex array come back through the pipeline
    // and bind caches, invalidated at the end (as the post pass does).
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
    GL33Bind::Vao(s_sky.vao);

    // Pass 1: regions 0 and 1 from the transmittance (unit 0) and multiple
    // scattering (unit 8, where the table itself is not bound meanwhile).
    SetModelUniforms(s_sky.progView, sunElevationRad, haze);
    GL33Bind::Tex2D(0, s_sky.transTex);
    GL33Bind::Tex2D(kSkyLutUnit, s_sky.msTex);
    glBindFramebuffer(GL_FRAMEBUFFER, s_sky.scratchFbo);
    glViewport(0, 0, 2 * n, n);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    // Pass 2: the table, regions 0 and 1 copied and each row's mean.
    SetModelUniforms(s_sky.progMean, sunElevationRad, haze);
    GL33Bind::Tex2D(0, s_sky.scratchTex);
    glBindFramebuffer(GL_FRAMEBUFFER, s_sky.lutFbo);
    glViewport(0, 0, rm::kSkyViewRegions * n, n);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    GL33Bind::Tex2D(0, 0);
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

    s_sky.built = true;
    s_sky.builtSunElevRad = sunElevationRad;
    s_sky.builtHaze = haze;
    ++s_sky.builds;
    BindSkyTable();
    GL33Bind::ActiveUnit(0);
    return true;
}

void EngineGL33::BeginRemasterSky(Poseidon::RemasterSky mode)
{
    if (s_sky.open)
        EndRemasterSky();
    // Classic lighting, the switches off, and the paths that stay classic (a
    // portrait, a 3D preview after the world, night vision) leave the sky
    // draws alone: off draws exactly the frame without the features.
    if (!_glContext || !_lightingActive || !GScene || !GScene->GetCamera() || !GScene->MainLight() ||
        _portraitTargetSize || _worldSceneEnded || _nightVision)
        return;
    const bool wantAtmosphere = rm::FeatureOn(_lighting.features, rm::Feature::Atmosphere);
    const bool wantClouds = SkyCloudsWanted();
    // The photoreal camera (photoLight) takes the sky as scene light too: the
    // dome and the sun disk draw through the remastered programs for it.
    const bool wantCamera = CameraWanted();
    // The sun disk is the clouds' (it dims behind a dome cloud) and the
    // camera's (it is light far brighter than its sprite).
    const bool sunDisk = mode == Poseidon::RemasterSky::Sun;
    if (sunDisk ? !wantClouds && !wantCamera : !wantAtmosphere && !wantClouds && !wantCamera)
        return;
    // Until the worker has the atmosphere tables the sky table stays off (the
    // gate reads "atmosphere tables building").
    const bool tables = !sunDisk && wantAtmosphere && PollSkyTables();
    if (!tables && !wantClouds && !wantCamera)
        return;
    // Draws queued so far keep their programs.
    FlushQueues();
    s_sky.forcedPrograms = !_worldEffectsOn;
    if (s_sky.forcedPrograms)
    {
        _worldEffectsOn = true;
        UpdateRemasterPrograms();
    }
    // The frame's rig and gates (the world's first 3D pass reuses the gates).
    UploadLitFrameConstants();
    LightSun* sun = GScene->MainLight();
    const float sunElevRad = SunElevationRad(sun);
    const float haze = HazeOf(_lighting);
    const bool atmosphere = tables && _featureGates[rm::Feature::Atmosphere].on;
    // What the clouds need of this draw: the dome turns with the sky's drift
    // whenever a clouds switch is on, the sky clouds are lit while lit clouds
    // run, and the sun disk dims while the sky-matched shadows run.
    bool clouds = false;
    switch (mode)
    {
        case Poseidon::RemasterSky::Dome:
            clouds = wantClouds;
            break;
        case Poseidon::RemasterSky::Clouds:
            clouds = _featureGates[rm::Feature::LitClouds].on;
            break;
        case Poseidon::RemasterSky::Sun:
            clouds = _featureGates[rm::Feature::SkyCloudShadows].on;
            break;
    }
    const bool camera = _camera.frameOn && _camera.frame == GetFrameCounter();
    if (!(sunDisk ? clouds || camera : atmosphere || clouds || camera))
    {
        if (s_sky.forcedPrograms)
        {
            s_sky.forcedPrograms = false;
            _worldEffectsOn = false;
            UpdateRemasterPrograms();
        }
        return;
    }
    const bool dome = mode == Poseidon::RemasterSky::Dome;
    if (dome)
        GpuMark(Poseidon::gpu::Mark::SkyBegin);
    if (atmosphere && rm::SkyTableNeedsRebuild(s_sky.built, s_sky.builtSunElevRad, s_sky.builtHaze, sunElevRad, haze) &&
        BuildSkyViewTableGpu(sunElevRad) && dome)
        GpuMark(Poseidon::gpu::Mark::SkyTableEnd);
    s_sky.open = true;
    s_sky.mode = static_cast<int>(mode);
    Landscape* land = GScene->GetLandscape();
    UploadLitSky(-sun->Direction(), land ? land->SkyThrough() : 1.0f);
    FlushLitConstants();
    BindSkyTable();
    BindSkyDensity();
    GL33Bind::ActiveUnit(0);
}

void EngineGL33::EndRemasterSky()
{
    if (!s_sky.open)
        return;
    // The sky's draws commit with its programs and sky mode.
    FlushQueues();
    const bool dome = s_sky.mode == static_cast<int>(Poseidon::RemasterSky::Dome);
    s_sky.open = false;
    s_sky.mode = 0;
    SetLitSkyMode(0);
    if (dome)
        GpuMark(Poseidon::gpu::Mark::SkyEnd);
    if (s_sky.forcedPrograms)
    {
        // Back to the classic programs for the stars, the sun and the moon
        // until the world's first 3D pass, unless a draw inside the sky pass
        // opened that pass already (it set the world's flags itself).
        s_sky.forcedPrograms = false;
        if (_worldPassOpen)
        {
            static bool s_warned = false;
            if (!s_warned)
                LOG_WARN(Graphics, "GL33 sky table: the world's 3D pass opened inside the sky pass");
            s_warned = true;
            return;
        }
        _worldEffectsOn = false;
        UpdateRemasterPrograms();
    }
}

void EngineGL33::UploadLitSky(const Vector3& toSun, float skyThrough)
{
    namespace LC = LitConst;
    LightSun* sun = GScene ? GScene->MainLight() : nullptr;
    Camera* camera = GScene ? GScene->GetCamera() : nullptr;
    const bool live = _featureGates.computed && _featureGates[rm::Feature::Atmosphere].on && s_sky.built && sun &&
                      camera && _worldEffectsOn;
    const float sunElevRad = std::asin(std::max(-1.0f, std::min(1.0f, toSun.Y())));
    const float day = live ? rm::SkyDayFade(sunElevRad * kDeg) : 0.0f;
    // The camera basis that turns a screen position into a view ray: the sky
    // table's dome and fog and the lit clouds read it.
    auto cameraBasis = [&]
    {
        Vector3 r = camera->DirectionAside();
        Vector3 u = camera->DirectionUp();
        Vector3 f = camera->Direction();
        r.Normalize();
        u.Normalize();
        f.Normalize();
        const float l = camera->Left(), t = camera->Top();
        SetLitSlot(LC::SkyRight, r.X() * l, r.Y() * l, r.Z() * l, rm::atmo::kMieG);
        SetLitSlot(LC::SkyUp, u.X() * t, u.Y() * t, u.Z() * t, 0.0f);
        SetLitSlot(LC::SkyFwd, f.X(), f.Y(), f.Z(), static_cast<float>(s_sky.mode));
    };
    if (day <= 0.0f)
    {
        // The vertex and fragment shaders test the two blends.
        if (camera && _featureGates.computed && _featureGates[rm::Feature::LitClouds].on && _worldEffectsOn)
        {
            cameraBasis();
        }
        else
        {
            SetLitSlot(LC::SkyRight, 0.0f, 0.0f, 0.0f, rm::atmo::kMieG);
            SetLitSlot(LC::SkyUp, 0.0f, 0.0f, 0.0f, 0.0f);
            SetLitSlot(LC::SkyFwd, 0.0f, 0.0f, 1.0f, static_cast<float>(s_sky.mode));
        }
        SetLitSlot(LC::Atmo, 0.0f, 0.0f, 0.0f, rm::kSkyCloudMaskDeadZone);
        SetLitSlot(LC::AtmoCloudSun, 0.0f, 0.0f, 0.0f, 0.0f);
        SetLitSlot(LC::AtmoGlow, 0.0f, 0.0f, 0.0f, 1.0f);
        SetLitSlot(LC::AtmoDome, 0.0f, 0.0f, 0.0f, 0.0f);
        return;
    }
    cameraBasis();
    SetLitSlot(LC::Atmo, day, rm::SkyOvercastCloudFloor(skyThrough), day, rm::kSkyCloudMaskDeadZone);

    // Brightness reference: the classic dome's vertex colour (TLVertexTable::
    // DoSkyLighting), its light SkyColor plus the sun glow SunSkyColor times
    // cos^6 toward the sun, both display. Each row of the sky keeps the classic
    // brightness (texture row times that colour, the glow by its row mean) on
    // average over its azimuths; the direction-dependent part and, with the
    // hue, the colour come from the table.
    Color dome = sun->SkyColor() * _accomodateEye;
    dome.SaturateMinMax();
    const Color glow = sun->SunSkyColor() * _accomodateEye;
    // Under the photoreal camera the clear sky takes the camera's hue knob
    // (the physical hue by default).
    const bool photoCamera = _camera.frameOn && _camera.frame == GetFrameCounter();
    const float hue = photoCamera ? _lighting.camSkyHue : _lighting.atmoHue;
    SetLitSlot(LC::AtmoDome, dome.R(), dome.G(), dome.B(), std::max(0.0f, std::min(1.0f, hue)));
    SetLitSlot(LC::AtmoGlow, glow.R(), glow.G(), glow.B(), _lighting.atmoSat);
    // The clouds' light: the classic dome light's luminance, split into the
    // sun seen from the cloud deck and the sky above (whose colour the shader
    // takes from the table).
    const float lumC = rm::Luminance(rm::ToLinear(rm::Rgb(dome.R(), dome.G(), dome.B())));
    const float sunShare = rm::SkyCloudSunShare(sunElevRad * kDeg);
    const rm::Rgb deckChroma = UnitLuminance(rm::AtmosphereTransmittance(
        s_sky.tables, rm::atmo::kGroundRadius + rm::kSkyCloudDeckHeight, std::sin(sunElevRad)));
    if (photoCamera && _camera.skyScale > 0.0f)
    {
        // Under the photoreal camera the clouds take the physical light, so
        // they stand in their physical ratio to the table's sky around them:
        // the sun as the deck sees it, through the air above the deck (at
        // dawn and dusk far brighter and less red than at the ground, so the
        // clouds glow while the land is in twilight), at kCloudSunLight of a
        // surface facing it; the sky above as on an up-facing surface; both at
        // the brightness a white surface shows (1.6 x the irradiance at the
        // rig's scale, LightRig::skyScale = 1.6 pi scale camSky).
        constexpr float kWhite = 1.6f, kCloudSunLight = 1.3f, kPi = 3.14159265f;
        const float scale = _camera.skyScale / (kWhite * kPi * std::max(_lighting.camSky, 1e-3f));
        const rm::Rgb deckT = rm::AtmosphereTransmittance(
            s_sky.tables, rm::atmo::kGroundRadius + rm::kSkyCloudDeckHeight, std::sin(sunElevRad));
        const float skyLum = rm::Luminance(rm::Rgb(_lightRigSky.R(), _lightRigSky.G(), _lightRigSky.B()));
        const rm::Rgb deckSun = deckT * (kWhite * kCloudSunLight * scale);
        SetLitSlot(LC::AtmoCloudSun, deckSun.r, deckSun.g, deckSun.b, kWhite * skyLum);
        return;
    }
    const rm::Rgb deckSun = deckChroma * (lumC * sunShare);
    SetLitSlot(LC::AtmoCloudSun, deckSun.r, deckSun.g, deckSun.b, lumC * (1.0f - sunShare));
}

std::string EngineGL33::RemasterSkyCheck(int texels)
{
    if (!_glContext)
        return "FAIL:no context";
    if (!s_sky.built || !s_sky.lutTex || !SkyTablesReady() || s_sky.builtHaze != s_sky.tables.haze)
        return "FAIL:no sky-view table built for the current haze";
    constexpr int n = rm::kSkyViewSize;
    constexpr int w = rm::kSkyViewRegions * n;
    FlushQueues();
    rm::SkyViewTable gpu;
    gpu.rgba.assign(static_cast<size_t>(w) * n * 4, 0.0f);
    gpu.sunElevationRad = s_sky.builtSunElevRad;
    gpu.haze = s_sky.builtHaze;
    GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_sky.lutTex);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, gpu.rgba.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    GL33Bind::Invalidate();

    // Regions 0 and 1 against SkyViewTexel at a grid of texels, region 2
    // against SkyViewRowMean of the GPU's own regions 0 and 1 (so each pass is
    // checked on its own) and its alpha against SkyViewRowGlow. The error is relative to the texel, with a floor of
    // 1e-3 of the region's largest value (texels under the horizon and deep
    // twilight are tiny).
    const int count = std::max(4, std::min(texels, n * n));
    const int step = std::max(1, static_cast<int>(std::sqrt(static_cast<float>(n * n) / count)));
    float worst[3] = {0.0f, 0.0f, 0.0f};
    int worstAt[3][2] = {};
    float peak[3] = {0.0f, 0.0f, 0.0f};
    for (int y = 0; y < n; ++y)
        for (int x = 0; x < w; ++x)
        {
            const rm::Rgb c = gpu.At(x, y);
            peak[x / n] = std::max(peak[x / n], std::max(c.r, std::max(c.g, c.b)));
        }
    int checked = 0;
    auto compare = [&](int region, int x, int y, const rm::Rgb& want)
    {
        const rm::Rgb got = gpu.At(region * n + x, y);
        const float floorV = 1e-3f * peak[region];
        const float want3[3] = {want.r, want.g, want.b};
        const float got3[3] = {got.r, got.g, got.b};
        for (int c = 0; c < 3; ++c)
        {
            const float err = std::fabs(got3[c] - want3[c]) / std::max(std::fabs(want3[c]), floorV);
            if (!(err <= worst[region])) // NaN counts as the worst
            {
                worst[region] = std::isfinite(err) ? err : 1e9f;
                worstAt[region][0] = x;
                worstAt[region][1] = y;
            }
        }
        ++checked;
    };
    for (int y = step / 2; y < n; y += step)
        for (int x = step / 2; x < n; x += step)
            for (int region = 0; region < 2; ++region)
                compare(region, x, y, rm::SkyViewTexel(s_sky.tables, region, x, y, s_sky.builtSunElevRad));
    // Region 2's alpha: the classic sun glow's row weight (an absolute error:
    // it runs from 0 to 1).
    float worstGlow = 0.0f;
    int worstGlowRow = 0;
    for (int y = 0; y < n; ++y)
    {
        compare(2, 0, y, rm::SkyViewRowMean(gpu, y));
        const float err = std::fabs(gpu.AlphaAt(2 * n, y) - rm::SkyViewRowGlow(s_sky.builtSunElevRad, y));
        if (!(err <= worstGlow))
        {
            worstGlow = std::isfinite(err) ? err : 1e9f;
            worstGlowRow = y;
        }
    }
    constexpr float kTolerance = 5e-3f;
    const bool ok =
        worst[0] <= kTolerance && worst[1] <= kTolerance && worst[2] <= kTolerance && worstGlow <= kTolerance;
    char buf[384];
    std::snprintf(buf, sizeof(buf),
                  "%s:texels=%d sun=%.3f haze=%.2f builds=%u rel_err rayleigh=%.2e@%d,%d mie=%.2e@%d,%d "
                  "mean=%.2e@row%d glow_abs=%.2e@row%d tolerance=%.0e",
                  ok ? "OK" : "FAIL", checked, static_cast<double>(s_sky.builtSunElevRad * kDeg),
                  static_cast<double>(s_sky.builtHaze), s_sky.builds, static_cast<double>(worst[0]), worstAt[0][0],
                  worstAt[0][1], static_cast<double>(worst[1]), worstAt[1][0], worstAt[1][1],
                  static_cast<double>(worst[2]), worstAt[2][1], static_cast<double>(worstGlow), worstGlowRow,
                  static_cast<double>(kTolerance));
    LOG_INFO(Graphics, "GL33 sky table check: {}", buf);
    return buf;
}
