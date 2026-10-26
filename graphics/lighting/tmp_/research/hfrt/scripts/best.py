"""BEST_<view>.png: Classic | Remastered (current) | Remastered + HFRT traced sun
(terrain + canopy, per pixel, step budget; mode 8), all three from one run, GPU
frame = minimum over the timing rounds of that same run (shared GPU)."""
import re
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))
from sheets import panel, sheet, R

def minima(run):
    out = {}
    for line in (run / "timing_rounds.md").read_text().splitlines()[2:]:
        c = [x.strip() for x in line.strip("|").split("|")]
        out[(c[0], c[1])] = (float(c[3]), int(c[2]))
    return out

CROPS = {
    "hill_1730": (380, 250, 960, 560),
    "hill_0700": (300, 240, 960, 600),
    "overcast_broken": (480, 230, 960, 470),
    "forest_edge_golden": (380, 40, 960, 420),
    "sunrise_ridge": (0, 180, 960, 420),
}
LABEL = {"classic": "Classic", "remastered": "Remastered (current)", "sun8": "Remastered + HFRT sun trace (terrain + canopy)"}

for view, run in (("sunrise_ridge", "v3"), ("forest_edge_golden", "v3"), ("hill_1730", "v3"),
                  ("overcast_broken", "v3"), ("hill_0700", "v4")):
    r = R / "runs" / run
    mins = minima(r)
    full, zoom = [], []
    for m in ("classic", "remastered", "sun8"):
        t = mins.get((view, m))
        note = f"GPU frame {t[0]:.2f} ms (min of {t[1]} rounds)" if t else "GPU frame: not timed in this run"
        full.append(panel(r / f"{view}__{m}.png", LABEL[m], note))
        c = CROPS[view]
        zoom.append(panel(r / f"{view}__{m}.png", LABEL[m] + " (crop x" + f"{960 / (c[2] - c[0]):.1f})",
                          crop=c, scale=960 / (c[2] - c[0])))
    sheet([full, zoom], f"{view}: Classic | Remastered (current) | Remastered + HFRT (mode 8), run {run}",
          R / f"BEST_{view}.png")
