"""Mean colour of a box in several shots: python boxstats.py <run> <view> x0,y0,x1,y1 label..."""
import sys, colorsys
from pathlib import Path
from PIL import Image
run, view, box = Path(sys.argv[1]), sys.argv[2], tuple(int(v) for v in sys.argv[3].split(','))
for lab in sys.argv[4:]:
    hits = sorted((run / 'tri').rglob(f'*_{view}__{lab}.png'))
    if not hits:
        print(lab, 'missing'); continue
    im = Image.open(hits[-1]).convert('RGB').crop(box)
    px = list(im.getdata()); n = len(px)
    r, g, b = (sum(p[i] for p in px) / n for i in range(3))
    h, s, v = colorsys.rgb_to_hsv(r / 255, g / 255, b / 255)
    print(f'{lab:14s} rgb {r:5.1f} {g:5.1f} {b:5.1f}  sat {s:.3f}  val {v:.3f}')
