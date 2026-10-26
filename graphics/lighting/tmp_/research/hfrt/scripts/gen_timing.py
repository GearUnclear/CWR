"""Timing-only test: modes alternated in rounds on fixed views, GPU timers read
after each settle; the GPU is shared with other agents' games, so the
collector takes the minimum and median over rounds."""
from pathlib import Path
import gen_tests as g

MODES = [
    ("classic", None),
    ("remastered", []),
    ("hfA", ["terrainShadows=2", "terrainOcclusion=1"]),
    ("hfB", ["terrainShadows=3", "terrainOcclusion=1"]),
    ("hfCan", ["terrainShadows=4", "terrainOcclusion=1"]),
    ("hfLite", ["terrainShadows=5", "terrainOcclusion=1"]),
]
ROUNDS = 6

def block(v):
    lines = [f"// ---- timing {v['id']}"]
    lines.append(f"[\"{v['id']}\", {v['ox']}, {v['oz']}, {v['eye']}, {v['heading']}, {v['pitch']}, {v['date']}, {v['overcast']}, 0, {v['vd']}] call capBegin")
    lines.append("triSimFrames 150")
    for r in range(ROUNDS):
        for label, settings in MODES:
            if settings is None:
                lines.append('"classic" call capMode')
            else:
                lines.append(f'["{label}", {g.sqf_list(settings)}] call hfVariant')
            lines.append("triSimFrames 45")
            lines.append("[] call capReadTimers")
            lines.append(f'call {{ format ["view=%1;event=timing;round={r};mode={label};gpu=%2", capView, capGpu] call capLog }}')
    lines.append("")
    return "\n".join(lines)

views = [v for v in g.ABEL_VIEWS if v["id"] in ("sunrise_ridge", "forest_edge_golden", "hill_1730")]
body = g.HEADER.format(world="Abel", ox=views[0]["ox"], oz=views[0]["oz"]) + "\n" + "\n".join(block(v) for v in views)
body += '\ncall { "view=done;event=done" call capLog }\ntriEndTest\n'
for root in (g.OUT, Path(r"D:\Arma_CWA_worktrees\gfx-hfrt\arma_CWR\tests\rendering\research\hfrt")):
    (root / "hfrt_timing.test.sqf").write_text(body, encoding="utf-8", newline="\n")
    (root / "hfrt_timing.test.toml").write_text(g.TOML.format(m="abel.Abel"), encoding="utf-8", newline="\n")
print("wrote timing", [v["id"] for v in views], body.count("event=timing"))
