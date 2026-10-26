"""v4 run: per-vertex traced sun (mode 9) against per-pixel (mode 8)."""
from pathlib import Path
import gen_tests as g

SHOTS = [
    ("remastered", [], 90),
    ("sun8", ["terrainShadows=8"], 90),
    ("sun9", ["terrainShadows=9"], 90),
    ("dbgsun_8", ["terrainShadows=8", "debug=1"], 20),
    ("dbgsun_9", ["terrainShadows=9", "debug=1"], 20),
]
TIMING = [
    ("remastered", []),
    ("sun8", ["terrainShadows=8"]),
    ("sun9", ["terrainShadows=9"]),
]
ROUNDS = 5

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
    for root in (g.OUT, Path(r"D:\Arma_CWA_worktrees\gfx-hfrt\arma_CWR\tests\rendering\research\hfrt")):
        (root / f"{name}.test.sqf").write_text(body, encoding="utf-8", newline="\n")
        (root / f"{name}.test.toml").write_text(g.TOML.format(m=mission), encoding="utf-8", newline="\n")
    print("wrote", name, [v["id"] for v in views])

build("Abel", "abel.Abel", [v for v in g.ABEL_VIEWS if v["id"] in ("forest_edge_golden", "hill_0700", "hill_1730")], "hfrt_v4_abel")
