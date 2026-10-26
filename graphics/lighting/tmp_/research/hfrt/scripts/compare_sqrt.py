"""SQRT_<view>.png: Remastered | 3.1 m canopy, linear density (mode 18) |
3.1 m canopy, sqrt crossing density (mode 20) | 6.25 m canopy mode 18 |
6.25 m canopy mode 20. Runs v10_k4 and v10_k2 (same view and time, separate
launches because the canopy resolution is a load-time setting)."""
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
K4, K2 = R / "runs" / "v10_k4", R / "runs" / "v10_k2"
for view, c in CROPS.items():
    cols = [
        (K4 / f"{view}__remastered.png", "Remastered (current)"),
        (K4 / f"{view}__fine18.png", "3.1 m canopy, density (18)"),
        (K4 / f"{view}__fine20.png", "3.1 m canopy, sqrt density (20)"),
        (K2 / f"{view}__fine18.png", "6.25 m canopy, density (18)"),
        (K2 / f"{view}__fine20.png", "6.25 m canopy, sqrt density (20)"),
    ]
    s = 576 / (c[2] - c[0])
    full = [panel(p, t, scale=0.6) for p, t in cols]
    zoom = [panel(p, t + " (crop)", crop=c, scale=s) for p, t in cols]
    sheet([full, zoom], f"{view}: canopy resolution x crossing density", R / f"SQRT_{view}.png")
