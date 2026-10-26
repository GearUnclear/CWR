"""Final BEST_<view>.png sheets: Classic | Remastered (current) | Remastered +
HFRT, all three from one run, same view and time, with GPU frame ms (minimum
over the interleaved timing rounds of that run, when the view was timed; the
GPU was shared with up to 3 other agents' games, so these are upper bounds).

Mode 15 (v7) for the Abel views; overcast_broken comes from v5 (mode 12, the
same 3.1 m canopy trace over the traced terrain; no Eden run of mode 15)."""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from sheets import panel, sheet, R  # noqa: E402

LABEL = {
    "classic": "Classic",
    "remastered": "Remastered (current)",
    "fine15": "Remastered + HFRT: 3.1 m canopy cone trace (mode 15)",
    "fine12": "Remastered + HFRT: 3.1 m canopy cone trace (mode 12)",
}
CROPS = {
    "hill_1730": (380, 250, 960, 560),
    "hill_0700": (300, 240, 960, 600),
    "overcast_broken": (480, 230, 960, 470),
    "forest_edge_golden": (380, 40, 960, 420),
    "sunrise_ridge": (0, 180, 960, 420),
    "slope_tele_1730": (400, 380, 960, 640),
    "ground_a_1730": (0, 300, 600, 640),
    "ground_b_1730": (0, 250, 640, 640),
}
JOBS = [
    ("sunrise_ridge", "v7", "fine15"),
    ("forest_edge_golden", "v7", "fine15"),
    ("hill_0700", "v7", "fine15"),
    ("hill_1730", "v7", "fine15"),
    ("slope_tele_1730", "v7", "fine15"),
    ("ground_a_1730", "v7", "fine15"),
    ("ground_b_1730", "v7", "fine15"),
    ("overcast_broken", "v5", "fine12"),
]


def minima(run):
    out = {}
    p = run / "timing_rounds.md"
    if p.exists():
        for line in p.read_text().splitlines()[2:]:
            c = [x.strip() for x in line.strip("|").split("|")]
            out[(c[0], c[1])] = (float(c[3]), int(c[2]))
    return out


for view, runname, best in JOBS:
    run = R / "runs" / runname
    mins = minima(run)
    full, zoom = [], []
    for m in ("classic", "remastered", best):
        t = mins.get((view, m))
        note = f"GPU frame {t[0]:.2f} ms (min of {t[1]} rounds, shared GPU)" if t else "GPU frame: view not timed"
        full.append(panel(run / f"{view}__{m}.png", LABEL[m], note))
        c = CROPS[view]
        s = 960 / (c[2] - c[0])
        zoom.append(panel(run / f"{view}__{m}.png", LABEL[m] + f" (crop x{s:.1f})", crop=c, scale=s))
    sheet([full, zoom], f"{view}: Classic | Remastered (current) | Remastered + HFRT canopy trace, run {runname}",
          R / f"BEST_{view}.png")
