import sys, numpy as np, colorsys
from PIL import Image
d = r'D:/Arma_CWA_worktrees/graphics-remaster/tmp/integration/A7/final/'
v = sys.argv[1]; c0, c1 = int(sys.argv[2]), int(sys.argv[3]); r0, r1, step = map(int, sys.argv[4:7])
a = {m: np.asarray(Image.open(d+f'atmosphere_{v}__{m}.png').convert('RGB')).astype(np.float64) for m in ('off','on')}
for y in range(r0, r1, step):
    s = []
    for m in ('off','on'):
        p = a[m][y:y+step, c0:c1].reshape(-1,3).mean(0)
        h, l, sat = colorsys.rgb_to_hls(*(p/255))
        s.append(f'{m} {p.round(0).astype(int)} hue {h*360:5.1f} sat {sat:.2f}')
    print(y, ' | '.join(s))
