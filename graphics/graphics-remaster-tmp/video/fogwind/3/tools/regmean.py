"""Agent 3: mean grey (0-255) per agent-1 region over frames [from, to) of clips; the look check."""
import sys, os
sys.path.insert(0, 'D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/fogwind/1/kit')
import numpy as np
import fogmotion as fm
a = 15
clips = sys.argv[1:]
names = list(fm.REGIONS)
print("clip".ljust(28), " ".join(n.rjust(8) for n in names), "  band-row(0.47-0.70) RGB")
for c in clips:
    vals = []
    for n in names:
        y0, y1, x0, x1 = fm.REGIONS[n]
        box = (int(y0 * 1080), int(y1 * 1080), int(x0 * 1920), int(x1 * 1920))
        st = fm.decode(c, a, a + 60, box)
        vals.append(st.mean())
    print(os.path.basename(c)[:28].ljust(28), " ".join(f"{v:8.2f}" for v in vals))
