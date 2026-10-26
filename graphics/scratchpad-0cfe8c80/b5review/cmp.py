import sys, os
import numpy as np
from PIL import Image

root = r"D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\B5"
final = os.path.join(root, "20261018-075851final-new")
before = os.path.join(root, "20261018-073656-before")
before1 = os.path.join(root, "20261018-071820-before")
views = ["water_coastline_sunset", "water_sea_noon", "water_shore_midsun", "water_night"]


def load(p):
    return np.asarray(Image.open(p).convert("RGB")).astype(np.int16)


def over(a, b, t=2):
    d = np.abs(a - b).max(axis=2)
    return int((d > t).sum()), float(d.mean()), int(d.max())


for v in views:
    fo = load(os.path.join(final, v + "__off.png"))
    bo = load(os.path.join(before, v + "__off.png"))
    bn = load(os.path.join(before, v + "__on.png"))
    fc = load(os.path.join(final, v + "__classic.png"))
    bc = load(os.path.join(before, v + "__classic.png"))
    print(v)
    print("  final off vs before off   ", over(fo, bo))
    print("  before on vs before off   ", over(bn, bo), "(base build, same game, switch inert)")
    p1 = os.path.join(before1, v + "__off.png")
    if os.path.exists(p1):
        print("  before1 off vs before off ", over(load(p1), bo), "(base build boot repeat)")
    print("  final classic vs before cl", over(fc, bc))
    # water-region colour for on vs classic (below the horizon row found by
    # the largest vertical gradient in the classic shot's middle column)
    fn = load(os.path.join(final, v + "__on.png"))
    h = fo.shape[0]
    for name, img in (("classic", fc), ("off", fo), ("on", fn)):
        lo = img[int(h * 0.75):, :, :].reshape(-1, 3).mean(axis=0)
        print("   near band mean %-8s" % name, np.round(lo, 1))
