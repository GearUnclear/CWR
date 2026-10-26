"""Find camera spots on Abel with a clump of trees between the camera and a low sun.

Reads the `PoseidonTools terrain objects` dump. Scores grid points: trees in a
wedge toward the sun azimuth at 15-90 m, nothing within 10 m, ground roughly
level (judged from nearby object elevations), few trees behind the camera.
"""
import math
import sys
from collections import defaultdict

path, az = sys.argv[1], float(sys.argv[2])
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
        cls = ' '.join(parts[1:-3]).lower()
        objs.append((cls, x, y, z))

BIG = ('borovice', 'osika', 'habr', 'buk', 'briza', 'pinie', 'les_', 'vrba', 'dub')
trees = [(x, y, z) for c, x, y, z in objs if any(b in c for b in BIG)]
other = [(x, y, z) for c, x, y, z in objs if not ('\\str' in c or '\\les' in c or '\\ker' in c or 'krovi' in c)]
print('trees', len(trees), 'other', len(other), file=sys.stderr)
cell = 50.0
grid = defaultdict(list)
for t in trees:
    grid[(int(t[0] // cell), int(t[2] // cell))].append(t)
ogrid = defaultdict(list)
for t in other:
    ogrid[(int(t[0] // cell), int(t[2] // cell))].append(t)


def near(g, x, z, r):
    out = []
    cx, cz = int(x // cell), int(z // cell)
    k = int(r // cell) + 1
    for i in range(cx - k, cx + k + 1):
        for j in range(cz - k, cz + k + 1):
            for t in g.get((i, j), []):
                if (t[0] - x) ** 2 + (t[2] - z) ** 2 <= r * r:
                    out.append(t)
    return out


sx, sz = math.sin(math.radians(az)), math.cos(math.radians(az))
res = []
for x in range(200, 12600, 15):
    for z in range(200, 12600, 15):
        nb = near(grid, x, z, 95)
        if not nb:
            continue
        close = [t for t in nb if (t[0] - x) ** 2 + (t[2] - z) ** 2 < 12 ** 2]
        if close:
            continue
        if near(ogrid, x, z, 8):
            continue
        wedge = 0
        behind = 0
        ys = []
        for t in nb:
            dx, dz = t[0] - x, t[2] - z
            d = math.hypot(dx, dz)
            ang = math.degrees(math.acos(max(-1, min(1, (dx * sx + dz * sz) / d))))
            if 15 <= d <= 90 and ang < 18:
                wedge += 1
            if ang > 120 and d < 60:
                behind += 1
            if d < 40:
                ys.append(t[1])
        if wedge < 8:
            continue
        spread = (max(ys) - min(ys)) if ys else 99
        if spread > 12:
            continue
        res.append((wedge - 0.5 * behind - spread, wedge, behind, spread, x, z, sum(ys) / len(ys) if ys else 0))
res.sort(reverse=True)
for r in res[:25]:
    print('score %.1f wedge %d behind %d spread %.1f  at %d %d  y~%.0f' % r)
