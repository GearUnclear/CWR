"""Assemble a post shader's full GLSL source from the engine's raw-string pieces.

usage: python assemble_shader.py <name, e.g. psFogInject> <out.glsl> [--root <engine dir>]
Reads the kPostShaders table in EngineGL33_PostShaders.cpp, finds each piece's
R"( ... )" literal (const char kX[] = R"(...)"; in any .cpp under PoseidonGL33)
and joins them in order, as PostShaderSource does. Read-only on the engine tree.
"""
import argparse, glob, os, re

ap = argparse.ArgumentParser()
ap.add_argument("name")
ap.add_argument("out")
ap.add_argument("--root", default="D:/Arma_CWA_worktrees/graphics-remaster/arma_CWR/engine/PoseidonGL33")
o = ap.parse_args()
table = open(os.path.join(o.root, "EngineGL33_PostShaders.cpp"), encoding="utf-8").read()
m = re.search(r'\{"%s",\s*\{(.*?)\}\}' % re.escape(o.name), table, re.S)
assert m, "no table entry"
pieces = [p.strip().split("::")[-1] for p in m.group(1).split(",") if p.strip()]
src = {}
for f in glob.glob(os.path.join(o.root, "*.cpp")):
    t = open(f, encoding="utf-8").read()
    for mm in re.finditer(r'const char (\w+)\[\] = R"(\w*)\((.*?)\)\2"', t, re.S):
        src.setdefault(mm.group(1), mm.group(3))
out = ""
for p in pieces:
    assert p in src, f"piece {p} not found as a raw string"
    out += src[p]
with open(o.out, "w", newline="\n", encoding="utf-8") as f:
    f.write(out)
print(o.name, "pieces", pieces, "chars", len(out))
