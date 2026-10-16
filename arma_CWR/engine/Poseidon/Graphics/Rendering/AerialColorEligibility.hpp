#pragma once
#include <Poseidon/Graphics/Rendering/RenderPassDescriptor.hpp>
namespace Poseidon { namespace render {
// Explicit world-atmosphere scope. Lighting mode is not a provenance substitute.
inline bool IsAerialColorReceiver(const RenderPassDescriptor& d, bool meshInput) {
    const bool world = d.pass == PassKind::WorldOpaque || d.pass == PassKind::WorldCutout ||
                       d.pass == PassKind::WorldTransparent || d.pass == PassKind::SurfaceOverlay ||
                       d.pass == PassKind::WorldWater;
    return meshInput && world && d.fog == FogMode::Enabled &&
           (d.depth == DepthMode::Normal || d.depth == DepthMode::ReadOnly) &&
           (d.blend == BlendMode::Opaque || d.blend == BlendMode::AlphaBlend) &&
           (d.shader == ShaderFamily::Normal || d.shader == ShaderFamily::Detail || d.shader == ShaderFamily::Grass ||
            (d.pass == PassKind::WorldWater && d.shader == ShaderFamily::Water));
}
}} // namespace Poseidon::render
