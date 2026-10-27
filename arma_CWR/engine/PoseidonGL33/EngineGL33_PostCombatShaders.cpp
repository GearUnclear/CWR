// Remastered lighting: the combat shadow mask's GLSL (the
// combatShadows switch; Graphics/Rendering/Lighting/CombatLights.hpp). No GL
// code: the unit tests compile these through PostShaderSource
// (EngineGL33_PostShaders.cpp kPostShaders, rows CombatMask and
// CombatMaskBlur). EngineGL33_PostCombat.cpp runs the passes at the start of
// each world pass.
//
// The mask holds, for up to four shadowed combat lights (one per channel), how
// much of each light reaches every surface the camera saw in the previous
// frame: a half-resolution texel per 2x2 frame-target pixels, the receiver the
// previous frame's linear depth (the LinearDepth post stage) at the centre of
// its block, and the march toward the light through the same depth. The
// forward shaders of this frame project their fragment into that screen
// (combatShadowVis, the rows of CombatMaskRows) and take one bilinear tap.

#include <PoseidonGL33/EngineGL33_Lighting.hpp>

namespace Poseidon::gl33lighting
{

// The trace. A step that lies behind what the camera saw there by more than a
// bias and less than an occluder's thickness is blocked; how deep it sits,
// against the cone toward the light's ball (radius r0 times the distance
// travelled from the receiver), gives a penumbra that is sharp at contact and
// soft far from it. Steps crowd toward the receiver (contact detail) and turn
// with interleaved gradient noise, per frame under TAA. Steps that leave the
// screen stop the march (an occluder off screen is unknown and counts as
// open); the receiver's own texel never blocks it. Receivers beyond a light's
// window are not traced (that light adds nothing there).
const char kPostCombatMaskGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D linDepthTex; // the previous frame's half-resolution view depth (m), 1e6 = sky
uniform vec4 maskProj;         // x 1/p11, y 1/p22, z p11, w p22 of that frame
uniform vec4 maskRect;         // its 3D viewport in uv (x, y, w, h)
uniform vec4 maskSize;         // xy its frame target (pixels), zw the half size
uniform vec4 maskLight[4];     // xyz a shadowed light in that frame's view space (m), w its core radius (0 = none)
uniform vec4 maskRange;        // each channel's window end (m)
uniform vec4 maskParams;       // x steps, y occluder thickness (m), z noise turn, w march limit (m)
out vec4 fragColor;

ivec2 maskTexel(vec3 p, out bool inside) {
    vec2 ndc = p.xy * maskProj.zw / p.z;
    vec2 px = (maskRect.xy + (ndc * 0.5 + 0.5) * maskRect.zw) * maskSize.xy;
    ivec2 t = ivec2(floor(px * 0.5));
    inside = all(greaterThanEqual(t, ivec2(0))) && all(lessThan(t, ivec2(maskSize.zw)));
    return t;
}

float traceLight(vec3 vp, vec4 light, float range, float jit, ivec2 self) {
    vec3 toLight = light.xyz - vp;
    float d = length(toLight);
    if (d < 1e-3 || d >= range) return 1.0;
    float r0 = light.w;
    float tEnd = clamp(min(1.0 - r0 / d, maskParams.w / d), 0.0, 1.0);
    float steps = maskParams.x;
    float occ = 0.0;
    for (int k = 0; k < 32; ++k) {
        if (float(k) >= steps) break;
        float u = (float(k) + jit) / steps;
        float t = u * u * tEnd;
        vec3 p = vp + toLight * t;
        if (p.z < 0.3) break;
        bool inside;
        ivec2 q = maskTexel(p, inside);
        if (!inside) break;
        if (q == self) continue;
        float zs = texelFetch(linDepthTex, q, 0).r;
        if (zs > 1e5) continue;
        float behind = p.z - zs;
        float bias = 0.08 + 0.01 * p.z;
        if (behind > bias && behind < maskParams.y) {
            float cone = max(r0 * t, 0.03);
            occ = max(occ, clamp((behind - bias) / cone, 0.0, 1.0));
            if (occ >= 0.999) break;
        }
    }
    return 1.0 - occ;
}

void main() {
    ivec2 t = ivec2(gl_FragCoord.xy);
    float z = texelFetch(linDepthTex, t, 0).r;
    if (z > 1e5) { fragColor = vec4(1.0); return; }
    // The receiver: the centre of the texel's 2x2 block of frame-target pixels
    // (CombatLights.hpp CombatMaskTexelView).
    vec2 ndc = ((2.0 * vec2(t) + 1.0) / maskSize.xy - maskRect.xy) / maskRect.zw * 2.0 - 1.0;
    vec3 vp = vec3(ndc.x * z * maskProj.x, ndc.y * z * maskProj.y, z);
    float jit = fract(fract(52.9829189 * fract(dot(gl_FragCoord.xy, vec2(0.06711056, 0.00583715)))) + maskParams.z);
    vec4 vis = vec4(1.0);
    for (int c = 0; c < 4; ++c)
        if (maskLight[c].w > 0.0) vis[c] = traceLight(vp, maskLight[c], maskRange[c], jit, t);
    fragColor = vis;
}
)";

// The blur: seven taps along one axis, Gaussian weights cut where the depth of
// a tap differs from the centre's (the shadow of a man does not bleed onto the
// ground behind him), run across and then down. It removes the march's noise.
const char kPostCombatMaskBlurGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D maskTex;     // the mask to blur
uniform sampler2D linDepthTex; // the depth it was traced from
uniform vec4 blurDir;          // xy the step (texels), zw the half size
out vec4 fragColor;
void main() {
    ivec2 t = ivec2(gl_FragCoord.xy);
    ivec2 lim = ivec2(blurDir.zw) - 1;
    vec4 sum = texelFetch(maskTex, t, 0);
    float z0 = texelFetch(linDepthTex, t, 0).r;
    if (z0 > 1e5) { fragColor = sum; return; }
    float wsum = 1.0;
    float tol = 0.04 * z0 + 0.1;
    for (int i = -3; i <= 3; ++i) {
        if (i == 0) continue;
        ivec2 q = clamp(t + ivec2(blurDir.xy) * i, ivec2(0), lim);
        float z = texelFetch(linDepthTex, q, 0).r;
        float w = exp(-float(i * i) / 4.5) * max(0.0, 1.0 - abs(z - z0) / tol);
        sum += texelFetch(maskTex, q, 0) * w;
        wsum += w;
    }
    fragColor = sum / wsum;
}
)";

} // namespace Poseidon::gl33lighting
