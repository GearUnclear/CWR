from PIL import Image
import numpy as np, glob, os
src = r'E:/look-lanes/look-surface/captures/final2/tri/'
for p in sorted(glob.glob(src + '*/*__surf.png')):
    d = os.path.dirname(p); v = os.path.basename(p)[4:-len('__surf.png')]
    b = glob.glob(os.path.join(d, f'*_{v}__base.png'))[0]; c = glob.glob(os.path.join(d, f'*_{v}__classic.png'))[0]
    def t(x): return np.asarray(Image.open(x).convert('RGB').resize((480, 320), Image.LANCZOS)).astype(float)
    S, B, C = t(p), t(b), t(c)
    def m(X, Y):
        dd = np.abs(X - Y).max(2); return round((dd > 12).mean() * 100, 1), round(np.abs(X - Y).mean(), 1)
    def sat(X):
        mx = X.max(2); mn = X.min(2); return round(((mx - mn) / np.maximum(mx, 1)).mean() * 100, 1)
    print(v.ljust(22), 'lane on/off >12:', m(S, B), ' orig/overhaul:', m(S, C), ' orig/base:', m(B, C), ' sat C/B/S', sat(C), sat(B), sat(S))
