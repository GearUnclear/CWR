"""Agent 1 (fogwind) cut-down probes of ud_valley_mist (overhaul side), in video-work's video test folder.

usage: python gen_probe_fog1.py <what> <static|moving> [--k0 270] [--n 120] [--hold 327] [--mode remastered]
Writes look_probe_fog1_<what>.test.sqf/.toml next to ud_valley_mist.test.sqf: the real test's staging
and recipe (exact 1/60 s step per shot) up to the fixed-step line, then n shots: clock and props at
k0..k0+n-1, the camera on the real path (moving) or held at frame k = hold (static).
"""
import argparse, os, re

VID = "D:/Arma_CWA_worktrees/video-work/arma_CWR/tests/rendering/captures/video"
ap = argparse.ArgumentParser()
ap.add_argument("what")
ap.add_argument("kind", choices=["static", "moving"])
ap.add_argument("--k0", type=int, default=270)
ap.add_argument("--n", type=int, default=120)
ap.add_argument("--hold", type=int, default=327)
ap.add_argument("--mode", default="remastered")
o = ap.parse_args()

src = open(os.path.join(VID, "ud_valley_mist.test.sqf"), encoding="utf-8").read().split("\n")
toml = open(os.path.join(VID, "ud_valley_mist.test.toml"), encoding="utf-8").read()
STEP_LINE = "call { capStepRes = triSetFixedStep (1/60); setAccTime 0 }"
cut = src.index(STEP_LINE)
head = src[:cut + 1]
mode_i = head.index('"remastered" call capMode')
head[mode_i] = f'"{o.mode}" call capMode'
name = f"look_probe_fog1_{o.what}"
head[0] = f"// agent 1 (fogwind) probe {name}: {o.kind}, k0={o.k0}, n={o.n}, hold={o.hold}, mode={o.mode} (gen_probe_fog1.py, from ud_valley_mist)"
out = list(head)
for i in range(o.n):
    k = o.k0 + i
    pk = o.hold if o.kind == "static" else k
    lab = f"{name}__ud__f{i + 1:04d}"
    out += [f"[{k}] call capClockAt", f"[{pk}, 570] call capPathAt", f"[{k}] call capPropsAt",
            "call { setAccTime 1; triSimFrames 1; setAccTime 0 }", f'triScreenshot "{lab}"', f'["{lab}", {k}] call capFrameLog']
out += ["[] call capClear", "call { capDriftRes = triPinCloudDrift [] }", "", 'call { "view=done;event=done" call capLog }',
        "triEndTest", ""]
with open(os.path.join(VID, name + ".test.sqf"), "w", encoding="utf-8", newline="\n") as f:
    f.write("\n".join(out))
t = toml.split("\n")
t[0] = f"# agent 1 (fogwind) probe {name} ({o.kind}), from ud_valley_mist. gen_probe_fog1.py"
with open(os.path.join(VID, name + ".test.toml"), "w", encoding="utf-8", newline="\n") as f:
    f.write("\n".join(t))
print("wrote", name, len(out), "lines")
