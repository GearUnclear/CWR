"""Run 6 generator: per-view variant lists on top of gen_tests.py.

    python gen_tests6.py <run-name> <out-dir> <plan>

plan "crowns+hulls": crown variants on the forest views, hue variants on the
low-sun boxes (lazy-VB build, ovr_v12 override).
plan "shipped": classic + remastered only (the shipped exe, no override).
plan "timing": alternating shipped rig / recommended set, timed (see TIMING).
"""
import sys
from pathlib import Path

import gen_tests as g

g.VIEWS.update({
    # Crowns at noon, same camera as forest_edge_golden: the crown variants must
    # not make midday trees only darker.
    "forest_edge_noon": ("Abel", 4674, 10156, 1.75, 75, 3, [1985, 7, 5, 12, 30], 0, 0, 1500, [
        (["SoldierWB"], 12, 2, 215, "man"),
    ]),
    # Evening-height sun on the lowsun_box ground (the rig and Classic depend on
    # the sun height only, so a 7.5 deg morning sun stands in for the evening):
    # 07:30 on 1 March, sun 7.5 deg at az 112.3, 90 deg left of the heading.
    "dawn_box": ("Abel", 7300, 10900, 1.75, 202.3, -3, [1985, 3, 1, 7, 30], 0, 0, 1500, [
        (["M113"], 15, 3, -45, "veh"),
        (["Jeep"], 10, -4.5, -45, "veh"),
        (["SoldierWB"], 6, 0.5, -60, "man"),
    ]),
    # 07:15, sun 4.8 deg at az 109.8.
    "dawn_box5": ("Abel", 7300, 10900, 1.75, 199.8, -3, [1985, 3, 1, 7, 15], 0, 0, 1500, [
        (["M113"], 15, 3, -45, "veh"),
        (["Jeep"], 10, -4.5, -45, "veh"),
        (["SoldierWB"], 6, 0.5, -60, "man"),
    ]),
    # Sinai at 16:35 on 21 July: sun about 7.6 deg at az 300.2, 90 deg left.
    "sinai_dusk_box": ("Sinai", 8350, 2950, 1.75, 30.2, -3, [2005, 7, 21, 16, 35], 0, 0, 2000, [
        (["LoBo_Zelda", "M113"], 15, 3, -45, "veh"),
        (["LoBoHMWV_Mag_Patrol", "Jeep"], 10, -4.5, -45, "veh"),
        (["LoBoGolaniWB", "SoldierWB"], 6, 0.5, -60, "man"),
    ]),
})

CROWNS = [
    ("v9s", ["localLights=9", "ao=0.71"]),
    ("v12s", ["localLights=12", "ao=0.71"]),
    ("v13s", ["localLights=13", "ao=0.71"]),
    ("v14s", ["localLights=14", "ao=0.71"], True),
    ("v15s", ["localLights=15", "ao=0.71"]),
    ("v16s", ["localLights=16", "ao=0.71"]),
]
HULLS = [
    ("v9s", ["localLights=9", "ao=0.71"], True),
    ("v11s", ["localLights=11", "ao=0.71"]),
    ("v14s", ["localLights=14", "ao=0.71"]),
]

# Knob-only candidates on the lazy-VB build (shipped rig, shipped composite):
# can triSetLighting defaults alone reach the targets?
KNOBS = [
    ("k1", ["sky=1.4"]),
    ("k2", ["sky=1.4", "warmth=1.5"]),
    ("k3", ["sky=1.8", "sun=1.2", "warmth=1.5"]),
    ("k4", ["bounce=2.5", "warmth=1.5"]),
    ("k5", ["sky=1.4", "warmth=1.5", "shafts=0.4"]),
    ("k6", ["exposure=1.15", "sky=1.3", "warmth=1.5", "saturation=1.2"]),
    ("v9s", ["localLights=9", "ao=0.71"]),
]

# C++ port of the V4 rig (LightRig.cpp, switched on by moonlight=1.01) against
# the shader prototype V4; both with the shipped composite (ao=0.7).
CPP = [
    ("v4", ["localLights=4"]),
    ("cpp", ["moonlight=1.01"]),
]

CROWNS2 = [
    ("v9s", ["localLights=9", "ao=0.71"]),
    ("v13s", ["localLights=13", "ao=0.71"]),
    ("v17s", ["localLights=17", "ao=0.71"], True),
    ("v18s", ["localLights=18", "ao=0.71"]),
]
HULLS2 = [
    ("v9s", ["localLights=9", "ao=0.71"]),
    ("v20s", ["localLights=20", "ao=0.71"]),
    ("v21s", ["localLights=21", "ao=0.71"]),
]

DIAG = [
    ("dbg1", ["debug=1", "post=0"]),
    ("dbg2", ["debug=2", "post=0"]),
    ("post0", ["post=0"]),
    ("ao0", ["ao=0"]),
    ("tocc0", ["terrainOcclusion=0"]),
    ("tsh0", ["terrainShadows=0"]),
]

CLOUDDIAG = [
    ("dbg1", ["debug=1", "post=0"]),
    ("dbg1cs0", ["debug=1", "post=0", "cloudShadows=0"]),
    ("cs0", ["cloudShadows=0"]),
]

FINAL_VIEWS = ["lowsun_fp", "lowsun_box", "dawn_box", "dawn_box5", "forest_edge_golden", "forest_edge_noon",
               "coastline_sunset", "sunrise_ridge", "noon_fields",
               "sinai_lowsun_fp", "sinai_box", "sinai_dusk_box", "sinai_desert_noon"]
FINAL = [("final", ["moonlight=1.01"], True)]

CLOUDFIX = [
    ("dbg1", ["debug=1", "post=0"]),
    ("final", ["moonlight=1.01"]),
    ("dbg1final", ["debug=1", "post=0", "moonlight=1.01"]),
]

PLANS = {
    "cloudfix": {"forest_edge_noon": CLOUDFIX, "coastline_sunset": CLOUDFIX},
    "final": {v: FINAL for v in FINAL_VIEWS},
    "shippedfinal": {v: [] for v in FINAL_VIEWS},
    "clouddiag": {
        "forest_edge_noon": CLOUDDIAG,
        "lowsun_fp": CLOUDDIAG,
        "sunrise_ridge": CLOUDDIAG,
        "noon_fields": CLOUDDIAG,
        "forest_edge_golden": CLOUDDIAG,
        "coastline_sunset": CLOUDDIAG,
        "sinai_lowsun_fp": CLOUDDIAG,
        "sinai_desert_noon": CLOUDDIAG,
    },
    "diagnoon": {
        "forest_edge_noon": DIAG,
        "noon_fields": DIAG,
    },
    "round10": {
        "forest_edge_golden": CROWNS2,
        "forest_edge_noon": CROWNS2,
        "lowsun_box": HULLS2,
        "dawn_box": HULLS2,
        "dawn_box5": HULLS2,
        "sinai_dusk_box": HULLS2,
        "sinai_box": HULLS2,
    },
    "cppport": {
        "lowsun_box": CPP,
        "dawn_box": CPP,
        "forest_edge_golden": CPP,
        "noon_fields": CPP,
        "sinai_box": CPP,
    },
    "knobs": {
        "lowsun_box": KNOBS,
        "dawn_box": KNOBS,
        "forest_edge_golden": KNOBS,
        "lowsun_fp": KNOBS,
    },
    "crowns+hulls": {
        "forest_edge_golden": CROWNS,
        "forest_edge_noon": CROWNS,
        "dawn_box": HULLS,
        "dawn_box5": HULLS,
        "sinai_dusk_box": HULLS,
    },
    "shipped": {
        "forest_edge_noon": [],
        "dawn_box": [],
        "dawn_box5": [],
        "sinai_dusk_box": [],
    },
}


TIMING_VIEWS = ["lowsun_fp", "forest_edge_golden", "sunrise_ridge", "noon_fields"]
TIMING_ROUNDS = 3
TIMING_MODES = [
    ("cl", None),
    ("v1", ["localLights=1", "ao=0.7"]),
    ("v9s", ["localLights=9", "ao=0.71"]),
    ("v14s", ["localLights=14", "ao=0.71"]),
]


def timing_block(vid, spec):
    world, x, z, eye, h, p, date, oc, fog, vd, props = spec
    out = [f"// ---- {vid} (timing)",
           f'["{vid}", {x}, {z}, {eye}, {h}, {p}, {g.sqf_list(date)}, {oc}, {fog}, {vd}] call capBegin']
    if props:
        out.append("call {")
        for cls, f, r, d, kind in props:
            cl = "[" + ", ".join(f'"{c}"' for c in cls) + "]"
            out.append(f'    [{cl}, {f}, {r}, {d}, "{kind}"] call capSpawn;')
        out.append("}")
    out.append("triSimFrames 150")
    for rnd in range(1, TIMING_ROUNDS + 1):
        for tag, knobs in TIMING_MODES:
            label = f"{vid}__t{rnd}{tag}"
            if knobs is None:
                out += ['"classic" call capMode']
            else:
                kn = "[" + ", ".join(f'"{k}"' for k in knobs) + "]"
                out += ['"remastered" call capMode',
                        f'call {{ ["t{rnd}{tag}", {kn}] call capVariant }}',
                        "call { capGpuOff = triGpuTimes false; capGpuOn = triGpuTimes true; capPerfRes = triPerfReset 0 }"]
            out += ["triSimFrames 120", "[] call capReadTimers", "[] call capPin", "triSimFrames 3",
                    f'triScreenshot "{label}"', f'"{label}" call capRecord']
    out += ["call { [] call capResetTuning }", "[] call capClear", ""]
    return "\n".join(out)


def main():
    run, out = sys.argv[1], Path(sys.argv[2])
    if sys.argv[3] == "timing":
        out.mkdir(parents=True, exist_ok=True)
        body = g.HEADER.format(world="Abel", x=g.VIEWS[TIMING_VIEWS[0]][1], z=g.VIEWS[TIMING_VIEWS[0]][2], run=run)
        body += "\n".join(timing_block(v, g.VIEWS[v]) for v in TIMING_VIEWS)
        body += '\ncall { "view=done;event=done" call capLog }\ntriEndTest\n'
        (out / f"{run}_abel.test.sqf").write_text(body, encoding="utf-8", newline="\n")
        (out / f"{run}_abel.test.toml").write_text(g.TOML["Abel"], encoding="utf-8", newline="\n")
        print("wrote", out / f"{run}_abel.test.sqf")
        return
    plan = PLANS[sys.argv[3]]
    out.mkdir(parents=True, exist_ok=True)
    for world, tag in (("Abel", "abel"), ("Sinai", "sinai")):
        vids = [v for v in plan if g.VIEWS[v][0] == world]
        if not vids:
            continue
        x, z = g.VIEWS[vids[0]][1], g.VIEWS[vids[0]][2]
        body = g.HEADER.format(world=world, x=x, z=z, run=run)
        body += "\n".join(g.view_block(v, g.VIEWS[v], plan[v]) for v in vids)
        body += '\ncall { "view=done;event=done" call capLog }\ntriEndTest\n'
        name = f"{run}_{tag}"
        (out / f"{name}.test.sqf").write_text(body, encoding="utf-8", newline="\n")
        (out / f"{name}.test.toml").write_text(g.TOML[world], encoding="utf-8", newline="\n")
        print("wrote", out / f"{name}.test.sqf", len(vids), "views")


if __name__ == "__main__":
    main()
