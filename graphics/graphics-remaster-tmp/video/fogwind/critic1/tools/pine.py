"""pine_shafts moving probe: NEW (production) vs ORIGINAL (approved look) in the shafts and the air.
D2 = rms of the per-pixel temporal second difference I(t+1) - 2 I(t) + I(t-1) after a 2 px blur (smooth
motion of smooth light cancels; flicker, stepping and crawl do not), per box. LATTICE = share of the screen
time mean's high-pass power at the froxel cell periods (10-14 px, either axis), a screen-locked cell grid.
Also writes a look sheet: 2x crops of the left shafts at frames 60-63 for both, and their D2 maps x16.
usage: python pine.py outdir new.mkv orig.mkv
"""
import sys, os
import numpy as np
from scipy import ndimage
from PIL import Image, ImageDraw
sys.path.insert(0, os.path.dirname(__file__))
from cache import decode

out, pnew, porig = sys.argv[1:4]
BOX = {"shafts_left": (600, 860, 420, 660), "shaft_right": (1050, 1300, 380, 470), "skyglow": (880, 1100, 200, 380),
       "air_far": (1300, 1520, 420, 540), "ground": (300, 900, 820, 1000)}
clips = {"new": decode(pnew, 0, 1080), "orig": decode(porig, 0, 1080)}
for lab, a in clips.items():
    a = a.astype(np.float32)
    clips[lab] = a
print("box: D2 new / orig | lattice share new / orig | time-std (after 9-frame high-pass) new / orig")
for k, (x0, x1, y0, y1) in BOX.items():
    r = {}
    for lab, a in clips.items():
        S = np.stack([ndimage.gaussian_filter(f[y0:y1, x0:x1], 1.0) for f in a[5:]])
        d2 = S[2:] - 2 * S[1:-1] + S[:-2]
        hp = S - ndimage.uniform_filter1d(S, 9, axis=0, mode="nearest")
        M = a[5:, y0:y1, x0:x1].mean(0)
        H = M - ndimage.gaussian_filter(M, 6.0)
        F = np.abs(np.fft.fft2((H - H.mean()) * np.outer(np.hanning(H.shape[0]), np.hanning(H.shape[1])))) ** 2
        fy = np.abs(np.fft.fftfreq(H.shape[0]))[:, None] + 0 * F
        fx = np.abs(np.fft.fftfreq(H.shape[1]))[None, :] + 0 * F
        lat = (((fx > 1 / 14) & (fx < 1 / 10) & (fy < 0.02)) | ((fy > 1 / 14) & (fy < 1 / 10) & (fx < 0.02)))
        band = (np.hypot(fx, fy) > 1 / 30)
        r[lab] = (d2.std(), F[lat].sum() / F[band].sum(), hp[4:-4].std())
    print(f"  {k:12s} D2 {r['new'][0]:.3f} / {r['orig'][0]:.3f} | lattice {r['new'][1]:.3f} / {r['orig'][1]:.3f} | hp-std {r['new'][2]:.3f} / {r['orig'][2]:.3f}")
# look sheet
x0, x1, y0, y1 = 560, 880, 400, 640
tiles = []
for lab, a in clips.items():
    for t in (60, 61, 62, 63):
        im = Image.fromarray(np.clip(a[t, y0:y1, x0:x1], 0, 255).astype(np.uint8)).resize(((x1 - x0) * 2, (y1 - y0) * 2), Image.LANCZOS)
        ImageDraw.Draw(im).text((4, 4), f"{lab} f{t}", fill=255)
        tiles.append(im)
    for t in (60, 61, 62, 63):
        d2 = a[t + 1, y0:y1, x0:x1] - 2 * a[t, y0:y1, x0:x1] + a[t - 1, y0:y1, x0:x1]
        im = Image.fromarray(np.clip(d2 * 16 + 128, 0, 255).astype(np.uint8)).resize(((x1 - x0) * 2, (y1 - y0) * 2), Image.NEAREST)
        ImageDraw.Draw(im).text((4, 4), f"{lab} d2 f{t} x16", fill=255)
        tiles.append(im)
W, H = (x1 - x0) * 2, (y1 - y0) * 2
sheet = Image.new("L", (W * 4, H * 4))
for i, im in enumerate(tiles):
    sheet.paste(im, ((i % 4) * W, (i // 4) * H))
sheet.save(f"{out}/pine_shafts_new_vs_orig.png")
print("sheet", sheet.size)
