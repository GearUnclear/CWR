#!/usr/bin/env python
"""Copy a generated v2 clip test (ud_<clip> or van_<clip>), cut it to its first
N frames, and apply a head-fix recipe, under a probe name of our own:

  python apply_fix.py --side ud  --clip checkpoint --frames 120 --tag fix [--preclip "<sqf>"]... [--step "<sqf>"]...
  python apply_fix.py --side van --clip checkpoint --frames 120 --tag base          (no fix: the before shot)

  --preclip  top-level statements inserted once right before frame 0 (after
             the clip's capMode and capPin)
  --step     the statements that replace each frame's  triSimFrames 2
             (omit to keep the clip's own step)
Writes look_probe_head8_<clip>_<tag> (video-work) or van_probe_head8_<clip>_<tag>
(vanilla-upstream) .test.sqf + .test.toml; never touches the source tests.
Run with run_repro.ps1 -Side ud|van -Agent 8 -Name <clip>_<tag> (no plate:
measure with metric.py --boxes ... --track).
"""
import argparse
import os
import re

DIRS = {
    "ud": (r"D:\Arma_CWA_worktrees\video-work\arma_CWR\tests\rendering\captures\video", "ud", "look_probe_head8_"),
    "van": (r"D:\Arma_CWA_worktrees\vanilla-upstream\tests\rendering\captures\video", "van", "van_probe_head8_"),
}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--side", choices=["ud", "van"], required=True)
    ap.add_argument("--clip", required=True)
    ap.add_argument("--frames", type=int, default=120)
    ap.add_argument("--tag", required=True)
    ap.add_argument("--setup", action="append", default=[], help="inserted right after [] call capStageSave (before the settle frames)")
    ap.add_argument("--preclip", action="append", default=[])
    ap.add_argument("--step", action="append", default=[])
    a = ap.parse_args()
    d, pre, out_pre = DIRS[a.side]
    src = os.path.join(d, f"{pre}_{a.clip}.test.sqf")
    lines = open(src, encoding="utf-8").read().split("\n")
    toml = open(src[:-4] + ".toml", encoding="utf-8").read()
    start = next(i for i, l in enumerate(lines) if l.strip() == "[0] call capClockAt")
    cut = next((i for i, l in enumerate(lines) if l.strip() == f"[{a.frames}] call capClockAt"), None)
    last_log = max(i for i, l in enumerate(lines) if l.rstrip().endswith("call capFrameLog"))
    tail = lines[last_log + 1:]
    body = lines[start:cut] if cut is not None else lines[start:last_log + 1]
    if a.step:
        nb = []
        for l in body:
            if l.strip() == "triSimFrames 2":
                nb += a.step
            else:
                nb.append(l)
        body = nb
    out = [f"// head-fix probe (agent 8): {a.side}_{a.clip} cut to {a.frames} frames, tag {a.tag}; made by tmp/video/heads/repro/apply_fix.py from {os.path.basename(src)}"]
    head = lines[:start]
    if a.setup:
        j = next(i for i, l in enumerate(head) if l.strip() == "[] call capStageSave")
        head = head[:j + 1] + a.setup + head[j + 1:]
    out += head + a.preclip + body + tail
    name = f"{out_pre}{a.clip}_{a.tag}"
    with open(os.path.join(d, name + ".test.sqf"), "w", newline="\n", encoding="utf-8") as f:
        f.write("\n".join(out))
    toml = re.sub(r"^# ", f"# head-fix probe {a.tag} from ", toml, count=1)
    with open(os.path.join(d, name + ".test.toml"), "w", newline="\n", encoding="utf-8") as f:
        f.write(toml)
    n = sum(1 for l in out if l.startswith("triScreenshot"))
    print(os.path.join(d, name + ".test.sqf"), "shots", n)


if __name__ == "__main__":
    main()
