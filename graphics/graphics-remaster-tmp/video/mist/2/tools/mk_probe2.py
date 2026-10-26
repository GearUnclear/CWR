#!/usr/bin/env python
"""Agent 2 (mist swarm): cut ud_valley_mist to a frame window and change the capture pacing.

  python mk_probe2.py --tag w4 --warm 4            K extra world renders at accTime 0 (pose held) before each shot
  python mk_probe2.py --tag rt --rt 10             one render per sim step: B shots per call statement
  python mk_probe2.py --tag base                   the clip's own loop (same as agent 6's base)

Based on tmp/video/mist/6/mk_probe.py (same window: 0-based frames 240..389, the
first 30 shots are warm-up; measure with bandmetric --from 30).

--warm K: the step line becomes
    call { setAccTime 1; triSimFrames 1; setAccTime 0; triSimFrames K }
  (K full AppIdle ticks at accTime 0: the world is drawn K more times at the same
  pose and the same sim state before the screenshot statement).
--rt B: every shot becomes
    [k] call capClockAt; [k, 570] call capPathAt; [k] call capPropsAt;
    triScreenshot "label"; setAccTime 1; triSimFrames 1; setAccTime 0; ["label", k] call capFrameLog
  and B shots go into one call { } statement, so inside a batch exactly ONE frame is
  rendered per 1/60 s sim step (as in real-time play at 60 fps); the screenshot is
  pending before the tick, so it captures that tick's presented frame.
"""
import argparse
import os
import re

D = "D:/Arma_CWA_worktrees/video-work/arma_CWR/tests/rendering/captures/video"
STEP = "call { setAccTime 1; triSimFrames 1; setAccTime 0 }"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--tag", required=True)
    ap.add_argument("--mode", default="remastered")
    ap.add_argument("--first", type=int, default=240)
    ap.add_argument("--last", type=int, default=390)
    ap.add_argument("--warm", type=int, default=0)
    ap.add_argument("--rt", type=int, default=0)
    a = ap.parse_args()
    src = os.path.join(D, "ud_valley_mist.test.sqf")
    lines = open(src, encoding="utf-8").read().split("\n")
    toml = open(src[:-4] + ".toml", encoding="utf-8").read()
    start = next(i for i, l in enumerate(lines) if l.strip() == "[0] call capClockAt")
    first = next(i for i, l in enumerate(lines) if l.strip() == f"[{a.first}] call capClockAt")
    cut = next((i for i, l in enumerate(lines) if l.strip() == f"[{a.last}] call capClockAt"), None)
    last_log = max(i for i, l in enumerate(lines) if l.rstrip().endswith("call capFrameLog"))
    head = lines[:start]
    assert sum(1 for l in head if l.strip() == '"remastered" call capMode') == 1
    head = [f'"{a.mode}" call capMode' if l.strip() == '"remastered" call capMode' else l for l in head]
    body = lines[first:cut] if cut is not None else lines[first:last_log + 1]
    tail = lines[last_log + 1:]
    body = [l for l in body if l.strip()]
    assert len(body) % 6 == 0, len(body)
    shots = [body[i:i + 6] for i in range(0, len(body), 6)]
    for s in shots:
        assert s[0].endswith("call capClockAt") and s[1].endswith("call capPathAt") and s[2].endswith("call capPropsAt")
        assert s[3] == STEP and s[4].startswith("triScreenshot ") and s[5].endswith("call capFrameLog"), s
    new = []
    if a.rt:
        for i in range(0, len(shots), a.rt):
            parts = []
            for s in shots[i:i + a.rt]:
                parts += [s[0], s[1], s[2], s[4], "setAccTime 1", "triSimFrames 1", "setAccTime 0", s[5]]
            new.append("call { " + "; ".join(parts) + " }")
    else:
        for s in shots:
            step = STEP if a.warm == 0 else f"call {{ setAccTime 1; triSimFrames 1; setAccTime 0; triSimFrames {a.warm} }}"
            new += [s[0], s[1], s[2], step, s[4], s[5]]
    desc = f"warm {a.warm}" if not a.rt else f"rt batches of {a.rt}"
    out = [f"// mist probe (agent 2): ud_valley_mist frames {a.first}..{a.last - 1} (0-based), mode {a.mode}, {desc}; made by tmp/video/mist/2/tools/mk_probe2.py"]
    out += head + new + tail
    name = f"look_probe_mist2_{a.tag}"
    with open(os.path.join(D, name + ".test.sqf"), "w", newline="\n", encoding="utf-8") as f:
        f.write("\n".join(out))
    toml = re.sub(r"^# ", f"# mist probe (agent 2) {a.tag} from ", toml, count=1)
    with open(os.path.join(D, name + ".test.toml"), "w", newline="\n", encoding="utf-8") as f:
        f.write(toml)
    n = sum(l.count("triScreenshot") for l in out)
    print(os.path.join(D, name + ".test.sqf"), "shots", n, desc)


if __name__ == "__main__":
    main()
