"""Generate the feature A/B capture test (plan sections 3.1 and 8 items 5 and 6).

    python gen_features_ab.py [--only <feature>[,<feature>...]] [--legacy] [--out <dir>]

Writes features_ab.test.sqf + .test.toml next to this file: one hero view per
feature (views.HERO, the start view of the feature's first row in plan section
7.3), each shot twice in one game, first with the feature's switches off
(`triSetLighting "remastered,<key>=0,..."`, label <view>__off) and then with
the shipped defaults (`"remastered"`, label <view>__on). Same camera, date,
weather, props, sim clock and clouds for both (capPin, triPinCloudDrift); the
sim clock is a fixed value per view (SIM_T0 + SIM_DT * index), so runs and
builds see the same sea too. 1920x1080, scale 1.0, 4x MSAA, the presented frame; every shot line carries
feat= (triFeatureState: each switch and its frame gate).

How the gate uses it:
  - item 6 (capture pair): <view>__off against <view>__on of one run, built by
    make_pairs.py into <view>__pair.png;
  - item 5 (off equals baseline): <view>__off of the new build against
    <view>__on of the previous step's build (banddiff.py run_new run_old
    --label-a off --label-b on), judged against the repeat noise of the same
    view (__off against __on of the previous build, where nothing differs).
    On a build older than A1 the switch keys are refused (triSetLighting answers
    FAIL:unknown knob, logged), so both of its shots are plain "remastered";
    such a build also lacks triPinCloudDrift, so generate its copy with
    --legacy (written as features_ab_legacy.test.sqf, cloud field not pinned).
"""
import argparse
from pathlib import Path

import capgen
import views

NAME = "features_ab"
# Each view's sim clock in seconds: fixed, so the sea and everything else keyed
# on Glob.time match between runs and builds; far apart, so no view lasts long
# enough to reach the next one's clock (time only jumps forward between views).
SIM_T0, SIM_DT = 2000, 600


def view_block(v, legacy, index):
    off = "remastered," + ",".join(f"{k}=0" for k in v["keys"])
    out = [f"\n// ---- {v['name']}: {v['feature']} (off = {off})"]
    out += capgen.begin(v, v["name"], legacy, sim_time=SIM_T0 + SIM_DT * index)
    out.append("triSimFrames 150")
    out += capgen.shot(v, v["name"], off, f"{v['name']}__off")
    out += capgen.shot(v, v["name"], "remastered", f"{v['name']}__on")
    out += capgen.end(v, legacy)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--only", default="", help="comma list of features (views.HERO 'feature' values or view names)")
    ap.add_argument("--legacy", action="store_true", help="for builds older than A1 (no triPinCloudDrift)")
    ap.add_argument("--out", default=str(capgen.HERE))
    a = ap.parse_args()
    want = {s.strip() for s in a.only.split(",") if s.strip()}
    vs = [v for v in views.HERO if not want or v["feature"] in want or v["name"] in want]
    if not vs:
        raise SystemExit(f"no hero view matches {sorted(want)}")
    name = NAME + ("_legacy" if a.legacy else "")
    title = "Feature A/B hero stills on Abel: gen_features_ab.py; <view>__off then <view>__on."
    if a.legacy:
        title += " Legacy copy for builds older than A1 (cloud field not pinned)."
    lines = [capgen.header("Abel", vs[0], title)]
    for v in vs:
        lines += view_block(v, a.legacy, views.HERO.index(v))
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = Path(a.out)
    capgen.write(out / f"{name}.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / f"{name}.test.toml", capgen.toml("Abel", title, views.WORLDS, timeout=3600))
    print(f"{name}: {len(vs)} hero views")


if __name__ == "__main__":
    main()
