"""A7 single timing reading: the sky table's cost, off against on in the same
view and round, plus a time-lapse that rebuilds the sky-view table on every
frame (its `skytable` GPU section).

    python gen_a7_timing.py <captures dir> [--name a7_timing] [--rounds 3]

Same rig as gen_timing.py (1920x1080, scale 1.0, 4x MSAA, presented frame).
Views: coastline_sunset (heading 250, the hero), sunrise_ridge (the long view)
and noon_fields; each round shoots "remastered,atmosphere=0" then "remastered"
(labels <view>_r<k>__off / __on, so make_pairs.py and summarize_timing.py read
them). The time-lapse runs sunrise_ridge from 04:40, one game minute per frame
for 60 frames (the sun moves about 0.15 degrees a minute, over the 0.02 degree
rebuild step), after a GPU timer reset, and logs triGpuTimes as event=timelapse.
"""
import argparse
import sys
from pathlib import Path

ap = argparse.ArgumentParser()
ap.add_argument("captures")
ap.add_argument("--name", default="a7_timing")
ap.add_argument("--rounds", type=int, default=3)
a = ap.parse_args()
sys.path.insert(0, a.captures)
import capgen  # noqa: E402
import views  # noqa: E402


def timing(base, **changes):
    for v in views.TIMING_ABEL:
        if v["name"] == base:
            out = dict(v)
            for k in ("f10", "note", "props", "refresh", "cleanup"):
                out.pop(k, None)
            out.update(changes)
            return out
    raise KeyError(base)


VIEWS = [
    timing("coastline_sunset", h=250),
    timing("sunrise_ridge"),
    timing("noon_fields"),
]

lines = [capgen.header("Abel", VIEWS[0], "A7 sky table timing: atmosphere off then on, interleaved rounds.")]
for rnd in range(1, a.rounds + 1):
    for v in VIEWS:
        label = f"{v['name']}_r{rnd}"
        lines.append(f"\n// ---- {label}")
        lines += capgen.begin(v, label)
        lines.append("triSimFrames 150")
        lines += capgen.shot(v, label, "remastered,atmosphere=0", f"{label}__off")
        lines += capgen.shot(v, label, "remastered", f"{label}__on")
        lines += capgen.end(v)

tl = timing("sunrise_ridge", name="timelapse", date=[1985, 7, 5, 4, 40])
lines.append("\n// ---- time-lapse: the sky-view table rebuilt on every frame")
lines += capgen.begin(tl, "timelapse")
lines.append("triSimFrames 150")
lines.append('call { capLit = triSetLighting "remastered"; capPinRes = triPinObjectDetail capDetail; '
             'capGpuOff = triGpuTimes false; capGpuOn = triGpuTimes true }')
for k in range(60):
    minute = 40 + k
    hour = 4 + minute // 60
    lines.append(f"call {{ setDate [1985, 7, 5, {hour}, {minute % 60}] }}")
    lines.append("triSimFrames 1")
lines.append('call { capGpu = triGpuTimes true; format ["view=timelapse;event=timelapse;feat=%1;gpu=%2", '
             'triFeatureState, capGpu] call capLog }')
lines += capgen.end(tl)
lines.append('\ncall { "view=done;event=done" call capLog }')
lines.append("triEndTest")
out = Path(a.captures)
capgen.write(out / f"{a.name}.test.sqf", "\n".join(lines) + "\n")
capgen.write(out / f"{a.name}.test.toml", capgen.toml("Abel", "A7 sky table timing", views.WORLDS, timeout=3600))
print(f"{a.name}: {len(VIEWS)} views x {a.rounds} rounds + time-lapse")
