import sys, os
from PIL import Image
import numpy as np
B = r"D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\B2"
def load(d, v, k): return np.asarray(Image.open(os.path.join(B, d, f"{v}__{k}.png")).convert("RGB")).astype(int)
def luma(a): return (a*[0.2126,0.7152,0.0722]).sum(2)
for v in ("lowsun_fp", "sinai_lowsun_fp", "lowsun_box"):
    bo = load("before", v, "off"); bn = load("before", v, "on"); ao = load("after", v, "off"); an = load("after", v, "on")
    for name, x, y in (("before off vs before on (noise)", bo, bn), ("after off vs before off", ao, bo), ("after off vs after on", ao, an)):
        d = np.abs(x-y).max(2)
        out = []
        for lo, hi in ((0, 360), (360, 600), (600, 1080)):
            dd = d[lo:hi]; out.append("%d/%d/mean %.2f" % ((dd>2).sum(), (dd>8).sum(), luma(x[lo:hi]).mean()-luma(y[lo:hi]).mean()))
        print(v, name, " | ".join(out))
