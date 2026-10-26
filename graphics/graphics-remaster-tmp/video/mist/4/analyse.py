"""Agent 4: is the mist-band flicker an exposure (global gain) effect or tied to the sun?

usage: python analyse.py <ud480.npy> [cam.csv] [--from a --to b]
Block grid as bandmetric.py (15 px blocks on 480x270, 18 x 32).
"""
import argparse, csv
import numpy as np

ap = argparse.ArgumentParser()
ap.add_argument("npy")
ap.add_argument("cam", nargs="?", default="")
ap.add_argument("--from", dest="a", type=int, default=0)
ap.add_argument("--to", dest="b", type=int, default=None)
o = ap.parse_args()

x = np.load(o.npy)[o.a:o.b].astype(np.float32)
T, H, W, _ = x.shape
Y = 0.299 * x[..., 0] + 0.587 * x[..., 1] + 0.114 * x[..., 2]
B = 15
gy, gx = H // B, W // B
blk = Y[:, :gy * B, :gx * B].reshape(T, gy, B, gx, B).mean(axis=(2, 4))
d2 = blk[1:-1] - 0.5 * (blk[:-2] + blk[2:])  # frames 1..T-2


def reg(r0, r1, c0, c1):
    return (slice(r0, r1 + 1), slice(c0, c1 + 1))


R = dict(band=reg(8, 13, 5, 31), sky=reg(0, 5, 0, 31), rock=reg(15, 17, 2, 12), hillL=reg(9, 12, 0, 4),
         skyNearSun=reg(1, 5, 22, 31), farhaze=reg(7, 9, 15, 31), bay=reg(10, 12, 22, 31))
print("frames", T)
for n, s in R.items():
    rms = np.sqrt((d2[(slice(None),) + s] ** 2).mean())
    lvl = blk[(slice(None),) + s].mean()
    print("%-11s level %6.1f  block d2 rms %.3f" % (n, lvl, rms))

# Common mode: a global gain would move every block the same sign. Per frame,
# the signed mean d2 of each region (spatially averaged), then correlations.
cm = {n: d2[(slice(None),) + s].mean(axis=(1, 2)) for n, s in R.items()}
print("\nregion-mean signed d2 rms (coherent part) and correlation with band's:")
for n in R:
    c = np.corrcoef(cm["band"], cm[n])[0, 1]
    print("  %-11s coherent rms %.3f  corr(band) %+.2f" % (n, np.sqrt((cm[n] ** 2).mean()), c))
# Coherence within the band: coherent rms / block rms
bd = d2[(slice(None),) + R["band"]]
print("band coherence: coherent %.3f of block rms %.3f" % (np.sqrt((cm["band"] ** 2).mean()), np.sqrt((bd ** 2).mean())))

# Relative (log) d2: exposure is multiplicative, so compare log changes.
lb = np.log2(np.maximum(blk, 1.0))
ld2 = lb[1:-1] - 0.5 * (lb[:-2] + lb[2:])
lcm = {n: ld2[(slice(None),) + s].mean(axis=(1, 2)) for n, s in R.items()}
print("\nlog2 coherent d2 rms (x1000):", {n: round(float(np.sqrt((v ** 2).mean()) * 1000), 2) for n, v in lcm.items()})

# Sun position per frame: centroid of near-white pixels in the upper 60%.
up = Y[:, : int(H * 0.6)]
sun = []
for t in range(T):
    m = up[t] >= 250
    if m.sum() < 2:
        sun.append((np.nan, np.nan, 0))
        continue
    yy, xx = np.nonzero(m)
    sun.append((xx.mean(), yy.mean(), int(m.sum())))
sun = np.array(sun)
print("\nsun x (480 px) first/mid/last: %.1f %.1f %.1f ; y %.1f %.1f %.1f ; area %d..%d" % (
    sun[0, 0], sun[T // 2, 0], sun[-1, 0], sun[0, 1], sun[T // 2, 1], sun[-1, 1], sun[:, 2].min(), sun[:, 2].max()))
sx = sun[:, 0]
dsx = np.diff(sx)
print("sun dx per frame: mean %.4f  std %.4f  max|.| %.3f" % (np.nanmean(dsx), np.nanstd(dsx), np.nanmax(np.abs(dsx))))
sd2 = sx[1:-1] - 0.5 * (sx[:-2] + sx[2:])
print("sun x d2 rms %.4f px (480 wide)" % np.sqrt(np.nanmean(sd2 ** 2)))
# The aureole: mean level in rings round the sun (radius in 480-px units)
yyg, xxg = np.mgrid[0:H, 0:W]
ring_lv = []
for t in range(T):
    r = np.hypot(xxg - sun[t, 0], yyg - sun[t, 1])
    ring_lv.append([Y[t][(r >= a) & (r < b)].mean() for a, b in ((4, 8), (8, 16), (16, 32), (32, 64))])
ring_lv = np.array(ring_lv)
rd2 = ring_lv[1:-1] - 0.5 * (ring_lv[:-2] + ring_lv[2:])
print("aureole ring levels mean", ring_lv.mean(axis=0).round(1), " d2 rms", np.sqrt((rd2 ** 2).mean(axis=0)).round(3))
for i, nm in enumerate(("r4-8", "r8-16", "r16-32", "r32-64")):
    print("  corr(band coherent d2, aureole %s d2) %+.2f" % (nm, np.corrcoef(cm["band"], rd2[:, i])[0, 1]))
print("  corr(band coherent d2, sun x d2) %+.2f" % np.corrcoef(cm["band"], np.nan_to_num(sd2))[0, 1])
print("  corr(|band block d2| per frame, sun x d2 abs) %+.2f" % np.corrcoef(np.abs(bd).mean(axis=(1, 2)), np.abs(np.nan_to_num(sd2)))[0, 1])

if o.cam:
    r = list(csv.DictReader(open(o.cam)))[o.a:o.b]
    ev = np.array([float(v["ev"]) for v in r])
    tg = np.array([float(v["target"]) for v in r])
    ring = np.array([float(v["ring"]) for v in r])
    ev2 = ev[1:-1] - 0.5 * (ev[:-2] + ev[2:])
    print("\nlog ev: range %.2f..%.2f, |d1| max %.3f, d2 rms %.4f (0.01 quantised)" % (
        ev.min(), ev.max(), np.abs(np.diff(ev)).max(), np.sqrt((ev2 ** 2).mean())))
    for n in ("band", "sky", "rock"):
        print("  corr(%s log coherent d2, ev d2) %+.2f" % (n, np.corrcoef(lcm[n], ev2)[0, 1]))
    absb = np.abs(bd).mean(axis=(1, 2))
    print("  corr(|band d2| per frame, |ev d2|) %+.2f, (|target d1|) %+.2f, ring %+.2f" % (
        np.corrcoef(absb, np.abs(ev2))[0, 1], np.corrcoef(absb, np.abs(np.diff(tg))[1:])[0, 1],
        np.corrcoef(absb, ring[1:-1])[0, 1]))
np.save(o.npy.replace(".npy", "_sun.npy"), sun)
