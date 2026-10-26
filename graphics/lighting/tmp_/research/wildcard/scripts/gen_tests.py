"""Generate the wildcard research capture tests (sky visibility maps).

    python gen_tests.py <run-name> [--views a,b] [--variants set]

Writes <worktree>/arma_CWR/tests/rendering/research/wildcard/<run>_<world>.test.sqf/.toml
plus a copy of capture_lib.sqf with the variant helpers appended. The views reuse
the committed capture missions and the committed rig (capture_lib.sqf); every
variant of a view is shot in the same game session with the same camera, date,
weather, sim clock and props.
"""
import argparse
import shutil
from pathlib import Path

WT = Path(r"D:\Arma_CWA_worktrees\gfx-wildcard\arma_CWR")
SRC = WT / "tests/rendering/captures"
DST = WT / "tests/rendering/research/wildcard"

RESET = ["skyOcc=0", "skyBounce=0", "skyAlbedo=0", "skyDebug=0", "skyCone=1", "skyBleed=0.15",
         "skyRadius=48", "skyDirs=16", "skyCrown=1", "ao=0.7",
         "skyLeaf=0.45", "skyShell=1", "skyFloor=0", "cloudShadows=1",
         "skyTint=0", "skyGain=1", "skyGround=0", "skyFaces=1"]

PHYS = ["skyOcc=1", "skyLeaf=0.25", "skyBleed=0.08", "skyAlbedo=1", "skyBounce=1"]
BG = PHYS + ["skyGround=1"]
E24 = BG + ["skyDirs=24", "skyCone=1.3"]
F64 = E24 + ["skyRadius=64"]
CAL = F64 + ["skyGain=1.15", "skyLeaf=0.45"]
FINAL = CAL + ["skyCrown=2"]
PHYSF = F64 + ["skyCrown=2"]
VARIANT_SETS = {
    "timing3": [
        ("t1_off", []), ("t1_final", FINAL), ("t1_k16", BG + ["skyCrown=2"]),
        ("t2_off", []), ("t2_final", FINAL), ("t2_k16", BG + ["skyCrown=2"]),
        ("t3_off", []), ("t3_final", FINAL), ("t3_k16", BG + ["skyCrown=2"]),
    ],
    "final4": [
        ("classic", None),
        ("remastered", []),
        ("final", FINAL),
        ("final_phys", PHYSF),
        ("dbg_final", FINAL + ["skyDebug=1"]),
    ],
    "nofol": [
        ("t1_off", []), ("t1_G", CAL), ("t1_H", CAL + ["skyCrown=2"]),
        ("t2_off", []), ("t2_G", CAL), ("t2_H", CAL + ["skyCrown=2"]),
    ],
    "dolly": [
        ("dolly_off", []),
        ("dolly_G", CAL),
        ("dolly_dbg", F64 + ["skyDebug=1"]),
    ],
    "timing2": [
        ("t1_off", []), ("t1_B", BG), ("t1_G", CAL),
        ("t2_off", []), ("t2_B", BG), ("t2_G", CAL),
        ("t3_off", []), ("t3_B", BG), ("t3_G", CAL),
    ],
    "final3": [
        ("classic", None),
        ("remastered", []),
        ("B_front", BG),
        ("E_k24", E24),
        ("F_r64", F64),
        ("G_cal", CAL),
        ("dbg_F", F64 + ["skyDebug=1"]),
        ("dbg_bounceF", F64 + ["skyDebug=2"]),
    ],
    "faces": [
        ("classic", None),
        ("remastered", []),
        ("B_front", BG),
        ("B_back", BG + ["skyFaces=0"]),
        ("C_front", BG + ["skyGain=1.2", "skyLeaf=0.45"]),
        ("dbg_front", BG + ["skyDebug=1"]),
        ("dbg_back", BG + ["skyFaces=0", "skyDebug=1"]),
    ],
    "lodbg": [
        ("dbg_L0", BG + ["skyDebug=40"]),
        ("dbg_L2", BG + ["skyDebug=42"]),
        ("dbg_L4", BG + ["skyDebug=44"]),
        ("dbg_L6", BG + ["skyDebug=46"]),
        ("dbg_d01", BG + ["skyDebug=11"]),
    ],
    "dirdbg2": [
        ("dbg_visB", BG + ["skyDebug=1"]),
        ("dbg_d01", BG + ["skyDebug=11"]),
        ("dbg_d06", BG + ["skyDebug=16"]),
        ("dbg_d09", BG + ["skyDebug=19"]),
        ("dbg_d14", BG + ["skyDebug=24"]),
    ],
    "final2": [
        ("classic", None),
        ("remastered", []),
        ("B_ground", BG),
        ("C_gain", BG + ["skyGain=1.2"]),
        ("dbg_visB", BG + ["skyDebug=1"]),
    ],
    "dirdbg": [
        ("dbg_visB", BG + ["skyDebug=1"]),
        ("dbg_cnt", BG + ["skyDebug=9"]),
        ("dbg_d02", BG + ["skyDebug=12"]),
        ("dbg_d06", BG + ["skyDebug=16"]),
        ("dbg_d10", BG + ["skyDebug=20"]),
        ("dbg_d14", BG + ["skyDebug=24"]),
    ],
    "timing": [
        ("t1_off", []), ("t1_on", BG), ("t1_d8", BG + ["skyDirs=8"]),
        ("t2_off", []), ("t2_on", BG), ("t2_d8", BG + ["skyDirs=8"]),
        ("t3_off", []), ("t3_on", BG), ("t3_d8", BG + ["skyDirs=8"]),
    ],
    "final1": [
        ("classic", None),
        ("remastered", []),
        ("B_ground", PHYS + ["skyGround=1"]),
        ("C_gain", PHYS + ["skyGround=1", "skyGain=1.2"]),
        ("B_nobounce", ["skyOcc=1", "skyLeaf=0.25", "skyBleed=0.08", "skyGround=1"]),
        ("dbg_visB", PHYS + ["skyGround=1", "skyDebug=1"]),
        ("dbg_bounceB", PHYS + ["skyGround=1", "skyDebug=2"]),
    ],
    "grid1": [
        ("classic", None),
        ("remastered", []),
        ("A_phys", PHYS),
        ("B_ground", PHYS + ["skyGround=1"]),
        ("C_gain", PHYS + ["skyGround=1", "skyGain=1.2"]),
        ("D_tint", PHYS + ["skyGround=1", "skyGain=1.2", "skyTint=0.5"]),
        ("dbg_visA", PHYS + ["skyDebug=1"]),
    ],
    "debug3": [
        ("remastered", []),
        ("dbg_vis", ["skyOcc=1", "skyDebug=1"]),
        ("dbg_nan", ["skyOcc=1", "skyDebug=7"]),
        ("dbg_leaf", ["skyOcc=1", "skyDebug=5"]),
        ("svm_occ", ["skyOcc=1"]),
    ],
    "debug2": [
        ("dbg_vis", ["skyOcc=1", "skyDebug=1"]),
        ("dbg_dir", ["skyOcc=1", "skyDebug=3"]),
        ("dbg_cov", ["skyOcc=1", "skyDebug=4"]),
        ("dbg_leaf", ["skyOcc=1", "skyDebug=5"]),
        ("dbg_recv", ["skyOcc=1", "skyDebug=6"]),
    ],
    "shipped": [
        ("classic", None),
        ("remastered", []),
    ],
    "main": [
        ("classic", None),
        ("remastered", []),
        ("svm_occ", ["skyOcc=1"]),
        ("svm_full", ["skyOcc=1", "skyBounce=1", "skyAlbedo=1"]),
        ("dbg_vis", ["skyOcc=1", "skyDebug=1"]),
    ],
    "debug": [
        ("remastered", []),
        ("dbg_vis", ["skyOcc=1", "skyDebug=1"]),
        ("dbg_dir", ["skyOcc=1", "skyDebug=3"]),
        ("dbg_cov", ["skyOcc=1", "skyDebug=4"]),
    ],
    # exploratory: does it work, what does the visibility look like
    "explore": [
        ("classic", None),
        ("remastered", []),
        ("dbg_vis", ["skyOcc=1", "skyDebug=1"]),
        ("svm_occ", ["skyOcc=1"]),
        ("svm_full", ["skyOcc=1", "skyBounce=1", "skyAlbedo=1"]),
        ("dbg_bounce", ["skyOcc=1", "skyBounce=1", "skyAlbedo=1", "skyDebug=2"]),
    ],
}

VIEWS = {
    "lowsun_fp": dict(world="Abel", args='["lowsun_fp", 7300, 10900, 1.75, 152.5, -3, [1985, 3, 1, 8, 0], 0, 0, 1500]',
                      props=['[["Jeep"], 10, -5, -107.5, "veh"] call capSpawn;',
                             '[["M113"], 16, 5, 72.5, "veh"] call capSpawn;',
                             '[["SoldierWB"], 7, 1, -112.5, "man"] call capSpawn;',
                             '[["LoBo_camnet", "CampEast", "Camp"], 22, -3, -152.5, "veh"] call capSpawn;']),
    "noon_fields": dict(world="Abel", args='["noon_fields", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500]',
                        props=['[["Jeep"], 13, 3, 60, "veh"] call capSpawn;',
                               '[["SoldierWB"], 8, -2, 160, "man"] call capSpawn;']),
    "forest_edge_golden": dict(world="Abel", args='["forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500]',
                               props=['[["SoldierWB"], 12, 2, 215, "man"] call capSpawn;']),
    # new: Saint Louis by day, the town of town_night (buildings, trees, street)
    "town_day": dict(world="Abel", args='["town_day", 7152, 8915, 1.75, 345, 3, [1985, 7, 5, 16, 30], 0, 0, 1200]',
                     props=['[["Jeep"], 14, 4, 80, "veh"] call capSpawn;',
                            '[["SoldierWB"], 9, -1, 170, "man"] call capSpawn;']),
    "town_night": dict(world="Abel", args='["town_night", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0, 0, 800]',
                       props=['capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn;',
                              'if (not (isNull capFire)) then { capFire inflame true };'],
                       pre_shot='call { if (not (isNull capFire)) then { capFire inflame true } }'),
    "overcast_broken": dict(world="Eden", args='["overcast_broken", 9200, 3700, 1.75, 288, -4, [1985, 7, 5, 14, 30], 0.5, 0, 3500]',
                            props=['[["Jeep"], 12, -5, 70, "veh"] call capSpawn;']),
    "overcast_heavy": dict(world="Eden", args='["overcast_heavy", 9200, 3700, 1.75, 288, -4, [1985, 7, 5, 14, 30], 0.9, 0, 3500]',
                           props=['[["Jeep"], 12, -5, 70, "veh"] call capSpawn;']),
    # woodland views of the shadows track (inside mixed wood, a pine stand at evening)
    "woods_mixed_noon": dict(world="Abel", args='["woods_mixed_noon", 2864, 6656, 1.75, 300, -5, [1985, 7, 5, 14, 0], 0, 0, 1500]',
                             props=['[["SoldierWB"], 7, 1, 160, "man"] call capSpawn;']),
    "woods_pine2_evening": dict(world="Abel", args='["woods_pine2_evening", 6940, 9964, 1.75, 240, -5, [1985, 7, 5, 17, 30], 0, 0, 1500]',
                                props=['[["SoldierWB"], 8, -1, 200, "man"] call capSpawn;']),
    "woods_overcast": dict(world="Abel", args='["woods_overcast", 2864, 6656, 1.75, 300, -5, [1985, 7, 5, 14, 0], 0.85, 0, 1500]',
                           props=['[["SoldierWB"], 7, 1, 160, "man"] call capSpawn;']),
    "town_street": dict(world="Abel", args='["town_street", 7140, 8930, 1.75, 315, 2, [1985, 7, 5, 16, 30], 0, 0, 1200]',
                        props=['[["SoldierWB"], 8, 0, 150, "man"] call capSpawn;']),
    # new: overcast close-up of a vehicle and a soldier on Eden grass (the RTX overcast look)
    "overcast_close": dict(world="Eden", args='["overcast_close", 9200, 3700, 1.75, 288, -8, [1985, 7, 5, 14, 30], 0.9, 0, 1500]',
                           props=['[["Jeep"], 8, -2, 40, "veh"] call capSpawn;',
                                  '[["M113"], 15, 5, 110, "veh"] call capSpawn;',
                                  '[["SoldierWB"], 6, 2, 200, "man"] call capSpawn;']),
}

WORLD_HDR = {
    "Abel": dict(mission="tests/rendering/captures/missions/udcap_abel.Abel", probe="[7300, 10900, 0]",
                 centers="createCenter west\ncreateCenter east"),
    "Eden": dict(mission="tests/rendering/captures/missions/udcap_eden.Eden", probe="[9200, 3700, 0]",
                 centers="createCenter west"),
}

LIB_EXTRA = r'''
// ---- wildcard research helpers -------------------------------------------
// "k=v" call capKnob: one triSetLighting knob, answer logged, never returned.
capKnob = {
    capKnobRes = triSetLighting _this;
    format ["view=%1;event=knob;set=%2;res=%3", capView, _this, capKnobRes] call capLog;
}
// ["label", ["k=v", ...]] call capVariant: remastered, the reset knobs, then the
// variant's knobs; restarts the GPU and CPU timers like capMode.
capVariant = {
    capModeName = _this select 0;
    capLit = triSetLighting "remastered";
    { _x call capKnob } forEach capReset;
    { _x call capKnob } forEach (_this select 1);
    capGpuOff = triGpuTimes false;
    capGpuOn = triGpuTimes true;
    capPerfRes = triPerfReset 0;
    format ["view=%1;event=mode;mode=%2;lit=%3;knobs=%4", capView, capModeName, capLit, _this select 1] call capLog;
}
'''


def sqf_list(items):
    return "[" + ", ".join('"%s"' % k for k in items) + "]"


OBJECT_SHADOWS = True
DOLLY_FRAMES = 8


def gen(run, views, variant_set, settle):
    DST.mkdir(parents=True, exist_ok=True)
    lib = (SRC / "capture_lib.sqf").read_text(encoding="utf-8") + LIB_EXTRA
    (DST / "capture_lib.sqf").write_text(lib, encoding="utf-8")
    variants = VARIANT_SETS[variant_set]
    by_world = {}
    for v in views:
        by_world.setdefault(VIEWS[v]["world"], []).append(v)
    written = []
    for world, vlist in by_world.items():
        h = WORLD_HDR[world]
        out = []
        out.append("// wildcard research capture (generated by gen_tests.py; do not edit by hand)")
        out.append('#include "capture_lib.sqf"')
        out.append("")
        out.append("triSimUntil { alive player }")
        out.append("")
        out.append('capWorld = "%s"' % world)
        out.append("capFov = 0.7")
        out.append("capSimT = 0")
        out.append("capPinTime = false")
        out.append("capUseView = false")
        out.append("capProps = []")
        out.append('capView = "setup"')
        out.append('capModeName = "none"')
        out.append('capLit = "none"')
        out.append('capGpu = "none"')
        out.append('capPerf = "none"')
        out.append("capReset = " + sqf_list(RESET))
        out.append("capFire = objNull")
        out.append("")
        if OBJECT_SHADOWS:
            out.append("// static objects (trees, buildings) cast: off in a fresh profile (shadows track finding)")
            out.append("triEnableShadows")
        out.append("triCaptureAtPresent true")
        out.append("triSetMsaa 4")
        out.append("triSetRenderScale 1.5")
        out.append("showCinemaBorder false")
        out.append("enableRadio false")
        out.append("player setCaptive true")
        out.append("player allowDamage false")
        out.append("")
        out.append(h["centers"])
        out.append("capGrpW = createGroup west")
        out.append('capProbe = "HeliHEmpty" createVehicle %s' % h["probe"])
        out.append('capCam = "camera" camCreate %s' % h["probe"].replace(", 0]", ", 2]"))
        out.append('capCam cameraEffect ["internal", "back"]')
        out.append("")
        out.append("triSimFrames 240")
        out.append('call { "view=setup;event=setup;world=%s;msaa=4;scale=1.5;window=960x640;capture=present;run=%s" call capLog }' % (world, run))
        out.append("")
        for v in vlist:
            d = VIEWS[v]
            out.append("// ---- %s" % v)
            out.append("%s call capBegin" % d["args"])
            if d.get("props"):
                out.append("call {")
                for p in d["props"]:
                    out.append("    " + p)
                out.append("}")
            out.append("triSimFrames 150")
            for label, knobs in variants:
                if knobs is None:
                    out.append('"classic" call capMode')
                else:
                    extra = [] if "overcast" in v else ["cloudShadows=0"]
                    out.append('["%s", %s] call capVariant' % (label, sqf_list(extra + knobs)))
                n = settle if not label.startswith("dbg") else 30
                out.append("triSimFrames %d" % n)
                out.append("[] call capReadTimers")
                out.append("[] call capPin")
                if d.get("pre_shot"):
                    out.append(d["pre_shot"])
                out.append("triSimFrames 3")
                out.append('triScreenshot "%s__%s"' % (v, label))
                out.append('"%s__%s" call capRecord' % (v, label))
                if label.startswith("dolly"):
                    for f in range(1, DOLLY_FRAMES):
                        out.append("call { capOX = capOX + 0.4 * (sin capH); capOZ = capOZ + 0.4 * (cos capH); [] call capPinView }")
                        out.append("triSimFrames 1")
                        out.append('triScreenshot "%s__%s_f%02d"' % (v, label, f))
                        out.append('"%s__%s_f%02d" call capRecord' % (v, label, f))
                    out.append("call { capOX = capOX - %f * (sin capH); capOZ = capOZ - %f * (cos capH) }" % (0.4 * (DOLLY_FRAMES - 1), 0.4 * (DOLLY_FRAMES - 1)))
            out.append("[] call capClear")
            out.append("")
        out.append('call { "view=done;event=done" call capLog }')
        out.append("triEndTest")
        name = "%s_%s" % (run, world.lower())
        (DST / (name + ".test.sqf")).write_text("\n".join(out) + "\n", encoding="utf-8")
        nshots = len(vlist) * len(variants)
        toml = "\n".join([
            "# wildcard research capture (generated by gen_tests.py)",
            'binary = "PoseidonGame"',
            'mission = "%s"' % h["mission"],
            'tags = ["lighting_capture", "headful", "exclusive"]',
            "timeout = %d" % max(900, 120 * nshots),
            "assert_timeout = 180",
            'extra_args = ["--no-strict", "--display-mode", "windowed", "--width", "960", "--height", "640"]',
        ]) + "\n"
        (DST / (name + ".test.toml")).write_text(toml, encoding="utf-8")
        written.append("tests/rendering/research/wildcard/%s.test.sqf" % name)
    print(",".join(written))


if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("--views", default="lowsun_fp")
    ap.add_argument("--variants", default="explore")
    ap.add_argument("--settle", type=int, default=90)
    ap.add_argument("--no-objshadows", action="store_true")
    a = ap.parse_args()
    OBJECT_SHADOWS = not a.no_objshadows
    gen(a.run, [v for v in a.views.split(",") if v], a.variants, a.settle)
