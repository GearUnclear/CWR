"""Find a dark forest-road spot on Abel for the headlight test.

Road segment with trees on both sides within 6-30 m, no building within 120 m,
no street lamp within 400 m. Prints candidates with the road heading
(from the nearest other road segments).
"""
import math
import sys
from collections import defaultdict

path = sys.argv[1]
objs = []
with open(path, encoding='utf-8', errors='replace') as f:
    for line in f:
        parts = line.split()
        if len(parts) < 5 or not parts[0].isdigit():
            continue
        try:
            x, y, z = float(parts[-3]), float(parts[-2]), float(parts[-1])
        except ValueError:
            continue
        objs.append((' '.join(parts[1:-3]).lower(), x, y, z))
ROAD = ('silnice25', 'asfaltka25', 'cesta25')
roads = [o for o in objs if any(r in o[0] for r in ROAD)]
allroads = [o for o in objs if any(r in o[0] for r in ('silnice', 'asfaltka', 'cesta'))]
lamps = [o for o in objs if 'lampa' in o[0]]
BIG = ('borovice', 'osika', 'habr', 'buk', 'briza', 'pinie', 'les_', 'vrba', 'dub', 'smrk', 'jablon', 'kastan')
trees = [o for o in objs if any(b in o[0] for b in BIG)]
bld = [o for o in objs if not any(k in o[0] for k in ('\str', '\les', '\ker', 'krovi', 'silnice', 'asfaltka', 'cesta', 'kr_', 'plot', 'zed', 'pole', 'kamen', 'skala'))]
cell = 50.0
def grid(lst):
    g = defaultdict(list)
    for o in lst:
        g[(int(o[1] // cell), int(o[3] // cell))].append(o)
    return g
gt, gb, gr = grid(trees), grid(bld), grid(allroads)
def near(g, x, z, r):
    out = []
    n = int(r // cell) + 1
    cx, cz = int(x // cell), int(z // cell)
    for i in range(cx - n, cx + n + 1):
        for j in range(cz - n, cz + n + 1):
            for o in g.get((i, j), []):
                if (o[1] - x) ** 2 + (o[3] - z) ** 2 <= r * r:
                    out.append(o)
    return out
res = []
for c, x, y, z in roads:
    if any((l[1] - x) ** 2 + (l[3] - z) ** 2 < 400 ** 2 for l in lamps):
        continue
    if near(gb, x, z, 120):
        continue
    nb = [o for o in near(gr, x, z, 40) if 5 < math.hypot(o[1] - x, o[3] - z)]
    if len(nb) < 2:
        continue
    # heading from the two farthest-apart neighbours
    best = max(((a, b) for a in nb for b in nb), key=lambda p: math.hypot(p[0][1] - p[1][1], p[0][3] - p[1][3]))
    hdg = math.degrees(math.atan2(best[1][1] - best[0][1], best[1][3] - best[0][3])) % 180
    fx, fz = math.sin(math.radians(hdg)), math.cos(math.radians(hdg))
    left = right = 0
    for t in near(gt, x, z, 30):
        dx, dz = t[1] - x, t[3] - z
        d = math.hypot(dx, dz)
        if d < 6:
            continue
        side = dx * fz - dz * fx
        if side > 0:
            right += 1
        else:
            left += 1
    if left >= 3 and right >= 3:
        res.append((min(left, right), left + right, x, z, y, hdg, c))
res.sort(reverse=True)
for r in res[:25]:
    print('minside %d total %d  x %.0f z %.0f elev %.1f road_hdg %.0f  %s' % r)
