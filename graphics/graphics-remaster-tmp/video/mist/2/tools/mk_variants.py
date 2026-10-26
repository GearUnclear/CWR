"""Write psFogInject shader-override variants (agent 2, mist swarm).

  nojit: the froxel injection samples a fixed point per cell (no per-frame jitter):
         a pure test of "the jitter drives the mist flicker".
  cellj: the same per-frame Weyl jitter, but rotated per cell by a hash
         (Cranley-Patterson rotation): each cell's history residual is independent,
         so it no longer moves the whole mist together. Candidate engine fix.
Each variant goes to shaders/<name>/psFogInject.glsl (pass that dir to
run-captures.ps1 -ShaderOverrideDir).
"""
import os

here = os.path.dirname(os.path.abspath(__file__))
root = os.path.join(here, "..", "shaders")
orig = open(os.path.join(root, "orig", "psFogInject.glsl"), encoding="utf-8").read()

A_SLICE = "    float z = fogSliceDepth((s + fogDepth.w) / fogGrid.z);\n"
A_MAIN = "    vec2 ndc = (gl_FragCoord.xy + fogFrame.xy) / fogGrid.xy * 2.0 - 1.0;\n"
A_FN = "// The cell of slice s on this column: this frame's jittered point, blended with\n"
assert orig.count(A_SLICE) == 1 and orig.count(A_MAIN) == 1 and orig.count(A_FN) == 1

HASH = ("float mistCellHash(vec3 p) {\n"
        "    p = fract(p * vec3(0.1031, 0.1030, 0.0973));\n"
        "    p += dot(p, p.yxz + 33.33);\n"
        "    return fract((p.x + p.y) * p.z);\n"
        "}\n\n")

variants = {
    "nojit": orig.replace(A_SLICE, "    float z = fogSliceDepth((s + 0.5) / fogGrid.z);\n")
                 .replace(A_MAIN, "    vec2 ndc = gl_FragCoord.xy / fogGrid.xy * 2.0 - 1.0;\n"),
    "cellj": orig.replace(A_FN, HASH + A_FN)
                 .replace(A_SLICE, "    float z = fogSliceDepth((s + fract(fogDepth.w + mistCellHash(vec3(gl_FragCoord.xy, s + 7.0)))) / fogGrid.z);\n")
                 .replace(A_MAIN, "    vec2 jxy = fract(fogFrame.xy + 0.5 + vec2(mistCellHash(vec3(gl_FragCoord.xy, 501.0)), mistCellHash(vec3(gl_FragCoord.xy, 907.0)))) - 0.5;\n"
                                  "    vec2 ndc = (gl_FragCoord.xy + jxy) / fogGrid.xy * 2.0 - 1.0;\n"),
}
for name, text in variants.items():
    d = os.path.join(root, name)
    os.makedirs(d, exist_ok=True)
    with open(os.path.join(d, "psFogInject.glsl"), "w", newline="\n", encoding="utf-8") as f:
        f.write(text)
    print(name, len(text))
