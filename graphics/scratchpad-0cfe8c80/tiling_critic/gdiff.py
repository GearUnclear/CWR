import difflib
base = r"D:\Arma_CWA_worktrees\lighting\tmp\research\tiling"
a = open(base+r"\glsl_dump_lighting\psRemasterCommon.glsl", encoding="utf-8", errors="replace").read().splitlines()
b = open(base+r"\ovr_final2\psRemasterCommon.glsl", encoding="utf-8", errors="replace").read().splitlines()
for l in difflib.unified_diff(a, b, lineterm="", n=0):
    if l.startswith("+") and not l.startswith("+++"): continue
    print(l)
print("---- added non-helper lines:")
add = [l for l in difflib.unified_diff(a, b, lineterm="", n=0) if l.startswith("@@")]
print(add)
