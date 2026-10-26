import sys
from PIL import Image
import numpy as np
R = sys.argv[1]
def load(p):
    return np.asarray(Image.open(p).convert("RGB")).astype(float)
def lum(a):
    return a @ [0.2126, 0.7152, 0.0722]
pit = (650, 440, 730, 465)
for label, run, pre in [("BEFORE spawned", "before", ""), ("AFTER spawned", "after", ""), ("AFTER placed", "after", "placed_"), ("BEFORE placed", "before", "placed_")]:
    r = load(f"{R}/{run}/{pre}town_night__remastered.png")
    d = load(f"{R}/{run}/{pre}town_night__debug.png")
    c = load(f"{R}/{run}/{pre}town_night__classic.png")
    pr = r[pit[1]:pit[3], pit[0]:pit[2]].reshape(-1, 3).mean(0)
    pd = d[pit[1]:pit[3], pit[0]:pit[2]].reshape(-1, 3).mean(0)
    print(f"{label}: frame luma remastered {lum(r).mean():.1f}, classic {lum(c).mean():.1f}, lower half remastered {lum(r[320:]).mean():.1f}; pit remastered RGB {pr.round(1)}, pit debug RGB {pd.round(1)}")
