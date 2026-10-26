"""Find eye points where deciduous crowns stand between the camera and a low sun.

    python find_backlit2.py abel_objs.txt <sun_az> <sun_el>

Faster rewrite of find_backlit.py with a KD tree. Ground height at a point is
estimated from the bases of the nearest objects (object Y = base ASL).
Scores trees 12-55 m away whose bearing is within +-18 deg of the sun azimuth
and whose crown (base + 5..16 m) spans the sun elevation as seen from the eye.
Deciduous trees (birch, aspen, beech, hornbeam, apple, hazel) count double.
"""
import math
import sys

import numpy as np
from scipy.spatial import cKDTree

sys.path.insert(0, __file__.rsplit("\\", 1)[0].rsplit("/", 1)[0])
from objs import load  # noqa: E402

DECID = ("str briza", "str osika", "str buk", "str habr", "str_jablon", "jablon", "str_briza_rovna", "str lipa", "str dub")


def main():
    rows = load(sys.argv[1])
    sun_az, sun_el = float(sys.argv[2]), float(sys.argv[3])
    allxy = np.array([[r[1], r[3]] for r in rows])
    allh = np.array([r[2] for r in rows])
    tall = cKDTree(allxy)
    trees = [r for r in rows if r[0].lower().startswith(("str", "jablon")) and "krovisko" not in r[0].lower()]
    txy = np.array([[r[1], r[3]] for r in trees])
    th = np.array([r[2] for r in trees])
    tdec = np.array([1.0 if r[0].lower().replace(".p3d", "") in DECID else 0.0 for r in trees])
    ttree = cKDTree(txy)
    # candidate eyes: around deciduous trees only
    cands = set()
    for i in np.nonzero(tdec)[0]:
        x, z = txy[i]
        # eye is on the anti-sun side of the tree, 15-45 m away
        for d in range(15, 46, 5):
            for off in (-10, 0, 10):
                a = math.radians(sun_az + 180 + off)
                cands.add((round((x + d * math.sin(a)) / 5) * 5, round((z + d * math.cos(a)) / 5) * 5))
    out = []
    saz = math.radians(sun_az)
    for gx, gz in cands:
        dn, idx = tall.query([gx, gz], k=6)
        if dn[0] < 5:
            continue
        near = [allh[j] for dd, j in zip(dn, idx) if dd < 30]
        if len(near) < 3:
            continue
        ground = float(np.median(near))
        if max(near) - min(near) > 6:
            continue  # steep or uneven
        eye = ground + 1.75
        score = 0.0
        hits = []
        for j in ttree.query_ball_point([gx, gz], 55):
            x, z = txy[j]
            dx, dz = x - gx, z - gz
            d = math.hypot(dx, dz)
            if d < 12:
                continue
            brg = (math.degrees(math.atan2(dx, dz)) + 360) % 360
            db = (brg - sun_az + 540) % 360 - 180
            if abs(db) > 18:
                continue
            lo = math.degrees(math.atan2(th[j] + 5 - eye, d))
            hi = math.degrees(math.atan2(th[j] + 16 - eye, d))
            if lo < sun_el + 2 and hi > sun_el - 2:
                w = (2.0 if tdec[j] else 1.0) * (1.5 if abs(db) < 6 else 1.0)
                score += w
                hits.append((round(d), round(db), trees[j][0].replace(".p3d", ""), round(th[j] - ground, 1)))
        # the ground toward the sun beyond the trees should not rise much
        far = [allh[j] for j in tall.query_ball_point([gx + 120 * math.sin(saz), gz + 120 * math.cos(saz)], 25)]
        if far and np.median(far) - ground > 120 * math.tan(math.radians(sun_el)) - 5:
            continue
        if score >= 4 and any(h[2].startswith(("str briza", "str osika", "str buk", "str habr", "str_jablon")) for h in hits):
            out.append((score, gx, gz, round(ground, 1), hits))
    out.sort(reverse=True)
    for s, gx, gz, g, hits in out[:30]:
        print(f"score {s:4.1f} eye ({gx}, {gz}) ground {g}: {hits[:6]}")


if __name__ == "__main__":
    main()
