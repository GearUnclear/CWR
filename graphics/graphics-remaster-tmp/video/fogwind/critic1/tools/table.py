"""Print critic1 motion tables. usage: python table.py out_dir [labels]"""
import sys, json
import numpy as np

d = sys.argv[1]
labs = sys.argv[2:] or ["orig", "f1", "new", "van"]
R = {l: json.load(open(f"{d}/motion_{l}.json"))[l] for l in labs}
regs = list(R[labs[0]].keys())
print("Per region, whole clip (all 30-frame windows): mean MID / FINE / SHEET / B60, mean C0, max TRAV (lag), max PEAK2-C0 at lag2 (lag), mean |SUBPX| x")
for r in regs:
    print(f"--- {r}")
    for l in labs:
        rows = R[l][r]
        if not rows:
            continue
        mid = np.mean([x["MID"] for x in rows]); fine = np.mean([x["FINE"] for x in rows])
        sh = np.mean([x["SHEET"] for x in rows]); b60 = np.mean([x["B60"] for x in rows if x["B60"] is not None])
        c0 = np.mean([x["C0"] for x in rows])
        it = int(np.argmax([x["TRAV"] for x in rows])); tr = rows[it]["TRAV"]; tl = rows[it]["PEAKLAG"]
        t2 = [x["PEAK2"] - max(x["C0"], 0) for x in rows]; i2 = int(np.argmax(t2))
        trm = np.mean([x["TRAV"] for x in rows])
        sx = np.mean([x["SUBPX"][0] for x in rows])
        print(f"  {l:5s} n={len(rows):2d} MID {mid:.3f} FINE {fine:.3f} SHEET {sh:.3f} B60 {b60:.3f} | C0 {c0:+.3f} TRAV mean {trm:.3f} max {tr:.3f} at {tl} (w {rows[it]['w'][0]}) | lag2 max {t2[i2]:+.3f} at {rows[i2]['PEAK2LAG']} | subpx x {sx:+.2f}")
print()
print("Per window, TRAV (peak lag) per clip, region by region")
for r in regs:
    print(f"--- {r}")
    ws = sorted({tuple(x["w"]) for l in labs for x in R[l][r]})
    for w in ws:
        s = f"  {w[0]:3d}-{w[1]:3d}"
        for l in labs:
            x = [y for y in R[l][r] if tuple(y["w"]) == w]
            if x:
                x = x[0]
                s += f" | {l} MID {x['MID']:.3f} B60 {x['B60']:.3f} T {x['TRAV']:.2f}@{x['PEAKLAG'][0]:+d},{x['PEAKLAG'][1]:+d}"
        print(s)
