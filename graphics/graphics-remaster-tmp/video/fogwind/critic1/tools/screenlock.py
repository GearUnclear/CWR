"""Screen-locked structure under the pan: what stays put on the screen while the scene slides by.

For 40-frame windows of the pan (the scene moves 35-42 px per window), the time mean in SCREEN
coordinates keeps whatever is fixed to the screen and smears the scene along x; an x high-pass
(sigma 4 px) then removes the smeared scene. PERSIST = rms(x-high-pass of the window mean) /
rms(x-high-pass of one frame): ~1 for a screen-locked pattern, small for scene detail.
Also writes 2x-gain images of the screen mean's x high-pass for looking at.
usage: python screenlock.py outdir label...
"""
import sys, os
import numpy as np
from scipy import ndimage
from PIL import Image
from common import load, Y0

BOX = {  # screen boxes (x0, x1, y0, y1), full-frame coords; generous, soldiers mostly outside on the right
    "haze": (1250, 1900, 470, 560),
    "lowsky": (1250, 1900, 410, 470),
    "bay": (1450, 1900, 630, 750),
    "farhill": (450, 950, 490, 560),
}
WINS = [(130, 170), (230, 270), (310, 350), (400, 440)]


def xhp(img, s=4.0):
    return img - ndimage.gaussian_filter1d(img, s, axis=1)


out = sys.argv[1]
os.makedirs(out, exist_ok=True)
labs = sys.argv[2:]
print("PERSIST and rms(x-hp of window mean) [codes] per box; windows", WINS)
imgs = {}
for lab in labs:
    a = load(lab)
    for (w0, w1) in WINS:
        M = a[w0:w1].astype(np.float32).mean(0)
        F = a[(w0 + w1) // 2].astype(np.float32)
        XM, XF = xhp(M), xhp(F)
        s = f"{lab:5s} {w0}-{w1}"
        for k, (x0, x1, y0, y1) in BOX.items():
            rm = XM[y0 - Y0:y1 - Y0, x0:x1].std()
            rf = XF[y0 - Y0:y1 - Y0, x0:x1].std()
            s += f" | {k} {rm:.3f} ({rm / rf:.2f})"
        print(s, flush=True)
        if (w0, w1) == (310, 350):
            imgs[lab] = XM
# a look sheet: the x-high-pass of the 310-350 screen mean, haze+bay crop x 1150-1900, y 410-760, gain 16
rows = []
for lab in labs:
    c = imgs[lab][410 - Y0:760 - Y0, 1150:1900] * 16 + 128
    rows.append(np.clip(c, 0, 255).astype(np.uint8))
sheet = np.concatenate(rows, 0)
Image.fromarray(sheet).save(f"{out}/screenmean_xhp_310_350_x16_{'_'.join(labs)}.png")
