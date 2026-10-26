"""Off/on statistics per view: pixels over 2 and 8 levels, mean luma per band
(rows 0-421 sky, 422-590 horizon, 591-1079 ground), R/G and B/G of the ground
band, and the same between two runs. Usage: stats.py <dirA> <dirB|-> [labels]"""
import sys
from pathlib import Path
import numpy as np
from PIL import Image

BANDS = {"sky": (0, 422), "horizon": (422, 591), "ground": (591, 1080)}


def load(p):
    return np.asarray(Image.open(p).convert("RGB")).astype(np.float64)


def luma(a):
    return a[..., 0] * 0.2126 + a[..., 1] * 0.7152 + a[..., 2] * 0.0722


def compare(a, b):
    d = np.abs(a - b).max(axis=2)
    out = {"px>2": int((d > 2).sum()), "px>8": int((d > 8).sum()), "max": int(d.max())}
    for n, (y0, y1) in BANDS.items():
        la, lb = luma(a[y0:y1]).mean(), luma(b[y0:y1]).mean()
        out[n] = f"{la:.2f}->{lb:.2f} ({(lb - la) / max(la, 1e-6) * 100:+.2f}%)"
    g0, g1 = a[591:], b[591:]
    out["ground R/G"] = f"{g0[..., 0].mean() / g0[..., 1].mean():.3f}->{g1[..., 0].mean() / g1[..., 1].mean():.3f}"
    out["ground B/G"] = f"{g0[..., 2].mean() / g0[..., 1].mean():.3f}->{g1[..., 2].mean() / g1[..., 1].mean():.3f}"
    return out


def find(d, label):
    hits = sorted(Path(d).rglob(f"*_{label}.png"))
    return hits[-1] if hits else None


if __name__ == "__main__":
    da, db = sys.argv[1], sys.argv[2]
    labels = sys.argv[3].split(",")
    for lab in labels:
        if db == "-":
            pa, pb = find(da, lab + "__off"), find(da, lab + "__on")
            title = f"{lab}: off -> on ({Path(da).name})"
        else:
            suffix = sys.argv[4] if len(sys.argv) > 4 else "off"
            pa, pb = find(da, lab + "__" + suffix), find(db, lab + "__" + suffix)
            title = f"{lab}__{suffix}: {Path(da).name} -> {Path(db).name}"
        if not pa or not pb:
            print(title, "missing")
            continue
        print(title, compare(load(pa), load(pb)))
