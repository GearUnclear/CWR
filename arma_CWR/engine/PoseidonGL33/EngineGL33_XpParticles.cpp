// Experiment xp-particles (overnight swarm, graphics-experimental): smoke,
// dust and fire puffs (Object::DrawDecal) that meet the world softly and take
// the sun from one side. Two switches, both off by default, read once:
//
//   POSEIDON_XP_SOFT_PARTICLES=1  soft particles. BeginTranslucentObjects
//       resolves the opaque world's depth into the post pass's depth texture
//       (which the post pass's own resolve overwrites later in the frame) and
//       leaves it on unit 17; each depth-tested puff passes its radius in its
//       second texture coordinate, and the object program fades its coverage
//       by how much of the puff's chord at that pixel lies in front of the
//       world (no hard line where a billboard cuts the ground or a wall).
//   POSEIDON_XP_LIT_SMOKE=1  lit smoke. Each lit puff works out its dominant
//       light (Object.cpp XpPuffLight): the key light (the rig's key share of
//       its light, the sun by day, the moon under the moon key) and the
//       combat lights (a fire under its own column, a burning wreck) summed as
//       one vector by how much light each gave it. Its direction (camera
//       space) and its share of the puff's light ride in the second texture
//       coordinate; the object program shades the billboard as a ball facing
//       the camera, brighter toward that light and darker away from it (the
//       disc's mean kept). A forward-scattering phase per light is constant
//       over one small puff and is folded into its colour on the CPU: the puff
//       glows with the light behind it and greys with the light behind the
//       camera.
//
// With both off nothing here runs: RemasterFragmentMain(PSNormal) returns the
// shipped main, no upload or copy is made, and every decal's second texture
// coordinate stays 0, so the frame is the tip's bit for bit.

#include <PoseidonGL33/EngineGL33.hpp>
#include <PoseidonGL33/EngineGL33_Lighting.hpp>
#include <PoseidonGL33/GL33BindCache.hpp>

#include <glad/gl.h>

#include <cmath>
#include <cstdlib>
#include <string>

namespace gl = Poseidon::gl33lighting;

bool EngineGL33::XpLitSmoke() const
{
    return (gl::XpParticleFlags() & gl::kXpLitSmoke) != 0;
}

float EngineGL33::ParticleKeyLight(float toward[3]) const
{
    for (int i = 0; i < 3; ++i)
        toward[i] = _particleKeyToward[i];
    if (!(gl::XpParticleFlags() & gl::kXpLitSmoke) || _particleKeyFrame != GetFrameCounter())
        return 0.0f;
    return _particleKeyFraction;
}

void EngineGL33::XpCopySceneDepth()
{
    if (!(gl::XpParticleFlags() & gl::kXpSoftParticles) || !_glContext || !_worldEffectsOn || _portraitTargetSize ||
        !SSAAActive() || _post.unavailable || !_post.depthFbo || !_post.depthTex)
        return;
    const std::uint32_t frame = GetFrameCounter();
    if (_xpDepthFrame == frame)
        return;
    int w = 0, h = 0;
    RenderTargetSize(w, h);
    if (_post.w != w || _post.h != h)
        return;
    FlushAndFreeAllQueues(_queueNo);
    // The multisample resolve of the frame target's depth (as PostResolve,
    // sample by sample nearest), the whole frame: a blit honours the scissor.
    const bool scissor = glIsEnabled(GL_SCISSOR_TEST) == GL_TRUE;
    if (scissor)
        glDisable(GL_SCISSOR_TEST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _ssaaFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _post.depthFbo);
    glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, _ssaaFbo); // the viewport is not framebuffer state: it stays
    if (scissor)
        glEnable(GL_SCISSOR_TEST);
    Poseidon::GL33Bind::ActiveUnit(gl::kXpSceneDepthUnit);
    glBindTexture(GL_TEXTURE_2D, _post.depthTex);
    glBindSampler(static_cast<GLuint>(gl::kXpSceneDepthUnit), 0);
    Poseidon::GL33Bind::ActiveUnit(0);
    // The projection's depth terms (PostFrameGeometry's projInfo.xy and
    // depthMap) in the padding slot 72, which nothing else writes.
    const GfxMatrix& proj = _frameState.projection;
    SetLitSlot(gl::LitConst::Planned, proj._33, proj._43, GLAD_GL_ARB_clip_control ? 1.0f : 2.0f,
               GLAD_GL_ARB_clip_control ? 0.0f : -1.0f);
    UploadLitSlots(gl::LitConst::Planned, 1);
    _xpDepthFrame = frame;
}

float EngineGL33::XpDecalCode(bool depthTested, float& u)
{
    const float radius = _decalXpRadius;
    const float share = _decalXpShare;
    const float light[3] = {_decalXpLight[0], _decalXpLight[1], _decalXpLight[2]};
    _decalXpRadius = 0.0f;
    _decalXpShare = 0.0f;
    u = 0.0f;
    const int flags = gl::XpParticleFlags();
    if (!flags)
        return 0.0f;
    // The soft fade's radius (m) as (radQ / 16)^2, radQ 1..127 (to 63 m), in
    // the layer coordinate's fraction (0 to 0.4: the array layer it names is
    // round(u), unchanged).
    int radQ = 0;
    // Only a depth-tested puff, in a frame whose opaque depth was copied.
    if ((flags & gl::kXpSoftParticles) && radius > 0.0f && _xpDepthFrame == GetFrameCounter() && depthTested)
    {
        radQ = static_cast<int>(std::lround(std::sqrt(radius) * 16.0f));
        radQ = radQ < 1 ? 1 : (radQ > 127 ? 127 : radQ);
    }
    // The dominant light: its share in 31sts and its camera-space direction
    // octahedrally in 32 x 32.
    int shareQ = 0, ox = 0, oy = 0;
    if ((flags & gl::kXpLitSmoke) && share > 0.0f)
    {
        shareQ = static_cast<int>(std::lround(share * 31.0f));
        shareQ = shareQ < 0 ? 0 : (shareQ > 31 ? 31 : shareQ);
        const float l1 = std::fabs(light[0]) + std::fabs(light[1]) + std::fabs(light[2]);
        float px = l1 > 1e-6f ? light[0] / l1 : 0.0f;
        float py = l1 > 1e-6f ? light[1] / l1 : 0.0f;
        if (light[2] < 0.0f)
        {
            const float qx = (1.0f - std::fabs(py)) * (px >= 0.0f ? 1.0f : -1.0f);
            const float qy = (1.0f - std::fabs(px)) * (py >= 0.0f ? 1.0f : -1.0f);
            px = qx;
            py = qy;
        }
        ox = static_cast<int>(std::lround((px * 0.5f + 0.5f) * 31.0f));
        oy = static_cast<int>(std::lround((py * 0.5f + 0.5f) * 31.0f));
        ox = ox < 0 ? 0 : (ox > 31 ? 31 : ox);
        oy = oy < 0 ? 0 : (oy > 31 ? 31 : oy);
    }
    if (!radQ && !shareQ)
        return 0.0f;
    u = static_cast<float>(radQ) * (0.4f / 128.0f);
    return 1000.0f + static_cast<float>(shareQ * 1024 + ox * 32 + oy);
}
