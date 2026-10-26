"""v6 run: fine canopy with the crown fix (no stipple), mode 13 (canopy sun
everywhere + canopy sky occlusion of the ground), new views chosen to show tree
shadows (telephoto slope, side-lit and backlit forest edge), and timing rounds
at the capture scale (1.5) and at 0.75 (a quarter of the pixels, the cost a
half-resolution trace would pay). Writes into the scratch worktree only."""
from pathlib import Path
import gen_tests as g

OUT = Path(r"D:\Arma_CWA_worktrees\gfx-hfrt\arma_CWR\tests\rendering\research\hfrt")

SHOTS = [
    ("remastered", [], 90),
    ("fine12", ["terrainShadows=12"], 90),
    ("fine13", ["terrainShadows=13"], 90),
    ("dbgsun_rem", ["debug=1"], 20),
    ("dbgsun_13", ["terrainShadows=13", "debug=1"], 20),
    ("dbgsky_rem", ["debug=2"], 20),
    ("dbgsky_13", ["terrainShadows=13", "debug=2"], 20),
]
TIMING = [
    ("classic", None),
    ("remastered", []),
    ("fine12", ["terrainShadows=12"]),
    ("fine13", ["terrainShadows=13"]),
]
TIMING_Q = [("remastered_q", []), ("fine13_q", ["terrainShadows=13"])]
ROUNDS = 6
ROUNDS_Q = 5

VIEWS = [
    dict(id="hill_1730", ox=5500, oz=6250, eye=15, heading=185, pitch=-14, date="[1985, 9, 20, 17, 30]",
         overcast=0, vd=4000, note="Malden summit looking S, sun 9.1 deg from the right", timing=True),
    dict(id="slope_tele_1730", ox=5500, oz=6250, eye=15, heading=200, pitch=-12, date="[1985, 9, 20, 17, 30]",
         overcast=0, vd=4000, fov=0.3, note="telephoto onto the wooded slope SSW of the summit", timing=False),
    dict(id="forest_side_1815", ox=4674, oz=10156, eye=1.75, heading=205, pitch=2, date="[1985, 7, 5, 18, 15]",
         overcast=0, vd=1500, note="wooded slope, sun 12 deg from the right", timing=True),
    dict(id="forest_back_1815", ox=4674, oz=10156, eye=1.75, heading=290, pitch=3, date="[1985, 7, 5, 18, 15]",
         overcast=0, vd=1500, note="wooded slope, looking into the sun", timing=False),
    dict(id="forest_edge_golden", ox=4674, oz=10156, eye=1.75, heading=75, pitch=3, date="[1985, 7, 5, 18, 15]",
         overcast=0, vd=1500, note="wooded slope lit from behind-left at 12 deg",
         props=['[["SoldierWB"], 12, 2, 215, "man"]'], timing=True),
]


def rounds(modes, n, tag=""):
    lines = []
    for r in range(n):
        for label, settings in modes:
            lines.append('"classic" call capMode' if settings is None else f'["{label}", {g.sqf_list(settings)}] call hfVariant')
            lines.append("triSimFrames 40")
            lines.append("[] call capReadTimers")
            lines.append(f'call {{ format ["view=%1;event=timing;round={r}{tag};mode={label};gpu=%2", capView, capGpu] call capLog }}')
    return lines


def timing_block():
    lines = rounds(TIMING, ROUNDS)
    lines.append("triSetRenderScale 0.75")
    lines.append("triSimFrames 30")
    lines += rounds(TIMING_Q, ROUNDS_Q, "q")
    lines.append("triSetRenderScale 1.5")
    lines.append("triSimFrames 30")
    return "\n".join(lines)


def main():
    parts = []
    for v in VIEWS:
        blk = g.view_block(v, SHOTS)
        if v.get("timing"):
            blk = blk.replace("[] call capClear", timing_block() + "\n[] call capClear")
        fov = v.get("fov")
        if fov:
            blk = f"capFov = {fov}\n" + blk + "\ncapFov = 0.7\n"
        parts.append(blk)
    body = g.HEADER.format(world="Abel", ox=VIEWS[0]["ox"], oz=VIEWS[0]["oz"]) + "\n" + "\n".join(parts)
    body += '\ncall { "view=done;event=done" call capLog }\ntriEndTest\n'
    (OUT / "hfrt_v6_abel.test.sqf").write_text(body, encoding="utf-8", newline="\n")
    (OUT / "hfrt_v6_abel.test.toml").write_text(g.TOML.format(m="abel.Abel").replace("timeout = 900", "timeout = 1500"),
                                                 encoding="utf-8", newline="\n")
    print("wrote hfrt_v6_abel", [v["id"] for v in VIEWS])


if __name__ == "__main__":
    main()
