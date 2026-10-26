"""Find camera spots where tree crowns stand between the eye and a low sun.

Reads `PoseidonTools terrain objects` output. For a grid of candidate eye
points, scores trees within 15-70 m whose bearing is within +-25 deg of the
sun azimuth, and whose crown (base + ~4..18 m) spans the sun elevation as
seen from the eye (eye ground height estimated from the nearest objects).
"""
import math
import sys

sys.path.insert(0, __file__.rsplit("\\", 1)[0].rsplit("/", 1)[0])
from objs import load  # noqa: E402

TREES = ("str", "ker", "jablon", "oliva", "les")


def is_tree(cls):
    c = cls.lower()
    return c.startswith(TREES) and "krovi" not in c


def main():
    path = sys.argv[1]
    sun_az = float(sys.argv[2])
    sun_el = float(sys.argv[3])
    rows = load(path)
    trees = [r for r in rows if is_tree(r[0])]
    allobj = rows
    xs = [r[1] for r in trees]
    zs = [r[3] for r in trees]
    best = []
    for gx in range(int(min(xs)), int(max(xs)), 10):
        for gz in range(int(min(zs)), int(max(zs)), 10):
            # ground height: nearest object base within 25 m
            near = sorted((math.hypot(o[1] - gx, o[3] - gz), o[2]) for o in allobj)
            if not near or near[0][0] < 6:  # do not stand inside a tree / bush
                continue
            g_est = [h for d, h in near[:4] if d < 30]
            if len(g_est) < 2:
                continue
            ground = sum(g_est) / len(g_est)
            eye = ground + 1.75
            score = 0.0
            hits = []
            for cls, x, y, z in trees:
                dx, dz = x - gx, z - gz
                d = math.hypot(dx, dz)
                if d < 15 or d > 70:
                    continue
                brg = (math.degrees(math.atan2(dx, dz)) + 360) % 360
                db = (brg - sun_az + 540) % 360 - 180
                if abs(db) > 25:
                    continue
                lo = math.degrees(math.atan2(y + 4 - eye, d))
                hi = math.degrees(math.atan2(y + 16 - eye, d))
                if lo < sun_el + 3 and hi > sun_el - 3:
                    w = 1.0 + (1.0 if abs(db) < 8 else 0.0)
                    score += w
                    hits.append((round(d), round(db), cls))
            if score >= 3:
                best.append((score, gx, gz, round(ground, 1), hits))
    best.sort(reverse=True)
    for s, gx, gz, g, hits in best[:25]:
        print(f"score {s:4.1f} eye ({gx}, {gz}) ground {g}: {hits[:6]}")


if __name__ == "__main__":
    main()
