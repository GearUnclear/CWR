import sys, numpy as np
sys.path.insert(0, "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/mist/tools")
import bandmetric as bm
src = sys.argv[1]; a = int(sys.argv[2]); b = int(sys.argv[3])
x = bm.load(src, a, b)
B = 15; gy, gx = 270 // B, 480 // B
blk = x[:, :gy*B, :gx*B].reshape(len(x), gy, B, gx, B).mean(axis=(2, 4))
d2 = blk[1:-1] - 0.5*(blk[:-2] + blk[2:])
rms = np.sqrt((d2**2).mean(axis=0))
np.set_printoptions(linewidth=250, precision=1, suppress=True)
print((rms*10).round().astype(int))
idx = np.dstack(np.unravel_index(np.argsort(-rms.ravel())[:8], rms.shape))[0]
for r, c in idx:
    print("block r%d c%d rms %.2f  px x%d-%d y%d-%d  mean %.1f" % (r, c, rms[r, c], c*60, c*60+60, r*60, r*60+60, blk[:, r, c].mean()))
r, c = idx[0]
print("series of worst block (frame index from a):")
print(np.round(blk[:, r, c], 2)[50:70])
