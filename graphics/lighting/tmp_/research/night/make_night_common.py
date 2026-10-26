"""Build the night-variant psRemasterCommon.glsl from the engine dump.

python make_night_common.py <dump psRemasterCommon.glsl> <out dir>

The variant is selected at runtime by litLightCount.y (tuning `night`, C++
prototype in gfx-night). night = 0 keeps every original code path.
"""
import sys, pathlib

src = pathlib.Path(sys.argv[1]).read_text(encoding="utf-8")
out = pathlib.Path(sys.argv[2]); out.mkdir(parents=True, exist_ok=True)

def rep(old, new, count=1):
    global src
    if src.count(old) != count:
        raise SystemExit(f"anchor count {src.count(old)} != {count}: {old[:60]!r}")
    src = src.replace(old, new)

NIGHT_FUNCS = r'''
// ---- NIGHT RESEARCH VARIANT (litLightCount.y >= 1, litCamPos.w = NightEffect) ----
bool g_nightLook = false;

// Low beam: wide horizontally, sharp cut-off just above the horizon line, a hot
// spot 1.5 degrees below it. Relative to the hot-spot intensity. L from the lamp.
float nightLowBeam(vec3 L, vec3 beam) {
    float c = dot(L, beam);
    if (c <= 0.0) return 0.0;
    vec3 right = normalize(cross(vec3(0.0, 1.0, 0.0), beam));
    vec3 u = cross(beam, right);
    float ah = atan(dot(L, right), c);
    float av = atan(dot(L, u), c);
    // Vertical: hot band 1.7 deg under the cut-off (3 deg wide), a faint spill
    // down to the bumper; horizontal: +-14 deg core and a wide +-35 deg spill.
    // Close road gets little light, so the beam reaches 40-60 m down the road.
    float cut = 1.0 - smoothstep(-0.004, 0.015, av);
    float hv = (av + 0.03) / 0.05;
    float vert = exp(-hv * hv) + 0.02 * smoothstep(-0.6, -0.1, av);
    float h1 = ah / 0.25;
    float h2 = ah / 0.6;
    float horiz = exp(-h1 * h1) + 0.15 * exp(-h2 * h2);
    return vert * horiz * cut;
}

// Physical local lights: inverse square from the bulb (no flat disc of full
// light out to the classic start distance), lamp colour in linear light, a
// low-beam pattern for headlights, a little bounce and a specular gleam on
// glossy materials. Returns linear irradiance times material; spec separately.
vec3 localLightsNight(vec3 n, vec3 v, vec3 kd, vec3 ka, out vec3 spec) {
    vec3 sum = vec3(0.0);
    spec = vec3(0.0);
    int count = int(litLightCount.x);
    float k = litLightCount.z; // nightP1
    float power = litMatSpecular.w;
    for (int i = 0; i < 8; ++i) {
        if (i >= count) break;
        vec3 toLight = litLightPos[i].xyz - vWorldRel;
        float d2 = dot(toLight, toLight);
        float start = litLightPos[i].w;
        float end2 = start * start * 100.0;
        bool isSpot = litLightColor[i].w > 0.5;
        if (!isSpot && d2 >= end2) continue;
        float d = sqrt(max(d2, 1e-6));
        vec3 L = toLight / d;
        float I;
        if (isSpot) {
            // The model beam axis is not trustworthy in pitch (the classic 12 degree
            // cone hides it): level it, half a degree down.
            vec3 bd = litLightDir[i].xyz;
            vec3 beam = normalize(vec3(bd.x, -0.009 * length(bd.xz), bd.z));
            I = 4000.0 * k * nightLowBeam(-L, beam);
            if (I <= 0.0) continue;
        } else {
            I = 4.0 * start * k; // start = 50 * config brightness: I linear in brightness
            I *= 1.0 - smoothstep(0.5 * end2, end2, d2);
        }
        float E = I / (d2 + 0.25);
        vec3 col = toLinear(litLightColor[i].rgb);
        float nl = max(dot(n, L), 0.0);
        sum += (kd * col * nl + ka * toLinear(litLightAmb[i].rgb) * 0.3) * E;
        if (power > 0.0) {
            vec3 h = normalize(L + v);
            float p = max(power, 1.0);
            spec += litMatSpecular.rgb * col * E * (pow(max(dot(n, h), 0.0), p) * (p + 8.0) / 28.0 * nl);
        }
    }
    return sum;
}

// Purkinje shift: dim light is seen by the rods, which see no hue and peak in
// blue-green. Below about 0.05 linear the colour fades into a rod-weighted grey
// with a slight cool cast; lamp pools stay warm and coloured.
vec3 nightPurkinje(vec3 c, float strength) {
    float Yp = dot(c, kLuma);
    float Ys = dot(c, vec3(0.03, 0.56, 0.41));
    float rod = (1.0 - smoothstep(0.003, 0.06, Yp)) * strength;
    vec3 tint = vec3(0.90, 0.97, 1.10);
    tint /= dot(tint, kLuma);
    return mix(c, Ys * tint, clamp(rod, 0.0, 1.0) * 0.9);
}
// ---- end NIGHT ----

// Sun highlight and sky reflection for glossy materials (specular power > 0).'''

rep("// Sun highlight and sky reflection for glossy materials (specular power > 0).", NIGHT_FUNCS)

rep("    vec3 local = toLinear(min(localLights(n, kd, ka) * litGround.w, vec3(1.5)));",
    """    vec3 local = vec3(0.0);
    vec3 localSpec = vec3(0.0);
    g_nightLook = litLightCount.y > 0.5 && litCamPos.w > 0.0;
    if (g_nightLook) {
        local = localLightsNight(n, v, kd, ka, localSpec) * litGround.w;
        localSpec *= litGround.w;
    } else {
        local = toLinear(min(localLights(n, kd, ka) * litGround.w, vec3(1.5)));
    }""")

rep("    vec3 radiance = (a * (direct + ambient + moon + local + emissive) + glossy(n, v, sunVisibility) * sunOn) * exposure;",
    """    vec3 radiance = (a * (direct + ambient + moon + local + emissive) + glossy(n, v, sunVisibility) * sunOn + localSpec) * exposure;
    if (g_nightLook) radiance = nightPurkinje(radiance, litCamPos.w * litLightCount.w);""")

rep("    if (nightEye) {", "    if (nightEye && !(g_nightLook && vLitMode > 0.5)) {")

(out / "psRemasterCommon.glsl").write_text(src, encoding="utf-8")
print("wrote", out / "psRemasterCommon.glsl")
