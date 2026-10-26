"""Generate the research capture tests for track "tiling".

Writes into <lighting worktree>/arma_CWR/tests/rendering/research/tiling/ (git
excluded): capture_lib.sqf (copy of the committed rig plus capVar) and one
test per island. Each view shoots Classic, then Remastered with variant k for
every k in VARIANTS (k = 0 is stock Remastered; k >= 1 are the prototype's
variants, selected through the saturation knob, see psRemasterCommon.glsl).

    python gen_tests.py [--variants 0,1,2,3,4] [--only abel,eden,sinai] [--suffix x]
"""
import argparse
from pathlib import Path

WT = Path(r"D:\Arma_CWA_worktrees\lighting\arma_CWR")
SRC = WT / "tests/rendering/captures"
DST = WT / "tests/rendering/research/tiling"

CAPVAR = r'''
// ---- research track "tiling": k call capVar ----
// Remastered variant k through the saturation knob (1.12 + 0.001 k; the
// override shader decodes k and uses 1.12). Restarts the timers like capMode.
capVar = {
    capModeName = format ["v%1", _this];
    capLit = triSetLighting (format ["saturation=%1", 1.12 + 0.001 * _this]);
    capGpuOff = triGpuTimes false;
    capGpuOn = triGpuTimes true;
    capPerfRes = triPerfReset 0;
    format ["view=%1;event=mode;mode=%2;lit=%3", capView, capModeName, capLit] call capLog;
}
'''

HEADER = '''#include "capture_lib.sqf"

triSimUntil {{ alive player }}

capWorld = "{world}"
capFov = 0.7
capSimT = 0
capPinTime = false
capUseView = false
capProps = []
capView = "setup"
capModeName = "none"
capLit = "none"
capGpu = "none"
capPerf = "none"

triCaptureAtPresent true
triSetMsaa 4
triSetRenderScale 1.5
showCinemaBorder false
enableRadio false
player setCaptive true
player allowDamage false

createCenter west
createCenter east
capGrpW = createGroup west
capProbe = "HeliHEmpty" createVehicle [{x}, {z}, 0]
capCam = "camera" camCreate [{x}, {z}, 2]
capCam cameraEffect ["internal", "back"]

triSimFrames 240
call {{ "view=setup;event=setup;world={world}" call capLog }}
'''

# id, x, z, eye, heading, pitch, date, overcast, fog, vd, props
ABEL = [
    ("noon_fields", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500,
     ['[["Jeep"], 13, 3, 60, "veh"] call capSpawn;', '[["SoldierWB"], 8, -2, 160, "man"] call capSpawn;']),
    ("forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500,
     ['[["SoldierWB"], 12, 2, 215, "man"] call capSpawn;']),
    ("lowsun_fp", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500,
     ['[["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;', '[["M113"], 16, 5, 72.5, "veh"] call capSpawn;',
      '[["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;']),
    # High vantage over the open upland fields of noon_fields (large field).
    ("high_fields_noon", 6250, 10800, 60, 165, -14, [1985, 7, 5, 12, 30], 0, 0, 2500, []),
    ("high_fields_eve", 6250, 10800, 60, 180, -14, [1985, 7, 5, 17, 30], 0, 0, 2500, []),
]
EDEN = [
    ("overcast_broken", 9200, 3700, 1.75, 288, -4, [1985, 7, 5, 14, 30], 0.5, 0, 3500,
     ['[["Jeep"], 12, -5, 70, "veh"] call capSpawn;']),
    ("high_valley_eden", 9200, 3700, 45, 288, -12, [1985, 7, 5, 14, 30], 0.5, 0, 3500, []),
]
SINAI = [
    ("sinai_desert_noon", 7300, 10300, 1.75, 210, 1, [2005, 7, 21, 12, 15], 0, 0, 4000,
     ['[["LoBoHMWV_Mag_Patrol", "Jeep"], 14, 4, 60, "veh"] call capSpawn;',
      '[["LoBoGolaniWB", "SoldierWB"], 8, -2, 160, "man"] call capSpawn;']),
    # High vantage over the desert plateau of sinai_desert_noon (large uniform
    # sand area: the land texture's dark specks repeat every cell).
    ("high_sinai_noon", 7300, 10300, 80, 210, -12, [2005, 7, 21, 12, 15], 0, 0, 4000, []),
    ("sinai_lowsun_fp", 8350, 2950, 1.75, 338.8, -3, [2005, 7, 21, 16, 10], 0, 0, 2000,
     ['[["LoBoHMWV_Mag_Patrol", "Jeep"], 10, -5, -107.8, "veh"] call capSpawn;',
      '[["LoBoGolaniWB", "SoldierWB"], 7, 1, -112.8, "man"] call capSpawn;']),
]

TOML = '''binary = "PoseidonGame"
mission = "tests/rendering/captures/missions/{mission}"
tags = ["lighting_capture", "headful", "exclusive"]
timeout = 900
assert_timeout = 240
extra_args = [{extra}"--no-strict", "--display-mode", "windowed", "--width", "960", "--height", "640"]
'''


def shot(label):
    return (f"triSimFrames 90\n[] call capReadTimers\n[] call capPin\ntriSimFrames 3\n"
            f'triScreenshot "{label}"\n"{label}" call capRecord\n')


def view_block(v, variants, classic=True):
    vid, x, z, eye, h, p, date, oc, fog, vd, props = v
    out = [f'["{vid}", {x}, {z}, {eye}, {h}, {p}, {date}, {oc}, {fog}, {vd}] call capBegin'.replace("'", "")]
    if props:
        out.append("call {\n    " + "\n    ".join(props) + "\n}")
    out.append("triSimFrames 150")
    if classic:
        out.append('"classic" call capMode')
        out.append(shot(f"{vid}__classic").rstrip())
    out.append('"remastered" call capMode')
    for k in variants:
        out.append(f"{k} call capVar")
        out.append(shot(f"{vid}__v{k}").rstrip())
    out.append("0 call capVar")
    out.append("[] call capClear\n")
    return "\n".join(out)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--variants", default="0,1,2,3,4")
    ap.add_argument("--only", default="abel,eden,sinai")
    ap.add_argument("--suffix", default="")
    ap.add_argument("--views", default="", help="comma list of view ids to keep (default all)")
    ap.add_argument("--no-classic", action="store_true")
    a = ap.parse_args()
    variants = [int(k) for k in a.variants.split(",") if k != ""]
    keep = set(a.views.split(",")) if a.views else None
    DST.mkdir(parents=True, exist_ok=True)
    lib = (SRC / "capture_lib.sqf").read_text(encoding="utf-8")
    (DST / "capture_lib.sqf").write_text(lib + CAPVAR, encoding="utf-8", newline="\n")
    sets = {
        "abel": ("Abel", 7300, 10900, "udcap_abel.Abel", ABEL, ""),
        "eden": ("Eden", 9200, 3700, "udcap_eden.Eden", EDEN, ""),
        "sinai": ("Sinai", 8350, 2950, "udcap_sinai.Sinai", SINAI, '"--mods-dir", ".", "--mod", "@LoBo", '),
    }
    for key in a.only.split(","):
        world, x, z, mission, views, extra = sets[key]
        body = HEADER.format(world=world, x=x, z=z)
        for v in views:
            if keep and v[0] not in keep:
                continue
            body += "\n" + view_block(v, variants, classic=not a.no_classic)
        body += '\ncall { "view=done;event=done" call capLog }\ntriEndTest\n'
        name = f"tiling_{key}{a.suffix}"
        (DST / f"{name}.test.sqf").write_text(body, encoding="utf-8", newline="\n")
        (DST / f"{name}.test.toml").write_text(TOML.format(mission=mission, extra=extra), encoding="utf-8",
                                                 newline="\n")
        print("wrote", DST / f"{name}.test.sqf")


if __name__ == "__main__":
    main()
