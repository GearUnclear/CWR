import sys
from PIL import Image
import numpy as np
A=sys.argv[1]; B=sys.argv[2]; out=sys.argv[3] if len(sys.argv)>3 else None
a=np.asarray(Image.open(A).convert('RGB')).astype(int); b=np.asarray(Image.open(B).convert('RGB')).astype(int)
d=np.abs(a-b).max(axis=2)
g=d[591:]
ys,xs=np.nonzero(g>8)
print(A.split('/')[-1], 'vs', B.split('/')[-2]+'/'+B.split('/')[-1], 'ground>8', len(ys), 'means a/b', a.mean().round(2), b.mean().round(2), 'ground mean', a[591:].mean().round(2), b[591:].mean().round(2))
if len(ys):
    print(' bbox y', ys.min()+591, ys.max()+591, 'x', xs.min(), xs.max(), 'signed mean diff there', (a[591:]-b[591:]).mean(axis=2)[g>8].mean().round(2))
if out:
    Image.fromarray((np.clip(d*8,0,255)).astype('uint8')).save(out)
