"""v8 run (launched with CWR_HFRT_CANOPY_RES=2, a 6.25 m canopy, 2048x2048):
mode 15 at half the canopy resolution against v7's 3.1 m canopy on the same
views, and mode 17 (15 without the canopy sky occlusion) to isolate what that
occlusion costs and shows. Writes into the scratch worktree only."""
from pathlib import Path
import gen_tests as g
import gen_v7 as v7

OUT = Path(r"D:\Arma_CWA_worktrees\gfx-hfrt\arma_CWR\tests\rendering\research\hfrt")

SHOTS = [
    ("remastered", [], 90),
    ("fine15", ["terrainShadows=15"], 90),
    ("fine17", ["terrainShadows=17"], 90),
    ("dbgsun_15", ["terrainShadows=15", "debug=1"], 20),
]
TIMING = [
    ("remastered", []),
    ("fine15", ["terrainShadows=15"]),
    ("fine17", ["terrainShadows=17"]),
]
ROUNDS = 10
KEEP = ("hill_1730", "slope_tele_1730", "ground_b_1730", "forest_edge_golden")


def timing_block():
    lines = []
    for r in range(ROUNDS):
        for label, settings in TIMING:
            lines.append(f'["{label}", {g.sqf_list(settings)}] call hfVariant')
            lines.append("triSimFrames 25")
            lines.append("[] call capReadTimers")
            lines.append(f'call {{ format ["view=%1;event=timing;round={r};mode={label};gpu=%2", capView, capGpu] call capLog }}')
    return "\n".join(lines)


def main():
    views = [v for v in v7.VIEWS if v["id"] in KEEP]
    parts = []
    for v in views:
        blk = g.view_block(v, SHOTS)
        blk = blk.replace("[] call capClear", timing_block() + "\n[] call capClear")
        if v.get("fov"):
            blk = f"capFov = {v['fov']}\n" + blk + "\ncapFov = 0.7\n"
        parts.append(blk)
    body = g.HEADER.format(world="Abel", ox=views[0]["ox"], oz=views[0]["oz"]) + "\n" + "\n".join(parts)
    body += '\ncall { "view=done;event=done" call capLog }\ntriEndTest\n'
    (OUT / "hfrt_v8_abel.test.sqf").write_text(body, encoding="utf-8", newline="\n")
    (OUT / "hfrt_v8_abel.test.toml").write_text(g.TOML.format(m="abel.Abel").replace("timeout = 900", "timeout = 1800"),
                                                 encoding="utf-8", newline="\n")
    print("wrote hfrt_v8_abel", [v["id"] for v in views])


if __name__ == "__main__":
    main()
