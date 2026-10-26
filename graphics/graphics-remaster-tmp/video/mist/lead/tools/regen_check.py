"""Would gen_video_v2.py (as it is now) write the same valley_mist tests that shot the trailer?
Writes both sides into tmp/video/mist/lead/regen/ (never into video-work or vanilla-upstream) and diffs.
usage: python regen_check.py [clip]   (plain python)"""
import filecmp, sys
from pathlib import Path
V = Path("D:/Arma_CWA_worktrees/video-work/arma_CWR/tests/rendering/captures/video")
sys.path.insert(0, str(V))
import gen_video_v2 as g
out = Path("D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/mist/lead/regen")
clip = sys.argv[1] if len(sys.argv) > 1 else "valley_mist"
g.HERE = out
g.VANILLA_DIR = out
c = next(c for c in g.CLIPS if c["clip"] == clip)
for side, ref in (("ud", V / f"ud_{clip}"), ("vanilla", g.VANILLA_ROOT / f"tests/rendering/captures/video/van_{clip}")):
    p, n = g.write_clip(c, 0, side)
    for ext in (".test.sqf", ".test.toml"):
        a, b = Path(str(p)[:-len(".test.sqf")] + ext), Path(str(ref) + ext)
        print(side, ext, n, "frames", "IDENTICAL" if filecmp.cmp(a, b, shallow=False) else "DIFFERS", a, b)
