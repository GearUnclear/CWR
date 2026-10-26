"""Generate the foliage research capture tests (Trident .test.sqf + .toml).

    python gen_tests.py <run-name> [--views a,b,...]

Writes tests/rendering/research/foliage/fol_<world>_<run>.test.{sqf,toml} in
the gfx-foliage scratch worktree. Every view is shot as Classic, Remastered
(research knobs all 0, the shipped foliage path) and each variant in VARIANTS
(knob sets f0..f7), plus optional debug shots. Labels: <view>__<variant>.
"""
import argparse
import sys
from pathlib import Path

ROOT = Path(r"D:\Arma_CWA_worktrees\gfx-foliage\arma_CWR")
OUT = ROOT / "tests" / "rendering" / "research" / "foliage"

# f0 model, f1 sigma, f2 translucency, f3 variation, f4 crown sky, f5 multiple scatter, f6 debug, f7 self-shadow
VARIANTS = {
    # r1 (ov2: sigma in 1/m)
    "r1A": [1, 0.30, 0.0, 0.0, 1.0, 0.5, 0, 1.0],
    "r1B": [1, 0.30, 1.0, 1.0, 1.0, 0.5, 0, 1.0],
    "r1C": [1, 0.45, 1.6, 1.5, 1.0, 0.8, 0, 1.0],
    # r2 (ov3: f1 = optical depth across a crown diameter)
    "vA": [1, 2.0, 0.0, 0.0, 1.0, 0.5, 0, 1.0],    # volume only: self-shadow, crown sky, in-crown scatter
    "vB": [1, 2.0, 1.0, 1.0, 1.0, 0.5, 0, 1.0],    # + translucency + per-tree variation
    "vC": [1, 1.5, 1.6, 1.0, 1.0, 0.8, 0, 0.6],    # brighter, glowier, softer self-shadow
    # r3 (ov4: f6 packs debug + 10 * tint green tenths + 100 * sky rim)
    "vD": [1, 2.0, 1.3, 1.0, 1.0, 0.6, 50, 1.0],   # vB, greener transmission tint, a bit more glow
    "vE": [1, 2.0, 1.3, 1.0, 1.0, 0.6, 150, 1.0],  # vD + sky light through thin crown edges
    "vF": [1, 1.5, 1.8, 1.2, 1.0, 0.8, 150, 0.7],  # strong
    # r4 (ov5: separate transmission / fill tints, narrower forward lobe, deeper transmission falloff)
    "vG": [1, 2.0, 1.3, 1.0, 1.0, 0.6, 50, 1.0],
    "vH": [1, 2.0, 1.3, 1.0, 1.0, 0.6, 150, 1.0],
    "vI": [1, 2.0, 1.8, 1.0, 1.0, 0.6, 170, 1.0],
    # r6 (ov7 + canopy build: geometric trunk mask; c0 canopy sky, c1 canopy sun far, c2 debug, c3 sigma)
    "vH2": [1, 2.0, 1.3, 1.0, 1.0, 0.6, 150, 1.0],
    "vHc": [1, 2.0, 1.3, 1.0, 1.0, 0.6, 150, 1.0, 1.0, 1.0, 0, 0.12],
    "vHc2": [1, 2.0, 1.3, 1.0, 1.0, 0.6, 150, 1.0, 1.0, 1.0, 0, 0.25],
    "vHs": [1, 2.0, 1.3, 1.0, 1.0, 0.6, 150, 1.0, 1.0, 0.0, 0, 0.12],   # canopy sky occlusion only
    # r8 final set (ov7 + canopy build)
    "vJ": [1, 2.0, 1.0, 1.0, 1.0, 0.5, 130, 1.0],   # milder: less glow, tint green 0.3, sky rim
}
DEBUG = {
    # r2 debug set (ov3); r5 uses only dbgleaf (ov6: f6 units 7 = alpha test / A2C flags)
    "dbgdepth": [1, 2.0, 1.0, 1.0, 1.0, 0.5, 1, 1.0],
    "dbgtrans": [1, 2.0, 1.0, 1.0, 1.0, 0.5, 2, 1.0],
    "dbgexit": [1, 2.0, 1.0, 1.0, 1.0, 0.5, 5, 1.0],
}
DEBUG_R5 = {"dbgleaf": [1, 2.0, 1.3, 1.0, 1.0, 0.6, 57, 1.0]}
DEBUG_R6 = {
    "dbgleaf": [1, 2.0, 1.3, 1.0, 1.0, 0.6, 157, 1.0],
    "dbgcover": [1, 2.0, 1.3, 1.0, 1.0, 0.6, 150, 1.0, 1.0, 1.0, 1, 0.12],
    "dbgcansun": [1, 2.0, 1.3, 1.0, 1.0, 0.6, 150, 1.0, 1.0, 1.0, 2, 0.12],
    "dbgcansky": [1, 2.0, 1.3, 1.0, 1.0, 0.6, 150, 1.0, 1.0, 1.0, 3, 0.12],
}

# id, x, z, eye, heading, pitch, date, overcast, fog, vd, props (sqf block or ""), debug?
VIEWS = {
    "abel": [
        ("forest_edge_golden", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 18, 15], 0, 0, 1500,
         '[["SoldierWB"], 12, 2, 215, "man"] call capSpawn;', "R6"),
        # Backlit: hornbeam and aspen 24-49 m toward the 18:15 sun (az 290, 12 deg).
        ("backlit_hornbeam", 7495, 10210, 1.75, 283, 6, [1985, 7, 5, 18, 15], 0, 0, 1500, "", "SUNVIS"),
        # Backlit: birch grove 26-54 m toward the sun on the 352 m upland.
        ("backlit_birch", 4915, 8860, 1.75, 283, 7, [1985, 7, 5, 18, 15], 0, 0, 1500, "", False),
        # Clearing inside the pine stand behind forest_edge_golden, at noon:
        # 19 pines within 30 m, the nearest 6.4 m away.
        ("pine_stand_noon", 4770, 10128, 1.75, 75, -3, [1985, 7, 5, 12, 30], 0, 0, 1500, "", "R7"),
        # The same grove at noon (sun 72 deg, az 202: high and to the left).
        ("noon_grove", 4915, 8860, 1.75, 283, 2, [1985, 7, 5, 12, 30], 0, 0, 1500, "", False),
        ("noon_fields", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 12, 30], 0, 0, 2500,
         '[["Jeep"], 13, 3, 60, "veh"] call capSpawn; [["SoldierWB"], 8, -2, 160, "man"] call capSpawn;', False),
        # The noon_fields spot at 18:30 (sun 9.3 deg, az 292, from the right):
        # tree lines 150-800 m away, beyond the shadow maps, cast long shadows.
        ("fields_golden", 6250, 10800, 1.75, 150, -2, [1985, 7, 5, 18, 30], 0, 0, 2500, "", "R6"),
        ("town_night", 7152, 8915, 1.75, 345, 3, [1985, 2, 27, 22, 0], 0, 0, 800, "FIRE", False),
    ],
    "eden": [
        ("overcast_broken", 9200, 3700, 1.75, 288, -4, [1985, 7, 5, 14, 30], 0.5, 0, 3500,
         '[["Jeep"], 12, -5, 70, "veh"] call capSpawn;', False),
        # The spruce edge at 18:15 under a clear sky (sun 12 deg, az 290: from the left).
        ("eden_forest_golden", 9200, 3700, 1.75, 15, 3, [1985, 7, 5, 18, 15], 0, 0, 3500, "", False),
        # Same spot, turned to the spruce edge 50-90 m north under the same sky.
        ("overcast_forest", 9200, 3700, 1.75, 15, 4, [1985, 7, 5, 14, 30], 0.5, 0, 3500, "", False),
    ],
}
MISSION = {"abel": "udcap_abel.Abel", "eden": "udcap_eden.Eden"}
PROBE = {"abel": (7300, 10900), "eden": (9200, 3700)}


def shot(lines, view, label, frames=90):
    lines += [
        f"triSimFrames {frames}",
        "[] call capReadTimers",
        "[] call capPin",
    ]
    if view[10] == "FIRE":
        lines.append("call { if (not (isNull capFire)) then { capFire inflame true } }")
    lines += [
        "triSimFrames 3",
        f'triScreenshot "{view[0]}__{label}"',
        f'"{view[0]}__{label}" call capRecord',
    ]


def _arr(vals):
    return "[" + ", ".join(repr(float(v)) if isinstance(v, float) else str(v) for v in vals) + "]"


def knobs(vals):
    """f0..f7 (and c0..c7 when 16 values are given; 8 values reset c0..c7 to 0
    when the run uses the canopy build, see CANOPY)."""
    out = _arr(vals[:8]) + " call capKnobs"
    if CANOPY:
        c = list(vals[8:16]) + [0] * (8 - len(vals[8:16]))
        out += chr(10) + _arr(c) + " call capCanopy"
    return out


a_debugset = "r2"
CANOPY = False


def gen(world, run, views, variants, debug):
    px, pz = PROBE[world]
    L = [
        "// Foliage research capture run (generated by tmp/research/foliage/scripts/gen_tests.py).",
        '#include "capture_lib.sqf"',
        "",
        "triSimUntil { alive player }",
        "",
        f'capWorld = "{world.capitalize()}"',
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
        "capFire = objNull",
        "triCaptureAtPresent true",
        "triSetMsaa 4",
        "triSetRenderScale 1.5",
        "showCinemaBorder false",
        "enableRadio false",
        "player setCaptive true",
        "player allowDamage false",
        "createCenter west",
        "createCenter east",
        "capGrpW = createGroup west",
        f'capProbe = "HeliHEmpty" createVehicle [{px}, {pz}, 0]',
        f'capCam = "camera" camCreate [{px}, {pz}, 2]',
        'capCam cameraEffect ["internal", "back"]',
        "triSimFrames 240",
        f'call {{ "view=setup;event=setup;world={world};msaa=4;scale=1.5;window=960x640;capture=present" call capLog }}',
        "",
    ]
    for v in VIEWS[world]:
        if views and v[0] not in views:
            continue
        vid, x, z, eye, hd, pt, date, oc, fog, vd, props, dbg = v
        L.append(f'["{vid}", {x}, {z}, {eye}, {hd}, {pt}, {date}, {oc}, {fog}, {vd}] call capBegin'.replace("'", ""))
        if props == "FIRE":
            L.append('call { capFire = [["Fire"], 9, 3, 0, "veh"] call capSpawn; if (not (isNull capFire)) then { capFire inflame true }; }')
        elif props:
            L.append("call { " + props + " }")
        # Street lamps switch on some time after the clock jumps to night.
        L.append("triSimFrames %d" % (900 if props == "FIRE" else 150))
        L.append('"classic" call capMode')
        shot(L, v, "classic")
        L.append(knobs([0] * 8))
        L.append('"remastered" call capMode')
        shot(L, v, "remastered")
        for name in variants:
            L.append(knobs(VARIANTS[name]))
            L.append(f'"{name}" call capVariant')
            shot(L, v, name)
        if dbg == "SUNVIS" or (dbg and debug and a_debugset == "r7"):
            L.append(knobs([0] * 8))
            L.append('call { capDbg = triSetLighting "debug=1" }')
            L.append('"dbgsunvis" call capVariant')
            shot(L, v, "dbgsunvis", frames=20)
            L.append('call { capDbg = triSetLighting "debug=0" }')
        if dbg and debug and (dbg is True or a_debugset == "r6"):
            for name, kv in {"r5": DEBUG_R5, "r6": DEBUG_R6}.get(a_debugset, DEBUG).items():
                L.append(knobs(kv))
                L.append(f'"{name}" call capVariant')
                shot(L, v, name, frames=20)
        L.append(knobs([0] * 8))
        L.append("[] call capClear")
        L.append("")
    L.append('call { "view=done;event=done" call capLog }')
    L.append("triEndTest")
    sqf = OUT / f"fol_{world}_{run}.test.sqf"
    sqf.write_text("\n".join(L) + "\n", encoding="utf-8", newline="\n")
    toml = OUT / f"fol_{world}_{run}.test.toml"
    toml.write_text(
        "# Foliage research capture run (generated). Not a regression test.\n"
        'binary = "PoseidonGame"\n'
        f'mission = "tests/rendering/captures/missions/{MISSION[world]}"\n'
        'tags = ["lighting_capture", "headful", "exclusive"]\n'
        "timeout = 900\n"
        "assert_timeout = 180\n"
        'extra_args = ["--no-strict", "--display-mode", "windowed", "--width", "960", "--height", "640"]\n',
        encoding="utf-8", newline="\n")
    print(sqf)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("--views", default="")
    ap.add_argument("--variants", default="vA,vB,vC")
    ap.add_argument("--no-debug", action="store_true")
    ap.add_argument("--worlds", default="abel,eden")
    ap.add_argument("--timing", default="", help="variant for an alternating GPU timing run")
    ap.add_argument("--debugset", default="r2")
    ap.add_argument("--canopy", action="store_true", help="the build has c0..c7 knobs")
    a = ap.parse_args()
    global a_debugset, CANOPY
    a_debugset = a.debugset
    CANOPY = a.canopy
    if a.timing:
        for w in a.worlds.split(","):
            gen_timing(w, a.run, [s for s in a.views.split(",") if s], a.timing)
        return
    views = [s for s in a.views.split(",") if s]
    variants = [s for s in a.variants.split(",") if s]
    for w in a.worlds.split(","):
        gen(w, a.run, views, variants, not a.no_debug)




def gen_timing(world, run, views, variant, reps=4):
    """Alternating Remastered / variant GPU timing, no screenshots: each block
    resets the timers, settles 120 frames and logs a UDCAP event=timing line.
    Alternation spreads the load of other games on the machine over both."""
    px, pz = PROBE[world]
    L = [
        "// Foliage research GPU timing run (generated by gen_tests.py --timing).",
        '#include "capture_lib.sqf"',
        "triSimUntil { alive player }",
        f'capWorld = "{world.capitalize()}"',
        "capFov = 0.7", "capSimT = 0", "capPinTime = false", "capUseView = false", "capProps = []",
        'capView = "setup"', 'capModeName = "none"', 'capLit = "none"', 'capGpu = "none"', 'capPerf = "none"',
        "triCaptureAtPresent true", "triSetMsaa 4", "triSetRenderScale 1.5", "showCinemaBorder false",
        "enableRadio false", "player setCaptive true", "player allowDamage false", "createCenter west",
        "capGrpW = createGroup west",
        f'capProbe = "HeliHEmpty" createVehicle [{px}, {pz}, 0]',
        f'capCam = "camera" camCreate [{px}, {pz}, 2]',
        'capCam cameraEffect ["internal", "back"]',
        "triSimFrames 240",
    ]
    for v in VIEWS[world]:
        if views and v[0] not in views:
            continue
        vid, x, z, eye, hd, pt, date, oc, fog, vd, props, dbg = v
        L.append(f'["{vid}", {x}, {z}, {eye}, {hd}, {pt}, {date}, {oc}, {fog}, {vd}] call capBegin')
        L.append("triSimFrames 150")
        for r in range(reps):
            for name, kv in [("remastered", [0] * 8)] + [(vn, VARIANTS[vn]) for vn in variant.split(",")]:
                L.append(knobs(kv))
                L.append(f'"{name}" call capVariant')
                L.append("triSimFrames 120")
                L.append("[] call capReadTimers")
                L.append(f'call {{ format ["view=%1;event=timing;label={name};rep={r};gpu=%2", capView, capGpu] call capLog }}')
        L.append(knobs([0] * 8))
        L.append("[] call capClear")
    L.append('call { "view=done;event=done" call capLog }')
    L.append("triEndTest")
    sqf = OUT / f"fol_{world}_{run}.test.sqf"
    sqf.write_text("\n".join(L) + "\n", encoding="utf-8", newline="\n")
    (OUT / f"fol_{world}_{run}.test.toml").write_text(
        (OUT / f"fol_{world}_r2.test.toml").read_text(encoding="utf-8"), encoding="utf-8", newline="\n")
    print(sqf)


if __name__ == "__main__":
    main()
