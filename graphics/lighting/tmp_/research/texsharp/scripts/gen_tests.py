"""Generate the texsharp research capture tests (Trident .test.sqf + .toml).

Each view: Classic, Remastered (stock knobs), then Remastered variants set with
the scratch build's triSetLighting texture knobs. Every variant flushes the
resident mip chains first (texFlush) so a finer level loaded for an earlier
shot never leaks into a later one, then settles 120 frames.

    python gen_tests.py <out_dir> [--set main|tune]
"""
import sys
from pathlib import Path

STOCK = ["texStreamFix=0", "texMipBias=0", "texLodBias=0", "texAniso=16", "texSharpen=0", "texPost2=0",
         "texK1=0", "texK2=0", "texK3=0", "texK4=0", "texRemip=0"]

BEST = ["texRemip=3", "texLodBias=-0.5", "texK1=0.5", "texK2=1", "texK3=0.4", "texK4=1", "texSharpen=0.5"]
BEST_NOREMIP = ["texLodBias=-0.5", "texK1=0.5", "texK2=1", "texK3=0.4", "texK4=1", "texSharpen=0.5"]

BEST2 = ["texK2=2", "texK1=1", "texSharpen=0.5", "texLodBias=-0.5", "texK4=1"]

BEST3 = ["texK3=-1", "texK2=2", "texK1=1", "texSharpen=0.5", "texLodBias=-0.5", "texK4=1"]
SELF_KEPT = ["texK3=-1", "texK2=2", "texK1=1"]

FINAL = ["texK3=-1.5", "texK2=2", "texK1=1", "texSharpen=0.3", "texPost2=1"]

FINAL_ROT = ["texK3=-11.5", "texK2=2", "texK1=1", "texSharpen=0.3", "texPost2=1"]

VARIANT_SETS = {
    # glsl7: K3 <= -10 adds a random per-cell rotation (strength -K3 - 10); K4 >= 2 sets the zoom
    "v9": [
        ("rsl15", ["texK3=-1.5"]),
        ("rsl15r", ["texK3=-11.5"]),
        ("rsl15z4", ["texK3=-1.5", "texK4=4"]),
        ("rsl15z8", ["texK3=-1.5", "texK4=8"]),
        ("rfinal", FINAL),
        ("rfinalr", FINAL_ROT),
    ],
    # glsl6: self-detail through a soft limiter, K4<0 second octave, texPost2 luma-only CAS
    "v8": [
        ("rsl1", ["texK3=-1"]),
        ("rsl15", ["texK3=-1.5"]),
        ("rsl2", ["texK3=-2"]),
        ("rsl15o", ["texK3=-1.5", "texK4=-0.6"]),
        ("rsl15k", ["texK3=-1.5", "texK2=2", "texK1=1"]),
        ("rcas3l", ["texSharpen=0.3", "texPost2=1"]),
        ("rfinal", FINAL),
    ],
    # cost probe with glsl6amp: shade = self-detail once, best = self-detail x16
    "costprobe": [(f"t{k}{i}", knobs) for i in range(1, 7) for k, knobs in
                  (("stock", []), ("shade", ["texK3=-1.5"]), ("best", ["texK3=-1.5", "texK4=1600"]))],
    "timing4": [(f"t{k}{i}", knobs) for i in range(1, 7) for k, knobs in
                (("stock", []), ("final", FINAL_ROT))],
    "timing3": [(f"t{k}{i}", knobs) for i in range(1, 7) for k, knobs in
                (("stock", []), ("final", FINAL))],
    # glsl5 meanings: K1 anti-tiling, K2 detail kept, K3<0 self-detail strength, K4 CR (>=2: zoom)
    "v7": [
        ("rd2t", ["texK2=2", "texK1=1"]),
        ("rsd1", ["texK3=-1"]),
        ("rsd2", ["texK3=-2"]),
        ("rsdk", SELF_KEPT),
        ("rcas5", ["texSharpen=0.5"]),
        ("rbest3", BEST3),
    ],
    "timing2": [(f"t{k}{i}", knobs) for i in range(1, 5) for k, knobs in
                (("stock", []), ("best", BEST3))],
    # glsl4 meanings: K1 anti-tiling, K2 mean-preserving detail boost, K3 relief, K4 Catmull-Rom
    "v6": [
        ("rd1", ["texK2=1"]),
        ("rd2", ["texK2=2"]),
        ("rd2t", ["texK2=2", "texK1=1"]),
        ("rrel30", ["texK3=3"]),
        ("rcas5", ["texSharpen=0.5"]),
        ("rbest2", BEST2),
    ],
    "remip": [
        ("rbox", ["texRemip=1"]),
        ("rlz", ["texRemip=2"]),
        ("rlzc", ["texRemip=3"]),
        ("rbest", BEST),
    ],
    "edge5": [
        ("rcas5", ["texSharpen=0.5"]),
        ("rmac", ["texK1=0.6"]),
        ("rrel8", ["texK3=0.8"]),
        ("rcr", ["texK4=1"]),
        ("rlzc", ["texRemip=3"]),
        ("rbest", BEST),
    ],
    "timing": [(f"t{k}{i}", knobs) for i in range(1, 5) for k, knobs in
               (("stock", []), ("shade", BEST_NOREMIP), ("best", BEST))],
    "v4": [
        ("rbias", ["texLodBias=-0.5"]),
        ("rcas5", ["texSharpen=0.5"]),
        ("rcas8", ["texSharpen=0.8"]),
        ("rdet1", ["texK2=1"]),
        ("rdet2", ["texK2=2"]),
        ("rrel3", ["texK3=0.3"]),
        ("rrel8", ["texK3=0.8"]),
        ("rmac", ["texK1=0.6"]),
        ("rcr", ["texK4=1"]),
        ("rbox", ["texRemip=1"]),
        ("rlz", ["texRemip=2"]),
        ("rlzc", ["texRemip=3"]),
        ("rbest", ["texRemip=3", "texLodBias=-0.5", "texK1=0.5", "texK2=1", "texK3=0.4", "texK4=1", "texSharpen=0.5"]),
    ],
    "v3": [
        ("rbox", ["texRemip=1"]),
        ("rlz", ["texRemip=2"]),
        ("rlzc", ["texRemip=3"]),
        ("rlzcb", ["texRemip=3", "texLodBias=-0.5"]),
    ],
    "v2": [
        ("rbias", ["texLodBias=-0.5"]),
        ("rcas5", ["texSharpen=0.5"]),
        ("rcas8", ["texSharpen=0.8"]),
        ("rdet1", ["texK2=1"]),
        ("rdet2", ["texK2=2"]),
        ("rrel3", ["texK3=0.3"]),
        ("rrel8", ["texK3=0.8"]),
        ("rmac", ["texK1=0.6"]),
        ("rbest", ["texLodBias=-0.5", "texK1=0.5", "texK2=1", "texK3=0.4", "texSharpen=0.5"]),
    ],
    "main": [
        ("rnoaniso", ["texAniso=1"]),
        ("rfix", ["texStreamFix=1"]),
        ("rfull", ["texMipBias=99"]),
        ("rfullb", ["texMipBias=99", "texLodBias=-0.5"]),
        ("rcas", ["texStreamFix=1", "texSharpen=0.5"]),
        ("rmacro", ["texStreamFix=1", "texK1=1", "texK2=9"]),
        ("rall", ["texStreamFix=1", "texK1=1", "texK2=9", "texSharpen=0.5"]),
    ],
}

VIEWS = {
    "abel": [
        dict(name="noon_fields", begin='["noon_fields", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500] call capBegin',
             props=['[["Jeep"], 13, 3, 60, "veh"] call capSpawn;', '[["SoldierWB"], 8, -2, 160, "man"] call capSpawn;']),
        dict(name="forest_edge_golden", begin='["forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500] call capBegin',
             props=['[["SoldierWB"], 12, 2, 215, "man"] call capSpawn;']),
        dict(name="lowsun_fp", begin='["lowsun_fp", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500] call capBegin',
             props=['[["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;', '[["M113"], 16, 5, 72.5, "veh"] call capSpawn;',
                    '[["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;',
                    '[["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"] call capSpawn;']),
    ],
    "eden": [
        dict(name="overcast_broken", begin='["overcast_broken", 9200, 3700, 1.75, 288, -4, [1985, 7, 5, 14, 30], 0.5, 0, 3500] call capBegin',
             props=['[["Jeep"], 12, -5, 70, "veh"] call capSpawn;'],
             # cloud-shadow drift is integrated per frame, not pinned by triSetSimTime:
             # it moved between shots in run3, so texture comparisons turn it off
             pre=["cloudShadows=0"]),
    ],
    "sinai": [
        dict(name="sinai_desert_noon", begin='["sinai_desert_noon", 7300, 10300, 1.75, 210, 1, [2005, 7, 21, 12, 15], 0, 0, 4000] call capBegin',
             props=['[["LoBoHMWV_Mag_Patrol", "Jeep"], 14, 4, 60, "veh"] call capSpawn;',
                    '[["LoBoGolaniWB", "SoldierWB"], 8, -2, 160, "man"] call capSpawn;']),
    ],
}

HEADERS = {
    "abel": ("Abel", 7300, 10900),
    "eden": ("Eden", 9200, 3700),
    "sinai": ("Sinai", 8350, 2950),
}

TOMLS = {
    "abel": '''binary = "PoseidonGame"
mission = "tests/rendering/captures/missions/udcap_abel.Abel"
tags = ["lighting_capture", "headful", "exclusive"]
timeout = 900
assert_timeout = 180
extra_args = ["--no-strict", "--display-mode", "windowed", "--width", "960", "--height", "640"]
''',
    "eden": '''binary = "PoseidonGame"
mission = "tests/rendering/captures/missions/udcap_eden.Eden"
tags = ["lighting_capture", "headful", "exclusive"]
timeout = 900
assert_timeout = 180
extra_args = ["--no-strict", "--display-mode", "windowed", "--width", "960", "--height", "640"]
''',
    "sinai": '''binary = "PoseidonGame"
mission = "tests/rendering/captures/missions/udcap_sinai.Sinai"
tags = ["lighting_capture", "lobo", "headful", "exclusive"]
timeout = 900
assert_timeout = 240
extra_args = ["--no-strict", "--mods-dir", ".", "--mod", "@LoBo", "--display-mode", "windowed", "--width", "960", "--height", "640"]
''',
}


def sqf_list(items):
    return "[" + ", ".join(f'"{x}"' for x in items) + "]"


def shot(view, label, settle=90):
    full = f"{view}__{label}"
    return [
        f"triSimFrames {settle}",
        "[] call capReadTimers",
        "[] call capPin",
        'call { capK = triSetLighting "texStats=1" }',
        "triSimFrames 3",
        f'triScreenshot "{full}"',
        f'"{full}" call capRecord',
    ]


def view_block(v, variants):
    out = ["", f"// ---- {v['name']}", v["begin"], "call {"]
    out += ["    " + p for p in v["props"]]
    out += ["}"]
    # stock knobs, flush whatever the previous view left resident
    out += [f"call {{ {sqf_list(STOCK + v.get('pre', []) + ['texFlush=60'])} call tsSet }}", "triSimFrames 150"]
    out += ['"classic" call capMode'] + shot(v["name"], "classic")
    out += ['"remastered" call capMode'] + shot(v["name"], "remastered")
    for label, knobs in variants:
        flush = ["texFlush=60"] if any(k.startswith(("texStreamFix", "texMipBias")) for k in knobs) else []
        out += [f'call {{ {sqf_list(STOCK + knobs + flush)} call tsSet; "{label}" call tsMode }}']
        out += shot(v["name"], label, settle=200 if label.startswith("t") else 120)
    out += [f"call {{ {sqf_list(STOCK)} call tsSet }}", "[] call capClear"]
    return out


MOTION_SETS = {
    "v8": [("mstock", []), ("mrdtile", ["texK2=2", "texK1=1"]), ("mself", ["texK3=-1.5"]), ("mbest", FINAL)],
    "v10": [("mstock", []), ("mrdtile", ["texK2=2", "texK1=1"]), ("mself", ["texK3=-11.5"]), ("mbest", FINAL_ROT)],
}
MOTION_VARIANTS = MOTION_SETS["v8"]


def motion_block(v, nframes=8, step=0.25):
    """The camera walks forward `step` m per frame; one screenshot per frame."""
    out = ["", f"// ---- motion: {v['name']}", v["begin"], "call {"]
    out += ["    " + p for p in v["props"]]
    out += ["}"]
    out += [f"call {{ {sqf_list(STOCK + v.get('pre', []) + ['texFlush=60'])} call tsSet }}", "triSimFrames 150",
            '"remastered" call capMode', "capOX0 = capOX", "capOZ0 = capOZ"]
    for label, knobs in MOTION_VARIANTS:
        out += [f'call {{ {sqf_list(STOCK + knobs)} call tsSet; "{label}" call tsMode }}', "triSimFrames 60"]
        for fr in range(nframes):
            out += [f"call {{ capOX = capOX0 + {fr * step} * (sin capH); capOZ = capOZ0 + {fr * step} * (cos capH); [] call capPinView }}",
                    "triSimFrames 2",
                    f'triScreenshot "{v["name"]}__{label}{fr}"',
                    f'"{v["name"]}__{label}{fr}" call capRecord']
    out += [f"call {{ {sqf_list(STOCK)} call tsSet }}", "[] call capClear"]
    return out


def make_test(island, variants, motion=False):
    world, px, pz = HEADERS[island]
    lines = [
        "// texsharp research capture run (generated by tmp/research/texsharp/scripts/gen_tests.py).",
        '#include "capture_lib.sqf"',
        "",
        "triSimUntil { alive player }",
        "",
        f'capWorld = "{world}"',
        "capFov = 0.7",
        "capSimT = 0",
        "capPinTime = false",
        "capUseView = false",
        "capProps = []",
        'capView = "setup"',
        'capModeName = "none"',
        'capLit = "none"',
        'capGpu = "none"',
        'capPerf = "none"',
        "",
        "// texsharp helpers: set knobs (logged), name a variant and restart the timers",
        'tsSet = { { capK = triSetLighting _x; format ["view=%1;event=knob;set=%2;res=%3", capView, _x, capK] call capLog } forEach _this }',
        'tsMode = { capModeName = _this; capGpuOff = triGpuTimes false; capGpuOn = triGpuTimes true; capPerfRes = triPerfReset 0; format ["view=%1;event=mode;mode=%2;lit=variant", capView, _this] call capLog }',
        "",
        "triCaptureAtPresent true",
        "triSetMsaa 4",
        "triSetRenderScale 1.5",
        "showCinemaBorder false",
        "enableRadio false",
        "player setCaptive true",
        "player allowDamage false",
        "",
        "createCenter west",
        "createCenter east",
        "capGrpW = createGroup west",
        f'capProbe = "HeliHEmpty" createVehicle [{px}, {pz}, 0]',
        f'capCam = "camera" camCreate [{px}, {pz}, 2]',
        'capCam cameraEffect ["internal", "back"]',
        "triSimFrames 240",
        f'call {{ "view=setup;event=setup;world={world};msaa=4;scale=1.5;window=960x640;capture=present;track=texsharp" call capLog }}',
    ]
    for v in VIEWS[island]:
        if motion:
            if v["name"] in ("forest_edge_golden", "noon_fields"):
                lines += motion_block(v)
        else:
            lines += view_block(v, variants)
    lines += ["", 'call { "view=done;event=done" call capLog }', "triEndTest", ""]
    return "\n".join(lines)


def main():
    out = Path(sys.argv[1])
    which = "main"
    if "--set" in sys.argv:
        which = sys.argv[sys.argv.index("--set") + 1]
    islands = sys.argv[sys.argv.index("--islands") + 1].split(",") if "--islands" in sys.argv else list(VIEWS)
    motion = "--motion" in sys.argv
    global MOTION_VARIANTS
    if motion and which in MOTION_SETS:
        MOTION_VARIANTS = MOTION_SETS[which]
    stem = "motion" if motion else "texsharp"
    out.mkdir(parents=True, exist_ok=True)
    for island in islands:
        toml = TOMLS[island].replace("timeout = 900", "timeout = 1800")
        (out / f"{island}_{stem}.test.sqf").write_text(make_test(island, VARIANT_SETS.get(which, []), motion),
                                                      encoding="utf-8", newline="\n")
        (out / f"{island}_{stem}.test.toml").write_text(toml, encoding="utf-8", newline="\n")
        print("wrote", out / f"{island}_{stem}.test.sqf")


if __name__ == "__main__":
    main()
