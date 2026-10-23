// Remastered lighting: the volumetric fog and atmosphere (the Air post stage;
// feature switch volFog, the look lane "fog", superseding B8's air pass on the
// same stage). The medium and its weather and time rules, the slice layout and
// the fog floor are pure (Graphics/Remaster/VolumetricFog.hpp), the shaders
// GL-free (EngineGL33_PostFogShaders.cpp).
//
// The stage runs after the occlusion and before the bloom, on the resolved
// frame, whenever the switch and its gate are on (it replaces the distance
// fog, so it always runs then):
//   1. psFogInject: the froxel volume's medium and the sky-matched light of
//      its haze, air, mist and fog, eight slices per draw into the injection
//      volume, blended with the last frame's injection reprojected (the
//      history), a jittered point per cell and frame;
//   2. psFogIntegrate: front to back, the light scattered toward the eye and
//      the transmittance up to each cell's far face;
//   3. psFogNear (when the sun is in the field of view of wet air, or lamps,
//      headlights or fires shine at night): half resolution, the sun's forward
//      lobe marched with its shadows in the first kFogBeamReach metres, and
//      each light's light in the air in closed form;
//   4. psFogApply: full resolution, C' = C T + S by each pixel's depth plus
//      the near field, into fog.outTex, which the bloom and the composite read
//      in place of the resolved frame (PostSceneTex). With the stage off they
//      read the resolved frame (or B8's air) as before, so "off" is the frame
//      before the feature.
// The legacy shafts hold off while it runs (PlanPostStages). The forward fog
// keeps only its view-distance edge (UpdateFogMedium sets litHaze.x;
// finishFragment). AI sight is not set here: the simulation computes it from
// the same inputs (WorldFogInputs) and knobs through the same ComputeFogMedium,
// whether or not this stage runs (TerrainLighting.hpp SimulateFogSight); the
// fog floor below is drawn from AI sight's own map.
//
// GL state: the stage binds its inputs to kPostUnits and unit 12 (the fog
// floor, kFogFloorUnit) and reads units 8 (the sky table, rebound here), 9 and
// 15 as the world bound them. Unit 2 takes the cascade array (or the depth
// stand-in, never texture 0: the F3 rule) under the comparison sampler for the
// injection and the near field and gets the post pass's sampler 0 back after
// them; the 3D volumes and the floor are unbound at the end of the stage.

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/EngineGL33_PostPass.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <Poseidon/Core/Application.hpp>
#include <Poseidon/Core/Config/EngineConfig.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/Graphics/Remaster/PostStages.hpp>
#include <Poseidon/Graphics/Remaster/VolumetricFog.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <Poseidon/World/Scene/Camera/Camera.hpp>
#include <Poseidon/World/Terrain/Landscape.hpp>
#include <Poseidon/World/Terrain/TerrainLighting.hpp>

#include <algorithm>
#include <cmath>
#include <string>

namespace rm = Poseidon::remaster;
namespace fl = Poseidon::lighting;
using namespace Poseidon::gl33post;
using namespace Poseidon::gl33lighting;

namespace
{

constexpr int kFogMaxLights = 16;         // lights in the near field's uniform arrays
constexpr float kFogLightRange = 300.0f;  // lights farther than this from the eye add nothing
constexpr float kFogHistoryShare = 0.12f; // this frame's share in the blend with the reprojected history
constexpr float kFogHistoryCut = 60.0f;   // a camera jump longer than this (m) starts the history again
constexpr float kFogMistDrift = 1.6f;     // m/s the mist banks drift with the wind at least
constexpr float kFogSkyFade = 0.044f;     // the sky keeps the clear air with exp(-sin(elevation) / this): 2.5 deg
// The near field marches the sun when it is this close to the view axis (deg):
// its forward lobe's excess over the isotropic level is gone 50 degrees off
// the sun, and the frame reaches about 40 degrees off the axis.
constexpr float kFogBeamViewAngle = 95.0f;
// The night air's light: the night sky's own share of the classic fog colour
// (the dome's horizon at night is far darker than the colour the classic fog
// faded to), linear.
// (Look director round 2: 0.35; at 0.18 a foggy street at night faded its
// houses into black, so the fog read as a darker night, not as fog.)
constexpr float kFogNightAirShare = 0.35f;
// The horizon ring: azimuth bins, the share of a new measurement in a bin
// (the rest is the last frame's), and the decay of what a bin knows per frame
// out of view.
constexpr int kFogRingBins = 256;
// Row 0 the sky's colour, row 1 the sine of the elevation it was measured at.
constexpr int kFogRingRows = 2;
constexpr float kFogRingShare = 0.3f;
constexpr float kFogRingDecay = 0.995f;
// The night glow's gain (the blurred light laid into the fog), and the share
// of the sixteenth-resolution blur in it (the broad part of the glow).
// (Look director round 2: 2.6 and 0.7, the lit town's light spread through
// its fog, so the fog itself reads at night.)
constexpr float kFogGlowGain = 2.6f;
constexpr float kFogGlowWide = 0.7f;
// The share of a street lamp's colour its light in the fog keeps.
constexpr float kFogLampAirChroma = 0.4f;

GLuint CompileFogShader(GLenum type, const char* src, const char* name)
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
        char log[2048];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        LOG_ERROR(Graphics, "GL33 fog: shader compile error [{}]: {}", name, log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

GLuint LinkFogProgram(GLuint vs, PostShader shader)
{
    GLuint fs = CompileFogShader(GL_FRAGMENT_SHADER, PostShaderSource(shader), PostShaderName(shader));
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
        char log[2048];
        glGetProgramInfoLog(p, sizeof(log), nullptr, log);
        LOG_ERROR(Graphics, "GL33 fog: program link error [{}]: {}", PostShaderName(shader), log);
        glDeleteProgram(p);
        return 0;
    }
    // The rig: the LitConstants block at its binding.
    const GLuint lit = glGetUniformBlockIndex(p, "LitConstants");
    if (lit != GL_INVALID_INDEX)
        glUniformBlockBinding(p, lit, kLitBinding);
    return p;
}

GLuint MakeVolume(const rm::FogGrid& g)
{
    GLuint t = 0;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_3D, t);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, g.cellsX, g.cellsY, g.slices, 0, GL_RGBA, GL_HALF_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);
    return t;
}

GLuint Make2D(GLenum internalFormat, GLenum format, GLenum type, int w, int h, const void* data = nullptr)
{
    GLuint t = 0;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, type, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    return t;
}

// One FBO per kFogSlicesPerDraw slices of a volume, a slice per attachment.
bool MakeSliceFbos(GLuint tex, int slices, std::vector<unsigned int>& fbos)
{
    static const GLenum kBufs[rm::kFogSlicesPerDraw] = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
        GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7};
    bool complete = true;
    for (int first = 0; first < slices; first += rm::kFogSlicesPerDraw)
    {
        GLuint f = 0;
        glGenFramebuffers(1, &f);
        glBindFramebuffer(GL_FRAMEBUFFER, f);
        for (int i = 0; i < rm::kFogSlicesPerDraw; ++i)
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, tex, 0, first + i);
        glDrawBuffers(rm::kFogSlicesPerDraw, kBufs);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            complete = false;
        fbos.push_back(f);
    }
    return complete;
}

bool MakeFbo(GLuint tex, unsigned int& fbo)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

void DeleteTex(unsigned int& t)
{
    if (t)
    {
        GL33Bind::OnTexDeleted(t);
        glDeleteTextures(1, &t);
    }
    t = 0;
}

void DeleteFbo(unsigned int& f)
{
    if (f)
        glDeleteFramebuffers(1, &f);
    f = 0;
}

void DeleteFbos(std::vector<unsigned int>& fbos)
{
    for (unsigned int& f : fbos)
        DeleteFbo(f);
    fbos.clear();
}

void SetMat3(GLuint prog, const char* name, const float* columnMajor)
{
    const GLint loc = glGetUniformLocation(prog, name);
    if (loc >= 0)
        glUniformMatrix3fv(loc, 1, GL_FALSE, columnMajor);
}

// The view matrix's rotation, inverted: view-space directions (x right, y up,
// z forward) to camera-relative world. View x = _11 X + _21 Y + _31 Z (the
// vertex shaders' `view * p` with the matrix uploaded from &_11). The rotation
// is orthonormal, so the inverse is the transpose.
void ViewToWorld(const GfxMatrix& v, float out[9])
{
    // Column c of the result is view axis c in world space: row c of the
    // world-to-view rotation.
    const float rows[3][3] = {{v._11, v._21, v._31}, {v._12, v._22, v._32}, {v._13, v._23, v._33}};
    for (int c = 0; c < 3; ++c)
        for (int r = 0; r < 3; ++r)
            out[c * 3 + r] = rows[c][r];
}

float ToLinear(float display)
{
    return std::pow(std::max(display, 0.0f), 2.2f);
}

// A sample of one of the sight map's grids at world x, z (the nearest cell,
// clamped to the edge), 0 without a map.
float SampleMap(const rm::FogSightMap* map, std::vector<float> rm::FogSightMap::* grid, float x, float z)
{
    if (!map || !map->Valid())
        return 0.0f;
    const int last = map->size - 1;
    const int xi = std::clamp(static_cast<int>(std::lround(x / map->cellSize)), 0, last);
    const int zi = std::clamp(static_cast<int>(std::lround(z / map->cellSize)), 0, last);
    return (map->*grid)[static_cast<size_t>(zi) * map->size + xi];
}

float Smooth(float e0, float e1, float x)
{
    const float t = std::clamp((x - e0) / (e1 - e0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

// The last frame that warned of a difference between the drawn air and AI
// sight (UpdateFogMedium), so the warning comes at most every 600 frames.
std::uint32_t s_sightWarnFrame = 0;

// A light in the air: its reach there (m) and its intensity (linear, I in
// E = I / r^2): the physical lamps and low beams (FrameLights.hpp), and a
// fire's or another light's classic colour at a strength of its kind.
struct AirLight
{
    float pos[3];
    float reach;
    float col[3];
    bool beam;
    float dir[3];
    float sigma;
};

} // namespace

void EngineGL33::UpdateFogMedium()
{
    // Once per frame, with the gates (the world's first pass computes them).
    const std::uint32_t frame = GetFrameCounter();
    if (_featureGates.computed && _featureGates.frame == frame && _fog.mediumFrame != frame && GScene)
    {
        _fog.mediumFrame = frame;
        // The fog is applied where the post effects run and, as the previous
        // frame showed, on a world with its depth (the gate; a gunner's optic
        // overlay keeps it, its mask left alone by the apply); anywhere else
        // the forward fog keeps its classic edge.
        const bool post =
            _lighting.postEffects && SSAAActive() && !_post.unavailable && !_fog.unavailable && !_prevWorldDepthCleared;
        const bool on = post && _featureGates[rm::Feature::VolFog].on;
        // The world's inputs, the same function AI sight's step takes them
        // from (the aridity, for the dust, is the renderer's own).
        rm::FogInputs in = Poseidon::WorldFogInputs(GScene->GetLandscape());
        in.aridity = _fog.aridity;
        _fog.medium = rm::ComputeFogMedium(in, _lighting, on);
        _fog.on = on;
        rm::SetDrawnFogMedium(on ? &_fog.medium : nullptr);
        // The drawn mist and humid air are AI sight's (the simulation step
        // computed them from the same inputs and knobs this frame); a
        // difference means a second source crept in.
        if (on)
        {
            const rm::FogMedium& ai = rm::FogSightMedium();
            auto differs = [](float a, float b) { return std::fabs(a - b) > 1e-4f * std::max(std::fabs(a), 1e-6f); };
            if ((differs(ai.groundSigma, _fog.medium.groundSigma) || differs(ai.mistSigma, _fog.medium.mistSigma) ||
                 differs(ai.mistLayer, _fog.medium.mistLayer) || differs(ai.groundHeight, _fog.medium.groundHeight)) &&
                frame - s_sightWarnFrame > 600u)
            {
                s_sightWarnFrame = frame;
                LOG_WARN(Graphics,
                         "GL33 fog: drawn air and AI sight differ: ground {:.3e} vs {:.3e}, mist {:.3e} vs {:.3e} "
                         "(a knob changed after the simulation step, or a second source of the medium)",
                         _fog.medium.groundSigma, ai.groundSigma, _fog.medium.mistSigma, ai.mistSigma);
            }
        }
        if (on && frame % 600u == 0u)
            LOG_INFO(Graphics,
                     "GL33 fog: medium haze={:.2e} ground={:.2e} weather={:.2e} mist={:.2e} dust={:.2e} event={:.2f} "
                     "morning={:.2f} night={:.2f} sky={:.2f} fogColor=({:.3f},{:.3f},{:.3f}) zFar={:.0f} stars={:.2f}",
                     _fog.medium.hazeSigma, _fog.medium.groundSigma, _fog.medium.weatherSigma, _fog.medium.mistSigma,
                     _fog.medium.dustSigma, _fog.medium.mistEvent, _fog.medium.morning, _fog.medium.night,
                     _fog.medium.skyThrough, _fogColor.R(), _fogColor.G(), _fogColor.B(), _fog.medium.zFar,
                     _fog.starKeep);
    }
    // While the fog runs it sets the classic fog's edge (B8's haze, whose
    // gates are off then, set it before this call otherwise).
    if (_fog.mediumFrame == frame && _fog.on)
        SetLitSlot(LitConst::Haze, _fog.medium.fogEdgeScale - 1.0f, 0.0f, 0.0f, 0.0f);
}

void EngineGL33::DestroyFogResources()
{
    DeleteFbos(_fog.injectFbo[0]);
    DeleteFbos(_fog.injectFbo[1]);
    DeleteFbos(_fog.integFbo);
    DeleteTex(_fog.injectTex[0]);
    DeleteTex(_fog.injectTex[1]);
    DeleteTex(_fog.integTex);
    DeleteFbo(_fog.outFbo);
    DeleteTex(_fog.outTex);
    DeleteTex(_fog.veilTex);
    DeleteFbo(_fog.snapFbo);
    DeleteTex(_fog.snapTex);
    _fog.snapW = _fog.snapH = 0;
    _fog.snapFrame = 0xffffffffu;
    DeleteFbo(_fog.nearFbo);
    DeleteTex(_fog.nearTex);
    for (int i = 0; i < 2; ++i)
    {
        DeleteFbo(_fog.ringFbo[i]);
        DeleteTex(_fog.ringTex[i]);
    }
    for (int i = 0; i < 4; ++i)
    {
        DeleteFbo(_fog.glowFbo[i]);
        DeleteTex(_fog.glowTex[i]);
    }
    DeleteTex(_fog.floorTex);
    for (unsigned int* prog :
         {&_fog.progInject, &_fog.progIntegrate, &_fog.progNear, &_fog.progApply, &_fog.progRing, &_fog.progGlow})
    {
        if (*prog)
            glDeleteProgram(*prog);
        *prog = 0;
    }
    _fog.grid = rm::FogGrid{};
    _fog.outW = _fog.outH = 0;
    _fog.nearW = _fog.nearH = 0;
    _fog.floorFor = nullptr;
    _fog.sightMap.reset();
    rm::SetDrawnFogMedium(nullptr);
    _fog.programsTried = false;
    _fog.unavailable = false;
    _fog.historyValid = false;
}

bool EngineGL33::EnsureFogResources(int w, int h, const rm::FogGrid& grid)
{
    if (!_fog.programsTried)
    {
        _fog.programsTried = true;
        GLuint vs = CompileFogShader(GL_VERTEX_SHADER, PostShaderSource(PostShader::Vertex),
                                     PostShaderName(PostShader::Vertex));
        _fog.progInject = LinkFogProgram(vs, PostShader::FogInject);
        _fog.progIntegrate = LinkFogProgram(vs, PostShader::FogIntegrate);
        _fog.progNear = LinkFogProgram(vs, PostShader::FogNear);
        _fog.progApply = LinkFogProgram(vs, PostShader::FogApply);
        _fog.progRing = LinkFogProgram(vs, PostShader::FogRing);
        _fog.progGlow = LinkFogProgram(vs, PostShader::FogGlow);
        if (vs)
            glDeleteShader(vs);
        if (!_fog.progInject || !_fog.progIntegrate || !_fog.progNear || !_fog.progApply || !_fog.progRing ||
            !_fog.progGlow)
        {
            LOG_ERROR(Graphics, "GL33 fog: programs failed to build; the classic distance fog stays");
            _fog.unavailable = true;
        }
    }
    if (_fog.unavailable || grid.slices <= 0)
        return false;

    bool complete = true;
    const rm::FogGrid& g = _fog.grid;
    if (g.cellsX != grid.cellsX || g.cellsY != grid.cellsY || g.slices != grid.slices || !_fog.integTex)
    {
        for (int i = 0; i < 2; ++i)
        {
            DeleteFbos(_fog.injectFbo[i]);
            DeleteTex(_fog.injectTex[i]);
        }
        DeleteFbos(_fog.integFbo);
        DeleteTex(_fog.integTex);
        GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
        for (int i = 0; i < 2; ++i)
            _fog.injectTex[i] = MakeVolume(grid);
        _fog.integTex = MakeVolume(grid);
        glBindTexture(GL_TEXTURE_3D, 0);
        GL33Bind::Invalidate();
        for (int i = 0; i < 2; ++i)
            complete = MakeSliceFbos(_fog.injectTex[i], grid.slices, _fog.injectFbo[i]) && complete;
        complete = MakeSliceFbos(_fog.integTex, grid.slices, _fog.integFbo) && complete;
        _fog.grid = grid;
        _fog.historyValid = false;
        LOG_INFO(Graphics, "GL33 fog: froxel volume {}x{}x{} ({} px cells)", grid.cellsX, grid.cellsY, grid.slices,
                 grid.cellPixels);
    }
    if (_fog.outW != w || _fog.outH != h || !_fog.outTex || !_fog.veilTex || !_fog.nearTex)
    {
        DeleteFbo(_fog.outFbo);
        DeleteTex(_fog.outTex);
        DeleteTex(_fog.veilTex);
        DeleteFbo(_fog.nearFbo);
        DeleteTex(_fog.nearTex);
        for (int i = 0; i < 4; ++i)
        {
            DeleteFbo(_fog.glowFbo[i]);
            DeleteTex(_fog.glowTex[i]);
        }
        GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
        // Half floats (look director round 2): the camera's curve stretches
        // the fogged frame, and an 8-bit store needed a dither the curve then
        // drew as a fine diagonal hatch in the sky's gradients.
        _fog.outTex = Make2D(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, w, h);
        _fog.veilTex = Make2D(GL_R8, GL_RED, GL_UNSIGNED_BYTE, w, h);
        _fog.glowW[0] = std::max(1, (w + 3) / 4);
        _fog.glowH[0] = std::max(1, (h + 3) / 4);
        _fog.glowW[1] = std::max(1, (_fog.glowW[0] + 3) / 4);
        _fog.glowH[1] = std::max(1, (_fog.glowH[0] + 3) / 4);
        for (int i = 0; i < 4; ++i)
            _fog.glowTex[i] = Make2D(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, _fog.glowW[i / 2], _fog.glowH[i / 2]);
        _fog.nearW = (w + 1) / 2;
        _fog.nearH = (h + 1) / 2;
        _fog.nearTex = Make2D(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, _fog.nearW, _fog.nearH);
        GL33Bind::Invalidate();
        complete = MakeFbo(_fog.outTex, _fog.outFbo) && complete;
        {
            // The apply's second output: the surface's share of the light.
            static const GLenum kOutBufs[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
            glBindFramebuffer(GL_FRAMEBUFFER, _fog.outFbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _fog.veilTex, 0);
            glDrawBuffers(2, kOutBufs);
            complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE && complete;
        }
        complete = MakeFbo(_fog.nearTex, _fog.nearFbo) && complete;
        for (int i = 0; i < 4; ++i)
            complete = MakeFbo(_fog.glowTex[i], _fog.glowFbo[i]) && complete;
        if (!_fog.ringTex[0])
        {
            GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
            // Two rows: the colour, and where it was measured (look fix "dawn").
            const std::vector<float> zero(static_cast<size_t>(kFogRingBins) * kFogRingRows * 4, 0.0f);
            for (int i = 0; i < 2; ++i)
            {
                _fog.ringTex[i] = Make2D(GL_RGBA16F, GL_RGBA, GL_FLOAT, kFogRingBins, kFogRingRows, zero.data());
                glBindTexture(GL_TEXTURE_2D, _fog.ringTex[i]);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glBindTexture(GL_TEXTURE_2D, 0);
                complete = MakeFbo(_fog.ringTex[i], _fog.ringFbo[i]) && complete;
            }
            GL33Bind::Invalidate();
        }
        _fog.outW = w;
        _fog.outH = h;
    }
    if (!complete)
    {
        LOG_WARN(Graphics, "GL33 fog: targets incomplete; the classic distance fog stays");
        DestroyFogResources();
        _fog.programsTried = true;
        _fog.unavailable = true;
        return false;
    }
    return true;
}

void EngineGL33::UpdateFogFloor()
{
    // The dust's aridity follows the terrain map's ground tints (render only).
    const rm::TerrainHorizonMap* map = (_terrainMap && _terrainMap->Valid()) ? _terrainMap.get() : nullptr;
    if (map != _fog.floorFor)
    {
        _fog.floorFor = map;
        if (map && map->height.size() == static_cast<size_t>(map->size) * static_cast<size_t>(map->size))
            _fog.aridity = rm::FogAridity(map->groundTint, map->height);
    }
    // The fog floor and the mist layer are AI sight's own map (built by the
    // simulation when the island loads, VolumetricFog.hpp BuildFogSightMap):
    // the drawn mist lies where AI sight finds it, from the first frame and
    // whatever the terrain lighting's settings.
    std::shared_ptr<const rm::FogSightMap> sight = rm::FogSightMapInUse();
    if (sight && !sight->Valid())
        sight.reset();
    if (sight == _fog.sightMap)
        return;
    DeleteTex(_fog.floorTex);
    _fog.sightMap = sight;
    if (!sight)
        return;
    std::vector<float> packed(sight->floorY.size() * 2);
    for (size_t i = 0; i < sight->floorY.size(); ++i)
    {
        packed[i * 2] = sight->floorY[i];
        packed[i * 2 + 1] = sight->layer[i];
    }
    GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    _fog.floorTex = Make2D(GL_RG32F, GL_RG, GL_FLOAT, sight->size, sight->size, packed.data());
    GL33Bind::Invalidate();
    _fog.floorCell = sight->cellSize;
    _fog.floorSize = sight->size;
    float layerMax = 0.0f;
    size_t layerCells = 0;
    for (float l : sight->layer)
    {
        layerMax = std::max(layerMax, l);
        layerCells += l > 1.0f ? 1u : 0u;
    }
    LOG_INFO(Graphics,
             "GL33 fog: fog floor {}x{} at {} m (AI sight's map); mist layer up to {:.1f} m over {:.1f}% of the map; "
             "aridity {:.2f}",
             sight->size, sight->size, sight->cellSize, layerMax,
             100.0 * static_cast<double>(layerCells) / static_cast<double>(_fog.sightMap->floorY.size()), _fog.aridity);
}

void EngineGL33::SnapshotWorldBeforeOverlay()
{
    // Only while the fog runs this frame (its medium is set before the world
    // pass draws) and the frame target is the scaled one.
    if (!_glContext || !_fog.on || _fog.mediumFrame != GetFrameCounter() || !SSAAActive() || !_ssaaFbo)
        return;
    int w = 0, h = 0;
    RenderTargetSize(w, h);
    if (w <= 0 || h <= 0)
        return;
    // The world's queued draws first: the snapshot is the world as drawn.
    FlushAndFreeAllQueues(_queueNo);
    if (_fog.snapW != w || _fog.snapH != h || !_fog.snapTex)
    {
        DeleteFbo(_fog.snapFbo);
        DeleteTex(_fog.snapTex);
        GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
        _fog.snapTex = Make2D(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, w, h);
        GL33Bind::Invalidate();
        if (!MakeFbo(_fog.snapTex, _fog.snapFbo))
        {
            DeleteFbo(_fog.snapFbo);
            DeleteTex(_fog.snapTex);
            BindFrameRenderTarget();
            return;
        }
        _fog.snapW = w;
        _fog.snapH = h;
    }
    const GLboolean scissor = glIsEnabled(GL_SCISSOR_TEST);
    if (scissor)
        glDisable(GL_SCISSOR_TEST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _ssaaFbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fog.snapFbo);
    glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    if (scissor)
        glEnable(GL_SCISSOR_TEST);
    // Back to the frame target for the overlay (its draw buffers are its own).
    glBindFramebuffer(GL_FRAMEBUFFER, _ssaaFbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    _fog.snapFrame = GetFrameCounter();
}

unsigned int EngineGL33::PostSceneTex(const PostFrame& f) const
{
    // The frame with the fog or B8's air applied when the Air stage ran.
    if (!f.plan.Runs(rm::PostStage::Air))
        return f.frameColor;
    if (!f.plan.fog)
        return _post.airOutTex;
    const bool fogged = _fog.appliedFrame == GetFrameCounter() && _fog.outTex;
    return fogged ? _fog.outTex : f.frameColor;
}

unsigned int EngineGL33::PostFogVeilTex(const PostFrame& f) const
{
    if (!f.plan.Runs(rm::PostStage::Air) || !f.plan.fog)
        return 0;
    const bool fogged = _fog.appliedFrame == GetFrameCounter() && _fog.outTex && _fog.veilTex;
    return fogged ? _fog.veilTex : 0;
}

void EngineGL33::PostFog(const PostFrame& f)
{
    // Under the photoreal camera the frame holds scene light on its encoding
    // (the ring and the glow decode it; the apply reads litCamera itself).
    const bool cameraFrame = f.camera;
    // A screen overlay in the world pass, with the world before it saved:
    // the ring, the glow and the apply read that world (the overlay's black
    // mask is not the sky nor a light), and the apply lays the overlay back.
    const bool overlaySnap = _worldOverlayDrawn && _fog.snapTex && _fog.snapFrame == GetFrameCounter() &&
                             _fog.snapW == f.w && _fog.snapH == f.h;
    const GLuint sceneIn = overlaySnap && cameraFrame ? _fog.snapTex : f.frameColor;
    Camera* camera = GScene ? GScene->GetCamera() : nullptr;
    LightSun* sun = GScene ? GScene->MainLight() : nullptr;
    if (!camera || !sun || _fog.mediumFrame != GetFrameCounter() || !_fog.on)
        return;
    const int vw = std::max(1, static_cast<int>(f.rect[2] * static_cast<float>(f.w) + 0.5f));
    const int vh = std::max(1, static_cast<int>(f.rect[3] * static_cast<float>(f.h) + 0.5f));
    // At night under the camera (look director round 2: the night town views
    // cost the most) the volume and the near field run at the coarse
    // quality: the night's air is dark and its halos come from the near
    // field's closed-form lamp light, which the coarser march keeps.
    const float quality =
        cameraFrame && _fog.medium.night > 0.5f ? std::min(_lighting.fogQuality, 0.0f) : _lighting.fogQuality;
    const rm::FogGrid grid = rm::FogGridFor(vw, vh, quality);
    if (!EnsureFogResources(f.w, f.h, grid))
        return;
    UpdateFogFloor();
    const PostTargets& p = _post;
    const rm::FogMedium& m = _fog.medium;
    const std::uint32_t frame = GetFrameCounter();
    const rm::FogSightMap* sight = _fog.sightMap.get();

    // The slices and the camera.
    const float zFar = m.zFar;
    const float k = rm::FogDepthK(zFar, grid.slices);
    const float invEk = k < 1e-3f ? 0.0f : 1.0f / (std::exp(k) - 1.0f);
    const GfxMatrix& v = _frameState.view;
    float viewToWorld[9];
    ViewToWorld(v, viewToWorld);
    const float rot[9] = {v._11, v._21, v._31, v._12, v._22, v._32, v._13, v._23, v._33}; // world to view, rows
    const float* cam = _frameState.cameraPos;
    const float p11 = _frameState.projection._11, p22 = _frameState.projection._22;
    const float dx = cam[0] - _fog.prevPos[0], dy = cam[1] - _fog.prevPos[1], dz = cam[2] - _fog.prevPos[2];
    const bool history = _fog.historyValid && _fog.appliedFrame + 1 == frame &&
                         dx * dx + dy * dy + dz * dz < kFogHistoryCut * kFogHistoryCut;
    // The previous view of a point given camera-relative now:
    // R_prev (rel + cam - cam_prev).
    float prevRow[3][4];
    for (int r = 0; r < 3; ++r)
    {
        const float* row = _fog.prevRot + r * 3;
        prevRow[r][0] = row[0];
        prevRow[r][1] = row[1];
        prevRow[r][2] = row[2];
        prevRow[r][3] = row[0] * dx + row[1] * dy + row[2] * dz;
    }
    // The frame's count steps the near field's march (fogNearGrid.w). The
    // cells take no jitter: psFogInject samples two fixed points of each
    // (VolumetricFog.hpp kFogTapZ, kFogTapXY), so the history has no residual
    // of a moving sample to pump (one offset for the frame) or to slide
    // through the fog (an offset per cell plus the frame's step).
    const std::uint32_t seq = _fog.sequence++;

    // The night air's light: the night sky's share of the classic fog colour.
    const float nightW = Smooth(0.1f, 0.8f, m.night);
    const float nightAir[3] = {ToLinear(_fogColor.R()) * kFogNightAirShare, ToLinear(_fogColor.G()) * kFogNightAirShare,
                               ToLinear(_fogColor.B()) * kFogNightAirShare};

    // The mist banks drift with the ground wind (and slowly in a calm).
    float drift[2] = {_fog.drift[0], _fog.drift[1]};
    {
        const float now = Glob.time.toFloat();
        if (_fog.driftTime >= 0.0f)
        {
            const float dt = std::clamp(now - _fog.driftTime, 0.0f, 0.5f);
            Landscape* land = GScene->GetLandscape();
            const Vector3 wind = land ? land->GetWind() : VZero;
            float wx = wind.X(), wz = wind.Z();
            if (wx * wx + wz * wz < kFogMistDrift * kFogMistDrift)
            {
                wx += kFogMistDrift * 0.8f;
                wz += kFogMistDrift * 0.6f;
            }
            drift[0] -= wx * dt;
            drift[1] -= wz * dt;
        }
        _fog.driftTime = now;
        _fog.drift[0] = drift[0];
        _fog.drift[1] = drift[1];
    }

    // Shared inputs of the injection and the near field.
    const bool shadowMap = ShadowMapsEnabled() && _shadowMapActive && _shadowMapTex && _shadowCascades > 0;
    const bool terrain = _terrainHorizonTex && _terrainHeightTex && _terrainMap && _terrainMap->Valid();
    const bool floorMap = _fog.floorTex && _fog.floorCell > 0.0f && _fog.floorSize > 0;
    auto setShared = [&](GLuint prog)
    {
        SetInt(prog, "shadowMap", kShadowMapUnit);
        SetInt(prog, "terrainHorizon", kTerrainHorizonUnit);
        SetInt(prog, "terrainHeight", kTerrainHeightUnit);
        SetInt(prog, "cloudNoise", kCloudNoiseUnit);
        SetInt(prog, "skyLut", kSkyLutUnit);
        SetInt(prog, "skyDensity", kSkyDensityUnit);
        SetInt(prog, "fogFloor", kFogFloorUnit);
        SetInt(prog, "canopySun", kCanopyVisUnit);
        SetInt(prog, "fogRing", kFogRingUnit);
        if (shadowMap)
        {
            const GLint loc = glGetUniformLocation(prog, "cascadeVP");
            if (loc >= 0)
                glUniformMatrix4fv(loc, std::min(_shadowCascades, kShadowCascades), GL_FALSE, _shadowMapVP);
            Set4(prog, "cascadeSplits", _shadowSplits[0], _shadowSplits[1], _shadowSplits[2], _shadowSplits[3]);
            Set4(prog, "cascadeCtl", static_cast<float>(_shadowCascades), _shadowTuning.fadeRange,
                 _shadowTuning.biasBase, static_cast<float>(_shadowOmniCount));
            Set4(prog, "camFwd", _shadowCamFwd[0], _shadowCamFwd[1], _shadowCamFwd[2], 0.0f);
        }
        Set4(prog, "shadowCtl", shadowMap ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f);
        Set4(prog, "shadowDelta", cam[0] - _shadowCamPos[0], cam[1] - _shadowCamPos[1], cam[2] - _shadowCamPos[2],
             0.0f);
        Set4(prog, "fogTerrain", floorMap ? 1.0f : 0.0f, floorMap ? 1.0f / _fog.floorCell : 0.0f,
             floorMap ? 1.0f / static_cast<float>(_fog.floorSize) : 0.0f, 0.0f);
        Set4(prog, "projInfo", f.projInfo[0], f.projInfo[1], f.projInfo[2], f.projInfo[3]);
        SetMat3(prog, "viewToWorld", viewToWorld);
        Set4(prog, "fogHaze", m.hazeSigma, m.hazeHeight, m.rayleigh, m.lobe);
        Set4(prog, "fogWeather", m.weatherSigma, m.weatherHeight, m.skyThrough, m.night);
        Set4(prog, "fogGround", m.groundSigma, m.groundHeight, m.dustSigma, m.dustHeight);
        Set4(prog, "fogMist", m.mistSigma, m.mistLayer, m.mistFall, m.mistNoise);
        Set4(prog, "fogLightK", m.sun, m.sky, m.g, m.lights);
        Set4(prog, "fogWind", drift[0], drift[1], m.hazeNearTo, m.hazeNearKeep);
        Set4(prog, "fogNightAir", nightAir[0], nightAir[1], nightAir[2], nightW);
    };
    auto bindShared = [&]()
    {
        // The sky table first, through the bind cache, then the raw binds,
        // then the cache told: the raw glActiveTexture calls left the cache's
        // active unit stale, and BindSkyTable after them bound the sky table
        // onto the last raw unit, the horizon ring's (look director round 2:
        // the air's light was the table's texel, not the sky above the land,
        // so rain and mist faded far hills to a dark slate).
        GL33Bind::Invalidate();
        BindSkyTable(); // unit 8: the table the world read, or its stand-in
        glActiveTexture(GL_TEXTURE0 + kShadowMapUnit);
        glBindTexture(GL_TEXTURE_2D_ARRAY, shadowMap ? _shadowMapTex : _dummyDepthArray);
        glBindSampler(kShadowMapUnit, _shadowCompareSampler);
        glActiveTexture(GL_TEXTURE0 + kTerrainHorizonUnit);
        glBindTexture(GL_TEXTURE_2D_ARRAY, terrain ? _terrainHorizonTex : _dummyHorizonTex);
        BindTex(kTerrainHeightUnit, terrain ? _terrainHeightTex : _dummyHeightTex);
        BindTex(kCloudNoiseUnit, _cloudNoiseTex);
        BindTex(kFogFloorUnit, _fog.floorTex ? _fog.floorTex : p.blackTex);
        BindTex(kFogRingUnit, _fog.ringTex[_fog.ringCur]);
        GL33Bind::Invalidate();
    };

    // ---- 0. The horizon ring ----------------------------------------------------
    // The sky's colour just above the land in every azimuth in view, from the
    // resolved frame (before the fog); out of view the last frame's, fading.
    // A camera jump starts it again.
    {
        const int next = 1 - _fog.ringCur;
        const GLuint ring = _fog.progRing;
        glUseProgram(ring);
        SetInt(ring, "colorTex", 0);
        SetInt(ring, "ringPrev", 3);
        SetInt(ring, "depthTex", 5);
        BindTex(0, sceneIn);
        BindTex(3, _fog.ringTex[_fog.ringCur]);
        BindTex(5, p.depthTex);
        Set4(ring, "ringView0", rot[0], rot[1], rot[2], 0.0f);
        Set4(ring, "ringView1", rot[3], rot[4], rot[5], 0.0f);
        Set4(ring, "ringView2", rot[6], rot[7], rot[8], 0.0f);
        Set4(ring, "ringProj", p11, p22, history ? 0.0f : 1.0f, kFogRingShare);
        Set4(ring, "worldRect", f.rect[0], f.rect[1], f.rect[2], f.rect[3]);
        // z: how grey the sky is (no sun glare to avoid, and rain streaks over
        // the horizon are dark: the ring takes the mean of its samples there
        // instead of the darker half, so far air in rain matches the sky; and
        // it samples the deck above the classic horizon fill, not the fill).
        Set4(ring, "ringCtl", static_cast<float>(kFogRingBins), kFogRingDecay, rm::FogGreySky(m.skyThrough), 0.0f);
        const Vector3 toSun = -sun->SunDirection();
        Set4(ring, "ringSun", toSun.X(), toSun.Y(), toSun.Z(), 0.0f);
        Set4(ring, "fogCam", cameraFrame ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f);
        DrawTo(_fog.ringFbo[next], kFogRingBins, kFogRingRows);
        _fog.ringCur = next;
    }

    // ---- 1. Injection --------------------------------------------------------
    const int cur = _fog.current;
    const int prev = 1 - cur;
    const GLuint inject = _fog.progInject;
    glUseProgram(inject);
    SetInt(inject, "fogHistory", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, _fog.injectTex[prev]);
    bindShared();
    setShared(inject);
    Set4(inject, "fogDepth", zFar, k, invEk, 0.0f);
    Set4(inject, "fogFrame", 0.0f, 0.0f, history ? kFogHistoryShare : 1.0f, Glob.time.toFloat());
    Set4(inject, "prevRow0", prevRow[0][0], prevRow[0][1], prevRow[0][2], prevRow[0][3]);
    Set4(inject, "prevRow1", prevRow[1][0], prevRow[1][1], prevRow[1][2], prevRow[1][3]);
    Set4(inject, "prevRow2", prevRow[2][0], prevRow[2][1], prevRow[2][2], prevRow[2][3]);
    Set4(inject, "prevProj", _fog.prevP11, _fog.prevP22, _fog.prevFar, _fog.prevK);
    Set4(inject, "fogColor", _fogColor.R(), _fogColor.G(), _fogColor.B(), 1.0f);
    for (int g = 0; g * rm::kFogSlicesPerDraw < grid.slices; ++g)
    {
        Set4(inject, "fogGrid", static_cast<float>(grid.cellsX), static_cast<float>(grid.cellsY),
             static_cast<float>(grid.slices), static_cast<float>(g * rm::kFogSlicesPerDraw));
        DrawTo(_fog.injectFbo[cur][g], grid.cellsX, grid.cellsY);
    }

    // ---- 2. Integration -------------------------------------------------------
    const GLuint integrate = _fog.progIntegrate;
    glUseProgram(integrate);
    SetInt(integrate, "fogInject", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, _fog.injectTex[cur]);
    Set4(integrate, "fogDepth", zFar, k, invEk, 0.0f);
    Set4(integrate, "projInfo", f.projInfo[0], f.projInfo[1], f.projInfo[2], f.projInfo[3]);
    for (int g = 0; g * rm::kFogSlicesPerDraw < grid.slices; ++g)
    {
        Set4(integrate, "fogGrid", static_cast<float>(grid.cellsX), static_cast<float>(grid.cellsY),
             static_cast<float>(grid.slices), static_cast<float>(g * rm::kFogSlicesPerDraw));
        DrawTo(_fog.integFbo[g], grid.cellsX, grid.cellsY);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, 0);

    // ---- 3. Near field --------------------------------------------------------
    // The sun's beams: the sun up and within reach of the view, wet air near
    // the ground (mist, fog, humid air) to scatter it.
    const rm::GateInputs& gi = _featureGates.inputs;
    const float sunUp = Smooth(-1.0f, 2.0f, gi.sunElevationDeg) * (1.0f - sun->NightEffect());
    const bool beams = m.sun > 0.0f && sunUp > 0.0f && gi.sunViewAngleDeg < kFogBeamViewAngle &&
                       m.groundSigma + m.weatherSigma + m.mistSigma > 1e-5f && _sunEnabled;
    // Lamps, headlights and fires at night: the nearest kFogMaxLights within
    // kFogLightRange. The scene's active list is cleared by now: walk its
    // lights.
    AirLight air[kFogMaxLights] = {};
    int lights = 0;
    // Under the photoreal camera its own night (CameraNight, 0 until the sun
    // is 4.5 degrees under): the classic night effect is 1 at any sun under
    // the horizon, and a dusk firefight's muzzle flashes drew large yellow
    // halos in the still bright air (look director round 2).
    const float classicNight = sun->NightEffect();
    const float night =
        cameraFrame ? std::min(classicNight, std::clamp(_camera.nightEffect, 0.0f, 1.0f)) : classicNight;
    if (night > rm::kGateNightMinEffect && m.lights > 0.0f)
    {
        struct Near
        {
            Light* light;
            float d2;
        };
        Near nearest[kFogMaxLights] = {};
        for (int i = 0; i < GScene->NLights(); ++i)
        {
            Light* light = GScene->GetLight(i);
            if (!light || !light->IsOn())
                continue;
            LightDescription desc;
            light->GetDescription(desc);
            if (desc.type != LTPoint && desc.type != LTSpotLight)
                continue;
            const float lx = desc.pos.X() - cam[0], ly = desc.pos.Y() - cam[1], lz = desc.pos.Z() - cam[2];
            const float d2 = lx * lx + ly * ly + lz * lz;
            if (d2 > kFogLightRange * kFogLightRange)
                continue;
            int at = lights < kFogMaxLights ? lights++ : kFogMaxLights;
            if (at == kFogMaxLights)
            {
                if (d2 >= nearest[kFogMaxLights - 1].d2)
                    continue;
                at = kFogMaxLights - 1;
            }
            while (at > 0 && nearest[at - 1].d2 > d2)
            {
                nearest[at] = nearest[at - 1];
                --at;
            }
            nearest[at] = Near{light, d2};
        }
        const float lampPerStart = fl::kLampIntensityPerStart * std::max(_lighting.nightLamp, 0.0f);
        const float spotPeak = fl::kLowBeamPeak * std::max(_lighting.nightSpot, 0.0f);
        for (int i = 0; i < lights; ++i)
        {
            LightDescription desc;
            Light* light = nearest[i].light;
            light->GetDescription(desc);
            fl::FrameLight frameLight;
            const bool spot = desc.type == LTSpotLight;
            frameLight.kind = spot ? fl::FrameLightKind::Spot : light->Kind();
            frameLight.diffuse[0] = desc.diffuse.R();
            frameLight.diffuse[1] = desc.diffuse.G();
            frameLight.diffuse[2] = desc.diffuse.B();
            AirLight& a = air[i];
            a.pos[0] = desc.pos.X() - cam[0];
            a.pos[1] = desc.pos.Y() - cam[1];
            a.pos[2] = desc.pos.Z() - cam[2];
            Vector3 beam = desc.dir;
            beam.Normalize();
            a.dir[0] = beam.X();
            a.dir[1] = beam.Y();
            a.dir[2] = beam.Z();
            // A headlight is aimed from its vehicle's body, as the shading aims it.
            if (spot)
                if (auto* attached = dynamic_cast<AttachedOnVehicle*>(light))
                    if (const Object* owner = attached->AttachedOn())
                    {
                        const Vector3 body = owner->Direction();
                        const float lampDir[3] = {a.dir[0], a.dir[1], a.dir[2]};
                        const float bodyForward[3] = {body.X(), body.Y(), body.Z()};
                        fl::LowBeamAim(lampDir, bodyForward, a.dir);
                    }
            float intensity = 1.0f;
            float colour[3] = {ToLinear(desc.diffuse.R()), ToLinear(desc.diffuse.G()), ToLinear(desc.diffuse.B())};
            switch (frameLight.kind)
            {
                case fl::FrameLightKind::Lamp:
                    intensity = lampPerStart * desc.startAtten;
                    a.reach = 40.0f;
                    fl::PhysicalColour(frameLight, colour);
                    break;
                case fl::FrameLightKind::Spot:
                    intensity = spotPeak;
                    a.reach = 60.0f;
                    fl::PhysicalColour(frameLight, colour);
                    break;
                case fl::FrameLightKind::Fire:
                    intensity = 15.0f;
                    a.reach = 25.0f;
                    break;
                case fl::FrameLightKind::Combat:
                    intensity = 30.0f;
                    a.reach = 40.0f;
                    break;
                default:
                    intensity = 8.0f;
                    a.reach = 20.0f;
                    break;
            }
            a.beam = spot;
            // A street lamp's light in the fog reads near white (its head is
            // white in the frame): its colour, mostly desaturated. A fire
            // keeps its own.
            if (frameLight.kind == fl::FrameLightKind::Lamp)
            {
                const float lum = 0.2126f * colour[0] + 0.7152f * colour[1] + 0.0722f * colour[2];
                for (int c = 0; c < 3; ++c)
                    colour[c] = lum + (colour[c] - lum) * kFogLampAirChroma;
            }
            const float s = intensity * night * m.lights;
            for (int c = 0; c < 3; ++c)
                a.col[c] = colour[c] * s;
            // The air's scattering at the light (the medium without the mist's
            // noise or the eye's near thinning).
            const float wx = desc.pos.X(), wy = desc.pos.Y(), wz = desc.pos.Z();
            const float ground = SampleMap(sight, &rm::FogSightMap::ground, wx, wz);
            const float floorY = SampleMap(sight, &rm::FogSightMap::floorY, wx, wz);
            const float layer = SampleMap(sight, &rm::FogSightMap::layer, wx, wz);
            a.sigma = rm::FogExtinctionAt(m, wy, floorY, wy - ground, layer);
        }
    }
    const bool nearOn = beams || lights > 0;
    if (nearOn)
    {
        const GLuint nearProg = _fog.progNear;
        glUseProgram(nearProg);
        SetInt(nearProg, "linDepth", 0);
        BindTex(0, p.linDepthTex);
        bindShared();
        setShared(nearProg);
        Set4(nearProg, "worldRect", f.rect[0], f.rect[1], f.rect[2], f.rect[3]);
        Set4(nearProg, "fogNearGrid", static_cast<float>(_fog.nearW), static_cast<float>(_fog.nearH),
             static_cast<float>(rm::FogNearSteps(quality)), static_cast<float>(seq % 64u));
        // The sun's radiance in the air is the shading's (the shader reads the
        // rig) times the beams' gain and fogSun.
        const float beamGain = rm::kFogBeamGain * std::min(m.sun, 4.0f);
        Set4(nearProg, "fogNearSun", beamGain, beamGain, beamGain, beams ? 1.0f : 0.0f);
        Set4(nearProg, "fogNearCtl", rm::kFogBeamReach, static_cast<float>(lights), zFar, 0.0f);
        if (lights > 0)
        {
            float pos[kFogMaxLights * 4] = {}, col[kFogMaxLights * 4] = {}, dir[kFogMaxLights * 4] = {};
            for (int i = 0; i < lights; ++i)
            {
                const AirLight& a = air[i];
                for (int c = 0; c < 3; ++c)
                {
                    pos[i * 4 + c] = a.pos[c];
                    col[i * 4 + c] = a.col[c];
                    dir[i * 4 + c] = a.dir[c];
                }
                pos[i * 4 + 3] = a.reach;
                col[i * 4 + 3] = a.beam ? 1.0f : 0.0f;
                dir[i * 4 + 3] = a.sigma;
            }
            const GLint lp = glGetUniformLocation(nearProg, "fogLightPos");
            const GLint lc = glGetUniformLocation(nearProg, "fogLightCol");
            const GLint ld = glGetUniformLocation(nearProg, "fogLightDir");
            if (lp >= 0)
                glUniform4fv(lp, lights, pos);
            if (lc >= 0)
                glUniform4fv(lc, lights, col);
            if (ld >= 0)
                glUniform4fv(ld, lights, dir);
        }
        DrawTo(_fog.nearFbo, _fog.nearW, _fog.nearH);
    }
    // The post pass's unit 2: no sampler object (BeginPostFullscreen).
    glBindSampler(kShadowMapUnit, 0);

    // ---- 3b. The night glow --------------------------------------------------
    // At night in wet air: the frame's light blurred at a quarter and a
    // sixteenth of the resolution, combined into glowTex[1].
    const float glowGain = kFogGlowGain * nightW * std::min(m.lights, 4.0f);
    const bool glow = glowGain > 1e-3f && m.weatherSigma + m.mistSigma + m.groundSigma > 1e-5f;
    if (glow)
    {
        const GLuint gp = _fog.progGlow;
        glUseProgram(gp);
        Set4(gp, "fogCam", cameraFrame ? 1.0f : 0.0f, 0.0f, 0.0f, 0.0f);
        SetInt(gp, "srcTex", 0);
        SetInt(gp, "src2Tex", 3);
        SetInt(gp, "depthTex", 5);
        Set4(gp, "projInfo", f.projInfo[0], f.projInfo[1], f.projInfo[2], f.projInfo[3]);
        Set4(gp, "depthMap", f.depthScale, f.depthBias, 0.0f, 0.0f);
        const float qw = static_cast<float>(_fog.glowW[0]), qh = static_cast<float>(_fog.glowH[0]);
        const float sw = static_cast<float>(_fog.glowW[1]), sh = static_cast<float>(_fog.glowH[1]);
        auto pass = [&](int mode, GLuint src, float tx, float ty, float dx, float dy, int dst, float share = 0.0f)
        {
            BindTex(0, src);
            Set4(gp, "glowCtl", static_cast<float>(mode), zFar, tx, ty);
            Set4(gp, "glowDir", dx, dy, share, 0.0f);
            const int lvl = dst / 2;
            DrawTo(_fog.glowFbo[dst], _fog.glowW[lvl], _fog.glowH[lvl]);
        };
        BindTex(5, p.depthTex);
        // The frame to a quarter (texel steps of a full-resolution pixel).
        pass(0, sceneIn, 1.0f / static_cast<float>(f.w), 1.0f / static_cast<float>(f.h), 0.0f, 0.0f, 0);
        pass(2, _fog.glowTex[0], 0.0f, 0.0f, 2.0f / qw, 0.0f, 1);
        pass(2, _fog.glowTex[1], 0.0f, 0.0f, 0.0f, 2.0f / qh, 0);
        // A quarter to a sixteenth, blurred wider.
        pass(1, _fog.glowTex[0], 1.0f / qw, 1.0f / qh, 0.0f, 0.0f, 2);
        pass(2, _fog.glowTex[2], 0.0f, 0.0f, 2.0f / sw, 0.0f, 3);
        pass(2, _fog.glowTex[3], 0.0f, 0.0f, 0.0f, 2.0f / sh, 2);
        // Combined at a quarter.
        BindTex(3, _fog.glowTex[2]);
        pass(3, _fog.glowTex[0], 0.0f, 0.0f, 0.0f, 0.0f, 1, kFogGlowWide);
    }

    // ---- 4. Apply ------------------------------------------------------------
    // The sky's inputs: the eye's height, the fog floor under it and the wet
    // air there (the humid ground air and the mist, and how deep they lie).
    const float floorEye = SampleMap(sight, &rm::FogSightMap::floorY, cam[0], cam[2]);
    const float groundEye = SampleMap(sight, &rm::FogSightMap::ground, cam[0], cam[2]);
    const float layerEye = SampleMap(sight, &rm::FogSightMap::layer, cam[0], cam[2]);
    const float wetEye =
        rm::FogSightExtinctionAt(m, cam[1], floorEye, cam[1] - groundEye, layerEye) * rm::kMistNearKeep;
    const float wetDepth = std::max(m.groundHeight, layerEye * m.mistLayer);
    // The stars behind the wet air over the eye (DrawPoints reads it).
    _fog.starKeep = rm::FogStarKeep(m, cam[1], floorEye, cam[1] - groundEye, layerEye);
    const GLuint apply = _fog.progApply;
    glUseProgram(apply);
    SetInt(apply, "colorTex", 0);
    SetInt(apply, "fogVolume", 2);
    SetInt(apply, "nearTex", 3);
    SetInt(apply, "linDepth", 4);
    SetInt(apply, "depthTex", 5);
    SetInt(apply, "glowTex", 1);
    BindTex(0, f.frameColor);
    SetInt(apply, "worldTex", 6);
    BindTex(6, overlaySnap ? _fog.snapTex : p.blackTex);
    BindTex(1, glow ? _fog.glowTex[1] : p.blackTex);
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_3D, _fog.integTex);
    BindTex(3, nearOn ? _fog.nearTex : p.blackTex);
    BindTex(4, p.linDepthTex);
    BindTex(5, p.depthTex);
    Set4(apply, "projInfo", f.projInfo[0], f.projInfo[1], f.projInfo[2], f.projInfo[3]);
    Set4(apply, "depthMap", f.depthScale, f.depthBias, 0.0f, 0.0f);
    Set4(apply, "worldRect", f.rect[0], f.rect[1], f.rect[2], f.rect[3]);
    Set4(apply, "fogGrid", static_cast<float>(grid.cellsX), static_cast<float>(grid.cellsY),
         static_cast<float>(grid.slices), 0.0f);
    Set4(apply, "fogDepth", zFar, k, invEk, 0.0f);
    // x: no dither into the half-float target.
    Set4(apply, "fogApply", 0.0f, _lighting.debugView == rm::kDebugViewAir ? 1.0f : 0.0f, kFogSkyFade,
         nearOn ? 1.0f : 0.0f);
    // z: a gunner's optic overlay was drawn over the world this frame (its
    // black mask and reticle stay as drawn).
    Set4(apply, "fogNearSize", static_cast<float>(_fog.nearW), static_cast<float>(_fog.nearH),
         _worldOverlayDrawn ? 1.0f : 0.0f, overlaySnap && cameraFrame ? 1.0f : 0.0f);
    Set4(apply, "fogSkyEye", cam[1], floorEye, wetEye, wetDepth);
    // z: how grey the sky is (the air replaces the classic horizon fill).
    Set4(apply, "fogSkyWeather", m.weatherSigma, m.weatherHeight, rm::FogGreySky(m.skyThrough), 0.0f);
    Set4(apply, "fogSkyClear", m.hazeSigma, m.hazeHeight, m.rayleigh * 13.558e-6f, m.dustSigma * rm::kDustSkyShare);
    Set4(apply, "fogSkyDust", m.dustHeight, cam[0], cam[2], groundEye);
    Set4(apply, "fogGlow", glow ? glowGain : 0.0f, 0.0f, 0.0f, 0.0f);
    // The sun's forward glow in the clear air past the far end (the sky's
    // horizon band toward a low sun), as the volume's lobe.
    const float lobeSky = m.lobe * rm::kFogLobeSkyShare * (0.15f + 0.85f * std::clamp(m.skyThrough, 0.0f, 1.0f)) *
                          (1.0f - nightW) * sunUp;
    Set4(apply, "fogSkyLobe", lobeSky, 0.0f, 0.0f, 0.0f);
    SetMat3(apply, "viewToWorld", viewToWorld);
    DrawTo(_fog.outFbo, f.w, f.h);

    // Hand the units back: no 3D volume or floor left bound, nothing on unit 1.
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE0 + kFogFloorUnit);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    GL33Bind::Invalidate();

    // The next frame's history: this frame's volume and camera.
    _fog.appliedFrame = frame;
    _fog.historyValid = true;
    _fog.current = prev;
    for (int i = 0; i < 9; ++i)
        _fog.prevRot[i] = rot[i];
    for (int i = 0; i < 3; ++i)
        _fog.prevPos[i] = cam[i];
    _fog.prevP11 = p11;
    _fog.prevP22 = p22;
    _fog.prevFar = zFar;
    _fog.prevK = k;
}
