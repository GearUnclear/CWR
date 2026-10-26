import sys
from PIL import Image, ImageOps
d = r'D:/Arma_CWA_worktrees/graphics-remaster/tmp/integration/A7/final/'
v, x0, y0, x1, y1, out = sys.argv[1], *map(int, sys.argv[2:6]), sys.argv[6]
stretch = len(sys.argv) > 7
ims = []
for m in ('off', 'on'):
    im = Image.open(d + f'atmosphere_{v}__{m}.png').convert('RGB').crop((x0, y0, x1, y1))
    ims.append(im)
w, h = ims[0].size
canvas = Image.new('RGB', (w, 2 * h + 6), (0, 0, 0))
canvas.paste(ims[0], (0, 0)); canvas.paste(ims[1], (0, h + 6))
if stretch:
    canvas = ImageOps.autocontrast(canvas, cutoff=1)
canvas.save(out)
