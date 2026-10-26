"""Find camera spots on Abel with casters (olive trees or houses) toward a low sun.

find_spot2.py <objects dump> <sun azimuth deg> <kind: oliva|house>
Scores points on a 10 m grid: casters inside a +-20 deg wedge toward the sun
at 12-80 m, nothing within 6 m (the camera must stand in the open), and for
houses at least 3 different buildings in the wedge so there are gaps between
them for beams.
"""
import math
import sys
from collections import defaultdict

path, az, kind = sys.argv[1], float(sys.argv[2]), sys.argv[3]
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
HOUSES = ('deutshe', 'hruzdum', 'cihlovej_dum', 'dum01', 'kaple', 'zvonice', 'ryb_domek', 'dum_rasovna', 'plechbud')
if kind == 'oliva':
    cast = [o for o in objs if 'oliva' in o[0]]
else:
    cast = [o for o in objs if any(h in o[0] for h in HOUSES)]
solid = [o for o in objs if not any(k in o[0] for k in ('silnice', 'asfaltka', 'asfatlka', 'cesta', 'kr_'))]
cell = 50.0


def grid(lst):
    g = defaultdict(list)
    for o in lst:
        g[(int(o[1] // cell), int(o[3] // cell))].append(o)
    return g


gc, gs = grid(cast), grid(solid)


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


seen = set()
res = []
for c in cast:
    for dx in range(-80, 81, 10):
        for dz in range(-80, 81, 10):
            x, z = round((c[1] + dx) / 10) * 10, round((c[3] + dz) / 10) * 10
            if (x, z) in seen:
                continue
            seen.add((x, z))
            if near(gs, x, z, 6):
                continue
            wedge = []
            behind = 0
            for o in near(gc, x, z, 80):
                d = math.hypot(o[1] - x, o[3] - z)
                if d < 12:
                    continue
                b = math.degrees(math.atan2(o[1] - x, o[3] - z)) % 360
                diff = abs((b - az + 180) % 360 - 180)
                if diff < 20:
                    wedge.append(o)
                elif diff > 120:
                    behind += 1
            if kind == 'house' and len(wedge) < 3:
                continue
            if len(wedge) < 4:
                continue
            ys = [o[2] for o in near(gs, x, z, 40)]
            rough = (max(ys) - min(ys)) if ys else 0
            res.append((len(wedge) - 0.5 * behind - 0.2 * rough, len(wedge), behind, x, z, rough))
res.sort(reverse=True)
for r in res[:20]:
    print('score %.1f wedge %d behind %d  x %d z %d  elev-range %.1f' % r)
