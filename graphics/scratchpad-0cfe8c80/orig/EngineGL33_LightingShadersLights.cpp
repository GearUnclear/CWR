// Remastered lighting shaders: the local-lights piece of the fragment
// prelude (FragmentPiece::Lights, joined by RemasterFragmentCommon() in
// EngineGL33_LightingShaders.cpp): headlights, lamps and fires per fragment.
// Further kinds of local light belong here.

#include <PoseidonGL33/EngineGL33_Lighting.hpp>

namespace Poseidon::gl33lighting
{

const char kRemasterLightsGLSL[] = R"(
// Point lights and spotlights per fragment: the classic quadratic falloff past
// the start distance, ending in a smooth window instead of a hard cut at 10x
// the start distance, and a smoothed spotlight cone.
vec3 localLights(vec3 n, vec3 kd, vec3 ka) {
    vec3 sum = vec3(0.0);
    int count = int(litLightCount.x);
    for (int i = 0; i < 8; ++i) {
        if (i >= count) break;
        vec3 toLight = litLightPos[i].xyz - vWorldRel;
        float d2 = dot(toLight, toLight);
        float start2 = litLightPos[i].w * litLightPos[i].w;
        float end2 = start2 * 100.0;
        if (d2 >= end2) continue;
        float cone = 1.0;
        if (litLightColor[i].w > 0.5) {
            float inside = -dot(toLight, litLightDir[i].xyz);
            if (inside <= 0.0) continue;
            float cos2 = inside * inside / max(d2, 1e-6);
            cone = smoothstep(0.9415, 0.9806, cos2); // 14 deg soft edge, full inside 8 deg
            if (cone <= 0.0) continue;
        }
        float atten = (d2 > start2) ? start2 / d2 : 1.0;
        atten *= 1.0 - smoothstep(0.5 * end2, end2, d2);
        float cosFi = max(dot(toLight, n), 0.0) * inversesqrt(max(d2, 1e-6));
        sum += (kd * litLightColor[i].rgb * cosFi + ka * litLightAmb[i].rgb) * (atten * cone);
    }
    return sum;
}
)";

} // namespace Poseidon::gl33lighting
