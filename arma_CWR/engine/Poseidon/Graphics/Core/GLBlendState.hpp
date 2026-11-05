#pragma once

#include <glad/gl.h>

// Atomic GL blend-state bundles.
//
// Each helper here is the unique callsite of `glEnable(GL_BLEND)` /
// `glDisable(GL_BLEND)` / `glBlendFunc` / `glBlendFuncSeparate` for one named
// blend mode.  Callers pick a helper by name; partial state ("blend enabled but
// blend func not set") is unrepresentable because each helper sets both flags
// atomically — there is one entry point per mode and each writes the full
// bundle.

namespace Poseidon
{
namespace render::blend
{

inline void Opaque()
{
    glDisable(GL_BLEND);
}

inline void AlphaBlend()
{
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
}

inline void Additive()
{
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ONE, GL_ZERO);
}

// A temporary alpha overlay owns the full blend bundle, including equations.
// Restore actual incoming state, not an assumed engine-cache mode.
class ScopedAlphaOverlay
{
    GLboolean _enabled;
    GLint _state[6];
    bool _active = true;
    static constexpr GLenum Fields[6] = {GL_BLEND_SRC_RGB, GL_BLEND_DST_RGB, GL_BLEND_SRC_ALPHA,
                                          GL_BLEND_DST_ALPHA, GL_BLEND_EQUATION_RGB, GL_BLEND_EQUATION_ALPHA};

public:
    ScopedAlphaOverlay() : _enabled(glIsEnabled(GL_BLEND))
    {
        for (int i = 0; i < 6; ++i) glGetIntegerv(Fields[i], &_state[i]);
        AlphaBlend();
        glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    }
    ScopedAlphaOverlay(const ScopedAlphaOverlay&) = delete;
    ScopedAlphaOverlay& operator=(const ScopedAlphaOverlay&) = delete;
    ~ScopedAlphaOverlay() { Restore(); }
    void Restore()
    {
        if (!_active) return;
        glBlendFuncSeparate(_state[0], _state[1], _state[2], _state[3]);
        glBlendEquationSeparate(_state[4], _state[5]);
        if (_enabled) glEnable(GL_BLEND); else glDisable(GL_BLEND);
        _active = false;
    }
    bool MatchesSavedState() const
    {
        if (glIsEnabled(GL_BLEND) != _enabled) return false;
        for (int i = 0; i < 6; ++i) {
            GLint value = 0;
            glGetIntegerv(Fields[i], &value);
            if (value != _state[i]) return false;
        }
        return true;
    }
};

// Shadow darken: (ZERO, ONE_MINUS_SRC_ALPHA) — multiplies dst by
// (1 - src.a), used by the stencil-shadow fullscreen darken quad.
inline void Shadow()
{
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
}

} // namespace render::blend

} // namespace Poseidon
