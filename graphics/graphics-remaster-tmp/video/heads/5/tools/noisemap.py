"""Temporal noise map of a probe run: mean |frame-to-frame luma change| per pixel.
usage: noisemap.py <dir with *__fNNNN.png> <out.png> [x,y,w,h ...]  (boxes to report)
Prints for each box: mean |dL|, mean |dL| on edge pixels (top 15% gradient) and on flat pixels."""
import sys, glob, os, re
import numpy as np
from PIL import Image
from scipy import ndimage
d, out = sys.argv[1], sys.argv[2]
boxes = [[int(v) for v in b.split(',')] for b in sys.argv[3:]]
files = sorted([f for f in glob.glob(os.path.join(d, '**', '*.png'), recursive=True) if re.search(r'__f\d{4}\.png$', f)])
x0 = min(b[0] for b in boxes); y0 = min(b[1] for b in boxes)
x1 = max(b[0] + b[2] for b in boxes); y1 = max(b[1] + b[3] for b in boxes)
def L(f):
    a = np.asarray(Image.open(f).convert('RGB'), dtype=np.float32)[y0:y1, x0:x1]
    return a @ np.array([0.2126, 0.7152, 0.0722], dtype=np.float32)
prev = L(files[0]); acc = np.zeros_like(prev); mean = prev.copy()
for f in files[1:]:
    cur = L(f); acc += np.abs(cur - prev); mean += cur; prev = cur
md = acc / (len(files) - 1); mean /= len(files)
g = np.hypot(ndimage.sobel(mean, 0), ndimage.sobel(mean, 1))
for b in boxes:
    s = (slice(b[1] - y0, b[1] - y0 + b[3]), slice(b[0] - x0, b[0] - x0 + b[2]))
    m, gg = md[s], g[s]
    t = np.percentile(gg, 85)
    print(f'box {b}: mean|dL| {m.mean():.3f}  edges {m[gg >= t].mean():.3f}  flat {m[gg < np.percentile(gg, 50)].mean():.3f}  p99 {np.percentile(m, 99):.2f}')
img = np.concatenate([np.clip(mean, 0, 255), np.clip(md / 4.0 * 255, 0, 255)], axis=1).astype(np.uint8)
Image.fromarray(img).resize((img.shape[1] * 2, img.shape[0] * 2), Image.NEAREST).save(out)
print('frames', len(files), 'map', out)
