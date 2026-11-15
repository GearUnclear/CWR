// Long tree shadows: the GLSL of the pass that fills the canopy's
// sun-visibility map (EngineGL33_Canopy.cpp runs it before the world when the
// sun or the camera moved; it is not a post stage). No GL code, so the unit
// tests compile this very source; kPostShaders in EngineGL33_PostShaders.cpp
// assembles it with the fullscreen vertex shader the post passes share.
//
// It is the GPU side of Graphics/Remaster/Canopy.cpp: CanopySunAt traces the
// same texel step for step (the shape constants below are Canopy.hpp's, which
// the tests check), so triCanopyCheck can compare the two.

#include <PoseidonGL33/EngineGL33_Lighting.hpp>

namespace Poseidon::gl33lighting
{

// One texel of the map is a ground point at world (x, z): from there it
// cone-traces the canopy heightfield toward the sun. The step grows with the
// distance and the mip level follows the step, so a far occluder blurs into a
// wide, light penumbra and a near one stays crisp. A sample shades the ray
// where the ray runs under the canopy's top (soft over the sun-disk cone
// width); a partly covered texel is crossed with the probability
// sqrt(density) (less the thinnest density, so the crossing starts from 0).
// Output: r the ground's visibility, g the lowest raised receiver's (1.5 m up,
// skipping the first 6 m: its own crown), b the height above the ground below
// which a raised receiver is in a neighbour's shadow (/ 48 m), a that edge's
// softness (/ 16 m). The edge is the envelope of every sample's own edge plus
// and minus its softness; a thin sample's edge drops out of it smoothly, so
// the map never jumps where a density crosses a threshold.
const char kPostCanopySunGLSL[] = R"(#version 330 core
in vec2 vUV;
out vec4 fragColor;
uniform sampler2D canopyMap;     // unit 10: r = top (base + r * range, m), g = density; mips
uniform sampler2D terrainHeight; // unit 4: r = terrain height (m)
uniform vec4 canopyField;        // x 1 / the field's extent (1/m), y level-0 texel (m), z height base (m), w height range (m)
uniform vec4 terrainGrid;        // x 1 / cell size (1/m), y 1 / texels a side (the world shaders' litTerrain.yz)
uniform vec4 sunRay;             // xy toward the sun (horizontal, unit), z tan(sun elevation), w tan(edge softness)
uniform vec4 sunMap;             // xy the map's origin (m), z its texel (m), w the trace length (m)

const int kMaxSteps = 40;
const float kGrowth = 0.15;
const float kMinDensity = 0.003;
const float kMinTransmit = 0.02;
const float kGroundLift = 0.3;
const float kRaisedLift = 1.8;
const float kRaisedSkip = 6.0;
const float kEdgeCrossFrom = 0.3;
const float kEdgeCrossTo = 0.6;
const float kEdgeRange = 48.0;
const float kSoftRange = 16.0;
const float kMinSoft = 0.5;

void main() {
    vec2 p = sunMap.xy + gl_FragCoord.xy * sunMap.z;
    float y0 = textureLod(terrainHeight, (p * terrainGrid.x + 0.5) * terrainGrid.y, 0.0).r;
    float c0 = canopyField.y;
    float t = 0.5 * c0;
    float ground = 1.0;
    float raised = 1.0;
    float upper = kMinSoft;
    float lower = -kMinSoft;
    for (int i = 0; i < kMaxSteps; ++i) {
        if (t > sunMap.w) break;
        float stepLen = max(c0, kGrowth * t);
        float lod = log2(stepLen / c0);
        vec2 c = textureLod(canopyMap, (p + sunRay.xy * t) * canopyField.x, lod).rg;
        if (c.y > kMinDensity) {
            float top = canopyField.z + c.x * canopyField.w;
            float s = t * sunRay.w + 0.25 * stepLen + 0.5;
            float crossing = sqrt(c.y - kMinDensity);
            float rise = t * sunRay.z;
            ground *= max(1.0 - crossing * smoothstep(-0.5, 1.0, (top - (y0 + kGroundLift + rise)) / s), kMinTransmit);
            if (t >= kRaisedSkip) {
                raised *= max(1.0 - crossing * smoothstep(-0.5, 1.0, (top - (y0 + kRaisedLift + rise)) / s), kMinTransmit);
                float w = smoothstep(kEdgeCrossFrom, kEdgeCrossTo, crossing);
                float h = (top - rise - y0 + s + 1.0) * w - s - 1.0;
                upper = max(upper, h + s);
                lower = max(lower, h - s);
            }
        }
        t += stepLen;
    }
    float edge = 0.5 * (upper + lower);
    float soft = 0.5 * (upper - lower);
    fragColor = vec4(ground, raised, clamp(edge / kEdgeRange, 0.0, 1.0), clamp(soft / kSoftRange, 0.0, 1.0));
}
)";

} // namespace Poseidon::gl33lighting
