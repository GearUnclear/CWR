// Remastered lighting shaders: the sky piece of the fragment prelude
// (FragmentPiece::Sky, joined by RemasterFragmentCommon() in
// EngineGL33_LightingShaders.cpp): the sky light falling on a surface and the
// fog colour toward the sun. Sky lookups for the dome, the fog and reflections
// belong here.

#include <PoseidonGL33/EngineGL33_Lighting.hpp>

namespace Poseidon::gl33lighting
{

const char kRemasterSkyGLSL[] = R"(
// Irradiance from the sky hemisphere (and the ground below it) onto a surface
// with normal n. The horizon band is brighter, and takes the sky's colour,
// toward the sun; opposite the sun it keeps the cool zenith colour, so a low
// sun gives warm light and cool shade.
vec3 skyIrradiance(vec3 n) {
    float h = length(n.xz);
    float sl = length(litSunDir.xz);
    float toward = (h > 1e-3 && sl > 1e-3) ? 0.5 + 0.5 * dot(n.xz / h, litSunDir.xz / sl) : 0.5;
    // The bounce takes the colour of the ground below: the rig's horizon holds
    // half the (neutral) ground bounce, so swap that half for the tinted one.
    vec3 ground = litGround.rgb * g_groundTint;
    vec3 horizon = mix(litHorizon.rgb, litHorizonSun.rgb, toward) + 0.5 * (ground - litGround.rgb);
    float t = clamp(n.y, -1.0, 1.0);
    return t >= 0.0 ? mix(horizon, litZenith.rgb, t) : mix(horizon, ground, -t);
}

// Fog toward the sun takes the sky dome's own sun glow (TLVertexTable::
// DoSkyLighting adds SunSkyColor * cos^6 to SkyColor, times the sky texture),
// so distant ground fades into the colour of the sky behind it instead of a
// flat band. Away from the sun this is exactly the classic fog colour.
vec3 fogColorToward(vec3 rel) {
    if (litFogSun.w < 0.5) return fogColor.rgb;
    float c = max(dot(rel * inversesqrt(max(dot(rel, rel), 1e-12)), litSunDir.xyz), 0.0);
    float c2 = c * c;
    return min(fogColor.rgb + litFogSun.rgb * (c2 * c2 * c2), max(litFogCap.rgb, fogColor.rgb));
}
)";

} // namespace Poseidon::gl33lighting
