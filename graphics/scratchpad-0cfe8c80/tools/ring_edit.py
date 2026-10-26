import os
os.chdir(r'D:\Arma_CWA_worktrees\look-fog\arma_CWR')


def edit(p, pairs):
    s = open(p, encoding='utf-8', newline='').read()
    for a, b in pairs:
        assert s.count(a) == 1, (p, a[:80], s.count(a))
        s = s.replace(a, b)
    open(p, 'w', encoding='utf-8', newline='').write(s)


RING = r'''// The horizon ring: the sky's own colour just above the land in every
// azimuth, measured from the resolved frame (before the fog) and kept for
// the azimuths out of view. Two rows of kFogRingBins texels, u = the azimuth
// from north (clockwise) over a full turn: row 0 the sky just above the
// silhouette (the lowest sky in that azimuth, the limit far ground melts
// into), row 1 the sky about 8 degrees up (the Rayleigh air's light); rgb
// linear (the world's shoulder undone), a how sure (1 measured this frame,
// decaying while out of view; 0 never seen: the model's colour then).
const char kPostFogRingGLSL[] = R"(#version 330 core
in vec2 vUV;
uniform sampler2D colorTex;  // unit 0: the resolved frame (display)
uniform sampler2D ringPrev;  // unit 3: last frame's ring
uniform sampler2D depthTex;  // unit 5: the world depth
uniform vec4 ringView0;      // camera-relative world to view space, by rows
uniform vec4 ringView1;
uniform vec4 ringView2;
uniform vec4 ringProj;       // x _11, y _22, z 1 = start again (a camera jump), w this frame's share
uniform vec4 worldRect;      // the 3D viewport in uv
uniform vec4 ringCtl;        // x bins, y the decay per frame out of view
out vec4 fragColor;

vec3 ringExpand(vec3 lin) {
    float peak = max(lin.r, max(lin.g, lin.b));
    if (peak <= 0.8) return lin;
    float hdr = 0.6 + 0.04 / (1.0 - min(peak, 0.995));
    return lin * (hdr / peak);
}

// The frame's uv of a world direction, w 0 when it is off screen.
vec3 ringProject(vec3 d) {
    vec3 v = vec3(dot(ringView0.xyz, d), dot(ringView1.xyz, d), dot(ringView2.xyz, d));
    if (v.z < 0.05) return vec3(0.0);
    vec2 ndc = vec2(v.x * ringProj.x, v.y * ringProj.y) / v.z;
    if (abs(ndc.x) > 0.985 || abs(ndc.y) > 0.985) return vec3(0.0);
    return vec3(worldRect.xy + (ndc * 0.5 + 0.5) * worldRect.zw, 1.0);
}

vec3 ringDir(float az, float el) {
    return vec3(sin(az) * cos(el), sin(el), cos(az) * cos(el));
}

void main() {
    int row = int(gl_FragCoord.y);
    float az = (gl_FragCoord.x / ringCtl.x) * 6.2831853;
    vec4 prev = texelFetch(ringPrev, ivec2(gl_FragCoord.xy), 0);
    if (ringProj.z > 0.5) prev = vec4(0.0);
    // From 3 degrees under the horizon upward: the first sky in this azimuth.
    float found = -100.0;
    for (int i = 0; i < 70; ++i) {
        float el = radians(-3.0 + 0.5 * float(i));
        vec3 p = ringProject(ringDir(az, el));
        if (p.z < 0.5) continue;
        if (textureLod(depthTex, p.xy, 0.0).r >= 0.999999) {
            found = el;
            break;
        }
    }
    vec4 outc = vec4(prev.rgb, prev.a * ringCtl.y);
    if (found > -99.0) {
        // Three samples above it, the darkest: not the sun's disc or a star.
        float base = row == 0 ? found : max(found + radians(1.5), radians(8.0));
        vec3 best = vec3(-1.0);
        float bestL = 1e9;
        for (int k = 0; k < 3; ++k) {
            vec3 p = ringProject(ringDir(az, base + radians(0.6 + 0.8 * float(k))));
            if (p.z < 0.5 || textureLod(depthTex, p.xy, 0.0).r < 0.999999) continue;
            vec3 c = ringExpand(pow(clamp(textureLod(colorTex, p.xy, 0.0).rgb, 0.0, 1.0), vec3(2.2)));
            float l = dot(c, vec3(0.2126, 0.7152, 0.0722));
            if (l < bestL) { bestL = l; best = c; }
        }
        if (best.r >= 0.0)
            outc = vec4(prev.a > 0.0 ? mix(prev.rgb, best, ringProj.w) : best, 1.0);
    }
    fragColor = outc;
}
)";

'''

edit('engine/PoseidonGL33/EngineGL33_PostFogShaders.cpp', [
    ('''// The apply: full resolution, the frame after the occlusion pass's inputs.''', RING + '''// The apply: full resolution, the frame after the occlusion pass's inputs.'''),
    # shared inputs: the ring
    ('''uniform vec4 fogNightAir;   // rgb the night air's light (linear), w its weight (the night)
)";''', '''uniform vec4 fogNightAir;   // rgb the night air's light (linear), w its weight (the night)
uniform sampler2D fogRing;  // unit 13: the horizon ring (kPostFogRingGLSL)
)";'''),
    # inject main: use the ring
    ('''    g_air = mix(toLinear(disp), fogNightAir.rgb, fogNightAir.w) * fogLightK.y;
    g_ray = mix(toLinear(dispUp), fogNightAir.rgb, fogNightAir.w) * fogLightK.y;''',
     '''    g_air = mix(toLinear(disp), fogNightAir.rgb, fogNightAir.w);
    g_ray = mix(toLinear(dispUp), fogNightAir.rgb, fogNightAir.w);
    // Where the ring has seen the sky in this azimuth, the sky's own colour:
    // far ground converges to exactly the sky behind it, day and night.
    float az = atan(d.x, d.z) * (1.0 / 6.2831853);
    vec4 ring0 = texture(fogRing, vec2(az, 0.25));
    vec4 ring1 = texture(fogRing, vec2(az, 0.75));
    g_air = mix(g_air, ring0.rgb, clamp(ring0.a, 0.0, 1.0)) * fogLightK.y;
    g_ray = mix(g_ray, ring1.rgb, clamp(ring1.a, 0.0, 1.0)) * fogLightK.y;'''),
    # beams: gain and soft clip
    ('''        float lobe = kFogPi * max(fogMiePhase(dot(d, litSunDir.xyz)) - kFogIso, 0.0);
        S += fogSunRadiance() * fogNearSun.rgb * (lobe * acc);''',
     '''        float lobe = kFogPi * max(fogMiePhase(dot(d, litSunDir.xyz)) - kFogIso, 0.0);
        vec3 beam = fogSunRadiance() * fogNearSun.rgb * (lobe * acc);
        // Rolled off toward the sun's own glow, so the lobe never whitens the
        // sky around the disc into a blob while the shafts stay linear.
        float bl = dot(beam, kFogLuma);
        S += beam / (1.0 + bl / 0.45);'''),
])

# C++: units and pass
edit('engine/PoseidonGL33/EngineGL33_Lighting.hpp', [
    ('''constexpr int kFogFloorUnit = kLightDataUnit;''', '''constexpr int kFogFloorUnit = kLightDataUnit;
// The volumetric fog's horizon ring (EngineGL33_PostFog.cpp), read by its
// injection: a unit no world draw binds during the post pass.
constexpr int kFogRingUnit = 13;'''),
    ('''extern const char kPostFogNearMainGLSL[];''', '''extern const char kPostFogNearMainGLSL[];
extern const char kPostFogRingGLSL[];'''),
    ('''    FogInject,
    FogIntegrate,
    FogNear,
    FogApply,''', '''    FogInject,
    FogIntegrate,
    FogNear,
    FogApply,
    FogRing, // the horizon ring: the sky's colour above the land in every azimuth'''),
])
edit('engine/PoseidonGL33/EngineGL33_PostShaders.cpp', [
    ('''    {"psFogApply", {Poseidon::gl33lighting::kPostFogApplyGLSL}},''', '''    {"psFogApply", {Poseidon::gl33lighting::kPostFogApplyGLSL}},
    {"psFogRing", {Poseidon::gl33lighting::kPostFogRingGLSL}},'''),
])
print('ok')
