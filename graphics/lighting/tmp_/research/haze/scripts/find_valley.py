"""Score camera spots for a 'looking down a valley' morning view."""
import numpy as np, sys, os, math
d = os.path.dirname(os.path.abspath(__file__))
world = sys.argv[1] if len(sys.argv) > 1 else "abel"
H = np.load(os.path.join(d, "..", "terrain", world + "_h.npy"))
n = H.shape[0]; cell = 50.0
def h(x, z):
    gx, gz = x / cell, z / cell
    if gx < 0 or gz < 0 or gx >= n - 1 or gz >= n - 1: return None
    x0, z0 = int(gx), int(gz); fx, fz = gx - x0, gz - z0
    return (H[z0, x0] * (1 - fx) * (1 - fz) + H[z0, x0 + 1] * fx * (1 - fz) + H[z0 + 1, x0] * (1 - fx) * fz + H[z0 + 1, x0 + 1] * fx * fz)
sun_az = 76.0
res = []
for gz in range(8, n - 8, 2):
    for gx in range(8, n - 8, 2):
        x, z = gx * cell, gz * cell
        h0 = h(x, z)
        if h0 is None or h0 < 60: continue
        eye = h0 + 1.75
        for hd in range(0, 360, 10):
            dx, dz = math.sin(math.radians(hd)), math.cos(math.radians(hd))
            maxang = -1e9; vis_low = 0; vis_far = 0; near_block = False; lows = []; ok = True
            for i, t in enumerate(np.arange(50, 4000, 50)):
                hh = h(x + dx * t, z + dz * t)
                if hh is None: ok = False; break
                hh = max(hh, 0.0)  # sea surface
                ang = math.atan2(hh - eye, t)
                if t < 300 and ang > math.radians(1.0): near_block = True
                if ang > maxang:
                    maxang = ang
                    if 400 <= t <= 2500 and hh < eye - 60 and hh > 2: vis_low += 1; lows.append(hh)
                    if t > 2000 and hh > 2: vis_far += 1
            if not ok or near_block: continue
            # valley: visible low ground, then far ground rising behind (ridge)
            if vis_low < 8 or vis_far < 4: continue
            rel = (hd - sun_az + 540) % 360 - 180
            side = abs(rel)
            sunscore = 1.0 if 20 <= side <= 70 else (0.6 if side < 20 else 0.3)
            depth = eye - min(lows)
            res.append((vis_low * sunscore + depth / 20.0, x, z, round(h0), hd, vis_low, vis_far, round(depth), rel))
res.sort(reverse=True)
seen = []
for r in res:
    if any(abs(r[1] - s[1]) < 600 and abs(r[2] - s[2]) < 600 for s in seen): continue
    seen.append(r)
    print(r)
    if len(seen) >= 15: break
