"""sharp.py <off> <on> [x0,y0,x1,y1 ...]: sharpness = mean |neighbour difference| of luma (prototype's metric), off vs on."""
import sys
import numpy as np
from PIL import Image
def luma(p):
    a = np.asarray(Image.open(p).convert('RGB')).astype(np.float64) / 255.0
    return a[..., 0] * 0.2126 + a[..., 1] * 0.7152 + a[..., 2] * 0.0722
def sharp(l):
    return 0.5 * (np.abs(np.diff(l, axis=0)).mean() + np.abs(np.diff(l, axis=1)).mean())
A, B = luma(sys.argv[1]), luma(sys.argv[2])
boxes = [(0, 0, A.shape[1], A.shape[0])] + [tuple(map(int, s.split(','))) for s in sys.argv[3:]]
for (x0, y0, x1, y1) in boxes:
    sa, sb = sharp(A[y0:y1, x0:x1]), sharp(B[y0:y1, x0:x1])
    print(f"{x0},{y0},{x1},{y1}: off {sa:.5f} on {sb:.5f} change {100 * (sb - sa) / sa:+.1f}%")
