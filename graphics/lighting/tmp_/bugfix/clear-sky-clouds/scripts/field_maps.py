"""Top-down maps of the shipped cloud-shadow field (BuildCloudNoise(256, 1),
6 km tile) over a 12 km square, at the covers the rig hands the shader before
and after the clear-sky fix. CPU reconstruction, not an in-game capture."""
import sys
import numpy as np
from PIL import Image, ImageDraw
from noise_scale import build

def S(e0, e1, x):
    t = min(1, max(0, (x - e0) / (e1 - e0))); return t * t * (3 - 2 * t)
def ss(e0, e1, x):
    t = np.clip((x - e0) / (e1 - e0), 0, 1); return t * t * (3 - 2 * t)

tex = build(256, 1).astype(np.float64) / 255.0
# 12 km at 25 m per pixel = 480 px; the 6 km tile repeats twice each way.
N = 480
coords = (np.arange(N) + 0.5) * 25.0 / 6000.0 * 256 - 0.5
x0 = np.floor(coords).astype(int); t = coords - x0
def bil(img):
    a = img[np.ix_(x0 % 256, x0 % 256)]; b = img[np.ix_(x0 % 256, (x0 + 1) % 256)]
    c = img[np.ix_((x0 + 1) % 256, x0 % 256)]; d = img[np.ix_((x0 + 1) % 256, (x0 + 1) % 256)]
    tx = t[None, :]; ty = t[:, None]
    return (a + (b - a) * tx) * (1 - ty) + (c + (d - c) * tx) * ty
field = bil(tex)

def panel(cover, label):
    if cover <= 0:
        vis = np.ones_like(field)
    else:
        e = 0.08 * min(1, max(0, 4 * cover * (1 - cover)))
        cov = ss(1 - cover - e, 1 - cover + e, field)
        vis = 1 - cov * 0.8
    img = Image.fromarray((vis * 255).astype(np.uint8), "L").convert("RGB")
    canvas = Image.new("RGB", (N, N + 26), (20, 20, 20))
    canvas.paste(img, (0, 26))
    ImageDraw.Draw(canvas).text((6, 6), label, fill=(235, 235, 235))
    return canvas, (1 - vis).mean() / 0.8

panels = [
    (0.0414, "overcast 0, shipped: cover 0.041"),
    (0.0, "overcast 0, fixed: cover 0"),
    (0.0931 * S(0.05, 0.1, 0.0931), "overcast 0.1, fixed: cover 0.088"),
    (0.30, "overcast 0.5, both: cover 0.30"),
]
out = Image.new("RGB", (len(panels) * (N + 8), N + 26 + 22), (20, 20, 20))
for i, (c, lab) in enumerate(panels):
    p, frac = panel(c, lab)
    out.paste(p, (i * (N + 8), 0))
    print(f"{lab}: shadowed share {frac:.3f}")
ImageDraw.Draw(out).text((6, N + 30), "Cloud-shadow sun visibility over a 12 x 12 km square (white = sun, grey = 0.2 under a cloud). CPU reconstruction of the shipped field, not an in-game capture.", fill=(200, 200, 200))
out.save(sys.argv[1])
