// Remastered lighting shaders: the surface piece of the fragment
// prelude (FragmentPiece::Surface, joined by RemasterFragmentCommon() in
// EngineGL33_LightingShaders.cpp): the sun highlight and sky reflection of
// glossy materials. Material and wet-surface responses belong here.

#include <PoseidonGL33/EngineGL33_Lighting.hpp>

namespace Poseidon::gl33lighting
{

const char kRemasterSurfaceGLSL[] = R"(
// Sun highlight and sky reflection for glossy materials (specular power > 0).
vec3 glossy(vec3 n, vec3 v, float sunVisibility) {
    float power = litMatSpecular.w;
    if (power <= 0.0) return vec3(0.0);
    vec3 l = litSunDir.xyz;
    float nl = dot(n, l);
    vec3 h = normalize(l + v);
    float nh = max(dot(n, h), 0.0);
    float vh = max(dot(v, h), 0.0);
    float p = max(power, 1.0);
    // Normalised Blinn-Phong (tighter lobes are brighter) with a Schlick-style
    // grazing boost.
    float norm = (p + 8.0) / 28.0;
    float fresnel = 1.0 + 2.0 * pow(clamp(1.0 - vh, 0.0, 1.0), 5.0);
    vec3 sunSpec = litMatSpecular.rgb * litSun.rgb * (pow(nh, p) * norm * fresnel * smoothstep(0.0, 0.15, nl) * sunVisibility);
    vec3 r = reflect(-v, n);
    float fv = pow(clamp(1.0 - dot(n, v), 0.0, 1.0), 5.0);
    float gloss = clamp(dot(litMatSpecular.rgb, kLuma), 0.0, 1.0);
    vec3 skySpec = skyIrradiance(r) * (gloss * (0.04 + 0.96 * fv) * 0.6);
    return (sunSpec + skySpec) * litHorizon.w;
}
)";

} // namespace Poseidon::gl33lighting
