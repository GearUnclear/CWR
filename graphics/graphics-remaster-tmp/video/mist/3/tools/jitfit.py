"""Fit a region series (series.csv) on the modelled fog-history residual of the frame-global jitter.
usage: python jitfit.py <run dir> [label prefix]   (reads udcap.log frame= and series.csv; offsets -40..0)"""
import re, sys, glob, os, numpy as np
run = sys.argv[1]
log = glob.glob(os.path.join(run, '**', 'udcap.log'), recursive=True)[0]
fr = {}
for line in open(log, encoding='utf-8', errors='replace'):
    m = re.search(r'label=\w+__f(\d+);k=(\d+).*? frame=(\d+) ', line)
    if m: fr[int(m.group(1))] = int(m.group(3))
d = np.genfromtxt(os.path.join(run, 'series.csv'), delimiter=',', names=True)
F = np.array([fr[int(i)] for i in d['frame']], dtype=np.int64)
print('frames per shot', np.bincount(np.diff(F)))
def frac(x): return x - np.floor(x)
def feats(seqN, a=0.12, n=80):
    i = np.arange(n); w = a * (1 - a) ** i; out = []
    for s in seqN:
        sq = (s - i).astype(np.float64); row = []
        for c in (0.6180339887, 0.7548776662, 0.5698402910):
            j = frac(c * sq); row += [(w*(j-0.5)).sum(), (w*np.sin(2*np.pi*j)).sum(), (w*np.cos(2*np.pi*j)).sum()]
        out.append(row)
    return np.array(out)
def r2fit(X, y):
    Xh = X - np.array([np.convolve(X[:, c], np.ones(9)/9, 'same') for c in range(X.shape[1])]).T
    sl = slice(6, -6); A = np.c_[Xh[sl], np.ones(len(y[sl]))]
    coef, *_ = np.linalg.lstsq(A, y[sl], rcond=None)
    return 1 - ((y[sl] - A @ coef) ** 2).sum() / ((y[sl] - y[sl].mean()) ** 2).sum()
for name in ['valley', 'bay', 'farhill', 'haze_r']:
    s = d[name]; hp = s - np.convolve(s, np.ones(9)/9, 'same')
    prof = [(off, r2fit(feats(F + off), hp)) for off in range(-40, 1)]
    best = max(prof, key=lambda p: p[1]); med = np.median([p[1] for p in prof])
    print('%-8s hp rms %.3f  best R2 %.2f at offset %d (median over offsets %.2f)' % (name, np.sqrt((hp[6:-6]**2).mean()), best[1], best[0], med))
