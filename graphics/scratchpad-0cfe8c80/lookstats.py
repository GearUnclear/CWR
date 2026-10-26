import sys, os, glob
import numpy as np
from PIL import Image

final = sys.argv[1]
base = sys.argv[2]

def load(p):
    im = Image.open(p).convert('RGB')
    a = np.asarray(im).astype(np.float32)
    h = a.shape[0]
    # drop the 20 px label strip at the bottom that run-captures does not add (raw shots)
    return a

def bands(a):
    h = a.shape[0]
    sky = a[: int(h * 0.30)]
    hor = a[int(h * 0.30): int(h * 0.55)]
    gnd = a[int(h * 0.55):]
    return sky, hor, gnd

def stats(x):
    m = x.reshape(-1, 3).mean(0)
    lum = 0.2126 * m[0] + 0.7152 * m[1] + 0.0722 * m[2]
    return m, lum

def pct(x, q):
    l = 0.2126 * x[..., 0] + 0.7152 * x[..., 1] + 0.0722 * x[..., 2]
    return np.percentile(l, q)

views = sorted(set(os.path.basename(p).split('__')[0] for p in glob.glob(os.path.join(final, '*__remastered.png'))))
print('| view | band | Classic L / R-B | A0 Remastered L / R-B | final L / R-B | final B-(R+G)/2 | final p10 L (A0, Classic) |')
print('|---|---|---|---|---|---|---|')
for v in views:
    c = load(os.path.join(final, v + '__classic.png'))
    f = load(os.path.join(final, v + '__remastered.png'))
    bp = os.path.join(base, v + '__remastered.png')
    b = load(bp) if os.path.exists(bp) else None
    for name, i in (('sky', 0), ('horizon', 1), ('ground', 2)):
        cs = bands(c)[i]; fs = bands(f)[i]
        cm, cl = stats(cs); fm, fl = stats(fs)
        if b is not None:
            bs = bands(b)[i]; bm, bl = stats(bs)
            bt = '%.0f / %+.0f' % (bl, bm[0] - bm[2]); bp10 = '%.0f' % pct(bs, 10)
        else:
            bt = 'n/a'; bp10 = 'n/a'
        print('| %s | %s | %.0f / %+.0f | %s | %.0f / %+.0f | %+.0f | %.0f (%s, %.0f) |' % (
            v, name, cl, cm[0] - cm[2], bt, fl, fm[0] - fm[2], fm[2] - (fm[0] + fm[1]) / 2,
            pct(fs, 10), bp10, pct(cs, 10)))
