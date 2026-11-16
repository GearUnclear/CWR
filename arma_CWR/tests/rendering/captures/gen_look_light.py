"""Generate the look-light capture tests: the eight hero views of the photoreal
light and camera lane (switch photoLight), each shot in several lighting modes
in one game, so the Original (Classic) and the overhaul share camera, clock,
weather, props and clouds.

    python gen_look_light.py --modes "classic=classic;off=remastered,photoLight=0;on=remastered" [--tag NAME]
                             [--only view,view] [--frames N]

Writes look_<tag>_<world>.test.sqf + .test.toml next to this file (one test
per world that has a selected view). Each mode is `label=setting`; the shot of
view V in mode label L is saved as <V>__<L>.png. 1920x1080 windowed, render
scale 1.0, 4x MSAA, the presented frame (capgen.header).
"""
import argparse
from pathlib import Path

import capgen
import views

HERO_NAMES = {
    "Abel": ["lowsun_fp", "noon_fields", "forest_edge_golden", "sunrise_ridge", "coastline_sunset", "town_night",
             "woods_pine2_evening", "blue_hour_coast"],
    "Eden": ["overcast_broken"],
    "Sinai": ["sinai_desert_noon"],
}
TABLES = {"Abel": views.TIMING_ABEL, "Eden": views.TIMING_EDEN, "Sinai": views.TIMING_SINAI}
SIM_T0, SIM_DT = 2000, 600
# The lane's own views: the blue hour over the west coast (sun 4.9 deg under
# the horizon, looking into the afterglow).
EXTRA = {
    "blue_hour_coast": dict(name="blue_hour_coast", x=1550, z=5150, eye=3, h=295, p=2, date=[1985, 7, 5, 19, 55],
                            vd=4000),
}


def find(world, name):
    if name in EXTRA:
        return dict(EXTRA[name])
    for v in TABLES[world]:
        if v["name"] == name:
            v = dict(v)
            v.pop("f10", None)
            v.pop("note", None)
            return v
    raise KeyError(name)


def shot(v, label, setting, shot_label, frames):
    out = [f'"{setting}" call capMode',
           f"triSimFrames {frames}",
           "[] call capReadTimers",
           "[] call capPin"]
    if v.get("refresh"):
        out.append(v["refresh"])
    out += ["triSimFrames 3",
            f'triScreenshot "{shot_label}"',
            f'"{shot_label}" call capRecord']
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--modes", required=True, help="label=setting;label=setting;...")
    ap.add_argument("--tag", default="light")
    ap.add_argument("--only", default="")
    ap.add_argument("--frames", type=int, default=90)
    ap.add_argument("--out", default=str(capgen.HERE))
    a = ap.parse_args()
    modes = []
    for item in a.modes.split(";"):
        item = item.strip()
        if not item:
            continue
        label, setting = item.split("=", 1)
        modes.append((label.strip(), setting.strip()))
    want = {s.strip() for s in a.only.split(",") if s.strip()}
    written = []
    index = 0
    for world, names in HERO_NAMES.items():
        vs = [find(world, n) for n in names if not want or n in want]
        if not vs:
            continue
        title = f"look-light hero views on {world}: gen_look_light.py, modes {', '.join(m[0] for m in modes)}."
        lines = [capgen.header(world, vs[0], title)]
        for v in vs:
            lines.append(f"\n// ---- {v['name']}")
            lines += capgen.begin(v, v["name"], False, sim_time=SIM_T0 + SIM_DT * index)
            index += 1
            lines.append("triSimFrames 150")
            for label, setting in modes:
                lines += shot(v, v["name"], setting, f"{v['name']}__{label}", a.frames)
            lines += capgen.end(v, False)
        lines.append('\ncall { "view=done;event=done" call capLog }')
        lines.append("triEndTest")
        name = f"look_{a.tag}_{world.lower()}"
        out = Path(a.out)
        capgen.write(out / f"{name}.test.sqf", "\n".join(lines) + "\n")
        capgen.write(out / f"{name}.test.toml", capgen.toml(world, title, views.WORLDS, timeout=3600))
        written.append(f"tests/rendering/captures/{name}.test.sqf")
    print(",".join(written))


if __name__ == "__main__":
    main()
