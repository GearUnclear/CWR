"""critic1 travel table from the stored correlation surfaces (lags y -6..6, x -14..14).
FAR = best correlation at |lag| >= 3 px minus max(0, best within |lag| <= 1.5) (consecutive residuals);
FAR2 = the same for residuals two frames apart at |lag| >= 4 vs <= 2.5. A coherent travel of v px/frame
shows FAR at v and FAR2 at 2v (if 2v is in range); screen-locked detail sits near lag +1 (inside the core).
usage: python table2.py outdir [labels]
"""
import sys, json
import numpy as np

d = sys.argv[1]
labs = sys.argv[2:] or ["orig", "f1", "new", "van"]
R = {l: json.load(open(f"{d}/motion_{l}.json"))[l] for l in labs}
ly, lx = np.mgrid[-6:7, -14:15]
r = np.hypot(ly, lx)


def far(S, rin, rout):
    S = np.array(S)
    core = S[r <= rin].max()
    i = np.argmax(np.where(r >= rout, S, -9))
    return S.flat[i] - max(core, 0.0), (int(lx.flat[i]), int(ly.flat[i])), core


regs = list(R[labs[0]].keys())
print("region: per clip mean MID/FINE/SHEET/B60 | FAR mean, max (lag, window) | FAR2 max (lag) | core C mean")
summary = {}
for reg in regs:
    print(f"--- {reg}")
    for l in labs:
        rows = R[l][reg]
        fs = [far(x["Csurf"], 1.5, 3) for x in rows]
        f2 = [far(x["C2surf"], 2.5, 4) for x in rows]
        i = int(np.argmax([f[0] for f in fs]))
        j = int(np.argmax([f[0] for f in f2]))
        mid = np.mean([x["MID"] for x in rows]); fine = np.mean([x["FINE"] for x in rows])
        sh = np.mean([x["SHEET"] for x in rows]); b60 = np.mean([x["B60"] for x in rows])
        summary[(reg, l)] = dict(mid=mid, fine=fine, sheet=sh, b60=b60, farmean=np.mean([f[0] for f in fs]), farmax=fs[i][0])
        print(f"  {l:5s} MID {mid:.3f} FINE {fine:.3f} SHEET {sh:.3f} B60 {b60:.3f} | FAR mean {np.mean([f[0] for f in fs]):.3f} max {fs[i][0]:.3f} at {fs[i][1]} (w{rows[i]['w'][0]}) | FAR2 max {f2[j][0]:.3f} at {f2[j][1]} (w{rows[j]['w'][0]}) | core {np.mean([f[2] for f in fs]):+.3f}")
print()
print("Ratios NEW / VANILLA (MID, FINE, B60) and NEW / ORIGINAL B60 per region")
for reg in regs:
    n, v, o = summary[(reg, "new")], summary[(reg, "van")], summary[(reg, "orig")]
    print(f"  {reg:9s} MID {n['mid'] / v['mid']:.2f}  FINE {n['fine'] / v['fine']:.2f}  B60 {n['b60'] / v['b60']:.2f}  | B60 new/orig {n['b60'] / o['b60']:.3f}  FARmax new {n['farmax']:.3f} van {v['farmax']:.3f} f1 {summary[(reg, 'f1')]['farmax']:.3f}")
