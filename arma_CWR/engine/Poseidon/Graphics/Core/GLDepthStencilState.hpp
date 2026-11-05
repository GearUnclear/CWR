#pragma once

#include <glad/gl.h>

// Atomic GL depth + stencil state bundles.
//
// Depth + stencil are bundled at the GL state level — D3D11's
// `D3D11_DEPTH_STENCIL_DESC` carries both — so the helpers here
// set both atomically per named mode.  Each helper is the unique
// callsite of its `glDepthFunc` / `glDepthMask` / `glStencilFunc` /
// `glStencilOp` combination.
//
// `hasStencil` is a runtime flag the engine checks once at init;
// the stencil portion of each helper is gated on it so a context
// without a stencil buffer doesn't issue `glStencilFunc` calls.

namespace Poseidon
{
namespace render::depthstencil
{

namespace detail
{
inline void StencilAlwaysReplaceZero()
{
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
}
inline void StencilEqualZeroIncr()
{
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
}
} // namespace detail

// Scoped background draw: neither test nor write depth/stencil, without
// changing comparison functions or stencil operations. Restore actual captured
// state exactly; callers still invalidate their pipeline cache after the scope.
class ScopedBackground
{
    GLboolean _depthEnabled, _stencilEnabled, _depthWrite;
    bool _active = true;

  public:
    ScopedBackground()
    {
        _depthEnabled = glIsEnabled(GL_DEPTH_TEST);
        _stencilEnabled = glIsEnabled(GL_STENCIL_TEST);
        glGetBooleanv(GL_DEPTH_WRITEMASK, &_depthWrite);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
        glDepthMask(GL_FALSE);
    }
    ScopedBackground(const ScopedBackground&) = delete;
    ScopedBackground& operator=(const ScopedBackground&) = delete;
    ~ScopedBackground() { Restore(); }
    void Restore()
    {
        if (!_active)
            return;
        if (_depthEnabled)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);
        if (_stencilEnabled)
            glEnable(GL_STENCIL_TEST);
        else
            glDisable(GL_STENCIL_TEST);
        glDepthMask(_depthWrite);
        _active = false;
    }
};

// Standard depth test + write enabled.  Stencil ALWAYS + REPLACE 0
// (clears stencil for any rendered pixel, enabling subsequent
// shadow exclusion).
inline void Normal(bool hasStencil)
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    if (hasStencil)
        detail::StencilAlwaysReplaceZero();
}

// Depth test on, write off — alpha-blended transparent draws that
// shouldn't write z.
inline void ReadOnly(bool hasStencil)
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    if (hasStencil)
        detail::StencilAlwaysReplaceZero();
}

// Depth test always passes, no write — 2D overlay draws.
inline void Disabled(bool hasStencil)
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glDepthMask(GL_FALSE);
    if (hasStencil)
        detail::StencilAlwaysReplaceZero();
}

// Per-poly shadow draw — stencil EQUAL ref=0 + INCR so a pixel only
// receives one INCR even if multiple shadow polys cover it.  Depth
// test on, write off (when stencil available; falls back to write
// on if no stencil for the simple shadow path).
inline void Shadow(bool hasStencil)
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    if (hasStencil)
    {
        glDepthMask(GL_FALSE);
        detail::StencilEqualZeroIncr();
    }
    else
    {
        glDepthMask(GL_TRUE);
    }
}

} // namespace render::depthstencil

} // namespace Poseidon
