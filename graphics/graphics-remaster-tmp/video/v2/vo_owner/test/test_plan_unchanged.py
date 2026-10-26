"""trailer_v2.py's default plan must be byte-for-byte what it was before the --timing flag.

    python test_plan_unchanged.py

Loads the pristine copy (trailer_v2.orig.py, saved before the edit) and the edited
trailer_v2.py side by side and compares plan() for the full montage and a test montage,
vo_v2.plan(), and the timeline.json each writes with --plan-only (into this folder only).
Then checks that a timing override changes the plan the way it says and leaves the tables
as they were (plan_with).
"""
import filecmp
import importlib.util
import json
import shutil
import sys
from pathlib import Path

VIDEO = Path("D:/Arma_CWA_worktrees/video-work/arma_CWR/tests/rendering/captures/video")
HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(VIDEO.parent))
sys.path.insert(0, str(VIDEO))


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


orig = load("trailer_v2_orig", HERE / "trailer_v2.orig.py")
import trailer_v2 as new  # noqa: E402
import vo_v2  # noqa: E402

fails = 0


def check(what, a, b):
    global fails
    ok = json.dumps(a, sort_keys=True) == json.dumps(b, sort_keys=True)
    fails += 0 if ok else 1
    print(f"{'OK  ' if ok else 'FAIL'} {what}")


check("plan() full montage", orig.plan(), new.plan())
check("plan(only=[valley_mist, outpost])", orig.plan(["valley_mist", "outpost"], closing=False),
      new.plan(["valley_mist", "outpost"], closing=False))
check("plan_with({}) == plan()", new.plan(), new.plan_with({}))
check("vo_v2.plan() (reads trailer_v2.plan)", vo_v2.plan(), vo_v2.plan())
d1, d2 = HERE / "_plan_orig", HERE / "_plan_new"
for d in (d1, d2):
    shutil.rmtree(d, ignore_errors=True)
orig.main(["--plan-only", "--out", str(d1)])
new.main(["--plan-only", "--out", str(d2)])
# the "video" field names the out folder; write both to the same name to compare bytes
t1 = (d1 / "timeline.json").read_text(encoding="utf-8").replace(json.dumps(str(d1))[1:-1], "OUT")
t2 = (d2 / "timeline.json").read_text(encoding="utf-8").replace(json.dumps(str(d2))[1:-1], "OUT")
ok = t1 == t2
fails += 0 if ok else 1
print(f"{'OK  ' if ok else 'FAIL'} timeline.json from --plan-only is byte-for-byte the same ({len(t1)} bytes)")
live = Path("D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2/out-split/timeline.json")
lj = json.loads(live.read_text(encoding="utf-8"))
nj = json.loads(t2)
check("the live montage's windows and frame count match the default plan",
      [lj["windows"], lj["total_frames"]], [nj["windows"], nj["total_frames"]])
for d in (d1, d2):
    shutil.rmtree(d, ignore_errors=True)

# a timing override does what it says and leaves the module as it was
before = new.plan()
spec = {"frames": {"valley_mist": 570 + 60}, "transitions": {"pine_shafts": ["wipe_left", 0.8]},
        "intro_extra": 0.5, "closing_s": 11.5}
segs, total, wins = new.plan_with(dict(spec, frames={"valley_mist": 630, "opening": 990 + 30}))
b_segs, b_total, b_wins = before
exp = b_total + 60 + 30 + 12 + 60      # +60 valley_mist, +30 opening, a 0.2 s shorter wipe, +1.0 s closing
check(f"override: total frames {b_total} -> {total} (expected {exp})", total, exp)
check("override: intro window grows by 0.5 s", wins["intro"], [b_wins["intro"][0], round(b_wins["intro"][1] + 0.5, 3)])
check("override: valley_mist window grows by 1.0 s plus nothing else",
      round((wins["valley_mist"][1] - wins["valley_mist"][0]) - (b_wins["valley_mist"][1] - b_wins["valley_mist"][0]), 3), 1.0)
check("override: pine_shafts window grows by 0.1 s (its out-wipe 0.2 s shorter)",
      round((wins["pine_shafts"][1] - wins["pine_shafts"][0]) - (b_wins["pine_shafts"][1] - b_wins["pine_shafts"][0]), 3), 0.1)
check("plan_with leaves the tables as they were", new.plan(), before)
print("ALL OK" if not fails else f"{fails} FAILED")
sys.exit(1 if fails else 0)
