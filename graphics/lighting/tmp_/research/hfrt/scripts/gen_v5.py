"""v5 run: fine canopy (CWR_HFRT_CANOPY_RES=4, 3.1 m) cone-traced sun shadows
(modes 10/11/12) against the coarse canopy (mode 8) and the shipped map, plus
timing rounds. Writes into the scratch worktree's research folder only."""
from pathlib import Path
import gen_tests as g

OUT = Path(r"D:\Arma_CWA_worktrees\gfx-hfrt\arma_CWR\tests\rendering\research\hfrt")

SHOTS = [
    ("remastered", [], 90),
    ("sun8", ["terrainShadows=8"], 90),
    ("fine10", ["terrainShadows=10"], 90),
    ("fine11", ["terrainShadows=11"], 90),
    ("fine12", ["terrainShadows=12"], 90),
    ("dbgsun_rem", ["debug=1"], 20),
    ("dbgsun_8", ["terrainShadows=8", "debug=1"], 20),
    ("dbgsun_10", ["terrainShadows=10", "debug=1"], 20),
    ("dbgsun_12", ["terrainShadows=12", "debug=1"], 20),
    ("dbgsky_11", ["terrainShadows=11", "debug=2"], 20),
]
TIMING = [
    ("classic", None),
    ("remastered", []),
    ("sun8", ["terrainShadows=8"]),
    ("fine10", ["terrainShadows=10"]),
    ("fine11", ["terrainShadows=11"]),
    ("fine12", ["terrainShadows=12"]),
]
ROUNDS = 4


def timing_block():
    lines = []
    for r in range(ROUNDS):
        for label, settings in TIMING:
            lines.append('"classic" call capMode' if settings is None else f'["{label}", {g.sqf_list(settings)}] call hfVariant')
            lines.append("triSimFrames 45")
            lines.append("[] call capReadTimers")
            lines.append(f'call {{ format ["view=%1;event=timing;round={r};mode={label};gpu=%2", capView, capGpu] call capLog }}')
    return "\n".join(lines)


def build(world, mission, views, name):
    parts = []
    for v in views:
        blk = g.view_block(v, SHOTS)
        blk = blk.replace("[] call capClear", timing_block() + "\n[] call capClear")
        parts.append(blk)
    body = g.HEADER.format(world=world, ox=views[0]["ox"], oz=views[0]["oz"]) + "\n" + "\n".join(parts)
    body += '\ncall { "view=done;event=done" call capLog }\ntriEndTest\n'
    (OUT / f"{name}.test.sqf").write_text(body, encoding="utf-8", newline="\n")
    (OUT / f"{name}.test.toml").write_text(g.TOML.format(m=mission), encoding="utf-8", newline="\n")
    print("wrote", name, [v["id"] for v in views])


if __name__ == "__main__":
    build("Abel", "abel.Abel", g.ABEL_VIEWS, "hfrt_v5_abel")
    build("Eden", "eden.Eden", g.EDEN_VIEWS, "hfrt_v5_eden")
