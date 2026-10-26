import sys, numpy as np
sys.path.insert(0, "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/mist/tools")
import bandmetric as bm
mkv, frames_txt = sys.argv[1], sys.argv[2]
x = bm.load(mkv, 0, None)
d = np.loadtxt(frames_txt)
fr = d[:, 1]
B = 15; gy, gx = 270 // B, 480 // B
blk = x[:, :gy*B, :gx*B].reshape(len(x), gy, B, gx, B).mean(axis=(2, 4))
# worst region rows 9-13, cols 17-24 (x1020-1500, y540-840)
reg = blk[:, 9:14, 17:25].mean(axis=(1, 2))
glob = x.mean(axis=(1, 2))
sky = blk[:, 0:5, :].mean(axis=(1, 2))
n = len(reg)
dreg = np.diff(reg); dglob = np.diff(glob); dsky = np.diff(sky)
df = np.diff(fr)[:n-1]
print("shots", n, "frames", len(fr))
print("corr(d region, render frames between shots) = %.3f" % np.corrcoef(dreg[30:], df[30:])[0, 1])
print("corr(d region, d global) = %.3f" % np.corrcoef(dreg[30:], dglob[30:])[0, 1])
print("corr(d region, d sky) = %.3f" % np.corrcoef(dreg[30:], dsky[30:])[0, 1])
for v in sorted(set(df.astype(int))):
    m = df[30:] == v
    print("df=%d n=%d mean dreg %.2f mean dglob %.2f" % (v, m.sum(), dreg[30:][m].mean(), dglob[30:][m].mean()))
np.set_printoptions(precision=2, suppress=True, linewidth=200)
print("dreg", dreg[30:70]); print("df  ", df[30:70]); print("dglb", dglob[30:70])
