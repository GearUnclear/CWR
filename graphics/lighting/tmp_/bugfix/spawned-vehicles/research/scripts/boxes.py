import sys
from PIL import Image
import numpy as np
R = sys.argv[1]
def m(p, box):
    a = np.asarray(Image.open(p).convert("RGB")).astype(float)[box[1]:box[3], box[0]:box[2]].reshape(-1, 3)
    return a.mean(0)
cases = [("lowsun_fp", "Jeep body", (60, 330, 250, 395)), ("lowsun_fp", "M113 side (shade)", (660, 275, 800, 330)),
         ("noon_fields", "Jeep", (550, 380, 700, 440)), ("sinai_lowsun_fp", "HMMWV side", (10, 270, 290, 330)),
         ("sinai_lowsun_fp", "Zelda slats", (640, 260, 830, 320))]
for view, name, box in cases:
    out = []
    for label, run, pre in [("before", "before", ""), ("after", "after", ""), ("placed", "after", "placed_")]:
        for mode in ("remastered", "classic"):
            v = m(f"{R}/{run}/{pre}{view}__{mode}.png", box)
            out.append(f"{label}/{mode} luma {v @ [0.2126, 0.7152, 0.0722]:.1f} rgb {v.round(1).tolist()}")
    print(f"{view} {name} {box}:")
    for o in out:
        print("   ", o)
