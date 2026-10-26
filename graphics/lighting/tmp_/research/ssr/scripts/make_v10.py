"""v10 = v9 + side-exit edge hits (a ray that leaves through the left/right edge over
geometry takes the edge pixel, so mirrored hills do not stop in a vertical seam)."""
from pathlib import Path
src = Path(r"D:\Arma_CWA_worktrees\lighting\tmp\research\ssr\glsl_v9\psPostSsr.glsl").read_text()
dst = Path(r"D:\Arma_CWA_worktrees\lighting\tmp\research\ssr\glsl_v10\psPostSsr.glsl")

def rep(s, old, new):
    assert s.count(old) == 1, (old, s.count(old))
    return s.replace(old, new)

s = src
s = rep(s,
    "    float tc = 1.0;\n"
    "    if (dS.x > 1e-4) tc = min(tc, (hi.x - s0.x) / dS.x); else if (dS.x < -1e-4) tc = min(tc, (lo.x - s0.x) / dS.x);\n"
    "    if (dS.y > 1e-4) tc = min(tc, (hi.y - s0.y) / dS.y); else if (dS.y < -1e-4) tc = min(tc, (lo.y - s0.y) / dS.y);\n",
    "    float tcx = 1.0, tcy = 1.0;\n"
    "    if (dS.x > 1e-4) tcx = (hi.x - s0.x) / dS.x; else if (dS.x < -1e-4) tcx = (lo.x - s0.x) / dS.x;\n"
    "    if (dS.y > 1e-4) tcy = (hi.y - s0.y) / dS.y; else if (dS.y < -1e-4) tcy = (lo.y - s0.y) / dS.y;\n"
    "    float tc = min(1.0, min(tcx, tcy));\n"
    "    bool sideExit = tcx < tcy && tcx < 1.0;\n")
s = rep(s,
    "    if (sceneZ(exitUV) > 1e8) { kind = 2; travel = 1e9; }\n    return exitUV;\n",
    "    if (sceneZ(exitUV) > 1e8) { kind = 2; travel = 1e9; return exitUV; }\n"
    "    // v10: left the frame sideways over geometry that is beyond the start:\n"
    "    // take the edge pixel (the scene continues past the edge).\n"
    "    float ez = sceneZ(exitUV);\n"
    "    if (sideExit && ez > p0.z) {\n"
    "        kind = 1;\n"
    "        travel = abs(r.z) > 1e-3 ? abs((ez - p0.z) / r.z) : len * tc;\n"
    "    }\n"
    "    return exitUV;\n")
dst.parent.mkdir(exist_ok=True)
dst.write_text(s)
print("wrote", dst, len(s))
