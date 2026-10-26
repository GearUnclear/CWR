#!/usr/bin/env python
"""Agent 1 (mist swarm): real-time vs stepped probe of ud_valley_mist on the mist-1 scratch build.

  python mk_probe1.py --tag rtst [--rt 360] [--st 150] [--order rt,st] [--allst 0]

Keeps the clip's whole staging (spawn, gaze hold, settle, capMode, capPin, kneel), drops
the clip's frame loop, and replaces it with segments that use two scratch-only verbs of
the mist-1 build:
  triMistPan [east, eyeASL, north, h0, p0, h1, p1, dur, t0]: the clip's own smoothstep
    pan (heading 28 -> 48, pitch -6 -> -5 over 569/60 s from the path start), driven by
    the accumulated accelerated sim step (so real time and stepped play the same pan).
  triMistRec [path, n, mode, half]: in-memory recorder of presented frames at the
    triScreenshot capture point (mode 0 every presented frame, 1 only triScreenshot frames).
Segments:
  rt: real time (triSetFixedStep 0, setAccTime 1): pan from t0 = -0.5 s, record n
      consecutive presented frames while the main loop runs freely (the runner only polls
      triMistRecWait; a single long statement would exceed tri's 30 s eval timeout).
  st: the clip's exact head-recipe stepping (triSetFixedStep 1/60, setAccTime 0 between
      shots, capClockAt per shot, one tick per shot, triScreenshot at present,
      capFrameLog) over the same pan from t0 = -0.5 s; the recorder (mode 1) keeps the
      shot frames too.
  allst: as st but the recorder keeps EVERY presented frame (half res) for that many
      shots, to show the render-only frames between shots.
"""
import argparse
import os
import re

D = "D:/Arma_CWA_worktrees/video-work/arma_CWR/tests/rendering/captures/video"
OUT = "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/mist/1/rec"
PAN = "6095.77, capEyeASL, 4992.05, 28, -6, 48, -5, 569 / 60"


def seg_rt(tag, n):
    return [
        "// ---- agent 1: REAL-TIME segment",
        "call { capStepRes = triSetFixedStep 0; setAccTime 1 }",
        f"call {{ capPanRes = triMistPan [{PAN}, -100] }}",
        "triSimFrames 90",
        f'call {{ capPanRes = triMistPan [{PAN}, -0.5]; capRecRes = triMistRec ["{OUT}/{tag}_rt.raw", {n}, 0, 0]; '
        f'format ["view=%1;event=a1_rt;pan=%2;rec=%3", capView, capPanRes, capRecRes] call capLog }}',
        "triMistRecWait",
        'call { format ["view=%1;event=a1_rt_done;state=%2", capView, triMistRecState] call capLog }',
    ]


def seg_rt60(tag, n):
    """Real-time cadence at a perfect 60 fps: accTime 1, a fixed 1/60 s sim step, one
    render and present per sim tick (n consecutive AppIdle ticks in one statement), every
    presented frame recorded. The world, the pan and every per-frame filter see exactly
    what a player at 60 fps sees; only wall-clock items (exposure adaptation dt) differ."""
    return [
        "// ---- agent 1: REAL-TIME 60 FPS CADENCE segment (fixed 1/60 s step, one render per tick)",
        "call { capStepRes = triSetFixedStep 0; setAccTime 1 }",
        f"call {{ capPanRes = triMistPan [{PAN}, -100] }}",
        "triSimFrames 90",
        "call { capSimRes = triSetSimTime capSimT; capStepRes = triSetFixedStep (1/60); setAccTime 1 }",
        f'call {{ capPanRes = triMistPan [{PAN}, -0.5]; capRecRes = triMistRec ["{OUT}/{tag}_rt60.raw", {n}, 0, 0]; '
        f'format ["view=%1;event=a1_rt60;pan=%2;rec=%3", capView, capPanRes, capRecRes] call capLog }}',
        "triMistRecWait",
        'call { capStepRes = triSetFixedStep 0; format ["view=%1;event=a1_rt60_done;state=%2", capView, triMistRecState] call capLog }',
    ]


def seg_st(tag, n, label, mode, half, first_k=0):
    out = [
        f"// ---- agent 1: STEPPED segment ({label})",
        "call { capStepRes = triSetFixedStep 0; setAccTime 1 }",
        f"call {{ capPanRes = triMistPan [{PAN}, -100] }}",
        "triSimFrames 90",
        "call { capSimRes = triSetSimTime capSimT; capStepRes = triSetFixedStep (1/60); setAccTime 0 }",
        f'call {{ capPanRes = triMistPan [{PAN}, -0.5]; capRecRes = triMistRec ["{OUT}/{tag}_{label}.raw", {n if mode == 1 else n * 40}, {mode}, {half}]; '
        f'format ["view=%1;event=a1_{label};pan=%2;rec=%3", capView, capPanRes, capRecRes] call capLog }}',
    ]
    for k in range(n):
        lab = f"valley_mist__a1{label}__f{k + 1:04d}"
        out += [f"[{first_k + k}] call capClockAt",
                "call { setAccTime 1; triSimFrames 1; setAccTime 0 }",
                f'triScreenshot "{lab}"',
                f'["{lab}", {first_k + k}] call capFrameLog']
    out += [f'call {{ capRecRes = triMistRec ["", 0, 0, 0]; format ["view=%1;event=a1_{label}_stop;rec=%2", capView, capRecRes] call capLog }}',
            "triSimFrames 2",
            "triMistRecWait",
            f'call {{ format ["view=%1;event=a1_{label}_done;state=%2", capView, triMistRecState] call capLog }}']
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--tag", required=True)
    ap.add_argument("--order", default="rt,st")
    ap.add_argument("--rt", type=int, default=360)
    ap.add_argument("--st", type=int, default=150)
    ap.add_argument("--allst", type=int, default=0)
    a = ap.parse_args()
    src = os.path.join(D, "ud_valley_mist.test.sqf")
    lines = open(src, encoding="utf-8").read().split("\n")
    toml = open(src[:-4] + ".toml", encoding="utf-8").read()
    start = next(i for i, l in enumerate(lines) if l.strip() == "[0] call capClockAt")
    head = [l for l in lines[:start] if "triSetFixedStep" not in l]
    last_log = max(i for i, l in enumerate(lines) if l.rstrip().endswith("call capFrameLog"))
    tail = lines[last_log + 1:]
    body = ["[0, 570] call capPathAt"]
    for seg in a.order.split(","):
        if seg == "rt":
            body += seg_rt(a.tag, a.rt)
        elif seg == "rt60":
            body += seg_rt60(a.tag, a.rt)
        elif seg == "st":
            body += seg_st(a.tag, a.st, "st", 1, 0)
        elif seg == "allst":
            body += seg_st(a.tag, a.allst, "allst", 0, 1)
    out = [f"// mist probe (agent 1, mist-1 scratch build only): {a.order}; made by tmp/video/mist/1/mk_probe1.py"]
    out += head + body + tail
    name = f"look_probe_mist1_{a.tag}"
    with open(os.path.join(D, name + ".test.sqf"), "w", newline="\n", encoding="utf-8") as f:
        f.write("\n".join(out))
    toml = re.sub(r"^# ", f"# mist probe (agent 1) {a.tag}, mist-1 build only, from ", toml, count=1)
    with open(os.path.join(D, name + ".test.toml"), "w", newline="\n", encoding="utf-8") as f:
        f.write(toml)
    n = sum(1 for l in out if l.startswith("triScreenshot"))
    print(os.path.join(D, name + ".test.sqf"), "shots", n)


if __name__ == "__main__":
    main()
