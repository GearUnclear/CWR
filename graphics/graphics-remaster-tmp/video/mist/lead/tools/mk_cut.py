#!/usr/bin/env python
"""Mist swarm lead: cut a generated ud_<clip> test to a frame window (agent 6's mk_probe.py, any clip).

  python mk_cut.py --clip cloud_shadows --first 120 --last 270

Writes look_probe_mistlead_<clip>.test.sqf/.toml next to ud_<clip> in video-work (the source test is
never touched). The probe keeps the clip's whole staging (spawn, gaze hold, settle, capMode, capPin)
and then shoots frames with 0-based index first..last-1 exactly as the real clip does (exact 1/60 s
step per shot, the same camera path index, the same screenshot names <clip>__ud__fNNNN, 1-based).
The first 30 shots are warm-up for the temporal filters after the jump from the staging view: measure
with bandmetric --from 30 and compare with the real clip's --from first+30 --to last.
Run with plain python.
"""
import argparse
import os
import re

D = "D:/Arma_CWA_worktrees/video-work/arma_CWR/tests/rendering/captures/video"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--clip", required=True)
    ap.add_argument("--first", type=int, required=True)
    ap.add_argument("--last", type=int, required=True)
    a = ap.parse_args()
    src = os.path.join(D, f"ud_{a.clip}.test.sqf")
    lines = open(src, encoding="utf-8").read().split("\n")
    toml = open(src[:-4] + ".toml", encoding="utf-8").read()
    start = next(i for i, l in enumerate(lines) if l.strip() == "[0] call capClockAt")
    first = next(i for i, l in enumerate(lines) if l.strip() == f"[{a.first}] call capClockAt")
    cut = next((i for i, l in enumerate(lines) if l.strip() == f"[{a.last}] call capClockAt"), None)
    last_log = max(i for i, l in enumerate(lines) if l.rstrip().endswith("call capFrameLog"))
    head = lines[:start]
    body = lines[first:cut] if cut is not None else lines[first:last_log + 1]
    tail = lines[last_log + 1:]
    out = [f"// mist lead probe: ud_{a.clip} frames {a.first}..{a.last - 1} (0-based), unchanged otherwise; "
           f"made by tmp/video/mist/lead/tools/mk_cut.py"]
    out += head + body + tail
    name = f"look_probe_mistlead_{a.clip}"
    with open(os.path.join(D, name + ".test.sqf"), "w", newline="\n", encoding="utf-8") as f:
        f.write("\n".join(out))
    toml = re.sub(r"^# ", f"# mist lead probe from ", toml, count=1)
    with open(os.path.join(D, name + ".test.toml"), "w", newline="\n", encoding="utf-8") as f:
        f.write(toml)
    n = sum(1 for l in out if l.startswith("triScreenshot"))
    print(os.path.join(D, name + ".test.sqf"), "shots", n)


if __name__ == "__main__":
    main()
