import os
os.chdir(r'D:\Arma_CWA_worktrees\look-fog\arma_CWR')


def edit(p, pairs):
    s = open(p, encoding='utf-8', newline='').read()
    for a, b in pairs:
        assert s.count(a) == 1, (p, a[:80], s.count(a))
        s = s.replace(a, b)
    open(p, 'w', encoding='utf-8', newline='').write(s)


sh = 'engine/PoseidonGL33/EngineGL33_PostFogShaders.cpp'
edit(sh, [
    # ring: sun avoidance, fixed band, no HDR expansion of the disc
    ('''uniform vec4 ringCtl;        // x bins, y the decay per frame out of view
out vec4 fragColor;''', '''uniform vec4 ringCtl;        // x bins, y the decay per frame out of view
uniform vec4 ringSun;        // xyz the direction to the sun (world)
out vec4 fragColor;'''),
    ('''    vec4 outc = vec4(prev.rgb, prev.a * ringCtl.y);
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
        }''', '''    vec4 outc = vec4(prev.rgb, prev.a * ringCtl.y);
    if (found > -99.0) {
        // Samples from two degrees up (the band above the land, the same
        // height in every azimuth), the darkest of them, and none within five
        // degrees of the sun: the sky's colour, not the sun's disc and glare
        // or a star.
        float base = row == 0 ? max(found, radians(2.0)) : max(found + radians(1.5), radians(8.0));
        vec3 best = vec3(-1.0);
        float bestL = 1e9;
        for (int k = 0; k < 6; ++k) {
            vec3 dd = ringDir(az, base + radians(0.6 + 1.0 * float(k)));
            if (dot(dd, ringSun.xyz) > 0.9962) continue;
            vec3 p = ringProject(dd);
            if (p.z < 0.5 || textureLod(depthTex, p.xy, 0.0).r < 0.999999) continue;
            vec3 c = pow(clamp(textureLod(colorTex, p.xy, 0.0).rgb, 0.0, 1.0), vec3(2.2));
            float l = dot(c, vec3(0.2126, 0.7152, 0.0722));
            if (l < bestL) { bestL = l; best = c; }
            if (k >= 2 && best.r >= 0.0) break;
        }'''),
    # injection: blurred ring, shadow keep
    ('''    float az = atan(d.x, d.z) * (1.0 / 6.2831853);
    vec4 ring0 = texture(fogRing, vec2(az, 0.25));
    vec4 ring1 = texture(fogRing, vec2(az, 0.75));''', '''    // Five taps over 17 degrees of azimuth: the sky's broad colour, not the
    // gaps between crowns.
    float az = atan(d.x, d.z) * (1.0 / 6.2831853);
    vec4 ring0 = vec4(0.0), ring1 = vec4(0.0);
    for (int k = -2; k <= 2; ++k) {
        float w = 3.0 - abs(float(k));
        float u = az + float(k) * (3.0 / 256.0);
        ring0 += w * texture(fogRing, vec2(u, 0.25));
        ring1 += w * texture(fogRing, vec2(u, 0.75));
    }
    ring0 /= 9.0;
    ring1 /= 9.0;
    ring0.rgb /= max(ring0.a, 1e-4);
    ring1.rgb /= max(ring1.a, 1e-4);'''),
    ('''    float lit = (1.0 - g_sunShare) + g_sunShare * vis * tSun;
    // Dense mist and fog glow in their own shade''', '''    // Air in the shade keeps a third of its light at least: the sky and the
    // light scattered more than once (a ridge's shadow in the haze stays
    // grey, not black).
    float share = min(g_sunShare, 0.65);
    float lit = (1.0 - share) + share * vis * tSun;
    // Dense mist and fog glow in their own shade'''),
])
# the ring stores premultiplied-by-confidence? no: ring rgb is the colour, a the confidence; weighted sum above
# needs premultiplication: fix the weighting to multiply rgb by a
s = open(sh, encoding='utf-8', newline='').read()
s = s.replace('''        ring0 += w * texture(fogRing, vec2(u, 0.25));
        ring1 += w * texture(fogRing, vec2(u, 0.75));''', '''        vec4 a = texture(fogRing, vec2(u, 0.25));
        vec4 b = texture(fogRing, vec2(u, 0.75));
        ring0 += w * vec4(a.rgb * a.a, a.a);
        ring1 += w * vec4(b.rgb * b.a, b.a);''')
open(sh, 'w', encoding='utf-8', newline='').write(s)

edit('engine/PoseidonGL33/EngineGL33_PostFog.cpp', [
    ('''        Set4(ring, "ringCtl", static_cast<float>(kFogRingBins), kFogRingDecay, 0.0f, 0.0f);''',
     '''        Set4(ring, "ringCtl", static_cast<float>(kFogRingBins), kFogRingDecay, 0.0f, 0.0f);
        const Vector3 toSun = -sun->SunDirection();
        Set4(ring, "ringSun", toSun.X(), toSun.Y(), toSun.Z(), 0.0f);'''),
])

edit('engine/Poseidon/Graphics/Remaster/VolumetricFog.hpp', [
    ('''constexpr float kRayleighScale = 5.0f;''', '''constexpr float kRayleighScale = 3.5f;'''),
    ('''constexpr float kFogBeamGain = 2.0f;''', '''constexpr float kFogBeamGain = 5.0f;'''),
    ('''constexpr float kMistNearKeep = 0.3f;
constexpr float kMistNearFrom = 15.0f;
constexpr float kMistNearTo = 150.0f;''', '''constexpr float kMistNearKeep = 0.15f;
constexpr float kMistNearFrom = 20.0f;
constexpr float kMistNearTo = 300.0f;'''),
    ('''constexpr float kGroundHaze = 1.0e-3f;''', '''constexpr float kGroundHaze = 8.0e-4f;'''),
])
edit(sh, [
    ('''const float kMistNearKeep = 0.3;
const float kMistNearFrom = 15.0;
const float kMistNearTo = 150.0;''', '''const float kMistNearKeep = 0.15;
const float kMistNearFrom = 20.0;
const float kMistNearTo = 300.0;'''),
])
edit('tests/unit/engine/Poseidon/Graphics/test_volumetric_fog.cpp', [
    ('''    CHECK(CountOf(inject, "const float kMistNearKeep = 0.3;") == 1);
    CHECK(kMistNearKeep == 0.3f);''', '''    CHECK(CountOf(inject, "const float kMistNearKeep = 0.15;") == 1);
    CHECK(kMistNearKeep == 0.15f);'''),
])
p = 'tests/unit/engine/Poseidon/Graphics/test_remaster_lighting.cpp'
s = open(p, encoding='utf-8', newline='').read()
s = s.replace('    // injection, integration, near field and apply\n    CHECK(post == 23);', '    // injection, integration, near field, apply and horizon ring\n    CHECK(post == 24);')
s = s.replace('    // injection, integration, near field and apply\r\n    CHECK(post == 23);', '    // injection, integration, near field, apply and horizon ring\r\n    CHECK(post == 24);')
assert 'CHECK(post == 24);' in s
open(p, 'w', encoding='utf-8', newline='').write(s)
print('ok')
