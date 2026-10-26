"""Camera-pan stability captures for the look-surface lane.

For each view and mode: settle on the view, switch the sim to a fixed 1/60 s
step, release the pinned view to the cutscene camera and start a slow pan
(capCam commits a turn of PAN_DEG over PAN_S seconds), let the TAA history run
in motion, then shoot two frames back to back (<view>__<mode>_a / _b). The
runner gates each statement on the previous screenshot file, so the two shots
are a few frames apart; scripts/pan_stats.py aligns them by the pan's shift.

  python gen_pan.py <captures dir>
"""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import gen_hero  # noqa: E402

PAN_DEG = 6.0
PAN_S = 6.0
MODES = [("base", "remastered,surfaces=0"), ("surf", "remastered"), ("aodbg", "remastered,debug=4")]
VIEWS = ["lowsun_fp", "forest_morning", "town_noon"]


def pan_block(v):
    d = v["date"]
    out = [f"// ---- {v['id']} (pan)",
           f"[\"{v['id']}\", {v['x']}, {v['z']}, {v['eye']}, {v['h']}, {v['p']}, [{d[0]}, {d[1]}, {d[2]}, {d[3]}, {d[4]}], {v['oc']}, {v['fog']}, {v['vd']}] call capBegin",
           "call { capCloudRes = triPinCloudDrift [1830, 970, 0] }",
           "call {"]
    out += [gen_hero.prop_line(p) for p in v["props"]]
    out.append("}")
    out.append("triSimFrames 150")
    h2 = v["h"] + PAN_DEG
    for suffix, setting in MODES:
        out += [f"\"{setting}\" call capMode",
                "triSimFrames 90",
                "[] call capReadTimers",
                "call { capUseView = true; [] call capPin }",
                "triSimFrames 30",
                "call { capFsRes = triSetFixedStep 0.0166667 }",
                "call { capUseView = false; capClearRes = triClearView }",
                ("call { capCam camSetTarget [capOX + 1000 * (sin %s) * (cos capP), capOZ + 1000 * (cos %s) * (cos capP), "
                 "capEyeASL + 1000 * (sin capP)]; capCam camCommit %s }") % (h2, h2, PAN_S),
                "triSimFrames 40",
                f"triScreenshot \"{v['id']}__{suffix}_a\"",
                f"triScreenshot \"{v['id']}__{suffix}_b\"",
                f"\"{v['id']}__{suffix}_b\" call capRecord",
                "call { capFsRes = triSetFixedStep 0 }",
                "call { capUseView = true }"]
    out.append("[] call capClear")
    out.append("call { capCloudRes = triPinCloudDrift [] }")
    return "\n".join(out) + "\n"


def main():
    outdir = Path(sys.argv[1])
    views = [v for v in gen_hero.ABEL if v["id"] in VIEWS]
    sqf, (w, h) = gen_hero.header("Abel", views[0], True)
    for v in views:
        sqf += "\n" + pan_block(v)
    sqf += '\ncall { "view=done;event=done" call capLog }\ntriEndTest\n'
    (outdir / "look_surface_pan_abel.test.sqf").write_text(sqf, encoding="utf-8", newline="\n")
    t = gen_hero.toml("Abel", "tests/rendering/captures/missions/udcap_abel.Abel", False, 0)
    t = t.replace("%W%", str(w)).replace("%H%", str(h))
    (outdir / "look_surface_pan_abel.test.toml").write_text(t, encoding="utf-8", newline="\n")
    print("pan views", [v["id"] for v in views])


if __name__ == "__main__":
    main()
