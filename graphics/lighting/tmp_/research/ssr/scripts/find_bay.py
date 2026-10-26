import numpy as np, math, sys
h = np.load(sys.argv[1])
cell = 50.0
N = h.shape[0]
def H(x, z):
    gx, gz = x / cell, z / cell
    if gx < 0 or gz < 0 or gx >= N - 1 or gz >= N - 1: return -50.0
    i, j = int(gx), int(gz); fx, fz = gx - i, gz - j
    return (h[j, i] * (1 - fx) * (1 - fz) + h[j, i + 1] * fx * (1 - fz) + h[j + 1, i] * (1 - fx) * fz + h[j + 1, i + 1] * fx * fz)
print('check coastline origin', H(1550, 5150))
res = []
for j in range(2, N - 2):
    for i in range(2, N - 2):
        x, z = i * cell, j * cell
        g = H(x, z)
        if not (1.0 < g < 12.0): continue
        eye = g + 2.0
        for hd in range(0, 360, 10):
            dx, dz = math.sin(math.radians(hd)), math.cos(math.radians(hd))
            # water must begin within 150 m
            d = 10.0; first = None
            while d < 150:
                if H(x + dx * d, z + dz * d) < -0.3: first = d; break
                d += 10
            if first is None: continue
            d = first; 
            while d < 4000 and H(x + dx * d, z + dz * d) < 0.0: d += 20
            if d >= 4000 or d < 400: continue
            span = d - first
            # far land elevation angle
            best = -90
            dd = d
            while dd < d + 3000:
                hh = H(x + dx * dd, z + dz * dd)
                ang = math.degrees(math.atan2(hh - eye, dd))
                best = max(best, ang)
                dd += 50
            if best > 1.0:
                res.append((best * min(span, 2000) / 1000.0, i * cell, j * cell, hd, round(g, 1), first, d, round(best, 2)))
res.sort(reverse=True)
for r in res[:40]: print(r)
