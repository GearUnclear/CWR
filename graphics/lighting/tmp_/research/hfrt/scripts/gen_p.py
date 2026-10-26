"""Variant P run (scratch build 2 with CWR_HFRT_CANOPY_HORIZON=1): the shipped
shader reading a horizon map whose occluders include the landscape canopy."""
from pathlib import Path
import gen_tests as g

SHOTS = [
    ("canopyHorizon", [], 90),
    ("sun8", ["terrainShadows=8"], 90),
    ("dbgsun_P", ["debug=1"], 20),
    ("dbgsky_P", ["debug=2"], 20),
]
TIMING = [("classic", None), ("canopyHorizon", [])]
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
    for root in (g.OUT, Path(r"D:\Arma_CWA_worktrees\gfx-hfrt\arma_CWR\tests\rendering\research\hfrt")):
        (root / f"{name}.test.sqf").write_text(body, encoding="utf-8", newline="\n")
        (root / f"{name}.test.toml").write_text(g.TOML.format(m=mission), encoding="utf-8", newline="\n")
    print("wrote", name, [v["id"] for v in views])

build("Abel", "abel.Abel", g.ABEL_VIEWS, "hfrt_p_abel")
build("Eden", "eden.Eden", g.EDEN_VIEWS, "hfrt_p_eden")
