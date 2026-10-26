"""Agent 3: per-frame d2 of 60 px block means in agent 1's regions; where and when the whole-band pulses happen.
usage: python pulse.py clip.mkv [--from 15]"""
import sys
sys.path.insert(0, 'D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/fogwind/1/kit')
import numpy as np, fogmotion as fm
src = sys.argv[1]; a = 15
for n, (y0, y1, x0, x1) in fm.REGIONS.items():
    box = (int(y0 * 1080), int(y1 * 1080), int(x0 * 1920), int(x1 * 1920))
    st = fm.decode(src, a, 0, box)
    m = st.mean((1, 2))
    d2 = m[2:] - 2 * m[1:-1] + m[:-2]
    top = np.argsort(-np.abs(d2))[:6] + a + 1
    print(f"{n:8s} d2 rms {np.sqrt((d2**2).mean()):.3f}  top frames {sorted(top.tolist())}  max {np.abs(d2).max():.3f}")
