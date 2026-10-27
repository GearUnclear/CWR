// Remastered lighting: the volumetric fog in every view the player can take.
// AI sight is cut by the whole fog medium whatever the player looks through
// (VolumetricFog.hpp), so the fog is drawn in each view, never only in the
// open third-person and on-foot ones:
//   - a vehicle interior (Transport::Draw clears the world's depth and draws
//     the cockpit with its own near plane): the world's depth and colour are
//     saved at that clear (SaveWorldBeforeInterior), the fog's stages read
//     the saved depth (PostFogDepthTex) and the saved colour, and the apply
//     leaves the pixels the interior drew as drawn (EngineGL33_PostFog.cpp).
//     The fog keeps running across the switch into and out of the cockpit,
//     so the forward fog never loses its edge for a frame either;
//   - a gunner's optic or a rifle's sight overlay: the overlay snapshot
//     (SnapshotWorldBeforeOverlay), as before;
//   - night vision: the world is drawn by the classic programs in the
//     intensifier's colours, with no world post pass. Their forward fog keeps
//     only its edge (its range rescaled to the remastered programs' edge,
//     PostStages.hpp ScaleForwardFogRange) and, after the world, the volume
//     of the whole medium fades the land toward the forward fog's colour
//     (PostFogNightVision). The volume's light is not used there: the
//     intensifier's fog colour stands for it, as in the classic fog.

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/EngineGL33_PostPass.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>
#include <PoseidonGL33/TextureGL33.hpp>
#include <Poseidon/Graphics/Core/GLBlendState.hpp>
#include <Poseidon/Graphics/Core/GLDepthStencilState.hpp>
#include <Poseidon/Graphics/Core/GLPipelineState.hpp>
#include <Poseidon/Core/Global.hpp>
#include <Poseidon/Foundation/Framework/Log.hpp>
#include <Poseidon/Graphics/Remaster/FeatureGates.hpp>
#include <Poseidon/Graphics/Remaster/PostStages.hpp>
#include <Poseidon/Graphics/Remaster/VolumetricFog.hpp>
#include <Poseidon/World/Scene/Scene.hpp>
#include <algorithm>
#include <iterator>
#include <Poseidon/World/Terrain/TerrainLighting.hpp>

namespace rm = Poseidon::remaster;

namespace
{

GLuint MakeSaveTex(GLenum internalFormat, GLenum format, GLenum type, int w, int h, GLenum filter)
{
    GLuint t = 0;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, type, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(filter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(filter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    return t;
}

bool MakeSaveFbo(GLenum attachment, GLuint tex, unsigned int& fbo)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, tex, 0);
    if (attachment != GL_COLOR_ATTACHMENT0)
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
    return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

} // namespace

bool EngineGL33::FogDrawsThisFrame() const
{
    return (_fog.on || _fog.nvg) && _fog.mediumFrame == GetFrameCounter();
}

bool EngineGL33::NightVisionFogOpen() const
{
    return _fog.nvg && _nightVision && _fog.mediumFrame == GetFrameCounter() && !_worldSceneEnded;
}

bool EngineGL33::FogInteriorOpen() const
{
    return _fog.intFrame == GetFrameCounter() && (_worldPassOpen || NightVisionFogOpen());
}

rm::SsrStencil EngineGL33::InteriorGlassStencil(TextureGL33* tex, const Poseidon::render::RenderPassDescriptor& d)
{
    namespace pr = Poseidon::render;
    rm::SsrStencil st;
    // The projected shadows count in the low bits, under the marks.
    if (d.depth == pr::DepthMode::Shadow)
    {
        st.shadowCompareMask = rm::ssrtag::kShadowBits;
        return st;
    }
    const bool blended = d.blend == pr::BlendMode::AlphaBlend || d.blend == pr::BlendMode::Additive;
    bool glass = false;
    if (tex && blended)
    {
        EnsureSurfaceTable();
        const rm::SurfaceClass cls = tex->SurfaceClassFor(_surfaceTable, _surfaceTableGen, d.objectKind,
                                                          d.surface == pr::SurfaceMode::OnSurface);
        glass = rm::InteriorGlassDraw(blended, cls == rm::SurfaceClass::Glass || cls == rm::SurfaceClass::Lens);
    }
    // Glass sets its mark and leaves the solid one; anything else sets the
    // solid mark and clears the glass one.
    st.ref = static_cast<std::uint8_t>(glass ? rm::kInteriorGlassStencil : rm::kInteriorSolidStencil);
    st.writeMask = static_cast<std::uint8_t>(glass ? rm::kInteriorGlassStencil
                                                   : rm::kInteriorGlassStencil | rm::kInteriorSolidStencil);
    return st;
}

void EngineGL33::FarInteriorGlass(const PostFrame& f)
{
    if (!_hasStencilBuffer || _fog.intFrame != GetFrameCounter() || !_post.depthFbo)
        return;
    // A fullscreen triangle at the far depth (the depth range 1..1), written
    // where the stencil holds the glass mark, into the resolved depth only.
    glBindFramebuffer(GL_FRAMEBUFFER, _post.depthFbo);
    glViewport(0, 0, f.w, f.h);
    glUseProgram(_post.progLinearDepth); // its colour is not written
    glBindVertexArray(_post.vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _post.blackTex);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_CULL_FACE);
    Poseidon::render::blend::Opaque();
    Poseidon::render::pipeline::SetColorMask(false);
    Poseidon::render::depthstencil::AlwaysWrite();
    glDepthRange(1.0, 1.0);
    // Glass and no solid part of the interior under it: the world shows there.
    Poseidon::render::depthstencil::StencilTestEqual(static_cast<GLint>(rm::kInteriorGlassStencil),
                                                     rm::kInteriorGlassStencil | rm::kInteriorSolidStencil);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDepthRange(0.0, 1.0);
    Poseidon::render::depthstencil::StencilOffFullMask();
    Poseidon::render::depthstencil::Disabled(/*hasStencil*/ false);
    Poseidon::render::pipeline::SetColorMask(true);
    GL33Bind::Invalidate();
    InvalidatePipelineCache();
}

unsigned int EngineGL33::PostFogDepthTex() const
{
    const bool saved = _fog.intFrame == GetFrameCounter() && _fog.intDepthTex != 0;
    return saved ? _fog.intDepthTex : _post.depthTex;
}

void EngineGL33::DestroyInteriorSave()
{
    for (unsigned int* fbo : {&_fog.intColorFbo, &_fog.intDepthFbo})
    {
        if (*fbo)
            glDeleteFramebuffers(1, fbo);
        *fbo = 0;
    }
    for (unsigned int* tex : {&_fog.intColorTex, &_fog.intDepthTex})
    {
        if (*tex)
        {
            GL33Bind::OnTexDeleted(*tex);
            glDeleteTextures(1, tex);
        }
        *tex = 0;
    }
    _fog.intW = _fog.intH = 0;
    _fog.intFrame = 0xffffffffu;
}

void EngineGL33::SaveWorldBeforeInterior()
{
    // The first depth clear of the frame holds the whole world: a later one
    // (none is known) would only see the interior.
    const std::uint32_t frame = GetFrameCounter();
    if (_fog.intFrame == frame)
        return;
    if (!_glContext || !FogDrawsThisFrame() || !SSAAActive() || !_ssaaFbo)
        return;
    int w = 0, h = 0;
    RenderTargetSize(w, h);
    if (w <= 0 || h <= 0)
        return;
    // The world's queued draws first: the save is the world as drawn.
    FlushAndFreeAllQueues(_queueNo);
    if (_fog.intW != w || _fog.intH != h || !_fog.intColorTex || !_fog.intDepthTex)
    {
        DestroyInteriorSave();
        GL33Bind::ActiveUnit(kUploadUnit - GL_TEXTURE0);
        _fog.intColorTex = MakeSaveTex(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, w, h, GL_LINEAR);
        // As the post pass's depth (EnsurePostResources): a depth blit needs
        // the frame target's format.
        _fog.intDepthTex = MakeSaveTex(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, w, h, GL_NEAREST);
        GL33Bind::Invalidate();
        const bool complete = MakeSaveFbo(GL_COLOR_ATTACHMENT0, _fog.intColorTex, _fog.intColorFbo) &&
                              MakeSaveFbo(GL_DEPTH_STENCIL_ATTACHMENT, _fog.intDepthTex, _fog.intDepthFbo);
        if (!complete)
        {
            LOG_ERROR(Graphics,
                      "GL33 fog: the interior save {}x{} is incomplete; vehicle interiors keep the classic "
                      "distance fog",
                      w, h);
            DestroyInteriorSave();
            _fog.intFailed = true;
            glBindFramebuffer(GL_FRAMEBUFFER, _ssaaFbo);
            glReadBuffer(GL_COLOR_ATTACHMENT0);
            return;
        }
        _fog.intW = w;
        _fog.intH = h;
    }
    const GLboolean scissor = glIsEnabled(GL_SCISSOR_TEST);
    if (scissor)
        glDisable(GL_SCISSOR_TEST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _ssaaFbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fog.intColorFbo);
    glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fog.intDepthFbo);
    glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    if (scissor)
        glEnable(GL_SCISSOR_TEST);
    // Back to the frame target for the clear and the interior (its draw
    // buffers are its own).
    glBindFramebuffer(GL_FRAMEBUFFER, _ssaaFbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    // The world's projection: the interior has set its own near plane
    // (Transport::Draw, UpdateProjection) before it clears the depth.
    const GfxMatrix& proj = _fog.worldProjFrame == frame ? _fog.worldProj : _frameState.projection;
    _fog.intProjInfo[0] = proj._33;
    _fog.intProjInfo[1] = proj._43;
    _fog.intProjInfo[2] = 1.0f / proj._11;
    _fog.intProjInfo[3] = 1.0f / proj._22;
    _fog.intFrame = frame;
    _fog.intFailed = false;
}

EngineGL33::PostFrame EngineGL33::PostFogFrame(const PostFrame& f) const
{
    PostFrame g = f;
    if (PostFogDepthTex() != _post.depthTex)
        std::copy(std::begin(_fog.intProjInfo), std::end(_fog.intProjInfo), std::begin(g.projInfo));
    return g;
}

void EngineGL33::UpdateFogMediumNightVision()
{
    // Night vision's world passes only (not a portrait, nor a 3D preview drawn
    // after the world).
    if (!_nightVision || _worldSceneEnded || !GScene)
        return;
    const std::uint32_t frame = GetFrameCounter();
    if (_fog.mediumFrame != frame)
    {
        // Once per frame, with the gates (the world's first pass computed them).
        if (!_featureGates.computed || _featureGates.frame != frame)
            return;
        _fog.mediumFrame = frame;
        const bool on = _lightingActive && _lighting.postEffects && SSAAActive() && !_post.unavailable &&
                        !_fog.unavailable && !(_prevWorldDepthCleared && _fog.intFailed) &&
                        _featureGates[rm::Feature::VolFog].on;
        // The same inputs and function as the world's fog (UpdateFogMedium).
        const rm::FogInputs in = Poseidon::WorldFogInputs(GScene->GetLandscape());
        _fog.medium = rm::ComputeFogMedium(in, _lighting, on);
        _fog.on = false; // no world post pass: the classic programs draw
        _fog.nvg = on;
    }
    // (The classic branch of the lighting upload clears it at every pass.)
    rm::SetDrawnFogMedium(_fog.nvg ? &_fog.medium : nullptr);
    if (!_fog.nvg)
        return;
    // The forward fog keeps only its edge, as in the remastered programs: its
    // range in the classic programs' terms. Each pass builds the frame state
    // again; a pass that already has the rescaled range keeps it.
    float* fog = _frameState.fogParams;
    if (fog[0] == _fog.nvgFog[0] && fog[1] == _fog.nvgFog[1])
        return;
    const rm::ForwardFogRange r = rm::ScaleForwardFogRange(fog[0], fog[1], _fog.medium.fogEdgeScale);
    fog[0] = r.start;
    fog[1] = r.invRange;
    _fog.nvgFog[0] = r.start;
    _fog.nvgFog[1] = r.invRange;
    UploadFrameConstants(_frameState);
}

void EngineGL33::PostFogNightVision()
{
    const std::uint32_t frame = GetFrameCounter();
    if (!_fog.nvg || _fog.mediumFrame != frame || !_glContext || !_lightingActive || !SSAAActive())
        return;
    // The intensified world's queued draws first.
    FlushAndFreeAllQueues(_queueNo);
    PostFrame f;
    PostFrameGeometry(f);
    f = PostFogFrame(f);
    RenderTargetSize(f.w, f.h);
    if (f.w <= 0 || f.h <= 0 || !EnsurePostResources(f.w, f.h))
        return;
    f.hw = (f.w + 1) / 2;
    f.hh = (f.h + 1) / 2;
    f.frameColor = _post.colorTex;
    f.plan.fog = true;
    // The frame and its depth resolved, the fog laid over it straight into
    // the frame target (EngineGL33_PostFog.cpp, its night-vision apply).
    PostResolve(f);
    BeginPostFullscreen();
    PostFog(f);
    EndPostFullscreen();
}
