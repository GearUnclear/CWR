"""Frame means, the fire pool's red over blue and the plume region, per still.

    python levels.py <run dir> [<run dir> ...]
"""
import sys
from pathlib import Path

import numpy as np
from PIL import Image


def load(p):
    return np.asarray(Image.open(p).convert("RGB")).astype(np.float64)


def lum(a):
    return 0.2126 * a[..., 0] + 0.7152 * a[..., 1] + 0.0722 * a[..., 2]


def rb(a):
    crop = a[560:700, 150:700]
    m = lum(crop) > 25
    if m.sum() < 50:
        return float("nan")
    return crop[..., 0][m].mean() / max(crop[..., 2][m].mean(), 1e-6)


def main():
    views = ["combat_fires", "combat_eye_k00", "combat_eye_k05", "combat_eye_k06", "combat_eye_k08", "combat_eye_k11",
             "combat_shell_k05", "combat_shell_k06", "combat_shell_k08", "combat_shell_k11", "combat_shell_k16",
             "combat_flare_k90", "combat_flare_k240", "combat_muzzle_k04", "combat_muzzle_k05", "combat_day_k00",
             "combat_day_k05", "combat_day_k06"]
    for run in sys.argv[1:]:
        run = Path(run)
        print(f"## {run.name}")
        print("| view | setting | frame mean | pool R/B (rows 560-700, cols 150-700, lum > 25) | plume (rows 60-300, cols 1250-1500) | sky (rows 0-60) |")
        print("|---|---|---|---|---|---|")
        for v in views:
            for s in ("classic", "off", "on", "smoke0", "scatter0"):
                p = run / f"{v}__{s}.png"
                if not p.exists():
                    continue
                a = load(p)
                print(f"| {v} | {s} | {lum(a).mean():.1f} | {rb(a):.2f} | {lum(a[60:300, 1250:1500]).mean():.1f} | "
                      f"{lum(a[0:60]).mean():.1f} |")


if __name__ == "__main__":
    main()
