"""v7 run: the production candidate (mode 15: baked horizon map for the terrain
x 3.1 m canopy cone trace everywhere + canopy sky occlusion; mode 16 = half the
canopy steps) against mode 13 (v6, traced terrain), on the required views plus
the telephoto slope and two ground-level guesses among the summit trees, with
10 short timing rounds per timed view. Writes into the scratch worktree only."""
from pathlib import Path
import gen_tests as g

OUT = Path(r"D:\Arma_CWA_worktrees\gfx-hfrt\arma_CWR\tests\rendering\research\hfrt")

SHOTS = [
    ("remastered", [], 90),
    ("fine13", ["terrainShadows=13"], 90),
    ("fine15", ["terrainShadows=15"], 90),
    ("fine16", ["terrainShadows=16"], 90),
    ("dbgsun_rem", ["debug=1"], 20),
    ("dbgsun_15", ["terrainShadows=15", "debug=1"], 20),
    ("dbgsky_15", ["terrainShadows=15", "debug=2"], 20),
]
TIMING = [
    ("classic", None),
    ("remastered", []),
    ("fine13", ["terrainShadows=13"]),
    ("fine15", ["terrainShadows=15"]),
    ("fine16", ["terrainShadows=16"]),
]
ROUNDS = 10

VIEWS = [
    dict(id="hill_1730", ox=5500, oz=6250, eye=15, heading=185, pitch=-14, date="[1985, 9, 20, 17, 30]",
         overcast=0, vd=4000, note="Malden summit looking S, sun 9.1 deg from the right", timing=True),
    dict(id="slope_tele_1730", ox=5500, oz=6250, eye=15, heading=200, pitch=-12, date="[1985, 9, 20, 17, 30]",
         overcast=0, vd=4000, fov=0.3, note="telephoto onto the wooded slope SSW of the summit", timing=True),
    dict(id="ground_a_1730", ox=5490, oz=6180, eye=2.5, heading=195, pitch=-8, date="[1985, 9, 20, 17, 30]",
         overcast=0, vd=4000, note="guess: on the slope below the summit, looking down at the trees", timing=False),
    dict(id="ground_b_1730", ox=5440, oz=6200, eye=2.5, heading=180, pitch=-8, date="[1985, 9, 20, 17, 30]",
         overcast=0, vd=4000, note="guess: west of a, sun from the right", timing=False),
    dict(id="hill_0700", ox=5500, oz=6250, eye=15, heading=185, pitch=-14, date="[1985, 10, 10, 7, 0]",
         overcast=0, vd=4000, note="same vantage, sun 9.8 deg from the left", timing=False),
    dict(id="sunrise_ridge", ox=4400, oz=3300, eye=1.75, heading=60, pitch=2, date="[1985, 7, 5, 5, 8]",
         overcast=0, vd=4000, note="sun 5.1 deg just clear of the central massif", timing=False),
    dict(id="forest_edge_golden", ox=4674, oz=10156, eye=1.75, heading=75, pitch=3, date="[1985, 7, 5, 18, 15]",
         overcast=0, vd=1500, note="wooded slope lit from behind-left at 12 deg",
         props=['[["SoldierWB"], 12, 2, 215, "man"]'], timing=True),
]


def timing_block():
    lines = []
    for r in range(ROUNDS):
        for label, settings in TIMING:
            lines.append('"classic" call capMode' if settings is None else f'["{label}", {g.sqf_list(settings)}] call hfVariant')
            lines.append("triSimFrames 25")
            lines.append("[] call capReadTimers")
            lines.append(f'call {{ format ["view=%1;event=timing;round={r};mode={label};gpu=%2", capView, capGpu] call capLog }}')
    return "\n".join(lines)


def main():
    parts = []
    for v in VIEWS:
        blk = g.view_block(v, SHOTS)
        if v.get("timing"):
            blk = blk.replace("[] call capClear", timing_block() + "\n[] call capClear")
        if v.get("fov"):
            blk = f"capFov = {v['fov']}\n" + blk + "\ncapFov = 0.7\n"
        parts.append(blk)
    body = g.HEADER.format(world="Abel", ox=VIEWS[0]["ox"], oz=VIEWS[0]["oz"]) + "\n" + "\n".join(parts)
    body += '\ncall { "view=done;event=done" call capLog }\ntriEndTest\n'
    (OUT / "hfrt_v7_abel.test.sqf").write_text(body, encoding="utf-8", newline="\n")
    (OUT / "hfrt_v7_abel.test.toml").write_text(g.TOML.format(m="abel.Abel").replace("timeout = 900", "timeout = 1800"),
                                                 encoding="utf-8", newline="\n")
    print("wrote hfrt_v7_abel", [v["id"] for v in VIEWS])


if __name__ == "__main__":
    main()
