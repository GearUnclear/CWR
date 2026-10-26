"""Assemble the engine's psFogInject (and other fog programs) GLSL from the C++ raw-string pieces,
exactly as EngineGL33_PostShaders.cpp kPostShaders joins them, for CWR_SHADER_OVERRIDE_DIR.

usage: python assemble_fog.py <engine PoseidonGL33 dir> <out dir> [variant ...]
variants: base (unchanged), frozen (no cell or depth jitter), percell (per-cell decorrelated jitter)
Writes <out dir>/<variant>/psFogInject.glsl.
"""
import os, re, sys

SRC = sys.argv[1]
OUT = sys.argv[2]
VARIANTS = sys.argv[3:] or ["base"]

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

SLICE_OLD = "    float z = fogSliceDepth((s + fogDepth.w) / fogGrid.z);\n"
NDC_OLD = "    vec2 ndc = (gl_FragCoord.xy + fogFrame.xy) / fogGrid.xy * 2.0 - 1.0;\n"
assert base.count(SLICE_OLD) == 1 and base.count(NDC_OLD) == 1


def variant(v):
    if v == "base":
        return base
    if v == "frozen":
        # Every cell samples its centre, every frame: no jitter, the history converges to a fixed value.
        s = base.replace(SLICE_OLD, "    float z = fogSliceDepth((s + 0.5) / fogGrid.z);\n")
        return s.replace(NDC_OLD, "    vec2 ndc = gl_FragCoord.xy / fogGrid.xy * 2.0 - 1.0;\n")
    if v == "percell":
        # The frame's jitter turned per cell (and per slice for the depth) by an interleaved-gradient
        # offset: neighbouring cells' history residuals are independent, so they average out in the
        # apply's trilinear lookup and the TAA instead of pumping whole regions together.
        helper = ("float fogIgn(vec2 p) { return fract(52.9829189 * fract(dot(p, vec2(0.06711056, 0.00583715)))); }\n"
                  "float g_cellJ = 0.0;\n")
        s = base.replace("vec4 fogSlice(float s, vec3 dirZ) {\n", helper + "vec4 fogSlice(float s, vec3 dirZ) {\n")
        s = s.replace(SLICE_OLD, "    float z = fogSliceDepth((s + fract(fogDepth.w + g_cellJ + 0.618034 * s)) / fogGrid.z);\n")
        return s.replace(NDC_OLD,
                         "    g_cellJ = fogIgn(gl_FragCoord.xy);\n"
                         "    vec2 cj = fract(fogFrame.xy + 0.5 + vec2(fogIgn(gl_FragCoord.xy + vec2(17.0, 59.0)), fogIgn(gl_FragCoord.xy + vec2(83.0, 29.0)))) - 0.5;\n"
                         "    vec2 ndc = (gl_FragCoord.xy + cj) / fogGrid.xy * 2.0 - 1.0;\n")
    raise SystemExit("unknown variant " + v)


for v in VARIANTS:
    d = os.path.join(OUT, v)
    os.makedirs(d, exist_ok=True)
    src = variant(v)
    with open(os.path.join(d, "psFogInject.glsl"), "w", encoding="utf-8", newline="\n") as f:
        f.write(src)
    print(v, len(src), "bytes ->", d)
