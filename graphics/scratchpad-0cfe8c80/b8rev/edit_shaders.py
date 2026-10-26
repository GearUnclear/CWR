from pathlib import Path

E = Path(r"D:\Arma_CWA_worktrees\gr-B8\arma_CWR\engine\PoseidonGL33")


def rep(path, old, new):
    s = path.read_text(encoding="utf-8")
    assert s.count(old) == 1, (path.name, old[:80], s.count(old))
    path.write_text(s.replace(old, new), encoding="utf-8", newline="\n")


sh = E / "EngineGL33_PostAirShaders.cpp"
rep(sh, """//   - psPostAirMarch (half resolution, two targets): per pixel of the linear
//     depth, the sun's single scattering in the beam mist marched over the
//     first kBeamDistance metres with the sun visibility of the shadow map,
//     the terrain horizon and the cloud field (light shafts); the headlight
//     cones; the sun visibility of the haze's part of the ray (ridge and cloud
//     shadows in the haze); and the valley mist's top and density at the
//     surface the ray ends on;
""", """//   - psPostAirMarch (half resolution, two targets): per pixel of the linear
//     depth, the sun's single scattering in the beam mist marched over the
//     first kBeamDistance metres with the sun visibility of the shadow map,
//     the terrain horizon and the cloud field (light shafts), and the same
//     scattering as if the whole ray saw the sun; the headlight cones; the
//     sun visibility of the haze's part of the ray (ridge and cloud shadows in
//     the haze); the valley mist's top and density at the surface the ray
//     ends on, and the view depth;
""")
rep(sh, """layout(location = 0) out vec4 fragColor; // r sun beams, g headlight beams, b haze sun visibility, a view depth (m)
layout(location = 1) out vec4 fragMist;  // r valley mist top (m), g its density factor
""", """layout(location = 0) out vec4 fragColor; // r sun beams, g headlight beams, b haze sun visibility, a sun beams as if all lit
layout(location = 1) out vec4 fragMist;  // r valley mist top (m), g its density factor, b view depth (m)
""")
rep(sh, """    float sunBeams = 0.0;
    if (airSun.x > 0.0) {
        float tEnd = min(rayLen, airMarch.x);
        int n = int(airMarch.y);
        float optical = 0.0;
        float sumLit = 0.0;
""", """    // sunOpen is the same scattering as if the whole ray saw the sun: the
    // apply's contrast works on the share that does (sunBeams / sunOpen).
    float sunBeams = 0.0;
    float sunOpen = 0.0;
    if (airSun.x > 0.0) {
        float tEnd = min(rayLen, airMarch.x);
        int n = int(airMarch.y);
        float optical = 0.0;
        float sumLit = 0.0;
        float sumOpen = 0.0;
""")
rep(sh, """            optical += sigma * dt;
            float vis = airShadowAt(rel) * cloudSunVisibility(pa);
            if (onMap) vis *= airTerrainSun(uv, hA);
            sumLit += w * vis;
        }""", """            optical += sigma * dt;
            float vis = airShadowAt(rel) * cloudSunVisibility(pa);
            if (onMap) vis *= airTerrainSun(uv, hA);
            sumLit += w * vis;
            sumOpen += w;
        }""")
rep(sh, """        sunBeams = phase * 0.25 * sumLit * airSun.x;
    }
""", """        sunBeams = phase * 0.25 * sumLit * airSun.x;
        sunOpen = phase * 0.25 * sumOpen * airSun.x;
    }
""")
rep(sh, """    fragColor = vec4(sunBeams, spots, vis, min(z, 6e4));
    fragMist = vec4(top, mistDensity, 0.0, 1.0);""", """    fragColor = vec4(sunBeams, spots, vis, sunOpen);
    fragMist = vec4(top, mistDensity, min(z, 6e4), 1.0);""")
# Blur.
rep(sh, """// Depth-aware separable blur of the march's first target (rgb; a is the view
// depth), run across and then down.
const char kPostAirBlurGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D airTex;
uniform vec2 stepUV;
out vec4 fragColor;
void main() {
    vec4 c = texture(airTex, vUV);
    vec3 sum = c.rgb;
    float wsum = 1.0;
    for (int i = -5; i <= 5; ++i) {
        if (i == 0) continue;
        vec4 s = texture(airTex, vUV + stepUV * float(i));
        float w = exp(-float(i * i) / 12.0) * max(0.0, 1.0 - abs(s.a - c.a) / (0.08 * c.a + 0.5));
        sum += s.rgb * w;
        wsum += w;
    }
    fragColor = vec4(sum / wsum, c.a);
}
""", """// Depth-aware separable blur of the march's first target (the view depth is
// the second target's b), run across and then down.
const char kPostAirBlurGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D airTex;
uniform sampler2D airDepthTex; // unit 1: the march's second target, b = view depth (m)
uniform vec2 stepUV;
out vec4 fragColor;
void main() {
    vec4 sum = texture(airTex, vUV);
    float zc = texture(airDepthTex, vUV).b;
    float wsum = 1.0;
    for (int i = -5; i <= 5; ++i) {
        if (i == 0) continue;
        vec2 uv = vUV + stepUV * float(i);
        float zs = texture(airDepthTex, uv).b;
        float w = exp(-float(i * i) / 12.0) * max(0.0, 1.0 - abs(zs - zc) / (0.08 * zc + 0.5));
        sum += texture(airTex, uv) * w;
        wsum += w;
    }
    fragColor = sum / wsum;
}
""")
# Mean.
rep(sh, """uniform sampler2D airTex;
uniform vec4 meanParams; // x radius (uv of the frame height), y aspect (width / height)
out vec4 fragColor;
void main() {
    float zc = texture(airTex, vUV).a;
    vec2 mean = vec2(0.0);
    float wm = 0.0;
    for (int k = 0; k < 24; ++k) {
        float a = float(k) * 2.39996;
        float r = meanParams.x * sqrt((float(k) + 0.5) / 24.0);
        vec2 o = vec2(cos(a) * r / meanParams.y, sin(a) * r);
        vec4 sm = texture(airTex, clamp(vUV + o, vec2(0.0), vec2(1.0)));
        float wk = exp(-abs(sm.a - zc) / (0.25 * zc + 2.0));
        mean += sm.rg * wk;
        wm += wk;
    }
    fragColor = vec4(mean / max(wm, 1e-4), 0.0, 1.0);
}
""", """uniform sampler2D airTex;
uniform sampler2D airDepthTex; // unit 1: the march's second target, b = view depth (m)
uniform vec4 meanParams; // x radius (uv of the frame height), y aspect (width / height)
out vec4 fragColor;      // the mean of airTex's r, g and a (sun beams, headlight beams, sun beams as if all lit)
void main() {
    float zc = texture(airDepthTex, vUV).b;
    vec3 mean = vec3(0.0);
    float wm = 0.0;
    for (int k = 0; k < 24; ++k) {
        float a = float(k) * 2.39996;
        float r = meanParams.x * sqrt((float(k) + 0.5) / 24.0);
        vec2 o = vec2(cos(a) * r / meanParams.y, sin(a) * r);
        vec2 uv = clamp(vUV + o, vec2(0.0), vec2(1.0));
        float wk = exp(-abs(texture(airDepthTex, uv).b - zc) / (0.25 * zc + 2.0));
        mean += texture(airTex, uv).rga * wk;
        wm += wk;
    }
    fragColor = vec4(mean / max(wm, 1e-4), 1.0);
}
""")
# Apply inputs.
rep(sh, """uniform sampler2D airTex;     // unit 2: half resolution: r sun beams, g headlight beams, b haze sun visibility, a view depth
uniform sampler2D airMistTex; // unit 3: half resolution: r valley mist top (m), g its density factor
uniform sampler2D airMeanTex; // unit 4: half resolution: rg the beams' local mean""",
    """uniform sampler2D airTex;     // unit 2: half resolution: r sun beams, g headlight beams, b haze sun visibility, a sun beams as if all lit
uniform sampler2D airMistTex; // unit 3: half resolution: r valley mist top (m), g its density factor, b view depth (m)
uniform sampler2D airMeanTex; // unit 4: half resolution: the local mean of airTex's r, g and a""")
# Apply main.
rep(sh, """    vec4 air = vec4(0.0, 0.0, 1.0, 0.0);
    vec2 mist = vec2(-1e4, 0.0);
    vec2 mean = vec2(0.0);
    float conf = 1.0;""", """    vec4 air = vec4(0.0, 0.0, 1.0, 0.0);
    vec2 mist = vec2(-1e4, 0.0);
    vec3 mean = vec3(0.0);
    float conf = 1.0;""")
rep(sh, """        float best = 1e9;
        float wsum = 0.0;
        vec3 sum = vec3(0.0);
        vec2 msum = vec2(0.0);
        vec2 esum = vec2(0.0);
        for (int j = 0; j < 2; ++j) {
            for (int i = 0; i < 2; ++i) {
                ivec2 tc = clamp(ivec2(base) + ivec2(i, j), ivec2(0), ivec2(airHalf.xy) - 1);
                vec4 s = texelFetch(airTex, tc, 0);
                float wb = (i == 0 ? 1.0 - f.x : f.x) * (j == 0 ? 1.0 - f.y : f.y);
                float dn = abs(s.a - z) / (0.05 * z + 0.3);
                best = min(best, dn);
                float w = wb / (1e-3 + dn) + 1e-6;
                sum += s.rgb * w;
                msum += texelFetch(airMistTex, tc, 0).rg * w;
                if (airBeams.x > 0.5) esum += texelFetch(airMeanTex, tc, 0).rg * w;
                wsum += w;
            }
        }
        air.rgb = sum / wsum;
        mist = msum / wsum;
        mean = esum / wsum;""", """        float best = 1e9;
        float wsum = 0.0;
        vec4 sum = vec4(0.0);
        vec2 msum = vec2(0.0);
        vec3 esum = vec3(0.0);
        for (int j = 0; j < 2; ++j) {
            for (int i = 0; i < 2; ++i) {
                ivec2 tc = clamp(ivec2(base) + ivec2(i, j), ivec2(0), ivec2(airHalf.xy) - 1);
                vec3 m = texelFetch(airMistTex, tc, 0).rgb;
                float wb = (i == 0 ? 1.0 - f.x : f.x) * (j == 0 ? 1.0 - f.y : f.y);
                float dn = abs(m.b - z) / (0.05 * z + 0.3);
                best = min(best, dn);
                float w = wb / (1e-3 + dn) + 1e-6;
                sum += texelFetch(airTex, tc, 0) * w;
                msum += m.rg * w;
                if (airBeams.x > 0.5) esum += texelFetch(airMeanTex, tc, 0).rgb * w;
                wsum += w;
            }
        }
        air = sum / wsum;
        mist = msum / wsum;
        mean = esum / wsum;""")
rep(sh, """        vec3 airAmb = mix(zen * (skyLum * 0.55), skyLim, mix(1.0, vis, sunOn));""",
    """        vec3 airAmb = mix(zen * (skyLum * kAirHazeShadowAmbient), skyLim, mix(1.0, vis, sunOn));""")
rep(sh, """const char kPostAirApplyMainGLSL[] = R"(
// The sky the haze fades toward:""", """const char kPostAirApplyMainGLSL[] = R"(
const float kAirHazeShadowAmbient = 0.55; // kHazeShadowAmbient

// The sky the haze fades toward:""")
rep(sh, """    // Beams: the local contrast of the scattering (what stands above its
    // depth-matched mean is boosted by gain - 1, the smooth glow loses up to
    // the high-pass share, never below zero), added like light on a screen.
    vec3 glow = vec3(0.0);
    if (airBeams.x > 0.5 && airHalf.z > 0.5) {
        vec2 shaft = air.rg;
        // No half-resolution tap matched this pixel's depth: the upsample
        // borrowed a neighbour's (the sky's) scattering. Clamp it to the mean
        // and give it no boost, so crowns against the glow get no bright rim.
        shaft = mix(min(shaft, mean), shaft, conf);
        float boost = (airBeams.z - 1.0) * conf;
        vec2 floorAt = (1.0 - airBeams.y) * shaft;
        shaft = max(shaft - airBeams.y * mean, floorAt) + boost * max(shaft - mean, vec2(0.0));
        glow = airSunColor.rgb * shaft.x + airSpotTint.rgb * shaft.y;""", """    // Beams: the local contrast of the scattering (what stands above its
    // depth-matched mean is boosted by gain - 1, the smooth glow loses up to
    // the high-pass share, never below zero), added like light on a screen.
    // For the sun the contrast works on the share of the scattering that sees
    // the sun (AirMedium.cpp's BeamShaft): a ray that only runs longer through
    // the ground mist, as over open ground toward the horizon, stands above
    // its mean in scattering but not in that share, so it gets no boost (no
    // band). The headlight cones are their own contrast.
    vec3 glow = vec3(0.0);
    if (airBeams.x > 0.5 && airHalf.z > 0.5) {
        vec2 shaft = vec2(clamp(air.r / max(air.a, 1e-7), 0.0, 1.0), air.g);
        vec2 local = vec2(clamp(mean.x / max(mean.z, 1e-7), 0.0, 1.0), mean.y);
        // No half-resolution tap matched this pixel's depth: the upsample
        // borrowed a neighbour's (the sky's) scattering. Clamp it to the mean
        // and give it no boost, so crowns against the glow get no bright rim.
        shaft = mix(min(shaft, local), shaft, conf);
        float boost = (airBeams.z - 1.0) * conf;
        vec2 floorAt = (1.0 - airBeams.y) * shaft;
        shaft = max(shaft - airBeams.y * local, floorAt) + boost * max(shaft - local, vec2(0.0));
        glow = airSunColor.rgb * (shaft.x * air.a) + airSpotTint.rgb * shaft.y;""")
print("ok")
