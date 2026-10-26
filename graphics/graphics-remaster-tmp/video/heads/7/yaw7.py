"""Agent 7 yaw proxy: python yaw7.py <mkv or tri dir> <start> <count> <hx>,<hy>,<hs> <tx>,<ty>,<ts>

The head box rides on the torso track (heads7.best_shift on the torso), so camera
motion cancels and a head turning in place stays in the box. Per frame:
  face  share of skin pixels in the head box (r > g + 8, r > b + 18, r > 50):
        a head facing the camera shows face, turned away shows helmet/hair
  fx    skin centroid x relative to the box centre, in box half sizes (-1..1):
        the face sliding left or right = yaw left or right
Prints the series summary: yaw steps (|d fx| > 0.15 or |d face| > 0.08 in one
frame), sign flips of d fx among those steps over each 60 frames, and the
largest one-frame step.
"""
import sys

import numpy as np

import heads7


def main():
    src, start, count = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
    hx, hy, hs = [int(v) for v in sys.argv[4].split(",")]
    tx, ty, ts = [int(v) for v in sys.argv[5].split(",")]
    m = 2 * 6 + 4 + min(300, 3 * count)
    x0, y0 = max(0, min(hx - hs, tx - ts) - m), max(0, min(hy - hs, ty - ts) - m)
    x1, y1 = min(1920, max(hx + hs, tx + ts) + m), min(1080, max(hy + hs, ty + ts) + m)
    rgb = heads7.read_frames(src, x0, y0, x1 - x0, y1 - y0, start, count).astype(np.int16)
    grey = rgb.astype(np.float32).mean(axis=3)
    ox, oy = hx - tx, hy - ty
    cx, cy = tx - x0, ty - y0
    face, fx = [], []
    for k in range(len(rgb)):
        if k:
            dx, dy, _ = heads7.best_shift(grey[k - 1], grey[k], cx, cy, ts)
            cx += dx
            cy += dy
        h = rgb[k, cy + oy - hs:cy + oy + hs, cx + ox - hs:cx + ox + hs]
        r, g, b = h[..., 0], h[..., 1], h[..., 2]
        sk = (r > g + 8) & (r > b + 18) & (r > 50)
        face.append(float(sk.mean()))
        if sk.sum() >= 4:
            xs = np.nonzero(sk)[1]
            fx.append(float((xs.mean() - hs) / hs))
        else:
            fx.append(float("nan"))
    face, fx = np.array(face), np.array(fx)
    dfx = np.diff(fx)
    dface = np.diff(face)
    step = (np.abs(np.nan_to_num(dfx)) > 0.15) | (np.abs(dface) > 0.08) | (np.isnan(fx[1:]) != np.isnan(fx[:-1]))
    out = []
    for w in range(0, len(dfx), 60):
        seg = slice(w, w + 60)
        s = step[seg]
        signed = np.sign(np.where(np.isnan(dfx[seg]), np.sign(dface[seg]), dfx[seg]))[s]
        flips = int(np.sum(signed[1:] != signed[:-1])) if len(signed) > 1 else 0
        out.append(f"[{w}-{min(w + 60, len(dfx))}) steps={int(s.sum())} dirflips={flips}")
    print(f"frames={len(face)} face mean={face.mean():.3f} range={face.min():.3f}..{face.max():.3f} "
          f"maxStepFace={np.abs(dface).max():.3f} maxStepFx={np.nanmax(np.abs(dfx)) if np.any(~np.isnan(dfx)) else 0:.2f} "
          f"steps={int(step.sum())} | " + " ".join(out))
    print("face%:", " ".join(f"{v * 100:.0f}" for v in face))


if __name__ == "__main__":
    main()
