"""Probe 8: depth-confidence edge fix for the beam-contrast composite (knob volEdge)."""
import sys

root = sys.argv[1]  # ...\arma_CWR\engine


def patch(rel, pairs):
    p = root + '\\' + rel
    s = open(p, encoding='utf-8').read()
    for old, new in pairs:
        assert s.count(old) == 1, (rel, old[:80])
        s = s.replace(old, new)
    open(p, 'w', encoding='utf-8', newline='').write(s)
    print('patched', rel)


patch('Poseidon\\Graphics\\Remaster\\LightRig.hpp', [(
    "    float volSpot = 0.0f;",
    "    float volEdge = 0.0f;        // 1: gain path uses full-res depth and distrusts poorly matched upsample taps\n"
    "    float volSpot = 0.0f;")])
patch('Poseidon\\Game\\Commands\\GameStateExtTestRender.cpp', [(
    '        {"volSpot", &remaster::LightingTuning::volSpot},\n',
    '        {"volSpot", &remaster::LightingTuning::volSpot},\n'
    '        {"volEdge", &remaster::LightingTuning::volEdge},\n')])
patch('PoseidonGL33\\EngineGL33_PostFX.cpp', [
    ("uniform vec4 volHp;      // RESEARCH: x high-pass strength (0 = off), y radius (uv of the frame height), z aspect w/h\n",
     "uniform vec4 volHp;      // RESEARCH: x high-pass strength (0 = off), y radius (uv of the frame height), z aspect w/h\n"
     "uniform vec4 volHp2;     // RESEARCH (probe 8): x 1 = edge fix (full-res depth for the mean, distrust poor upsample matches)\n"),
    ("""vec3 volumetricAt(vec2 uv) {
    float d = texture(fullDepthTex, uv).r;
    float z = d >= 0.999999 ? 1e5 : min(volProj.y / ((d * volProj.z + volProj.w) - volProj.x), 1e5);""",
     """float volZFull;  // this pixel's view depth (set by volumetricAt)
float volConf;   // 1 = some half-res tap matched this pixel's depth, 0 = none did
vec3 volumetricAt(vec2 uv) {
    float d = texture(fullDepthTex, uv).r;
    float z = d >= 0.999999 ? 1e5 : min(volProj.y / ((d * volProj.z + volProj.w) - volProj.x), 1e5);
    volZFull = z;
    float best = 1e9;"""),
    ("""            float wd = 1.0 / (1e-3 + abs(s.a - z) / (0.05 * z + 0.3));""",
     """            float dn = abs(s.a - z) / (0.05 * z + 0.3);
            best = min(best, dn);
            float wd = 1.0 / (1e-3 + dn);"""),
    ("""    return sum / wsum;
}""", """    volConf = 1.0 - smoothstep(0.5, 2.0, best);
    return sum / wsum;
}"""),
    ("""            float zc = texelFetch(shaftTex, clamp(ivec2(vUV * volUp.zw), ivec2(0), ivec2(volUp.zw) - 1), 0).a;""",
     """            float zc = volHp2.x > 0.5 ? volZFull : texelFetch(shaftTex, clamp(ivec2(vUV * volUp.zw), ivec2(0), ivec2(volUp.zw) - 1), 0).a;"""),
    ("""            vec3 base = max(shaft - volHp.x * mean, (1.0 - volHp.x) * shaft);
            shaft = base + (volHp.w - 1.0) * max(shaft - mean, vec3(0.0));""",
     """            float boost = volHp.w - 1.0;
            if (volHp2.x > 0.5) {
                // Thin foliage: when no half-res tap matched this pixel's
                // depth, the upsample borrowed the sky's (far brighter)
                // scattering. Clamp it to the depth-matched mean and give it
                // no boost, so crowns against the glow get no speckled rim.
                shaft = mix(min(shaft, mean), shaft, volConf);
                boost *= volConf;
            }
            vec3 base = max(shaft - volHp.x * mean, (1.0 - volHp.x) * shaft);
            shaft = base + boost * max(shaft - mean, vec3(0.0));"""),
    ("""        _lighting.volGain);""", """        _lighting.volGain);
    u4f(p.progComposite, "volHp2", _lighting.volEdge, 0.0f, 0.0f, 0.0f);"""),
])
