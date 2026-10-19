#pragma once

#include <glad/gl.h>

// Atomic GL depth + stencil state bundles.
//
// Depth + stencil are bundled at the GL state level — D3D11's
// `D3D11_DEPTH_STENCIL_DESC` carries both — so the helpers here
// set both atomically per named mode.  Each helper is the unique
// callsite of its `glDepthFunc` / `glDepthMask` / `glStencilFunc` /
// `glStencilOp` / `glStencilMask` combination.
//
// `hasStencil` is a runtime flag the engine checks once at init;
// the stencil portion of each helper is gated on it so a context
// without a stencil buffer doesn't issue `glStencilFunc` calls.
//
// `StencilTag` carries a draw's stencil reference and masks for the
// remastered screen-space reflections (Graphics/Remaster/ScreenReflections.hpp,
// SsrStencil): a surface tag in the top bits, written under a write mask, and
// the projected-shadow test comparing the low bits only. Its default is the
// state of before the tags (REPLACE 0 into every bit, compare every bit).

namespace Poseidon
{
namespace render::depthstencil
{

struct StencilTag
{
    GLint ref = 0;                   // what REPLACE writes
    GLuint writeMask = 0xFF;         // under this write mask
    GLuint shadowCompareMask = 0xFF; // Shadow: EQUAL 0 compares these bits
};

namespace detail
{
inline void StencilAlwaysReplace(const StencilTag& tag)
{
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, tag.ref, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(tag.writeMask);
}
inline void StencilEqualZeroIncr(const StencilTag& tag)
{
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, tag.shadowCompareMask);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    glStencilMask(0xFF);
}
} // namespace detail

// Standard depth test + write enabled.  Stencil ALWAYS + REPLACE 0
// (clears stencil for any rendered pixel, enabling subsequent
// shadow exclusion), or the draw's tag.
inline void Normal(bool hasStencil, const StencilTag& tag = {})
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    if (hasStencil)
        detail::StencilAlwaysReplace(tag);
}

// Depth test on, write off — alpha-blended transparent draws that
// shouldn't write z.
inline void ReadOnly(bool hasStencil, const StencilTag& tag = {})
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    if (hasStencil)
        detail::StencilAlwaysReplace(tag);
}

// Depth test always passes, no write — 2D overlay draws.
inline void Disabled(bool hasStencil, const StencilTag& tag = {})
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glDepthMask(GL_FALSE);
    if (hasStencil)
        detail::StencilAlwaysReplace(tag);
}

// Per-poly shadow draw — stencil EQUAL ref=0 + INCR so a pixel only
// receives one INCR even if multiple shadow polys cover it.  Depth
// test on, write off (when stencil available; falls back to write
// on if no stencil for the simple shadow path).
inline void Shadow(bool hasStencil, const StencilTag& tag = {})
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    if (hasStencil)
    {
        glDepthMask(GL_FALSE);
        detail::StencilEqualZeroIncr(tag);
    }
    else
    {
        glDepthMask(GL_TRUE);
    }
}

// Stencil test only, nothing written: a fullscreen pass that draws where
// (stencil & mask) == ref (the reflections' tag readout). The depth state is
// the caller's.
inline void StencilTestEqual(GLint ref, GLuint mask)
{
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, ref, mask);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0x00);
}

// Stencil test off and every bit writable again, as a stencil clear and the
// next draw expect.
inline void StencilOffFullMask()
{
    glDisable(GL_STENCIL_TEST);
    glStencilMask(0xFF);
}

// Every stencil bit writable (a stencil clear honours the write mask); the
// test and the function stay as they are.
inline void StencilFullWriteMask()
{
    glStencilMask(0xFF);
}

} // namespace render::depthstencil

} // namespace Poseidon
