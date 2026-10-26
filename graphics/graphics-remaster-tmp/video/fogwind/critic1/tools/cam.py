"""Per-frame camera shift of a clip, measured on several boxes (parallax check).

usage: python cam.py label [label...]
Saves <scratch>/<label>_cam.npy: (T, nbox, 2) consecutive shifts d_t (frame t -> t+1), d_0 = 0.
"""
import sys
import numpy as np
from common import load, crop, phase_shift, SCRATCH
from concurrent.futures import ThreadPoolExecutor

BOXES = {
    "nearhill": (250, 900, 470, 700),
    "farridge": (400, 1150, 460, 560),
    "shore":    (1380, 1900, 600, 780),
    "rightlow": (1500, 1900, 760, 840),
}

for lab in sys.argv[1:]:
    a = load(lab)
    T = a.shape[0]
    res = np.zeros((T, len(BOXES), 2), np.float32)

    def one(t):
        out = []
        for box in BOXES.values():
            out.append(phase_shift(crop(a[t], box), crop(a[t + 1], box)))
        return t, out

    with ThreadPoolExecutor(14) as ex:
        for t, out in ex.map(one, range(T - 1)):
            res[t + 1] = out
    np.save(f"{SCRATCH}/{lab}_cam.npy", res)
    names = list(BOXES)
    for w0 in range(0, T, 60):
        seg = res[w0 + 1:w0 + 61]
        s = "  ".join(f"{n} {seg[:, i, 1].mean():+.3f},{seg[:, i, 0].mean():+.3f}" for i, n in enumerate(names))
        print(f"{lab} frames {w0:3d}-{min(w0 + 60, T):3d} dx,dy px/frame  {s}")
