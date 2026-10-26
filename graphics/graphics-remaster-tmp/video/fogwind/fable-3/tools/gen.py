#!/usr/bin/env python
"""fable-3: generate psFogInject.glsl overrides from the d91a08e0 per-cell shader
(tmp/video/mist/1/shaders/percell/psFogInject.glsl, the whole assembled shader).

Variants (deterministic, "robust by construction": no temporal jitter anywhere):
  det1    ONE tap per cell at the slice mid and the cell centre, no jitter in depth or xy; the
          0.12 history read at the same (centre) point = a plain EMA smoother (agent 6's centre read
          falls out for free, the sample IS the centre).
  det1n   det1 + the mist top's noise (n1, n2 of fogMediumAt) band-limited along the slice: the mean of
          two lookups at 1/4 and 3/4 of the slice's ray span (cloudNoise has no mips; its finest
          octave is 5 m at the 310 m scale, below a far slice's 40..300 m span).
  det1nb  det1n + Act25's fixed tetrahedral history read (+-0.25 cell/slice): the shipped look's
          blur without its noise.
  det1nq  det1n + the noise also band-limited ACROSS the cell (the mean over the cell's footprint:
          2 x 2 lookups at +-0.25 cell in x and y, times 2 along the slice = 4 lookups per octave).

usage: python gen.py <variant> [<variant> ...]   -> fable-3/shaders/<variant>/psFogInject.glsl
"""
import os
import sys

BASE = "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/mist/1/shaders/percell/psFogInject.glsl"
OUT = "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/fogwind/fable-3/shaders"


def replace_once(src, old, new, what):
    n = src.count(old)
    if n != 1:
        raise SystemExit("%s: expected 1 occurrence, found %d" % (what, n))
    return src.replace(old, new)


OLD_SLICE = """float fogIgn(vec2 p) { return fract(52.9829189 * fract(dot(p, vec2(0.06711056, 0.00583715)))); }
float g_cellJ = 0.0;
vec4 fogSlice(float s, vec3 dirZ) {
    float z = fogSliceDepth((s + fract(fogDepth.w + g_cellJ + 0.618034 * s)) / fogGrid.z);
    vec3 rel = dirZ * z;
    vec4 cur = fogCell(rel);
"""

NEW_SLICE = """// fable-3 det: one deterministic tap per cell at the slice mid on the cell-centre ray, no jitter.
// The history is read at the same point (the centre), so the blend is a plain EMA, not the shipped
// leaky sum of jitter offsets. g_span: half the slice's ray span (for the band-limited noise).
vec4 fogSlice(float s, vec3 dirZ) {
    float z0 = fogSliceDepth(s / fogGrid.z);
    float z1 = fogSliceDepth((s + 1.0) / fogGrid.z);
    float z = 0.5 * (z0 + z1);
    g_span = dirZ * (0.5 * (z1 - z0));
    vec3 rel = dirZ * z;
    vec4 cur = fogCell(rel);
"""

OLD_MAIN = """    g_cellJ = fogIgn(gl_FragCoord.xy);
    vec2 cj = fract(fogFrame.xy + 0.5 + vec2(fogIgn(gl_FragCoord.xy + vec2(17.0, 59.0)), fogIgn(gl_FragCoord.xy + vec2(83.0, 29.0)))) - 0.5;
    vec2 ndc = (gl_FragCoord.xy + cj) / fogGrid.xy * 2.0 - 1.0;
"""

NEW_MAIN = """    // fable-3 det: the cell-centre ray, no xy jitter (gl_FragCoord sits at the cell's centre).
    vec2 ndc = gl_FragCoord.xy / fogGrid.xy * 2.0 - 1.0;
    g_cellStep = vec2(2.0 / fogGrid.x * projInfo.z, 2.0 / fogGrid.y * projInfo.w);
"""

OLD_MEDIUM_HEAD = "FogMediumCell fogMediumAt(vec3 rel) {\n"
NEW_MEDIUM_HEAD = """vec3 g_span = vec3(0.0);     // half the slice's ray span (per metre of view depth times the span)
vec2 g_cellStep = vec2(0.0); // one cell in view-space direction units (x, y) per metre of depth
FogMediumCell fogMediumAt(vec3 rel) {
"""

OLD_NOISE = """        vec2 q = p.xz + fogWind.xy;
        float n1 = texture(cloudNoise, q * (1.0 / 1100.0) + vec2(0.37, 0.11)).r;
        float n2 = texture(cloudNoise, q * (1.0 / 310.0) + vec2(0.71, 0.53) + p.y * 0.002).r;
"""

NEW_NOISE_Z = """        // fable-3 det1n: the noise band-limited along the slice (two lookups at 1/4 and 3/4 of
        // the slice's span), so a far slice holds the layer top's mean over its span, not one
        // point of a 5 m octave.
        vec3 pa = p - 0.5 * g_span, pb = p + 0.5 * g_span;
        vec2 qa = pa.xz + fogWind.xy, qb = pb.xz + fogWind.xy;
        float n1 = 0.5 * (texture(cloudNoise, qa * (1.0 / 1100.0) + vec2(0.37, 0.11)).r +
                          texture(cloudNoise, qb * (1.0 / 1100.0) + vec2(0.37, 0.11)).r);
        float n2 = 0.5 * (texture(cloudNoise, qa * (1.0 / 310.0) + vec2(0.71, 0.53) + pa.y * 0.002).r +
                          texture(cloudNoise, qb * (1.0 / 310.0) + vec2(0.71, 0.53) + pb.y * 0.002).r);
"""

NEW_NOISE_ZQ = """        // fable-3 det1nq: the noise band-limited along the slice (1/4, 3/4 of its span) and across
        // the cell (+-0.25 cell in x and y): the layer top's mean over the froxel.
        float n1 = 0.0, n2 = 0.0;
        for (int i = 0; i < 2; ++i) {
            vec3 pz = p + (float(i) - 0.5) * g_span;
            for (int j = 0; j < 2; ++j) {
                vec3 pq = pz + g_cellLat[j] * length(rel);
                vec2 qq = pq.xz + fogWind.xy;
                n1 += texture(cloudNoise, qq * (1.0 / 1100.0) + vec2(0.37, 0.11)).r;
                n2 += texture(cloudNoise, qq * (1.0 / 310.0) + vec2(0.71, 0.53) + pq.y * 0.002).r;
            }
        }
        n1 *= 0.25;
        n2 *= 0.25;
"""

# For det1nq: two lateral offsets (world units per metre of distance) set once per column.
NEW_MEDIUM_HEAD_Q = """vec3 g_span = vec3(0.0);     // half the slice's ray span (per metre of view depth times the span)
vec2 g_cellStep = vec2(0.0); // one cell in view-space direction units (x, y) per metre of depth
vec3 g_cellLat[2];           // +-0.25 cell across (a diagonal pair), world units per metre of distance
FogMediumCell fogMediumAt(vec3 rel) {
"""
NEW_MAIN_Q = NEW_MAIN + """    {
        vec3 ax = viewToWorld * vec3(g_cellStep.x, 0.0, 0.0);
        vec3 ay = viewToWorld * vec3(0.0, g_cellStep.y, 0.0);
        g_cellLat[0] = 0.25 * (ax + ay);
        g_cellLat[1] = -0.25 * (ax + ay);
    }
"""

OLD_MIST_BLOCK = """    if (fogMist.x > 0.0 && depth > 0.5) {
        // The layer's top rises and falls with two drifting noise octaves; the
        // banks thicken and thin it.
        vec2 q = p.xz + fogWind.xy;
        float n1 = texture(cloudNoise, q * (1.0 / 1100.0) + vec2(0.37, 0.11)).r;
        float n2 = texture(cloudNoise, q * (1.0 / 310.0) + vec2(0.71, 0.53) + p.y * 0.002).r;
        float top = floorY + depth * mix(0.6, 1.35, n1) + (n2 - 0.5) * 0.35 * depth;
        float above = p.y - top;
        float banks = mix(1.0, 1.7 * smoothstep(0.2, 0.85, n1 * 0.6 + n2 * 0.4), fogMist.w);
        float shape = above <= 0.0 ? 1.0 : exp(-above / fogMist.z);
        sMist = fogMist.x * shape * banks * nearKeep;
        mistTau = fogMist.x * banks * (max(-above, 0.0) + fogMist.z * min(shape, 1.0));
    }
"""

NEW_MIST_BLOCK_4 = """    if (fogMist.x > 0.0 && depth > 0.5) {
        // fable-3 det1n4: the mist's density is the MEAN over 4 strata along the slice's span
        // (the layer top's noise has 5 m detail at the 310 m lookup, a far slice is 35..300 m
        // long), so a cell holds the slice's mean mist by construction, not one point of it.
        // The smooth parts (haze, weather, humid air, the ramps) stay at the slice mid: a fixed
        // mid tap is exact to < 1% on them.
        float sAcc = 0.0, tauAcc = 0.0;
        for (int k = 0; k < 4; ++k) {
            vec3 pk = p + (float(k) - 1.5) * 0.5 * g_span;
            vec2 q = pk.xz + fogWind.xy;
            float n1 = texture(cloudNoise, q * (1.0 / 1100.0) + vec2(0.37, 0.11)).r;
            float n2 = texture(cloudNoise, q * (1.0 / 310.0) + vec2(0.71, 0.53) + pk.y * 0.002).r;
            float top = floorY + depth * mix(0.6, 1.35, n1) + (n2 - 0.5) * 0.35 * depth;
            float above = pk.y - top;
            float banks = mix(1.0, 1.7 * smoothstep(0.2, 0.85, n1 * 0.6 + n2 * 0.4), fogMist.w);
            float shape = above <= 0.0 ? 1.0 : exp(-above / fogMist.z);
            sAcc += shape * banks;
            tauAcc += banks * (max(-above, 0.0) + fogMist.z * min(shape, 1.0));
        }
        sMist = fogMist.x * 0.25 * sAcc * nearKeep;
        mistTau = fogMist.x * 0.25 * tauAcc;
    }
"""

OLD_HIST = """                cur = mix(texture(fogHistory, vec3(puv, pt)), cur, fogFrame.z);
"""
NEW_HIST_TETRA = """            {
                // Act25's fixed tetrahedral history read (+-0.25 cell/slice): the shipped look's
                // blur (the mean of its jittered-point read) without its noise.
                vec3 tq = vec3(puv, pt), te = vec3(0.25) / fogGrid.xyz;
                vec4 hist = 0.25 * (texture(fogHistory, tq + te * vec3(1.0, 1.0, 1.0)) + texture(fogHistory, tq + te * vec3(1.0, -1.0, -1.0)) +
                                    texture(fogHistory, tq + te * vec3(-1.0, 1.0, -1.0)) + texture(fogHistory, tq + te * vec3(-1.0, -1.0, 1.0)));
                cur = mix(hist, cur, fogFrame.z);
            }
"""


def build(variant):
    src = open(BASE, encoding="utf-8").read()
    if variant == "det1nq":
        src = replace_once(src, OLD_MEDIUM_HEAD, NEW_MEDIUM_HEAD_Q, "medium head")
        src = replace_once(src, OLD_MAIN, NEW_MAIN_Q, "main jitter")
        src = replace_once(src, OLD_NOISE, NEW_NOISE_ZQ, "noise")
    elif variant in ("det1p", "det1pb", "det1pr", "det1prb"):
        # by construction: the sun's visibility band-limited to the froxel: the mean over 4 points
        # of the cell (a quincunx: +-0.25 cell across, +-0.25 slice along), the medium itself still
        # one mid tap. (The shipped jitter + history gave the air soft shadows; a fixed tap reads
        # the shadow map and the horizon map hard, per column: frozen's and det1's streaks.)
        src = replace_once(src, OLD_MEDIUM_HEAD, NEW_MEDIUM_HEAD_Q, "medium head")
        src = replace_once(src, OLD_MAIN, NEW_MAIN_Q, "main jitter")
        src = replace_once(src, "        vis = fogSunVisibility(rel);\n",
                           """        {
            // fable-3 det1p: vis = the mean of 4 points over the froxel's footprint.
            float dist = length(rel);
            vec3 la = g_cellLat[0] * dist, lb = g_cellLat[1] * dist;
            vis = 0.25 * (fogSunVisibility(rel + la + 0.5 * g_span) + fogSunVisibility(rel - la - 0.5 * g_span) +
                          fogSunVisibility(rel + lb - 0.5 * g_span) + fogSunVisibility(rel - lb + 0.5 * g_span));
        }
""", "vis")
    elif variant == "det1s":
        # attribution: only the shadow map off in the froxels (canopy, terrain horizon, clouds kept).
        src = replace_once(src, OLD_MEDIUM_HEAD, NEW_MEDIUM_HEAD, "medium head")
        src = replace_once(src, OLD_MAIN, NEW_MAIN, "main jitter")
        src = replace_once(src, "    return fogShadowMap(rel) * canopySunVisibility(p) * fogTerrainSun(p, g_fogAbove) * fogCloudSun(p);\n",
                           "    return canopySunVisibility(p) * fogTerrainSun(p, g_fogAbove) * fogCloudSun(p); // fable-3 det1s: no shadow map\n", "sunvis")
    else:
        src = replace_once(src, OLD_MEDIUM_HEAD, NEW_MEDIUM_HEAD, "medium head")
        src = replace_once(src, OLD_MAIN, NEW_MAIN, "main jitter")
        if variant in ("det1n", "det1nb"):
            src = replace_once(src, OLD_NOISE, NEW_NOISE_Z, "noise")
        if variant in ("det1n4", "det1n4b"):
            src = replace_once(src, OLD_MIST_BLOCK, NEW_MIST_BLOCK_4, "mist block")
    if variant in ("det1pr", "det1prb"):
        # by construction: the horizon ring read smoothly per column: 17 taps one bin apart under a
        # triangle of the same 8-bin half-width (the shipped 9 taps two bins apart leave the bins'
        # kinks at 1.4 deg = 2.6 columns, which a fixed tap shows as low-sky streaks: det1r).
        src = replace_once(src, """    for (int k = -4; k <= 4; ++k) {
        float w = 5.0 - abs(float(k));
        float u = az + float(k) * (2.0 / 256.0);
""", """    for (int k = -8; k <= 8; ++k) { // fable-3 det1pr: every bin, same support
        float w = 9.0 - abs(float(k));
        float u = az + float(k) * (1.0 / 256.0);
""", "ring loop")
        src = replace_once(src, "        ringSure = max(ringSure, a.a * (1.0 - 0.1 * abs(float(k))));\n",
                           "        ringSure = max(ringSure, a.a * (1.0 - 0.05 * abs(float(k))));\n", "ringSure")
        src = replace_once(src, "    ring /= 25.0;\n", "    ring /= 81.0;\n", "ring norm")
    if variant == "det1r":
        # attribution: the horizon ring's colour off (the column's air keeps the model's colour).
        src = replace_once(src, "    g_air = mix(g_air, ring.rgb, clamp(ringSure, 0.0, 1.0) * (1.0 - fogNightAir.w)) * fogLightK.y;\n",
                           "    g_air = g_air * fogLightK.y; // fable-3 det1r: attribution, no ring\n", "ring mix")
    if variant == "det1g":
        # attribution: the humid ground air and the mist off (the haze, the Rayleigh air and the ring
        # kept), so whatever streaks remain come from the clear air's own terms.
        src = replace_once(src, "    c.wet = sGround + sWeather + sMist;\n", "    c.wet = sWeather; // fable-3 det1g: attribution, no ground air, no mist\n", "wet")
        src = replace_once(src, "    c.dense = sWeather + sMist;\n", "    c.dense = sWeather;\n", "dense")
        src = replace_once(src, "    c.tauUp = mistTau + fogWeather.x * fogWeather.y * wExp;\n", "    c.tauUp = fogWeather.x * fogWeather.y * wExp;\n", "tauUp")
    if variant == "det1v":
        # attribution: the sun's visibility terms (shadow map, canopy, terrain horizon, clouds) off in
        # the froxels (vis = 1), everything else det1.
        src = replace_once(src, "        vis = fogSunVisibility(rel);\n", "        vis = 1.0; // fable-3 det1v: attribution, no sun-visibility read\n", "vis")
    src = replace_once(src, OLD_SLICE, NEW_SLICE, "fogSlice")
    if variant in ("det1nb", "det1n4b", "det1pb", "det1prb"):
        src = replace_once(src, OLD_HIST, NEW_HIST_TETRA, "history read")
    d = os.path.join(OUT, variant)
    os.makedirs(d, exist_ok=True)
    out = os.path.join(d, "psFogInject.glsl")
    open(out, "w", encoding="utf-8", newline="\n").write(src)
    print("wrote", out, len(src.splitlines()), "lines")


if __name__ == "__main__":
    for v in sys.argv[1:] or ["det1", "det1n", "det1nb"]:
        build(v)
