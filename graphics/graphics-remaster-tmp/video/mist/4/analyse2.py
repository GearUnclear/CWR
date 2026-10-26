"""Agent 4: shape of the coherent band flicker.

usage: python analyse2.py <ud480.npy>
 - per-block regression slope of d2 onto the band's coherent d2, against the
   block's mean level (an opacity/transmittance swing gives slope ~ (fog - bg):
   positive on dark terrain, negative on sky brighter than the haze);
 - the spectrum / autocorrelation of the coherent signal (a periodic flicker
   at the TAA jitter period points at a jittered sampler).
"""
import sys
import numpy as np

x = np.load(sys.argv[1]).astype(np.float32)
T = len(x)
Y = 0.299 * x[..., 0] + 0.587 * x[..., 1] + 0.114 * x[..., 2]
B = 15
blk = Y[:, :270, :480].reshape(T, 18, B, 32, B).mean(axis=(2, 4))
d2 = blk[1:-1] - 0.5 * (blk[:-2] + blk[2:])
s = d2[:, 9:13, 16:32].mean(axis=(1, 2))  # the core: mist over the valley + bay
s0 = s / np.sqrt((s ** 2).mean())
beta = (d2 * s0[:, None, None]).mean(axis=0)  # level units per unit of normalised signal
lvl = blk.mean(axis=0)
print("rows 4-14, cols 14-31: block level -> slope onto the core signal")
pairs = []
for r in range(4, 15):
    line = []
    for c in range(14, 32):
        line.append("%3d:%+.2f" % (lvl[r, c], beta[r, c]))
        pairs.append((lvl[r, c], beta[r, c], r, c))
    print("r%02d " % r + " ".join(line))
# Fit beta = k * (F - level) over sky+haze blocks (rows 4-9, right half) to find F.
P = np.array([(a, b) for a, b, r, c in pairs if r <= 9])
A = np.vstack([np.ones(len(P)), P[:, 0]]).T
coef, *_ = np.linalg.lstsq(A, P[:, 1], rcond=None)
print("fit over rows 4-9: beta = %.3f %+.4f * level  -> zero at level %.1f" % (coef[0], coef[1], -coef[0] / coef[1]))

# Temporal shape of the core signal.
lv = blk[:, 9:13, 16:32].mean(axis=(1, 2))
k = 15
sm = np.convolve(lv, np.ones(k) / k, mode="same")
hp = (lv - sm)[k:-k]
print("\ncore level %.1f; high-pass (15-frame box) rms %.3f, max %.3f" % (lv.mean(), hp.std(), np.abs(hp).max()))
f = np.fft.rfft(hp - hp.mean())
pw = np.abs(f) ** 2
fr = np.fft.rfftfreq(len(hp))
top = np.argsort(pw)[-8:][::-1]
print("top periods (frames):", [(round(1 / fr[i], 2) if fr[i] > 0 else 0, round(float(pw[i] / pw.sum()), 3)) for i in top])
ac = [np.corrcoef(s[:-L], s[L:])[0, 1] for L in range(1, 21)]
print("autocorr of core d2, lags 1..20:", " ".join("%+.2f" % a for a in ac))
# Distribution: is it a few big steps or continuous noise?
a = np.abs(s)
print("core coherent |d2|: median %.3f  p90 %.3f  p99 %.3f  max %.3f ; frames over 0.5: %d" % (
    np.median(a), np.percentile(a, 90), np.percentile(a, 99), a.max(), int((a > 0.5).sum())))
print("worst core frames (1-based):", [int(i) + 2 for i in np.argsort(a)[-10:][::-1]])
np.savetxt(sys.argv[1].replace(".npy", "_core.txt"), np.c_[np.arange(T), lv], fmt="%.3f")
