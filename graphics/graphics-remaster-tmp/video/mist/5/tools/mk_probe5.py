#!/usr/bin/env python
"""Agent 6 (mist swarm): cut ud_valley_mist to a frame window and swap its lighting mode.

  python mk_probe.py --tag base --mode "remastered" [--first 240] [--last 390] [--preclip "<sqf>"]...

Writes look_probe_mist5_<tag>.test.sqf/.toml next to ud_valley_mist in video-work
(never touches the source test). The probe keeps the clip's whole staging (spawn,
settle, capMode, capPin) and then shoots frames with 0-based index first..last-1
exactly as the real clip does (exact 1/60 s step per shot, same camera path index,
same screenshot names valley_mist__ud__fNNNN, 1-based). The first 30 shots are
warm-up for the temporal filters (TAA, fog history) after the jump from the staging
view; measure with bandmetric --from 30 to compare with the real clip's
--from 270 --to 390.

--mode replaces the "remastered" given to capMode (a triSetLighting setting, e.g.
"remastered,volFog=0"). --preclip statements go right before the first frame.
"""
import argparse
import os
import re

D = "D:/Arma_CWA_worktrees/video-work/arma_CWR/tests/rendering/captures/video"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--tag", required=True)
    ap.add_argument("--mode", default="remastered")
    ap.add_argument("--first", type=int, default=240)
    ap.add_argument("--last", type=int, default=390)
    ap.add_argument("--preclip", action="append", default=[])
    a = ap.parse_args()
    src = os.path.join(D, "ud_valley_mist.test.sqf")
    lines = open(src, encoding="utf-8").read().split("\n")
    toml = open(src[:-4] + ".toml", encoding="utf-8").read()
    start = next(i for i, l in enumerate(lines) if l.strip() == "[0] call capClockAt")
    first = next(i for i, l in enumerate(lines) if l.strip() == f"[{a.first}] call capClockAt")
    cut = next((i for i, l in enumerate(lines) if l.strip() == f"[{a.last}] call capClockAt"), None)
    last_log = max(i for i, l in enumerate(lines) if l.rstrip().endswith("call capFrameLog"))
    head = lines[:start]
    n_mode = sum(1 for l in head if l.strip() == '"remastered" call capMode')
    assert n_mode == 1, n_mode
    head = [f'"{a.mode}" call capMode' if l.strip() == '"remastered" call capMode' else l for l in head]
    body = lines[first:cut] if cut is not None else lines[first:last_log + 1]
    tail = lines[last_log + 1:]
    out = [f"// mist probe (agent 5, from agent 6 mk_probe.py): ud_valley_mist frames {a.first}..{a.last - 1} (0-based), mode {a.mode}; made by tmp/video/mist/5/tools/mk_probe5.py"]
    out += head + a.preclip + body + tail
    name = f"look_probe_mist5_{a.tag}"
    with open(os.path.join(D, name + ".test.sqf"), "w", newline="\n", encoding="utf-8") as f:
        f.write("\n".join(out))
    toml = re.sub(r"^# ", f"# mist probe (agent 5, from agent 6 mk_probe.py) {a.tag} from ", toml, count=1)
    with open(os.path.join(D, name + ".test.toml"), "w", newline="\n", encoding="utf-8") as f:
        f.write(toml)
    n = sum(1 for l in out if l.startswith("triScreenshot"))
    print(os.path.join(D, name + ".test.sqf"), "shots", n, "mode", a.mode)


if __name__ == "__main__":
    main()
