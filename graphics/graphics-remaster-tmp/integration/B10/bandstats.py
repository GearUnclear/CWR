"""Per view: mean display luminance of the sky, horizon and ground bands, off
against on, and the sky's cloud contrast (standard deviation of the luminance
and of a 6-pixel high-pass in the sky band).

    python bandstats.py <run dir> [--md out.md]
"""
import argparse
from pathlib import Path

import numpy as np
from PIL import Image

ap = argparse.ArgumentParser()
ap.add_argument("run")
ap.add_argument("--md", default="")
ap.add_argument("--a", default="off")
ap.add_argument("--b", default="on")
a = ap.parse_args()

BANDS = (("sky", 0, 422), ("horizon", 422, 591), ("ground", 591, 1080))


def lum(path):
    img = np.asarray(Image.open(path).convert("RGB")).astype(np.float64) / 255.0
    return img, img @ np.array([0.2126, 0.7152, 0.0722])


def highpass_std(l):
    k = 6
    pad = np.pad(l, k, mode="edge")
    c = np.cumsum(np.cumsum(pad, 0), 1)
    c = np.pad(c, ((1, 0), (1, 0)))
    n = 2 * k + 1
    box = (c[n:, n:] - c[:-n, n:] - c[n:, :-n] + c[:-n, :-n]) / (n * n)
    return float((l - box).std())


rows = []
run = Path(a.run)
for off in sorted(run.glob(f"*__{a.a}.png")):
    view = off.name[: -len(f"__{a.a}.png")]
    on = run / f"{view}__{a.b}.png"
    if not on.exists():
        continue
    io, lo = lum(off)
    i1, l1 = lum(on)
    cells = [view]
    for name, r0, r1 in BANDS:
        m0, m1 = lo[r0:r1].mean(), l1[r0:r1].mean()
        cells.append(f"{m0:.3f} / {m1:.3f} ({(m1 / max(m0, 1e-6) - 1) * 100:+.1f}%)")
    s0, s1 = lo[0:422].std(), l1[0:422].std()
    h0, h1 = highpass_std(lo[0:422]), highpass_std(l1[0:422])
    cells.append(f"{s0:.4f} / {s1:.4f}")
    cells.append(f"{h0:.4f} / {h1:.4f}")
    # Mean saturation of the sky band (max - min over max).
    def sat(img):
        mx, mn = img.max(axis=2), img.min(axis=2)
        return float(((mx - mn) / np.maximum(mx, 1e-3))[0:422].mean())
    cells.append(f"{sat(io):.3f} / {sat(i1):.3f}")
    rows.append(cells)

head = ["view", "sky lum off / on", "horizon lum", "ground lum", "sky std", "sky high-pass std", "sky saturation"]
lines = ["| " + " | ".join(head) + " |", "|" + "---|" * len(head)]
lines += ["| " + " | ".join(r) + " |" for r in rows]
text = "\n".join(lines)
print(text)
if a.md:
    Path(a.md).write_text(text + "\n", encoding="utf-8")
