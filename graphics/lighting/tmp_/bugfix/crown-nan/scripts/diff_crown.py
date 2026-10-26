"""Signed luma change AFTER - BEFORE, x8: red = brighter, blue = darker, over a dim copy of the frame."""
import sys
import numpy as np
from PIL import Image, ImageDraw
r = sys.argv[1].rstrip("/") + "/"
for v, i in [("forest_edge_golden", 0), ("woods_mixed_noon", 2), ("woods_pine2_evening", 4), ("town_day", 6)]:
    b = np.asarray(Image.open(f"{r}run_before/tri/crown_nan_abel/{i:03d}_{v}__remastered.png").convert("RGB")).astype(np.float32)
    a = np.asarray(Image.open(f"{r}run_after/tri/crown_nan_abel/{i:03d}_{v}__remastered.png").convert("RGB")).astype(np.float32)
    w = np.array([0.2126, 0.7152, 0.0722], np.float32)
    d = (a @ w) - (b @ w)
    base = (b @ w)[..., None] * 0.25
    out = np.repeat(base, 3, axis=2)
    pos = np.clip(d * 8.0, 0, 255)
    neg = np.clip(-d * 8.0, 0, 255)
    out[..., 0] += pos
    out[..., 2] += neg
    out[..., 1] += 0.0
    img = Image.fromarray(np.clip(out, 0, 255).astype(np.uint8))
    dr = ImageDraw.Draw(img)
    t = "AFTER - BEFORE luma x8: red brighter, blue darker"
    dr.rectangle([0, 0, 8 + 7 * len(t), 18], fill=(0, 0, 0))
    dr.text((4, 3), t, fill=(255, 255, 255))
    img.save(f"{r}{v}__diff_x8.png")
    print(v, "brighter px(>2):", int((d > 2).sum()), "darker px(<-2):", int((d < -2).sum()), "mean d:", round(float(d.mean()), 3))
