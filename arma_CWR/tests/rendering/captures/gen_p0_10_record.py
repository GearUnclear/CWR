"""P0-10 recorder check: the triRecord frames against triScreenshot of the same
frames, which must be byte-identical (0 differing pixels).

    python gen_p0_10_record.py [--out <dir>]

Writes p0_10_rec_check_abel.test.sqf/.toml next to this file (or into --out).
They need a build with the P0-10 verbs (triRecord, triRecordState,
triCamPath); on any other build the verbs are unknown commands (script errors).

One Abel session at valley_0515 (mist, haze and foliage, so consecutive frames
differ), staged as the generated capture tests are (capgen), settled in
"remastered", the whole run at the fixed 1/60 s step with setAccTime 1, so the
main loop renders one frame per simulation tick:
  shots  triRecord mode 1 (only frames a triScreenshot captured), 4 shots a
         few frames apart: rec_shots.raw frame k = shot rec_shots_k;
  every  triRecord mode 0 (every presented frame) during a slow triCamPath
         pan, with 3 shots inside it, then a stop: the frames flagged as shot
         frames in rec_every.raw.txt = rec_every_1..3;
  half   triRecord mode 2 (every frame, half size), 1 shot, then a stop: the
         flagged frame = the 2x2 box mean (a + b + c + d + 2) / 4 of
         rec_half_1.
triCaptureAtPresent true (capgen's header), so triScreenshot reads at present,
where the recorder reads. The comparison (gate/recorder.py check, through
squad-heavy -Kind analysis) expects 0 differing pixels on every pair and as
many flagged frames as shots.
"""
import argparse
from pathlib import Path

import capgen
import views

STEP = 1.0 / 60.0
SIM_T = 2000
VIEW = "valley_0515"


def rec_block(name, frames, mode, shots, gap, pan=False):
    out = [f"\n// ---- {name}: triRecord mode {mode}"]
    if pan:
        out.append("call { capPathRes = triCamPath [[0, capOX, capEyeASL, capOZ, capH, capP], "
                   "[6, capOX, capEyeASL, capOZ, capH + 12, capP + 1, 1]]; "
                   f'format ["view=%1;event=campath;rec={name};res=%2", capView, capPathRes] call capLog }}')
    out.append(f'call {{ capRecRes = triRecord ["p0_10_rec/rec_{name}.raw", {frames}, {mode}]; '
               f'format ["view=%1;event=record;rec={name};res=%2", capView, capRecRes] call capLog }}')
    for k in range(1, shots + 1):
        out += [f"triSimFrames {gap}", f'triScreenshot "rec_{name}_{k}"', f'"rec_{name}_{k}" call capRecord']
    if mode != 1:
        out += ["triSimFrames 4",
                f'call {{ capRecRes = triRecord ["p0_10_rec/rec_{name}.raw", 0, {mode}]; '
                f'format ["view=%1;event=record_stop;rec={name};res=%2", capView, capRecRes] call capLog }}']
    out += ["triRecordState",
            f'call {{ format ["view=%1;event=record_done;rec={name};state=%2", capView, triRecordState] call capLog }}']
    if pan:
        out.append('call { capPathRes = triCamPath []; [] call capPin }')
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default=str(capgen.HERE))
    a = ap.parse_args()
    out = Path(a.out)
    title = "P0-10 recorder check (triRecord frames against triScreenshot; needs the P0-10 verbs)."
    v = views._timing(VIEW)
    lines = [capgen.header("Abel", v, title + " Abel."),
             f'call {{ capStepRes = triSetFixedStep {STEP:.7f}; setAccTime 1; '
             'format ["view=setup;event=step;res=%1", capStepRes] call capLog }']
    lines.append(f"\n// ---- {VIEW}")
    lines += capgen.begin(v, VIEW, sim_time=SIM_T)
    lines.append("triSimFrames 150")
    lines += ['"remastered" call capMode', "[] call capPin"]
    if v.get("refresh"):
        lines.append(v["refresh"])
    lines.append("triSimFrames 60")
    lines += rec_block("shots", 4, 1, 4, 3)
    lines += rec_block("every", 600, 0, 3, 5, pan=True)
    lines += rec_block("half", 600, 2, 1, 5)
    lines += capgen.end(v)
    lines += ['\ncall { capStepRes = triSetFixedStep 0; "view=done;event=done" call capLog }', "triEndTest"]
    capgen.write(out / "p0_10_rec_check_abel.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / "p0_10_rec_check_abel.test.toml", capgen.toml("Abel", title, views.WORLDS, timeout=1800))
    print("p0_10_rec_check: abel valley_0515, shots 4 (mode 1), every 3 shots (mode 0, pan), half 1 shot (mode 2)")


if __name__ == "__main__":
    main()
