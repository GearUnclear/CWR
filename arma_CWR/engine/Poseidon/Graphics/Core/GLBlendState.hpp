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

// Shadow darken: (ZERO, ONE_MINUS_SRC_ALPHA) — multiplies dst by
// (1 - src.a), used by the stencil-shadow fullscreen darken quad.
inline void Shadow()
{
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
}

// The same three blended modes with the destination alpha kept (alpha factors
// ZERO, ONE). The remastered lighting stores each opaque pixel's ambient share
// in the frame alpha for its ambient-occlusion pass; blended draws on top
// (decals, smoke, glass) then leave it alone instead of overwriting it with
// their own coverage. The colour factors are unchanged.
inline void AlphaBlendKeepAlpha()
{
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
}

inline void AdditiveKeepAlpha()
{
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE);
}

inline void ShadowKeepAlpha()
{
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
}

// Premultiplied alpha with the destination alpha kept: the source colour is
// added as it is and covers (1 - src.a) of what is behind it, so one draw both
// adds light (an emissive flame's core) and darkens (its soot). The fire
// spread's flame sheets (EngineGL33_FireFlames.cpp).
inline void PremultipliedKeepAlpha()
{
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
}

// Light added over what stays: dst.rgb = src.rgb + dst.rgb * src.a, and the
// alpha the product of every draw's src.a (the transmittance of all of them,
// from a target cleared to alpha 1). The fire spread's linear flame target
// (EngineGL33_FireFlames.cpp).
inline void TransmittanceAccumulate()
{
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_ONE, GL_SRC_ALPHA, GL_ZERO, GL_SRC_ALPHA);
}

// The same colour factors with the destination alpha kept: that target laid
// over the frame as it is when the post pass cannot merge it.
inline void TransmittanceKeepAlpha()
{
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_ONE, GL_SRC_ALPHA, GL_ZERO, GL_ONE);
}

// What is there dimmed by the source's alpha, nothing added, the alpha kept:
// dst.rgb *= 1 - src.a. Smoke drawn again into the fire spread's flame target
// dims the flames behind it (EngineGL33::BeginFireFlameCover).
inline void DimBySourceKeepAlpha()
{
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
}

// Smoke over the flames (fix 02): the source laid over what is there as the
// frame blends it (rgb: src.rgb src.a + dst.rgb (1 - src.a)), and the alpha
// the share of what lies behind that the layers leave (dst.a *= 1 - src.a).
// Smoke drawn again into the fire spread's flame cover target, in front of
// the flames (EngineGL33::BeginFireFlameCover).
inline void SmokeOverKeepTransmittance()
{
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
}

} // namespace render::blend

} // namespace Poseidon
