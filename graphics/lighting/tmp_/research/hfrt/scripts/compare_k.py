"""K_<view>.png: Remastered | mode 15 with the 3.1 m canopy (k=4, run v7) |
mode 15 with the 6.25 m canopy (k=2, run v8_k2) | mode 17 = 15 without the
canopy sky occlusion (k=2, run v8_k2), with a crop row. Same view and time in
both runs (separate game launches, the canopy resolution is a load-time
setting)."""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from sheets import panel, sheet, R  # noqa: E402

CROPS = {
    "hill_1730": (380, 250, 960, 560),
    "forest_edge_golden": (380, 40, 960, 420),
    "slope_tele_1730": (400, 380, 960, 640),
    "ground_b_1730": (0, 250, 640, 640),
}
V7, V8 = R / "runs" / "v7", R / "runs" / "v8_k2"
for view, c in CROPS.items():
    cols = [
        (V7 / f"{view}__remastered.png", "Remastered (current)"),
        (V7 / f"{view}__fine15.png", "mode 15, 3.1 m canopy (k=4)"),
        (V8 / f"{view}__fine15.png", "mode 15, 6.25 m canopy (k=2)"),
        (V8 / f"{view}__fine17.png", "mode 17: k=2 without canopy sky occlusion"),
    ]
    s = 720 / (c[2] - c[0])
    full = [panel(p, t, scale=0.75) for p, t in cols]
    zoom = [panel(p, t + " (crop)", crop=c, scale=s) for p, t in cols]
    sheet([full, zoom], f"{view}: canopy resolution and canopy sky occlusion", R / f"K_{view}.png")
