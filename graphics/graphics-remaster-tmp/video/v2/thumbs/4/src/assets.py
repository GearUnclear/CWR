"""Poster assets for thumbnail designer 4: lettering masks and the fighter cutout.
Reads the owner's poster PNG (read only) and writes derived PNGs into ../assets."""
import os
import numpy as np
from PIL import Image
from scipy import ndimage as ndi

POSTER = r'D:/Arma_CWA/ChatGPT Image Oct 5, 2026, 09_49_52 PM.png'
HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, '..', 'assets')
os.makedirs(OUT, exist_ok=True)

im = Image.open(POSTER).convert('RGB')
a = np.array(im).astype(np.int32)
r, g, b = a[..., 0], a[..., 1], a[..., 2]
lum = 0.299 * r + 0.587 * g + 0.114 * b

# ---------- lettering ----------
X0, X1, Y0, Y1 = 27, 483, 39, 295
red = (r - g > 60) & (r > 130)
L = red[Y0:Y1, X0:X1].copy()
# fill speckles: small enclosed non-red holes
holes = ~L
lab, n = ndi.label(holes)
sizes = ndi.sum(holes, lab, range(1, n + 1))
border_labels = set(np.unique(np.concatenate([lab[0], lab[-1], lab[:, 0], lab[:, -1]])))
for i, s in enumerate(sizes, start=1):
    if s <= 20 and i not in border_labels:
        L[lab == i] = True
ys, xs = np.where(L)
L = L[ys.min():ys.max() + 1, xs.min():xs.max() + 1]
ox = X0 + xs.min()
Image.fromarray((L * 255).astype(np.uint8)).save(os.path.join(OUT, 'letter_full.png'))
# word split at the clean gap x 253..258 (poster coords)
cut = 253 - ox
uslu = L[:, :cut]
dur = L[:, cut + 6:]
def trim(m):
    ys, xs = np.where(m)
    return m[:, xs.min():xs.max() + 1]
Image.fromarray((trim(uslu) * 255).astype(np.uint8)).save(os.path.join(OUT, 'letter_uslu.png'))
Image.fromarray((trim(dur) * 255).astype(np.uint8)).save(os.path.join(OUT, 'letter_dur.png'))
gap = np.zeros((L.shape[0], 34), bool)
words = np.concatenate([trim(uslu), gap, trim(dur)], 1)
Image.fromarray((words * 255).astype(np.uint8)).save(os.path.join(OUT, 'letter_words.png'))
print('lettering', L.shape, trim(uslu).shape, trim(dur).shape)

# ---------- fighter ----------
FX0, FY0, FX1, FY1 = 60, 290, 262, 640
sub = a[FY0:FY1, FX0:FX1]
sl = lum[FY0:FY1, FX0:FX1]
sr, sg, sb = sub[..., 0], sub[..., 1], sub[..., 2]
dark = sl < 70
light_bg = (sl > 128) | ((sr - sb) > 90)   # cream, orange sun, lit buildings
passable = (((sr - sb) >= 55) & (sl > 105)) | (sl > 190)
# hand polygon (poster coords) that loosely hugs the figure
from PIL import ImageDraw
poly = [(140, 296), (190, 296), (196, 330), (206, 350), (232, 356), (232, 370),
        (240, 400), (238, 445), (228, 470), (226, 520), (222, 540), (222, 560), (223, 600), (230, 630), (233, 640),
        (100, 640), (100, 600), (106, 560), (108, 500), (90, 480), (92, 440), (96, 380),
        (118, 360), (130, 345), (126, 330)]
pm = Image.new('L', (FX1 - FX0, FY1 - FY0), 0)
ImageDraw.Draw(pm).polygon([(x - FX0, y - FY0) for x, y in poly], fill=255)
inside = np.array(pm) > 0
# seeds: everything outside polygon, plus light background on the bottom row
seed = ~inside
seed[-1, :] |= passable[-1, :]
bg = seed.copy()
cand = passable & inside
# region grow: background = connected to seed through passable pixels
grow_mask = cand | seed
lab, n = ndi.label(grow_mask)
seed_labels = np.unique(lab[seed])
bg = np.isin(lab, seed_labels[seed_labels > 0])
fig = inside & ~bg
fig = ndi.binary_fill_holes(fig)
# drop specks
lab, n = ndi.label(fig)
sizes = ndi.sum(fig, lab, range(1, n + 1))
keep = np.zeros_like(fig)
big = np.argmax(sizes) + 1
keep[lab == big] = True
warm = ((sr - sb) >= 55) & (sl > 105)
for _ in range(2):
    edge = keep & ~ndi.binary_erosion(keep)
    keep = keep & ~(edge & warm)
rgba = np.zeros((FY1 - FY0, FX1 - FX0, 4), np.uint8)
rgba[..., :3] = sub.astype(np.uint8)
rgba[..., 3] = keep * 255
# one poster pixel of outline ink around the figure, as pixel-art sprites have
ring = ndi.binary_dilation(keep, structure=np.ones((3, 3), bool)) & ~keep
ring[-1, :] = False
rgba[ring, :3] = (48, 40, 40)
rgba[ring, 3] = 255
Image.fromarray(rgba).save(os.path.join(OUT, 'fighter_raw.png'))
print('fighter', keep.sum())
