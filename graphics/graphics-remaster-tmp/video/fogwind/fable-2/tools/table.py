"""table.py [prefix ...]: one line per clip from out/*.json (fable-2 fogflow results). Valley first, then the max over regions."""
import glob, json, os, sys
O = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "out")
pref = sys.argv[1:] or [""]
rows = []
for f in sorted(glob.glob(os.path.join(O, "*.json"))):
    d = json.load(open(f))
    if not any(d["label"].startswith(p) for p in pref):
        continue
    R = d["regions"]
    v = R["valley"]
    reg = ["bay", "farhill", "haze"] if "rt60" in d["label"] else ["bay", "valley", "farhill", "haze"]   # rt60: the walking soldier is in the valley box
    tr = max(R[r]["travel"] for r in reg)
    trr = max(reg, key=lambda r: R[r]["travel"])
    low = max(R[r]["low"] for r in reg)
    wind = max(R[r]["wind"] for r in reg)
    slow = max(R[r]["slow"] for r in reg)
    ok = tr <= 0.05 and low <= 0.08 and wind <= 0.012
    rows.append((d["label"], d["frames"], d["cam_x"], v["mid"], v["low"], v["slow"], v["vx1"], v["vy1"], v["p1"], v["travel"], v["wind"],
                 tr, trr, R[trr]["vx1"], R[trr]["vy1"], low, slow, wind, "PASS" if ok else "FAIL"))
print("%-16s %3s %5s | %-40s | %s" % ("clip", "n", "cam", "VALLEY  MID   LOW   SLOW   v1(x,y)    p1  TRAVEL WIND", "MAX over bay/valley/farhill/haze: TRAVEL(region v1)  LOW   SLOW   WIND  verdict"))
for r in rows:
    print("%-16s %3d %+5.2f | %.3f %.3f %.3f %+5.1f %+5.1f %+.2f %+.2f %.3f | %+.2f (%-7s %+5.1f %+5.1f) %.3f %.3f %.3f  %s"
          % (r[0], r[1], r[2], r[3], r[4], r[5], r[6], r[7], r[8], r[9], r[10], r[11], r[12], r[13], r[14], r[15], r[16], r[17], r[18]))
