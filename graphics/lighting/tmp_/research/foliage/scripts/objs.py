"""Summarise / filter `PoseidonTools terrain objects` output (research helper)."""
import collections
import math
import sys


def load(path):
    rows = []
    for line in open(path):
        p = line.split()
        if len(p) < 5 or not p[0].isdigit():
            continue
        try:
            x, y, z = float(p[-3]), float(p[-2]), float(p[-1])
        except ValueError:
            continue
        cls = " ".join(p[1:-3]).split("\\")[-1]
        rows.append((cls, x, y, z))
    return rows


def is_tree(cls):
    c = cls.lower()
    return c.startswith("str") or c.startswith("ker") or c.startswith("les")


if __name__ == "__main__":
    rows = load(sys.argv[1])
    if len(sys.argv) == 2:
        c = collections.Counter(r[0] for r in rows)
        for k, v in c.most_common(40):
            print(v, k)
    else:
        # objs.py file ox oz heading: list trees with forward/right/distance/bearing
        ox, oz, h = float(sys.argv[2]), float(sys.argv[3]), float(sys.argv[4])
        hr = math.radians(h)
        out = []
        for cls, x, y, z in rows:
            if not is_tree(cls):
                continue
            dx, dz = x - ox, z - oz
            fwd = dx * math.sin(hr) + dz * math.cos(hr)
            right = dx * math.cos(hr) - dz * math.sin(hr)
            dist = math.hypot(dx, dz)
            bearing = (math.degrees(math.atan2(dx, dz)) + 360) % 360
            out.append((dist, fwd, right, bearing, cls, x, y, z))
        out.sort()
        for d, f, r, b, cls, x, y, z in out[:60]:
            print(f"{d:7.1f} fwd {f:7.1f} right {r:7.1f} brg {b:6.1f}  {cls:28s} {x:8.1f} {y:6.1f} {z:8.1f}")
