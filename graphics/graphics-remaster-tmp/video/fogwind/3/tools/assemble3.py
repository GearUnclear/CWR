"""Agent 3 (fogwind swarm): assemble psFogInject from the engine's C++ raw strings (as kPostShaders joins
them) and write multi-tap variants for CWR_SHADER_OVERRIDE_DIR.

usage: python assemble3.py <engine PoseidonGL33 dir> <out dir> [variant ...]
Writes <out dir>/<variant>/psFogInject.glsl.

The base source is the per-cell IGN fix (d91a08e0). Variants replace fogSlice and the column's
jitter in main:
  percell   unchanged (the shipped d91a08e0 shader)
  t4fix     4 taps per cell, a Latin hypercube: depth strata (k+0.5)/4 paired with a 4-rook xy pattern,
            the pairing rotated per slice; NO temporal jitter. The history blends at the cell centre.
  t8fix     the same with 8 taps (8 depth strata, 8-rook xy).
  t4cyc     t4fix, plus the frame's golden-ratio offset moving the whole 4-tap pattern within its strata
            (depth (k + jz)/4, xy rook + jxy/4): the history averages 4 x N positions, the residual is the
            4-tap estimate's change only.
  t2anti    2 antithetic taps per cell around the cell centre (depth c +- (jz - 0.5)/2.. as u and 1-u,
            xy +-j/2): first-order error cancels, frame-wide jitter kept.
"""
import os, re, sys

SRC = sys.argv[1]
OUT = sys.argv[2]
VARIANTS = sys.argv[3:] or ["percell"]

FILES = ["EngineGL33_PostFogShaders.cpp", "EngineGL33_LightingShaders.cpp", "EngineGL33_LightingShadersSky.cpp"]


def pieces():
    found = {}
    for fn in FILES:
        text = open(os.path.join(SRC, fn), encoding="utf-8").read()
        for m in re.finditer(r'const char (k\w+GLSL)\[\] = ', text):
            name = m.group(1)
            i = m.end()
            out = ""
            while True:
                while text[i] in " \t\r\n":
                    i += 1
                if text[i] == ";":
                    break
                if text.startswith('R"', i):
                    j = text.index("(", i)
                    delim = text[i + 2:j]
                    end = text.index(")" + delim + '"', j)
                    out += text[j + 1:end]
                    i = end + len(delim) + 2
                elif text[i] == '"':
                    j = i + 1
                    s = ""
                    while text[j] != '"':
                        if text[j] == "\\":
                            s += {"n": "\n", "t": "\t", '"': '"', "\\": "\\"}[text[j + 1]]
                            j += 2
                        else:
                            s += text[j]
                            j += 1
                    out += s
                    i = j + 1
                else:
                    raise SystemExit(f"unexpected token in {name}: {text[i:i+40]!r}")
            found[name] = out
    return found


P = pieces()
INJECT = ["kPostFogInjectHeadGLSL", "kPostFogSharedInputsGLSL", "kLitConstantsBlockGLSL", "kRemasterTerrainCloudsGLSL",
          "kPostFogSkyInputsGLSL", "kRemasterSkyGLSL", "kPostFogMediumGLSL", "kPostFogInjectMainGLSL"]
base = "".join(P[n] for n in INJECT)

# The per-cell fogSlice (whole function) and main's column jitter lines, as in d91a08e0.
i0 = base.index("vec4 fogSlice(float s, vec3 dirZ) {")
i1 = base.index("void main() {")
SLICE_FN = base[i0:i1]
MAIN_JIT = ("    g_cellJitter = fogCellNoise(gl_FragCoord.xy);\n"
            "    vec2 cellJ = fract(fogFrame.xy + 0.5 + vec2(fogCellNoise(gl_FragCoord.xy + vec2(17.0, 59.0)),\n"
            "                                                fogCellNoise(gl_FragCoord.xy + vec2(83.0, 29.0)))) - 0.5;\n"
            "    vec2 ndc = (gl_FragCoord.xy + cellJ) / fogGrid.xy * 2.0 - 1.0;\n"
            "    vec3 dirZ = viewToWorld * vec3(ndc.x * projInfo.z, ndc.y * projInfo.w, 1.0); // per metre of view depth\n")
assert base.count(MAIN_JIT) == 1, "main jitter block not found"
SLICE_CALLS = "".join(f"    fog{i} = fogSlice(base + {i}.0, dirZ);\n" for i in range(8))
assert base.count(SLICE_CALLS) == 1

# Shared multi-tap pieces: the history blend at a point, and per-tap column directions.
HIST = r"""
// The last frame's volume at a camera-relative point blended with this frame's estimate.
vec4 fogBlendHistory(vec4 cur, vec3 rel) {
    if (fogFrame.z < 1.0) {
        vec3 pv = vec3(dot(prevRow0.xyz, rel) + prevRow0.w, dot(prevRow1.xyz, rel) + prevRow1.w,
                       dot(prevRow2.xyz, rel) + prevRow2.w);
        if (pv.z > 0.05) {
            vec2 puv = pv.xy * prevProj.xy / pv.z * 0.5 + 0.5;
            float pt = prevProj.w < 1e-3 ? pv.z / prevProj.z
                                         : log(1.0 + pv.z * (exp(prevProj.w) - 1.0) / prevProj.z) / prevProj.w;
            if (puv.x >= 0.0 && puv.x <= 1.0 && puv.y >= 0.0 && puv.y <= 1.0 && pt <= 1.0)
                cur = mix(texture(fogHistory, vec3(puv, pt)), cur, fogFrame.z);
        }
    }
    return cur;
}
vec3 fogColumnDir(vec2 off) {
    vec2 ndc = (gl_FragCoord.xy + off) / fogGrid.xy * 2.0 - 1.0;
    return viewToWorld * vec3(ndc.x * projInfo.z, ndc.y * projInfo.w, 1.0);
}
"""


def multitap(n, cyc):
    # n-rook xy offsets (cells, in (-0.5, 0.5)): x stratum k, y stratum perm[k] (a rotated-grid style
    # permutation), depth stratum assigned per slice by a rotation.
    perm = {4: [1, 3, 0, 2], 8: [3, 6, 0, 5, 2, 7, 1, 4]}[n]
    decl = [f"vec3 g_tapDir[{n}];", "vec3 g_dirC = vec3(0.0);"]
    fn = HIST + "\n".join(decl) + "\n"
    fn += "vec4 fogSlice(float s, vec3 dirZ) {\n"
    fn += "    vec4 acc = vec4(0.0);\n"
    fn += f"    int rot = int(mod(s, {float(n)}));\n"
    jz = "fract(fogDepth.w)" if cyc else "0.5"
    fn += f"    for (int k = 0; k < {n}; ++k) {{\n"
    fn += f"        int kz = k + rot; if (kz >= {n}) kz -= {n};\n"
    fn += f"        float t = (s + (float(kz) + {jz}) / {float(n)}) / fogGrid.z;\n"
    fn += "        acc += fogCell(g_tapDir[k] * fogSliceDepth(t));\n"
    fn += "    }\n"
    fn += f"    vec4 cur = acc * {1.0 / n};\n"
    fn += "    return fogBlendHistory(cur, g_dirC * fogSliceDepth((s + 0.5) / fogGrid.z));\n"
    fn += "}\n\n"
    jit = "    g_dirC = fogColumnDir(vec2(0.0));\n"
    for k in range(n):
        ox = (k + 0.5) / n - 0.5
        oy = (perm[k] + 0.5) / n - 0.5
        if cyc:
            jit += f"    g_tapDir[{k}] = fogColumnDir(vec2({ox:.6f}, {oy:.6f}) + fogFrame.xy * {1.0 / n});\n"
        else:
            jit += f"    g_tapDir[{k}] = fogColumnDir(vec2({ox:.6f}, {oy:.6f}));\n"
    jit += "    vec3 dirZ = g_dirC;\n"
    s = base.replace(SLICE_FN, fn).replace(MAIN_JIT, jit)
    return s


def anti2():
    fn = HIST + "vec3 g_tapDir[2];\nvec3 g_dirC = vec3(0.0);\n"
    fn += "vec4 fogSlice(float s, vec3 dirZ) {\n"
    fn += "    float u = fract(fogDepth.w);\n"
    fn += "    vec4 a = fogCell(g_tapDir[0] * fogSliceDepth((s + u) / fogGrid.z));\n"
    fn += "    vec4 b = fogCell(g_tapDir[1] * fogSliceDepth((s + 1.0 - u) / fogGrid.z));\n"
    fn += "    return fogBlendHistory(0.5 * (a + b), g_dirC * fogSliceDepth((s + 0.5) / fogGrid.z));\n"
    fn += "}\n\n"
    jit = ("    g_dirC = fogColumnDir(vec2(0.0));\n"
           "    g_tapDir[0] = fogColumnDir(fogFrame.xy);\n"
           "    g_tapDir[1] = fogColumnDir(-fogFrame.xy);\n"
           "    vec3 dirZ = g_dirC;\n")
    return base.replace(SLICE_FN, fn).replace(MAIN_JIT, jit)


HASH = r"""
// A white hash of a cell and slice (pcg3d): no lattice structure, nothing that can travel.
vec3 fogHash3(uvec3 v) {
    v = v * 1664525u + 1013904223u;
    v.x += v.y * v.z; v.y += v.z * v.x; v.z += v.x * v.y;
    v ^= v >> 16u;
    v.x += v.y * v.z; v.y += v.z * v.x; v.z += v.x * v.y;
    return vec3(v) * (1.0 / 4294967296.0);
}
"""


def white4(coherent, n=4):
    # 4 taps: depth strata (kz + o)/4, xy 4-rook + (jxy)/4, the frame's golden-ratio offset o rotated per
    # cell (and per slice for depth) by a white hash unless coherent.
    perm = {1: [0], 2: [1, 0], 4: [1, 3, 0, 2]}[n]
    inv = 1.0 / n
    fn = HIST + HASH + f"vec3 g_tapDir[{n}];\nvec3 g_dirC = vec3(0.0);\n"
    fn += "vec4 fogSlice(float s, vec3 dirZ) {\n"
    if coherent:
        fn += "    float o = fract(fogDepth.w);\n"
    else:
        fn += "    float o = fract(fogDepth.w + fogHash3(uvec3(uvec2(gl_FragCoord.xy), uint(s) + 7919u)).x);\n"
    fn += "    vec4 acc = vec4(0.0);\n"
    fn += f"    int rot = int(mod(s, {float(n)}));\n"
    fn += f"    for (int k = 0; k < {n}; ++k) {{\n"
    fn += f"        int kz = k + rot; if (kz >= {n}) kz -= {n};\n"
    fn += f"        acc += fogCell(g_tapDir[k] * fogSliceDepth((s + (float(kz) + o) * {inv}) / fogGrid.z));\n"
    fn += "    }\n"
    fn += f"    return fogBlendHistory(acc * {inv}, g_dirC * fogSliceDepth((s + 0.5) / fogGrid.z));\n"
    fn += "}\n\n"
    jit = "    g_dirC = fogColumnDir(vec2(0.0));\n"
    if coherent:
        jit += "    vec2 jxy = fogFrame.xy;\n"
    else:
        jit += "    vec2 jxy = fract(fogFrame.xy + 0.5 + fogHash3(uvec3(uvec2(gl_FragCoord.xy), 104729u)).xy) - 0.5;\n"
    for k in range(n):
        ox = (k + 0.5) / n - 0.5
        oy = (perm[k] + 0.5) / n - 0.5
        jit += f"    g_tapDir[{k}] = fogColumnDir(vec2({ox:.6f}, {oy:.6f}) + jxy * {inv});\n"
    jit += "    vec3 dirZ = g_dirC;\n"
    return base.replace(SLICE_FN, fn).replace(MAIN_JIT, jit)


def alt4():
    # An 8-tap Latin hypercube (8 depth strata at their centres x 8-rook xy) split into two 4-tap halves
    # (even / odd taps). Each frame a cell takes one half: half = step(0.5, jz) xor the cell's (x+y+slice)
    # parity. Converges to the 8-tap estimate; the residual is the halves' difference through the EMA.
    perm8 = [3, 6, 0, 5, 2, 7, 1, 4]
    fn = HIST + "vec3 g_tapDir[8];\nvec3 g_dirC = vec3(0.0);\nint g_half = 0;\n"
    fn += "vec4 fogSlice(float s, vec3 dirZ) {\n"
    fn += "    int h = (g_half + int(s)) & 1;\n"
    fn += "    int rot = int(mod(s, 8.0));\n"
    fn += "    vec4 acc = vec4(0.0);\n"
    fn += "    for (int k = 0; k < 4; ++k) {\n"
    fn += "        int i = 2 * k + h;\n"
    fn += "        int kz = i + rot; if (kz >= 8) kz -= 8;\n"
    fn += "        acc += fogCell(g_tapDir[i] * fogSliceDepth((s + (float(kz) + 0.5) * 0.125) / fogGrid.z));\n"
    fn += "    }\n"
    fn += "    return fogBlendHistory(acc * 0.25, g_dirC * fogSliceDepth((s + 0.5) / fogGrid.z));\n"
    fn += "}\n\n"
    jit = "    g_dirC = fogColumnDir(vec2(0.0));\n"
    jit += "    ivec2 ci = ivec2(gl_FragCoord.xy);\n"
    jit += "    g_half = ((ci.x + ci.y) & 1) ^ (fract(fogDepth.w) < 0.5 ? 0 : 1);\n"
    for k in range(8):
        ox = (k + 0.5) / 8 - 0.5
        oy = (perm8[k] + 0.5) / 8 - 0.5
        jit += f"    g_tapDir[{k}] = fogColumnDir(vec2({ox:.6f}, {oy:.6f}));\n"
    jit += "    vec3 dirZ = g_dirC;\n"
    return base.replace(SLICE_FN, fn).replace(MAIN_JIT, jit)


def variant(v):
    if v == "percell":
        return base
    if v == "t4fix":
        return multitap(4, False)
    if v == "t8fix":
        return multitap(8, False)
    if v == "t4cyc":
        return multitap(4, True)
    if v == "t8cyc":
        return multitap(8, True)
    if v == "t4w":
        return white4(False)
    if v == "t1w":
        return white4(False, 1)
    if v == "t2w":
        return white4(False, 2)
    if v == "t4c":
        return white4(True)
    if v == "t4alt":
        return alt4()
    if v == "t2anti":
        return anti2()
    raise SystemExit("unknown variant " + v)


for v in VARIANTS:
    d = os.path.join(OUT, v)
    os.makedirs(d, exist_ok=True)
    src = variant(v)
    with open(os.path.join(d, "psFogInject.glsl"), "w", encoding="utf-8", newline="\n") as f:
        f.write(src)
    print(v, len(src), "bytes ->", d)
