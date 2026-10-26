"""v12 = v11 + an edge hit only counts when the edge pixel stands above the ray's start
(a mirror ray from the sea must not 'hit' the sea at the frame edge)."""
from pathlib import Path
src = Path(r"D:\Arma_CWA_worktrees\lighting\tmp\research\ssr\glsl_v11\psPostSsr.glsl").read_text()
dst = Path(r"D:\Arma_CWA_worktrees\lighting\tmp\research\ssr\glsl_v12\psPostSsr.glsl")
old = "    if (sideExit && ez > p0.z) {\n"
new = ("    float startY = dot(w2v[1], p0) + camWorld.y;\n"
       "    float exitY = dot(w2v[1], viewPos(exitUV, ez)) + camWorld.y;\n"
       "    if (sideExit && ez > p0.z && exitY > startY + 0.25) {\n")
assert src.count(old) == 1
dst.parent.mkdir(exist_ok=True)
dst.write_text(src.replace(old, new))
print("wrote", dst)
