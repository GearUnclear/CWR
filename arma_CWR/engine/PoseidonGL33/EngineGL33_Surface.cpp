// Surfaces and ambient light (feature switch `surfaces`, the look lane
// "surface"; Graphics/Remaster/SurfaceLight.hpp).
//
// Three parts here:
//   - the surface target: a second multisampled RGBA8 colour attachment of the
//     frame target that the remastered lit programs write during the world
//     pass (fragSurf: the share of each pixel's light the horizon occlusion may
//     darken, the albedo, the sun's share). Draw buffers 0 and 1 are on from
//     the world's first pass to EndWorldScene; attachment 1's colour mask
//     follows the bound program (only the remastered lit programs declare the
//     output; any other program would leave undefined values) and the blend
//     mode (opaque and alpha-blended draws write it, additive glows and shadow
//     quads do not). The post pass resolves it next to the colour;
//   - the frame constants (Surface, SurfaceCanopy, Surface2 and the sky
//     light's coefficients SurfaceSh);
//   - the per-draw surface class (the lit programs' uniform surfClass): the
//     materials step's class of the section's texture for the drawing
//     object's kind (SurfaceMaterials.hpp, kept on the texture), its object
//     kind, and whether the texture blends.
// Off (the switch, classic lighting, night vision, a portrait or a 3D preview
// after the world), the target is never bound, the slots are zero and the
// world programs and the post pass draw the frame of before.

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/TextureGL33.hpp>
#include <Poseidon/Core/Global.hpp>

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

using namespace Poseidon::gl33lighting;
namespace rm = Poseidon::remaster;

namespace
{
// The per-draw surface class rides in a plain uniform of the lit programs
// (surfClass), not in the LitConstants block: a block update per draw cost
// about 0.6 ms of CPU a frame, and slot 58 is the materials step's. Per
// program (indexed by its GL name): the uniform's location (-2 not looked up
// yet, -1 not read by that program) and the value it holds.
struct SurfClassProgram
{
    int loc = -2;
    float v[3] = {-1.0f, -1.0f, -1.0f};
};
std::vector<SurfClassProgram> s_surfClassPrograms;
} // namespace

bool EngineGL33::SurfaceWanted() const
{
    return _lightingActive && _remasterPrograms && _worldEffectsOn && !_worldSceneEnded && !_portraitTargetSize &&
           _featureGates.computed && _featureGates.frame == GetFrameCounter() &&
           _featureGates[rm::Feature::Surfaces].on && _lighting.postEffects && _lighting.ambientOcclusion > 0.0f &&
           _lighting.surfAo > 0.0f && SSAAActive();
}

void EngineGL33::DestroySurfaceTarget()
{
    if (_surfRb)
        glDeleteRenderbuffers(1, &_surfRb);
    _surfRb = 0;
    _surfRbSamples = 0;
    _surfTargetOn = false;
    _surfMaskOn = true;
    _surfFrameWritten = false;
}

void EngineGL33::BeginSurfaceTarget()
{
    if (_surfTargetOn || !SurfaceWanted())
        return;
    GLint drawFbo = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFbo);
    if (static_cast<unsigned int>(drawFbo) != _ssaaFbo)
        return;
    if (!_surfRb)
    {
        // Made on first use and attached for the target's life: the frame
        // target's draw buffers keep only attachment 0 outside the world pass,
        // so nothing else ever writes it.
        glGenRenderbuffers(1, &_surfRb);
        glBindRenderbuffer(GL_RENDERBUFFER, _surfRb);
        const GLsizei samples = _msaaSamples > 1 ? _msaaSamples : 0;
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA8, _ssaaW, _ssaaH);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, _surfRb);
        if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            LOG_WARN(Graphics, "GL33: surface target {}x{} ({} samples) incomplete; horizon occlusion off", _ssaaW,
                     _ssaaH, static_cast<int>(samples));
            glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, 0);
            glDeleteRenderbuffers(1, &_surfRb);
            _surfRb = 0;
            return;
        }
        _surfRbSamples = static_cast<int>(samples);
        LOG_INFO(Graphics, "GL33: surface target {}x{} ({} samples)", _ssaaW, _ssaaH, static_cast<int>(samples));
    }
    const GLenum bufs[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, bufs);
    // Nothing drawn yet reads as sky: no share to darken.
    const GLboolean scissor = glIsEnabled(GL_SCISSOR_TEST);
    if (scissor)
        glDisable(GL_SCISSOR_TEST);
    glColorMaski(1, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    const float zero[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    glClearBufferfv(GL_COLOR, 1, zero);
    if (scissor)
        glEnable(GL_SCISSOR_TEST);
    _surfTargetOn = true;
    _surfFrameWritten = true;
    UpdateSurfaceMask(true);
}

void EngineGL33::EndSurfaceTarget()
{
    if (!_surfTargetOn)
        return;
    GLint drawFbo = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _ssaaFbo);
    const GLenum buf0 = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &buf0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(drawFbo));
    glColorMaski(1, GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    _surfMaskOn = true;
    _surfTargetOn = false;
}

void EngineGL33::UpdateSurfaceMask(bool force)
{
    if (!_surfTargetOn)
        return;
    const bool want =
        _surfProgram && (_currentBlendMode == BlendMode::Opaque || _currentBlendMode == BlendMode::AlphaBlend);
    if (!force && want == _surfMaskOn)
        return;
    _surfMaskOn = want;
    const GLboolean m = want ? GL_TRUE : GL_FALSE;
    glColorMaski(1, m, m, m, m);
}

void EngineGL33::UploadLitSurface(const rm::LightRig& rig, float skyThrough)
{
    PollSkyLightSH();
    const rm::SurfaceFrameConstants c =
        rm::ComputeSurfaceFrameConstants(_lighting, _featureGates, rig, SkyLightSHNow(), skyThrough);
    SetLitSlot(LitConst::Surface, c.surface[0], c.surface[1], c.surface[2], c.surface[3]);
    float canopy[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    if (c.surface[0] > 0.0f && c.surface[2] > 0.0f)
        SurfaceCanopyConstants(canopy);
    SetLitSlot(LitConst::SurfaceCanopy, canopy[0], canopy[1], canopy[2], canopy[3]);
    SetLitSlot(LitConst::Surface2, c.surface2[0], c.surface2[1], c.surface2[2], c.surface2[3]);
    SetLitSlot(LitConst::Surface3, c.surface3[0], c.surface3[1], c.surface3[2], c.surface3[3]);
    for (int i = 0; i < LitConst::SurfaceShCount; ++i)
        SetLitSlot(LitConst::SurfaceSh + i, c.sh[i][0], c.sh[i][1], c.sh[i][2], c.sh[i][3]);
}

void EngineGL33::SurfaceEnterInterior()
{
    // The interior a vehicle draws after clearing the world's depth sees no
    // sky: the rig's hemisphere (its calibrated fill) instead of the sky
    // table's, no canopy, a quarter of the specular. The next frame's
    // constants put the outdoor values back (surface3.y 0).
    if (!_lightingActive || !_remasterPrograms || !_featureGates[rm::Feature::Surfaces].on)
        return;
    SetLitSlot(LitConst::Surface, 1.0f, 0.25f * std::max(_lighting.surfSpec, 0.0f), 0.0f, 0.0f);
    UploadLitSlots(LitConst::Surface, 1);
    // The crew compartment is drawn pre-lit (the classic vertex light): its
    // fragments hand the occlusion kSurfaceInteriorShare of their light
    // (litSurface3.y), so its corners, the breech and the hatch rims darken.
    SetLitSlot(LitConst::Surface3, std::max(_lighting.surfCavity, 0.0f), 1.0f, 0.0f, 0.0f);
    UploadLitSlots(LitConst::Surface3, 1);
}

void EngineGL33::ResetSurfaceClassPrograms()
{
    s_surfClassPrograms.clear();
    _surfBoundProgram = 0;
}

void EngineGL33::ApplySurfaceClass(TextureGL33* tex, const Poseidon::render::RenderPassDescriptor& d)
{
    if (!_surfBoundProgram || !_lightingActive || !_remasterPrograms || !_featureGates[rm::Feature::Surfaces].on)
        return;
    // The materials step's classes (SurfaceMaterials.hpp: the texture's name
    // and the drawing object's kind, classified once per texture and kind and
    // kept on it; TextureGL33::SurfaceClassFor) and its object kind.
    const bool onSurface = d.surface == Poseidon::render::SurfaceMode::OnSurface;
    const rm::SurfaceClass cls = tex ? tex->SurfaceClassFor(_surfaceTable, _surfaceTableGen, d.objectKind, onSurface)
                                     : rm::KindDefaultClass(d.objectKind, onSurface);
    const bool blend = tex && tex->GetAlphaClass() == Poseidon::AlphaStats::Blend;
    const float v[3] = {static_cast<float>(cls), static_cast<float>(d.objectKind), blend ? 1.0f : 0.0f};
    if (_surfBoundProgram >= s_surfClassPrograms.size())
        s_surfClassPrograms.resize(_surfBoundProgram + 64);
    SurfClassProgram& sp = s_surfClassPrograms[_surfBoundProgram];
    if (sp.loc == -2)
        sp.loc = glGetUniformLocation(_surfBoundProgram, "surfClass");
    if (sp.loc < 0 || (v[0] == sp.v[0] && v[1] == sp.v[1] && v[2] == sp.v[2]))
        return;
    for (int i = 0; i < 3; ++i)
        sp.v[i] = v[i];
    glUniform4f(sp.loc, v[0], v[1], v[2], 0.0f);
}
