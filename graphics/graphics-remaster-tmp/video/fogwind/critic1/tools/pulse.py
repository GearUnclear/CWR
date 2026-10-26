"""Whole-box brightness pulses: scene-tracked box means over the clip, temporal high-pass (minus a
15-frame centred mean), their rms, the largest single excursions, and the dominant period.
Boxes are small (60x30 px .. 200x60 px) so a local pulse is not averaged away.
usage: python pulse.py label...
"""
import sys
import numpy as np
from scipy import ndimage
from common import load, Y0, SCRATCH

BOXES = {  # at frame 328: x0, x1, y0, y1
    "shore_sun": (1700, 1800, 650, 680),
    "farshore": (1500, 1700, 560, 590),
    "bay_mid": (1550, 1750, 680, 730),
    "farvalley": (1160, 1280, 530, 580),
    "valleymist": (1150, 1240, 610, 680),
    "haze_hi": (1400, 1600, 480, 520),
    "farhill": (600, 800, 505, 545),
    "lowmist": (1450, 1600, 780, 820),
}


def track(name="bay"):
    tr = np.load(f"{SCRATCH}/camtrack.npy", allow_pickle=True).item()
    return np.cumsum(tr["reg"][name], 0)


TR = {"shore_sun": "bay", "farshore": "farvalley", "bay_mid": "bay", "farvalley": "farvalley",
      "valleymist": "valley", "haze_hi": "farvalley", "farhill": "farhill", "lowmist": "lowmist"}

labs = sys.argv[1:]
print("box: per clip rms(HP box mean) | max |excursion| at frame | dominant period (frames) and its share of HP power")
for name, (x0, x1, y0, y1) in BOXES.items():
    c = track(TR[name])
    print(f"--- {name}")
    for lab in labs:
        a = load(lab)
        T = a.shape[0]
        vals = np.full(T, np.nan)
        for t in range(T):
            dx = c[t, 1] - c[328, 1]
            dy = c[t, 0] - c[328, 0]
            X0, X1 = x0 + dx, x1 + dx
            if X0 < 2 or X1 > 1917:
                continue
            Yi0 = y0 + dy - Y0
            # fractional shift: sample with map_coordinates over the box grid
            yy, xx = np.mgrid[Yi0:Yi0 + (y1 - y0), X0:X0 + (x1 - x0)]
            vals[t] = ndimage.map_coordinates(a[t].astype(np.float32), [yy.ravel(), xx.ravel()], order=1).mean()
        ok = ~np.isnan(vals)
        idx = np.where(ok)[0]
        v = vals[idx]
        hp = v - ndimage.uniform_filter1d(v, 15, mode="nearest")
        sel = (idx >= 30) & (idx <= 560)
        h = hp[sel]
        if len(h) < 64:
            print(f"  {lab:5s} (box off screen)")
            continue
        im = int(np.argmax(np.abs(h)))
        F = np.abs(np.fft.rfft((h - h.mean()) * np.hanning(len(h)))) ** 2
        f = np.fft.rfftfreq(len(h))
        m = (f > 1 / 60) & (f < 0.5)
        k = np.argmax(np.where(m, F, 0))
        print(f"  {lab:5s} rms {h.std():.3f} | max {abs(h[im]):.3f} at f{idx[sel][im]} | period {1 / f[k]:.1f} share {F[k] / F[m].sum():.2f}  (frames {idx[sel][0]}-{idx[sel][-1]})")
        np.save(f"{SCRATCH}/pulse_{lab}_{name}.npy", vals)
