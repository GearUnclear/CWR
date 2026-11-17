"""Prints where each model's feet are (file space: x = his left, y up, z = back), read from the built P3D.
   python foot_points.py <dedushka.p3d> ..."""
import json
import sys
sys.path.insert(0, r"D:\Arma_CWA_worktrees\kaiju\arma_CWR\apps\tools\BlenderAddon\io_import_p3d")
import mlod_writer as mw
for path in sys.argv[1:]:
    lod = mw.read_mlod(open(path, "rb").read())[1][0]
    out = {}
    for side, keep in (("left", lambda p: p[0] > 1.0), ("right", lambda p: p[0] < -1.0)):
        low = [p for p in lod.points if p[1] < 1.0 and keep(p) and abs(p[0]) < 12 and p[2] < 8]
        tip = min(low, key=lambda p: p[2])
        xs = sorted(p[0] for p in low)
        zs = sorted(p[2] for p in low)
        out[side] = {"toe_tip": [round(c, 2) for c in tip], "x_mid": round((xs[0] + xs[-1]) / 2, 2),
                     "x": [round(xs[0], 2), round(xs[-1], 2)], "z": [round(zs[0], 2), round(zs[-1], 2)], "n": len(low)}
    ys = [p[1] for p in lod.points]
    zs = [p[2] for p in lod.points]
    print(path, json.dumps(out), "height", round(max(ys), 2), "front", round(min(zs), 2), "back", round(max(zs), 2))
    # lowest points near the hands (claws)
    for side, keep in (("left", lambda p: p[0] > 8), ("right", lambda p: p[0] < -8)):
        pts = [p for p in lod.points if keep(p)]
        lowp = min(pts, key=lambda p: p[1])
        print("   lowest far-%s point" % side, [round(c, 2) for c in lowp])
