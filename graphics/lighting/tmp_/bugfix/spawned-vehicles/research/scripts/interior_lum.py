import sys
from PIL import Image
import numpy as np
d = sys.argv[1]
def lum(n, box=None):
    a = np.asarray(Image.open(f"{d}/{n}.png").convert("RGB")).astype(float)
    if box: a = a[box[1]:box[3], box[0]:box[2]]
    return (a @ [0.2126, 0.7152, 0.0722]).mean()
box = (230, 150, 830, 500)  # interior, away from HUD panels
for n in ["tank_gunner_interior__classic", "tank_gunner_interior__remastered", "placed_tank_gunner_interior__classic", "placed_tank_gunner_interior__remastered"]:
    print(n, round(lum(n), 1), round(lum(n, box), 1))
