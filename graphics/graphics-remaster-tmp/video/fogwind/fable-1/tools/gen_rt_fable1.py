#!/usr/bin/env python
"""Agent 1 (fogwind): REAL-TIME 60 fps cadence probe of ud_valley_mist for the mist-1 scratch build
(its triMistPan / triMistRec verbs; see mist/1/REPORT.txt). One render per 1/60 s tick, every presented
frame recorded = what a player at a steady 60 fps sees.

  python gen_rt_probe.py <tag> [--n 200]
Writes video-work/.../video/look_probe_fog1_rt_<tag>.test.sqf/.toml with two segments:
  rt60      the clip's pan from t0 = -0.5 s, n frames  -> fogwind/1/rec/<tag>_rt60.raw
  rtstatic  the camera HELD at the frame 328 pose (heading 40.3, pitch -5.385), n frames, real time
            -> fogwind/1/rec/<tag>_rtst.raw
Then: python mist/1/rec2mkv.py <raw> (mkv, deletes the raw).
"""
import argparse, os

VID = "D:/Arma_CWA_worktrees/video-work/arma_CWR/tests/rendering/captures/video"
REC = "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/fogwind/fable-1/rec"
ap = argparse.ArgumentParser()
ap.add_argument("tag")
ap.add_argument("--n", type=int, default=200)
o = ap.parse_args()
src = open(os.path.join(VID, "look_probe_mist1_pc.test.sqf"), encoding="utf-8").read().split("\n")
cut = [i for i, l in enumerate(src) if "REAL-TIME 60 FPS CADENCE segment" in l][0]
head = src[:cut]
name = f"look_probe_fable1_rt_{o.tag}"
head[0] = f"// fable-1 (fogwind, from agent 1 gen_rt_probe.py) real-time probe {name}, mist-1 build only (gen_rt_probe.py, from look_probe_mist1_pc)"
PAN = "6095.77, capEyeASL, 4992.05, 28, -6, 48, -5, 569 / 60"
HOLD = "6095.77, capEyeASL, 4992.05, 40.3, -5.385, 40.3, -5.385, 569 / 60"
def seg(label, pan, t0, raw):
    return [
        f"// ---- agent 1 (fogwind): {label}",
        "call { capStepRes = triSetFixedStep 0; setAccTime 1 }",
        f"call {{ capPanRes = triMistPan [{pan}, -100] }}",
        "triSimFrames 90",
        "call { capSimRes = triSetSimTime capSimT; capStepRes = triSetFixedStep (1/60); setAccTime 1 }",
        f'call {{ capPanRes = triMistPan [{pan}, {t0}]; capRecRes = triMistRec ["{raw}", {o.n}, 0, 0]; '
        f'format ["view=%1;event=f1_{label};pan=%2;rec=%3", capView, capPanRes, capRecRes] call capLog }}',
        "triMistRecWait",
        f'call {{ capStepRes = triSetFixedStep 0; format ["view=%1;event=f1_{label}_done;state=%2", capView, triMistRecState] call capLog }}',
    ]
out = head + seg("rt60", PAN, -0.5, f"{REC}/{o.tag}_rt60.raw") + seg("rtstatic", HOLD, 0, f"{REC}/{o.tag}_rtst.raw")
out += ["[] call capClear", "call { capDriftRes = triPinCloudDrift [] }", "", 'call { "view=done;event=done" call capLog }',
        "triEndTest", ""]
with open(os.path.join(VID, name + ".test.sqf"), "w", encoding="utf-8", newline="\n") as f:
    f.write("\n".join(out))
toml = open(os.path.join(VID, "look_probe_mist1_pc.test.toml"), encoding="utf-8").read().split("\n")
toml[0] = f"# fable-1 (fogwind, from agent 1 gen_rt_probe.py) real-time probe {name}, mist-1 build only. gen_rt_probe.py"
with open(os.path.join(VID, name + ".test.toml"), "w", encoding="utf-8", newline="\n") as f:
    f.write("\n".join(toml))
os.makedirs(REC, exist_ok=True)
print("wrote", name, len(out), "lines")
