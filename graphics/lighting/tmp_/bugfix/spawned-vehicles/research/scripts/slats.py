import sys
from PIL import Image
import numpy as np
b, a = sys.argv[1], sys.argv[2]
def m(path, box):
    x = np.asarray(Image.open(path).convert("RGB")).astype(float)[box[1]:box[3], box[0]:box[2]]
    return x.reshape(-1, 3).mean(axis=0)
boxes = {"slats (shaded side)": (600, 170, 860, 290), "rear door (sunlit)": (480, 190, 555, 280)}
alb = f"{a}/sinai_hull_close__debug.png"
for name, box in boxes.items():
    A = m(alb, box)
    print(f"{name}: albedo RGB {A.round(1)}  r:g:b norm {np.round(A / A.max(), 3)}")
    for label, p in [("BEFORE spawned remastered (CPU path)", f"{b}/sinai_hull_close__remastered.png"),
                     ("AFTER spawned remastered (per-pixel)", f"{a}/sinai_hull_close__remastered.png"),
                     ("AFTER placed remastered", f"{a}/placed_sinai_hull_close__remastered.png"),
                     ("BEFORE spawned classic", f"{b}/sinai_hull_close__classic.png"),
                     ("AFTER spawned classic", f"{a}/sinai_hull_close__classic.png")]:
        S = m(p, box)
        L = S / A
        print(f"   {label}: shot RGB {S.round(1)}  light=shot/albedo {L.round(3)}  b/r {L[2]/L[0]:.2f}  g/r {L[1]/L[0]:.2f}")
