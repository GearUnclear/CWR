import sys, os, numpy as np
sys.path.insert(0, 'D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/fogwind/1/kit')
import fogmotion as fm
src = sys.argv[1]
st = fm.decode(src, 0, 0, (int(0.56*1080), int(0.70*1080), int(0.62*1920), int(0.97*1920)))
win = fm.hann(*st[0].shape)
sh = np.array([fm.phase_shift(st[t], st[t+1], win) for t in range(len(st)-1)])
dx = sh[:, 0] if sh.ndim == 2 else sh
print("frames", len(st))
print(" ".join("%d:%.2f" % (t+1, v) for t, v in enumerate(sh[:, 1] if sh.shape[1] > 1 else dx)))
