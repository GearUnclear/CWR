"""Mean 8-bit change per region vs the Remastered shot of the same view.

python regions.py <run dir> <view>
"""
import glob
import re
import sys
from pathlib import Path

import numpy as np
from PIL import Image

REGIONS = {
    "lowsun_fp": {"tent": (230, 340, 290, 490), "m113": (270, 340, 610, 800), "jeep": (310, 400, 40, 260),
                  "soldier": (310, 440, 500, 600), "ground": (450, 640, 0, 960), "sky": (0, 200, 0, 960)},
    "sinai_lowsun_fp": {"left": (250, 420, 0, 320), "mid": (250, 420, 320, 640), "right": (250, 420, 640, 960),
                        "ground": (450, 640, 0, 960), "sky": (0, 200, 0, 960)},
    "noon_fields": {"jeep": (350, 448, 540, 708), "soldier": (375, 495, 285, 352),  # boxes fixed in the v7 resume (were on empty ground)
                    "ground": (300, 640, 0, 960),
                    "far": (150, 260, 600, 960), "sky": (0, 140, 0, 960)},
    "forest_edge_golden": {"crown": (90, 260, 160, 360), "trees_r": (40, 300, 600, 960), "slope": (280, 520, 300, 700),
                           "soldier": (300, 400, 520, 600), "sky": (0, 60, 0, 400)},
    "town_night": {"house_l": (120, 260, 60, 360), "tree": (0, 260, 380, 700), "ground": (350, 640, 0, 960),
                   "right": (100, 300, 700, 960), "sky": (0, 80, 0, 300)},
    # v7 (resume) night views, boxes read off the 960x640 captures
    "town_dark_fire": {"trunk": (200, 500, 450, 500), "soldier": (400, 495, 520, 575), "house_behind_smoke": (280, 440, 600, 740),
                       "house_l": (180, 420, 60, 380), "ground_near": (480, 640, 300, 900), "sky": (0, 60, 0, 300)},
    "headlights_bounce": {"m113": (230, 420, 0, 330), "jeep": (260, 400, 600, 830), "soldier": (300, 480, 370, 460),
                          "horizon": (262, 300, 330, 600), "ground_pool": (420, 560, 0, 400), "sky": (0, 200, 0, 960)},
    "town_golden": {"left": (100, 340, 0, 320), "mid": (100, 340, 320, 640), "right": (100, 340, 640, 960),
                    "ground": (400, 640, 0, 960), "sky": (0, 80, 0, 960)},
}


def main():
    run, view = Path(sys.argv[1]), sys.argv[2]
    shots = {}
    for f in glob.glob(str(run / "tri" / "**" / "*.png"), recursive=True):
        m = re.match(r"\d+_(.+)__(.+)\.png$", Path(f).name)
        if m and m.group(1) == view:
            shots[m.group(2)] = np.asarray(Image.open(f).convert("RGB")).astype(float)
    base = shots.get(sys.argv[3] if len(sys.argv) > 3 else "remastered")
    if base is None:
        print("no remastered shot for", view)
        return
    regs = REGIONS.get(view, {"all": (0, 640, 0, 960)})
    print("region means of Remastered:", " ".join(f"{n}:{base[y0:y1, x0:x1].mean():.1f}" for n, (y0, y1, x0, x1) in regs.items()))
    for label, img in sorted(shots.items()):
        if label == (sys.argv[3] if len(sys.argv) > 3 else "remastered") or "motion" in label:
            continue
        d = (img - base).mean(axis=2)
        frame = f"frame:{d.mean():+5.2f} p99:{np.percentile(d, 99):+5.1f} p1:{np.percentile(d, 1):+5.1f} >2:{(np.abs(d) > 2).mean() * 100:4.1f}%"
        print(f"{label:22s}", frame, "|", " ".join(f"{n}:{(img[y0:y1, x0:x1] - base[y0:y1, x0:x1]).mean():+6.2f}" for n, (y0, y1, x0, x1) in regs.items()))


if __name__ == "__main__":
    main()
