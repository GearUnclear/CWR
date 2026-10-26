"""Shade against lit where the canopy shades: pixels the debug view (sun
visibility, debug=1) shows lit with the switch off (>= 0.85) and shaded with it
on (<= 0.35); per channel the mean display value on / off at those very pixels
(same texture, same pixel: the shade factor of the canopy), and the share of
the frame. Usage: shade.py <tri dir> <view> [x0 y0 x1 y1]"""
import sys
from pathlib import Path
import numpy as np
from PIL import Image


def find(d, label):
    hits = sorted(Path(d).rglob(f"*_{label}.png"))
    return np.asarray(Image.open(hits[-1]).convert("RGB")).astype(np.float64)


d, v = sys.argv[1], sys.argv[2]
off, on = find(d, v + "__off"), find(d, v + "__on")
doff, don = find(d, v + "__dbgoff")[..., 1] / 255.0, find(d, v + "__dbgon")[..., 1] / 255.0
box = tuple(int(x) for x in sys.argv[3:7]) if len(sys.argv) >= 7 else (0, 0, off.shape[1], off.shape[0])
x0, y0, x1, y1 = box
m = np.zeros(doff.shape, bool)
m[y0:y1, x0:x1] = True
m &= (doff >= 0.85) & (don <= 0.35)
n = int(m.sum())
if n == 0:
    print(v, "no newly shaded pixels")
    sys.exit()
ratio = [on[..., c][m].mean() / off[..., c][m].mean() for c in range(3)]
lum = lambda a: a[..., 0] * 0.2126 + a[..., 1] * 0.7152 + a[..., 2] * 0.0722
print(f"{v}: newly shaded px={n} ({n / m.size * 100:.2f}% of frame) shade/lit per channel "
      f"R={ratio[0]:.3f} G={ratio[1]:.3f} B={ratio[2]:.3f} luma={lum(on)[m].mean() / lum(off)[m].mean():.3f}; "
      f"B/R shade {on[..., 2][m].mean() / on[..., 0][m].mean():.3f} lit {off[..., 2][m].mean() / off[..., 0][m].mean():.3f}")
