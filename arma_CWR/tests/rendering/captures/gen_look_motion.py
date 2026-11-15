"""Generate the look-light motion captures: the camera moves while the
photoreal camera (switch photoLight) meters and adapts, to show it does not
flicker, pump or leave TAA ghosts.

    python gen_look_motion.py [--modes "on=remastered;off=remastered,photoLight=0"] [--tag motion]

Two moves on Abel, each run once per mode:
  - pan_sun: lowsun_fp's camera turns from 60 degrees left of its heading
    across the low sun to 30 degrees right of it, 1.5 degrees a frame (a
    brisk mouse turn), then holds;
  - shade_open: woods_pine2_evening's camera tilts from 30 degrees down at the
    shaded forest floor up to 25 degrees through the crowns into the sky and
    back down, 1 degree a frame, then holds.
Every frame logs the camera's readout (triFeatureState's "| cam ev= target="
part) as event=motion; every `--every` frames a screenshot <move>__<mode>_<NNN>.
"""
import argparse
from pathlib import Path

import capgen
import views

MOVES = {
    "pan_sun": dict(view="lowsun_fp", axis="h", path=[(-60.0, 30.0, 1.5)], hold=90),
    "shade_open": dict(view="woods_pine2_evening", axis="p", path=[(-30.0, 25.0, 1.0), (25.0, -30.0, 1.0)], hold=90),
}


def find(name):
    for v in views.TIMING_ABEL:
        if v["name"] == name:
            v = dict(v)
            v.pop("f10", None)
            v.pop("note", None)
            return v
    raise KeyError(name)


def steps(path):
    out = []
    for a, b, d in path:
        n = int(round(abs(b - a) / d))
        s = d if b > a else -d
        out += [a + s * i for i in range(n + 1)]
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--modes", default="on=remastered;off=remastered,photoLight=0")
    ap.add_argument("--tag", default="motion")
    ap.add_argument("--every", type=int, default=10)
    ap.add_argument("--out", default=str(capgen.HERE))
    a = ap.parse_args()
    modes = [tuple(x.strip() for x in m.split("=", 1)) for m in a.modes.split(";") if m.strip()]
    first = find(MOVES["pan_sun"]["view"])
    title = "look-light motion captures on Abel: gen_look_motion.py (pan across a low sun, shade to sky)."
    lines = [capgen.header("Abel", first, title)]
    index = 0
    for move, spec in MOVES.items():
        v = find(spec["view"])
        base = v["h"] if spec["axis"] == "h" else v["p"]
        for label, setting in modes:
            name = f"{move}__{label}"
            lines.append(f"\n// ---- {name}")
            lines += capgen.begin(v, name, False, sim_time=2000 + 600 * index)
            index += 1
            lines.append("triSimFrames 150")
            lines.append(f'"{setting}" call capMode')
            var = "capH" if spec["axis"] == "h" else "capP"
            offs = steps(spec["path"])
            lines.append(f"call {{ {var} = {capgen.num(base + offs[0])}; [] call capPinView }}")
            lines.append("triSimFrames 120")
            frame = 0
            for k, o in enumerate(offs + [offs[-1]] * spec["hold"]):
                lines.append(f"call {{ {var} = {capgen.num(base + o)}; [] call capPinView }}")
                lines.append("triSimFrames 1")
                lines.append(f'call {{ format ["view=%1;event=motion;move={move};mode={label};frame={frame};'
                             f'offset={capgen.num(o)};feat=%2", capView, triFeatureState] call capLog }}')
                if frame % a.every == 0:
                    lines.append(f'triScreenshot "{name}_{frame:03d}"')
                frame += 1
            lines += capgen.end(v, False)
    lines.append('\ncall { "view=done;event=done" call capLog }')
    lines.append("triEndTest")
    out = Path(a.out)
    stem = f"look_{a.tag}_abel"
    capgen.write(out / f"{stem}.test.sqf", "\n".join(lines) + "\n")
    capgen.write(out / f"{stem}.test.toml", capgen.toml("Abel", title, views.WORLDS, timeout=3600))
    print(f"tests/rendering/captures/{stem}.test.sqf")


if __name__ == "__main__":
    main()
