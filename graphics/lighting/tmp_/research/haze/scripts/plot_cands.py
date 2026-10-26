import numpy as np, math, os, sys
from PIL import Image, ImageDraw
d = os.path.dirname(os.path.abspath(__file__))
H = np.load(os.path.join(d, "..", "terrain", "abel_h.npy"))
n = H.shape[0]
gy, gx = np.gradient(H, 50.0)
shade = np.clip(0.6 + 0.4 * (-gx * 0.7 + gy * 0.7) / np.sqrt(1 + gx**2 + gy**2) * 3, 0, 1)
t = np.clip((H + 0) / 520.0, 0, 1)
rgb = np.stack([0.3 + 0.6 * t, 0.5 + 0.3 * t, 0.3 + 0.2 * t], -1) * shade[..., None]
rgb[H <= 0] = [0.15, 0.25, 0.5]
img = Image.fromarray((rgb[::-1] * 255).astype(np.uint8)).resize((1024, 1024), Image.NEAREST)
dr = ImageDraw.Draw(img)
S = 1024 / (n * 50.0)
cands = [(6100, 5000, 50), (5500, 9100, 100), (5600, 7500, 110), (6200, 6000, 130), (4700, 5200, 140), (6700, 4800, 30), (3000, 5500, 140)]
for i, (x, z, hd) in enumerate(cands):
    px, pz = x * S, 1024 - z * S
    for a in (hd - 20, hd + 20):
        dr.line([px, pz, px + 3000 * S * math.sin(math.radians(a)), pz - 3000 * S * math.cos(math.radians(a))], fill=(255, 255, 0), width=1)
    dr.text((px + 3, pz + 3), str(i), fill=(255, 0, 0))
img.save(os.path.join(d, "..", "terrain", "abel_cands.png"))
# profiles
for i, (x, z, hd) in enumerate(cands):
    prof = []
    for tt in range(0, 4001, 250):
        xx, zz = x + tt * math.sin(math.radians(hd)), z + tt * math.cos(math.radians(hd))
        prof.append(int(H[int(round(zz / 50)), int(round(xx / 50))]))
    print(i, (x, z, hd), prof)
