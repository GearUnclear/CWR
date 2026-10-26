"""Agent 5 (fogwind swarm): temporal-accumulation variants of psFogInject (+ psFogIntegrate) for
CWR_SHADER_OVERRIDE_DIR. Base = the engine's current sources (d91a08e0 per-cell IGN jitter).

usage: python assemble5.py <engine PoseidonGL33 dir> <out dir> [variant ...]
Writes <out dir>/<variant>/psFogInject.glsl and psFogIntegrate.glsl.

Variants
  percell  unchanged d91a08e0 (sanity; integrate unchanged)
  c8       strictly periodic 8-frame cycle. Frame counter c (0..15) kept in the SIGN bits of the injection
           volume (texel 0,0,0; r g b a = bits 0..3), so psFogIntegrate reads abs(). Per cell:
             depth index iz = (c + (x&1) + 2(y&1) + 4(s&1)) & 7 -> van der Corput order {0,4,2,6,1,5,3,7},
             depth offset = (vdc(iz) + dz) / 8 with dz a static white hash of (cell, slice) in [0,1);
             xy index ixy = (c + 2((x&1) + 2(y&1))) & 7 -> rank-1 lattice ((i+.5)/8, ((3i&7)+.5)/8) - .5.
           Every 2x2x2 block samples every depth stratum every frame; every cell's input is exactly
           8-periodic, so the history's residual has no component below 1/8 cycle per frame.
           History read at the cell CENTRE (agent 6: EMA, not leaky sum). Share unchanged (0.12).
  c8a      c8 with the share lowered with depth: 0.12 near, 0.05 past ~400 m (far slices barely move).
  c8z      c8 with dz = 0.5 (no static per-cell sub-offset: only 8 depths per slice anywhere).
  gcr      d91a08e0 + centre history read only (control for what the cycle adds).
"""
import os, re, sys

SRC = sys.argv[1]
OUT = sys.argv[2]
VARIANTS = sys.argv[3:] or ["c8"]

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
integ = P["kPostFogIntegrateGLSL"]

# --- anchors in the d91a08e0 source ---
SLICE_FN_OLD = """vec4 fogSlice(float s, vec3 dirZ) {
    float z = fogSliceDepth((s + fract(fogDepth.w + g_cellJitter + 0.618034 * s)) / fogGrid.z);
    vec3 rel = dirZ * z;
    vec4 cur = fogCell(rel);
    if (fogFrame.z < 1.0) {
        vec3 pv = vec3(dot(prevRow0.xyz, rel) + prevRow0.w, dot(prevRow1.xyz, rel) + prevRow1.w,
                       dot(prevRow2.xyz, rel) + prevRow2.w);"""
MAIN_OLD = """    g_cellJitter = fogCellNoise(gl_FragCoord.xy);
    vec2 cellJ = fract(fogFrame.xy + 0.5 + vec2(fogCellNoise(gl_FragCoord.xy + vec2(17.0, 59.0)),
                                                fogCellNoise(gl_FragCoord.xy + vec2(83.0, 29.0)))) - 0.5;
    vec2 ndc = (gl_FragCoord.xy + cellJ) / fogGrid.xy * 2.0 - 1.0;
    vec3 dirZ = viewToWorld * vec3(ndc.x * projInfo.z, ndc.y * projInfo.w, 1.0); // per metre of view depth
"""
HIST_OLD = "                cur = mix(texture(fogHistory, vec3(puv, pt)), cur, fogFrame.z);"
OUTS_OLD = [f"    fog{k} = fogSlice(base + {k}.0, dirZ);" for k in range(8)]
INTEG_OLD = "    vec4 v = texelFetch(fogInject, ivec3(cell, s), 0) * 0.001;"
for a in [SLICE_FN_OLD, MAIN_OLD, HIST_OLD, *OUTS_OLD]:
    assert base.count(a) == 1, a[:60]
assert integ.count(INTEG_OLD) == 1

HELPERS = """
// agent 5: the frame counter in the sign bits of the injection volume, the 8-cycle and the cell hash.
vec4 g_sign = vec4(1.0);  // this frame's sign bits
int g_cyc = 0;            // this frame's counter (0..15)
vec3 g_dirC = vec3(0.0);  // the cell centre's column (per metre of view depth), for the history lookup
float g_dz = 0.5;         // static sub-stratum of this cell (set per slice)
const int kVdc[8] = int[8](0, 4, 2, 6, 1, 5, 3, 7);
uint a5hash(uvec3 v) {
    v = v * 1664525u + 1013904223u;
    v.x += v.y * v.z; v.y += v.z * v.x; v.z += v.x * v.y;
    v ^= v >> 16u;
    v.x += v.y * v.z; v.y += v.z * v.x; v.z += v.x * v.y;
    return v.x ^ v.y ^ v.z;
}
float a5rand(uvec3 v) { return float(a5hash(v) >> 8u) * (1.0 / 16777216.0); }
vec4 a5enc(vec4 v) { return max(abs(v), vec4(1e-4)) * g_sign; }
"""


def slice_fn(dz_mode, far_share):
    dz = {"hash": "a5rand(uvec3(uvec2(gl_FragCoord.xy), uint(s) + 977u))", "half": "0.5"}[dz_mode]
    share = "fogFrame.z"
    if far_share:
        # 0.12 near, 0.05 past ~400 m: far slices barely move under a pan, their history can be longer.
        share = "(fogFrame.z >= 1.0 ? 1.0 : mix(fogFrame.z, 0.05, smoothstep(120.0, 400.0, zc)))"
    return f"""vec4 fogSlice(float s, vec3 dirZ) {{
    ivec2 ce = ivec2(gl_FragCoord.xy);
    int iz = (g_cyc + (ce.x & 1) + 2 * (ce.y & 1) + 4 * (int(s) & 1)) & 7;
    float dz = {dz};
    float z = fogSliceDepth((s + (float(kVdc[iz]) + dz) * 0.125) / fogGrid.z);
    float zc = fogSliceDepth((s + 0.5) / fogGrid.z);
    vec3 rel = dirZ * z;
    vec3 relC = g_dirC * zc; // the history is read at the cell centre (an EMA of the cell, not a leaky sum)
    vec4 cur = fogCell(rel);
    if (fogFrame.z < 1.0) {{
        vec3 pv = vec3(dot(prevRow0.xyz, relC) + prevRow0.w, dot(prevRow1.xyz, relC) + prevRow1.w,
                       dot(prevRow2.xyz, relC) + prevRow2.w);""", share


def variant(v):
    if v == "percell":
        return base, integ
    if v == "gcr":
        s = base.replace(SLICE_FN_OLD, SLICE_FN_OLD.replace(
            "    vec4 cur = fogCell(rel);\n",
            "    vec4 cur = fogCell(rel);\n    vec3 relC = g_dirC * fogSliceDepth((s + 0.5) / fogGrid.z);\n").replace(
            "dot(prevRow0.xyz, rel)", "dot(prevRow0.xyz, relC)").replace(
            "dot(prevRow1.xyz, rel)", "dot(prevRow1.xyz, relC)").replace(
            "dot(prevRow2.xyz, rel)", "dot(prevRow2.xyz, relC)"))
        s = s.replace("float g_cellJitter = 0.0;", "float g_cellJitter = 0.0;\nvec3 g_dirC = vec3(0.0);")
        s = s.replace(MAIN_OLD, MAIN_OLD + """    { vec2 nc = gl_FragCoord.xy / fogGrid.xy * 2.0 - 1.0;
      g_dirC = viewToWorld * vec3(nc.x * projInfo.z, nc.y * projInfo.w, 1.0); }
""")
        return s, integ
    dz_mode = "half" if v == "c8z" else "hash"
    far = v == "c8a"
    fn, share = slice_fn(dz_mode, far)
    s = base.replace("vec4 fogSlice(float s, vec3 dirZ) {", HELPERS + "vec4 fogSlice(float s, vec3 dirZ) {", 1)
    s = s.replace(SLICE_FN_OLD, fn)
    s = s.replace(HIST_OLD, f"                cur = mix(abs(texture(fogHistory, vec3(puv, pt))), cur, {share});")
    s = s.replace(MAIN_OLD, """    {
        vec4 h0 = texelFetch(fogHistory, ivec3(0), 0);
        int prev = (h0.r < 0.0 ? 1 : 0) | (h0.g < 0.0 ? 2 : 0) | (h0.b < 0.0 ? 4 : 0) | (h0.a < 0.0 ? 8 : 0);
        g_cyc = (prev + 1) & 15;
        g_sign = vec4((g_cyc & 1) != 0 ? -1.0 : 1.0, (g_cyc & 2) != 0 ? -1.0 : 1.0,
                      (g_cyc & 4) != 0 ? -1.0 : 1.0, (g_cyc & 8) != 0 ? -1.0 : 1.0);
    }
    ivec2 ce0 = ivec2(gl_FragCoord.xy);
    int ixy = (g_cyc + 2 * ((ce0.x & 1) + 2 * (ce0.y & 1))) & 7;
    vec2 cellJ = vec2((float(ixy) + 0.5) * 0.125, (float((3 * ixy) & 7) + 0.5) * 0.125) - 0.5;
    vec2 ndc = (gl_FragCoord.xy + cellJ) / fogGrid.xy * 2.0 - 1.0;
    vec3 dirZ = viewToWorld * vec3(ndc.x * projInfo.z, ndc.y * projInfo.w, 1.0); // per metre of view depth
    { vec2 nc = gl_FragCoord.xy / fogGrid.xy * 2.0 - 1.0;
      g_dirC = viewToWorld * vec3(nc.x * projInfo.z, nc.y * projInfo.w, 1.0); }
""")
    for k in range(8):
        s = s.replace(OUTS_OLD[k], f"    fog{k} = a5enc(fogSlice(base + {k}.0, dirZ));")
    i2 = integ.replace(INTEG_OLD, "    vec4 v = abs(texelFetch(fogInject, ivec3(cell, s), 0)) * 0.001; // agent 5: sign bits carry a counter")
    return s, i2


for v in VARIANTS:
    d = os.path.join(OUT, v)
    os.makedirs(d, exist_ok=True)
    inj, itg = variant(v)
    open(os.path.join(d, "psFogInject.glsl"), "w", encoding="utf-8", newline="\n").write(inj)
    if itg != integ:
        open(os.path.join(d, "psFogIntegrate.glsl"), "w", encoding="utf-8", newline="\n").write(itg)
    print("wrote", v, len(inj), "inject chars", "(+ integrate)" if itg != integ else "")
