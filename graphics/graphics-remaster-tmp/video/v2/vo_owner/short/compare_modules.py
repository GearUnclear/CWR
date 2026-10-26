"""Check that trailer_v2.py's split and sbs paths give exactly the same pixels as before the short layout.

Loads the snapshot taken before the edit (trailer_v2_orig.py, next to this file) and the edited module,
and compares, for --layout split and sbs: the plan, every closing-card frame, every transition kind at
several points, the cards, the scene titles, the typed line, the divider split and the side-by-side.
"""
import importlib.util
import sys
from pathlib import Path

import numpy as np

VID = Path("D:/Arma_CWA_worktrees/video-work/arma_CWR/tests/rendering/captures/video")
sys.path.insert(0, str(VID))
import trailer_v2 as NEW  # noqa: E402

spec = importlib.util.spec_from_file_location("trailer_v2_orig", Path(__file__).resolve().parent / "trailer_v2_orig.py")
OLD = importlib.util.module_from_spec(spec)
spec.loader.exec_module(OLD)

checks = 0


def same(a, b, what):
    global checks
    checks += 1
    if isinstance(a, np.ndarray):
        ok = a.dtype == b.dtype and a.shape == b.shape and np.array_equal(a, b)
    else:
        ok = a == b
    if not ok:
        raise SystemExit(f"DIFFERENT: {what}")


rng = np.random.default_rng(7)
same(OLD.plan(), NEW.plan(), "plan()")
same(OLD.plan_with({"transitions": {"valley_mist": ["fade", 0.8]}, "closing_s": 11.0}),
     NEW.plan_with({"transitions": {"valley_mist": ["fade", 0.8]}, "closing_s": 11.0}), "plan_with()")
segs = NEW.plan()[0]
clips = [s for s in segs if s["kind"] == "clip"]
for layout in ("split", "sbs"):
    co, cn = OLD.Composer(layout), NEW.Composer(layout)
    same((co.w, co.X.shape, co.Y.shape), (cn.w, cn.X.shape, cn.Y.shape), f"{layout} composer geometry")
    w = co.w
    # every closing-card frame
    cso, csn = OLD.ClosingSeg(segs[-1], co), NEW.ClosingSeg(segs[-1], cn)
    for k in range(segs[-1]["frames"]):
        same(cso.frame(), csn.frame(), f"{layout} closing frame {k}")
    # every transition kind
    p = rng.integers(0, 256, (OLD.H, w, 3), dtype=np.uint8)
    q = rng.integers(0, 256, (OLD.H, w, 3), dtype=np.uint8)
    for kind in ("fade", "dip", "flash", "wipe_left", "wipe_right", "wipe_up"):
        for x in (0.02, 0.25, 0.5, 0.75, 0.98):
            same(co.transition(kind, x, p, q), cn.transition(kind, x, p, q), f"{layout} transition {kind} {x}")
    # cards, titles, the typed line (uint8 and float frames)
    for lines in (OLD.INTRO, OLD.OUTRO):
        for alpha in (0.3, 1.0):
            same(co.typo.put_card(p.copy(), lines, alpha), cn.typo.put_card(p.copy(), lines, alpha),
                 f"{layout} card {lines[1]} {alpha}")
            same(co.typo.put_card(p.astype(np.float32), lines, alpha),
                 cn.typo.put_card(p.astype(np.float32), lines, alpha), f"{layout} card float {alpha}")
    for s in clips:
        c = NEW.G.clip_by_name(s["clip"])
        for t in (0.5, 1.2, 2.5, 4.6, 5.2):
            same(co.typo.put_title(p.copy(), c, t, 0.7), cn.typo.put_title(p.copy(), c, t, 0.7),
                 f"{layout} title {s['clip']} {t}")
    c = NEW.G.clip_by_name("opening")
    for t in np.arange(8.8, 15.0, 0.13):
        same(co.typo.put_typed(p.copy(), c, float(t)), cn.typo.put_typed(p.copy(), c, float(t)),
             f"{layout} typed {t:.2f}")
    # the composition itself
    a = rng.integers(0, 256, (OLD.H, OLD.W, 3), dtype=np.uint8)
    b = rng.integers(0, 256, (OLD.H, OLD.W, 3), dtype=np.uint8)
    if layout == "split":
        for clip in OLD.CHOREO:
            cho, chn = OLD.Choreo(OLD.CHOREO[clip], OLD.W), NEW.Choreo(NEW.CHOREO[clip], NEW.W)
            for t in (0.0, 1.5, 3.0, 4.4, 6.0, 8.0):
                same(cho.at(t), chn.at(t), f"choreo {clip} {t}")
                pp, ang = cho.at(t)
                same(co.split(a, b, pp, ang, 0.8), cn.split(a, b, pp, ang, 0.8), f"split {clip} {t}")
    else:
        for alpha in (0.0, 0.4, 1.0):
            same(co.sbs(a, b, alpha), cn.sbs(a, b, alpha), f"sbs {alpha}")
    print(f"{layout}: identical", flush=True)
print(f"all {checks} checks identical")
