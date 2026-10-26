import sys, glob, os
import numpy as np
from PIL import Image
E = r"D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\B10"
a = sorted(glob.glob(E + r"\final2\tri\b10_sweep\*.png"))
for pa in a:
    pb = pa.replace("final2", "final3")
    if not os.path.exists(pb):
        print("missing", pb); continue
    A = np.asarray(Image.open(pa).convert("RGB")).astype(int)
    B = np.asarray(Image.open(pb).convert("RGB")).astype(int)
    d = np.abs(A - B).max(axis=2)
    h = d.shape[0]
    sky = d[: h // 3]; gnd = d[h // 2 :]
    print(os.path.basename(pa), "max", d.max(), ">2", int((d > 2).sum()), ">8", int((d > 8).sum()), "sky>2", int((sky > 2).sum()), "gnd>8", int((gnd > 8).sum()))
