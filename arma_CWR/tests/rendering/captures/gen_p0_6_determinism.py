"""P0-6 determinism stills: the deterministic capture recipe (plan 3.3) on the
dry standard views and the wet town, two shots per view per session.

    python gen_p0_6_determinism.py [--out <dir>]

Writes p0_6_det_abel.test.sqf/.toml (valley_0515, noon_fields, sunrise_ridge,
town_night_rain) and p0_6_det_eden.test.sqf/.toml (broken_high) next to this
file. They need a build with the P0-6 verbs (triDeterministic, triResetTemporal,
triSeedRandom, triTemporalState); on any other build the verbs are unknown
commands (script errors).

Per view: stage it as the generated capture tests do (capgen: camera, date,
weather, props, the sim clock and the clouds pinned), settle in "remastered"
(capMode: detail pinned, triSettleScene), then twice: re-pin everything
(capPin), deterministic mode on, reset the temporal state with the mist drift
held at MIST, seed the random streams with 1, SHOT_FRAMES frames at the fixed
1/60 s step, shoot <view>__det1 (then __det2). The whole run steps the
simulation by 1/60 s (triSetFixedStep), so every shot follows its reset by the
same simulated time.

The claim (run the test in two boots of one build): <view>__det1 and __det2
are byte-identical across boots, 0 differing pixels, on the dry views; det1
against det2 in one boot is the session's own floor (the same frame count
after the same reset). town_night_rain is reported, not claimed: its fire and
smoke particles keep the state they had before the reset.
"""
import argparse
from pathlib import Path

import capgen
import views

SHOT_FRAMES = 32
SIM_T0, SIM_DT = 2000, 600
MIST = (0, 0)
STEP = 1.0 / 60.0

HELPERS = f"""
// P0-6 determinism helpers (a build with the P0-6 verbs).
capDetBlock = {{ capDetRes = triDeterministic true; capRstRes = triResetTemporal [{MIST[0]}, {MIST[1]}]; capSeedRes = triSeedRandom 1; format ["view=%1;event=det;det=%2;reset=%3;seed=%4", capView, capDetRes, capRstRes, capSeedRes] call capLog }}
capDetState = {{ format ["view=%1;event=detstate;label=%2;state=%3", capView, _this, triTemporalState] call capLog }}
call {{ capStepRes = triSetFixedStep {STEP:.7f}; format ["view=setup;event=step;res=%1", capStepRes] call capLog }}
"""


def shots(v, label):
    out = []
    for k in (1, 2):
        name = f"{label}__det{k}"
        out.append("[] call capPin")
        if v.get("refresh"):
            out.append(v["refresh"])
        out += ["[] call capDetBlock",
                f"triSimFrames {SHOT_FRAMES}",
                f'triScreenshot "{name}"',
                f'"{name}" call capRecord',
                f'"{name}" call capDetState']
    return out


def view_block(v, label, index):
    out = [f"\n// ---- {label}"]
    out += capgen.begin(v, label, sim_time=SIM_T0 + SIM_DT * index)
    out.append("triSimFrames 150")
    out += ['"remastered" call capMode', "[] call capPin"]
    if v.get("refresh"):
        out.append(v["refresh"])
    out.append("triSimFrames 60")
    out += shots(v, label)
    out += capgen.end(v)
    return out


def footer():
    return ["\ncall { capDetRes = triDeterministic false; capRstRes = triResetTemporal []; "
            'capStepRes = triSetFixedStep 0; "view=done;event=done" call capLog }',
            "triEndTest"]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default=str(capgen.HERE))
    a = ap.parse_args()
    out = Path(a.out)
    title = "P0-6 determinism stills (deterministic recipe, two shots per view; needs the P0-6 verbs)."

    abel_views = ["valley_0515", "noon_fields", "sunrise_ridge", "town_night_rain"]
    first = views._timing(abel_views[0])
    abel = [capgen.header("Abel", first, title + " Abel."), HELPERS]
    for i, name in enumerate(abel_views):
        abel += view_block(views._timing(name), name, i)
    abel += footer()
    capgen.write(out / "p0_6_det_abel.test.sqf", "\n".join(abel) + "\n")
    capgen.write(out / "p0_6_det_abel.test.toml", capgen.toml("Abel", title, views.WORLDS, timeout=3600))

    broken = [v for v in views.TIMING_EDEN if v["name"] == "broken_high"][0]
    eden = [capgen.header("Eden", broken, title + " Eden."), HELPERS]
    eden += view_block(broken, "broken_high", 0)
    eden += footer()
    capgen.write(out / "p0_6_det_eden.test.sqf", "\n".join(eden) + "\n")
    capgen.write(out / "p0_6_det_eden.test.toml", capgen.toml("Eden", title, views.WORLDS, timeout=1800))
    print("p0_6_det: abel 4 views, eden 1 view, 2 shots each")


if __name__ == "__main__":
    main()
