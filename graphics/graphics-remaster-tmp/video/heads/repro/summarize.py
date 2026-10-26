#!/usr/bin/env python
"""One line per metric_<label>.json: per head, yaw flips (all frames) and head
snaps (pairs with head |dL| > 6.5, and > 5 for reference).
In --track mode on a moving camera, the head box re-snaps by 1 px now and then
(|dL| 5.0-5.4 on small heads), so snaps count from 6.5 there; on fixed-camera
repros both thresholds agree."""
import json, sys
for p in sys.argv[1:]:
    r = json.load(open(p))
    flips = [y["flips_all"] for y in r.get("yaw", [])]
    sn65 = [sum(1 for x in h["series_k_dx_dy_shade"] if x[3] > 6.5) for h in r["heads"]]
    sn5 = [h["jumps_shade_over_5"] for h in r["heads"]]
    print(f"{p.replace(chr(92), "/").split("/")[-2] + ":" + r["label"]:36s} frames={r['frames']:3d}  yaw flips {'/'.join(map(str, flips)):12s} snaps>6.5 {'/'.join(map(str, sn65)):12s} (>5: {'/'.join(map(str, sn5))})")
