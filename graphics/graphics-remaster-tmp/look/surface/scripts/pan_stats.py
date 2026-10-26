"""Frame-to-frame change during a camera pan (gen_pan.py captures).

  python pan_stats.py <run dir> [--md out.md] [--png dir]

For each view and mode: <view>__<mode>_a / _b, two frames a few frames apart
while the camera turns. The second is aligned to the first by the pan's
shift (phase correlation with a sub-pixel peak, scipy shift, bilinear); the
mean absolute 8-bit luma change over the overlap is then the flicker plus the
resampling blur, which is the same for every mode of a view. Regions: the
whole overlap, the bottom third (near ground), the top half (crowns, roofs).
With --png, writes |b - a| x 8 for each pair.
"""
import argparse
import re
from pathlib import Path

import numpy as np
from PIL import Image
from scipy import ndimage

PAT = re.compile(r"^\d+_(?P<view>.+?)__(?P<mode>.+)_(?P<ab>[ab])\.png$")


def luma(p):
    a = np.asarray(Image.open(p).convert("RGB")).astype(np.float64)
    return a[..., 0] * 0.2126 + a[..., 1] * 0.7152 + a[..., 2] * 0.0722


def shift_of(a, b):
    """Sub-pixel (dy, dx) that moves b onto a."""
    wy = np.hanning(a.shape[0])[:, None]
    wx = np.hanning(a.shape[1])[None, :]
    A = np.fft.fft2((a - a.mean()) * wy * wx)
    B = np.fft.fft2((b - b.mean()) * wy * wx)
    R = A * np.conj(B)
    R /= np.maximum(np.abs(R), 1e-9)
    r = np.fft.ifft2(R).real
    iy, ix = np.unravel_index(np.argmax(r), r.shape)

    def sub(c, m1, p1):
        d = m1 - 2 * c + p1
        return 0.0 if abs(d) < 1e-12 else 0.5 * (m1 - p1) / d

    h, w = r.shape
    dy = iy + sub(r[iy, ix], r[(iy - 1) % h, ix], r[(iy + 1) % h, ix])
    dx = ix + sub(r[iy, ix], r[iy, (ix - 1) % w], r[iy, (ix + 1) % w])
    if dy > h / 2:
        dy -= h
    if dx > w / 2:
        dx -= w
    return dy, dx


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("--md")
    ap.add_argument("--png")
    a = ap.parse_args()
    found = {}
    for p in Path(a.run).rglob("*.png"):
        m = PAT.match(p.name)
        if m:
            found[(m.group("view"), m.group("mode"), m.group("ab"))] = p
    keys = sorted({(v, m) for (v, m, _ab) in found})
    out = ["| view | mode | shift (px) | mean abs d, overlap | bottom third | top half |", "|---|---|---|---|---|---|"]
    for v, m in keys:
        if (v, m, "a") not in found or (v, m, "b") not in found:
            continue
        A = luma(found[(v, m, "a")])
        B = luma(found[(v, m, "b")])
        dy, dx = shift_of(A, B)
        Bs = ndimage.shift(B, (dy, dx), order=1, mode="nearest")
        pad = int(np.ceil(max(abs(dx), abs(dy)))) + 2
        d = np.abs(A - Bs)[pad:-pad, pad:-pad]
        h = d.shape[0]
        out.append(f"| {v} | {m} | ({dx:+.2f}, {dy:+.2f}) | {d.mean():.2f} | {d[2 * h // 3:].mean():.2f} | {d[: h // 2].mean():.2f} |")
        if a.png:
            Path(a.png).mkdir(parents=True, exist_ok=True)
            Image.fromarray(np.clip(d * 8, 0, 255).astype(np.uint8)).save(Path(a.png) / f"pan_{v}__{m}.png")
    text = "\n".join(out)
    print(text)
    if a.md:
        Path(a.md).write_text(text + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
