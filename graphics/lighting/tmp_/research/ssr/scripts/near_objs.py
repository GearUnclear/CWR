import sys, math, collections
ox, oz, hd, rng = float(sys.argv[2]), float(sys.argv[3]), float(sys.argv[4]), float(sys.argv[5])
cnt = collections.Counter(); front = []
for line in open(sys.argv[1], encoding='utf-8', errors='replace'):
    p = line.split()
    if len(p) < 5 or not p[0].isdigit(): continue
    try: x, y, z = float(p[-3]), float(p[-2]), float(p[-1])
    except: continue
    cls = ' '.join(p[1:-3])
    d = math.hypot(x - ox, z - oz)
    if d > rng: continue
    ang = math.degrees(math.atan2(x - ox, z - oz)) % 360
    rel = (ang - hd + 540) % 360 - 180
    if abs(rel) < 40:
        front.append((round(d), round(rel), cls, round(y, 1)))
    cnt[cls] += 1
front.sort()
for f in front[:40]: print(f)
print(cnt.most_common(15))
