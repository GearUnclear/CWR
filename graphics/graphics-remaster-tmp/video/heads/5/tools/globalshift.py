"""Frame-to-frame global sub-pixel shift of a static region (phase correlation, upsampled peak).
usage: globalshift.py <dir with *__fNNNN.png> x,y,w,h   -> RMS and p95 of |shift| per pair"""
import sys, glob, os, re
import numpy as np
from PIL import Image
d = sys.argv[1]; x, y, w, h = [int(v) for v in sys.argv[2].split(',')]
files = sorted([f for f in glob.glob(os.path.join(d, '*.png')) if re.search(r'__f\d{4}\.png$', f)])
win = np.outer(np.hanning(h), np.hanning(w))
def L(f):
    a = np.asarray(Image.open(f).convert('RGB'), dtype=np.float64)[y:y + h, x:x + w] @ np.array([0.2126, 0.7152, 0.0722])
    return (a - a.mean()) * win
def shift(a, b, up=20):
    F = np.fft.fft2(a) * np.conj(np.fft.fft2(b)); F /= np.abs(F) + 1e-9
    r = np.fft.ifft2(F).real
    j, i = np.unravel_index(np.argmax(r), r.shape)
    # parabolic sub-pixel
    def sub(m1, c, p1):
        dd = m1 - 2 * c + p1
        return 0 if abs(dd) < 1e-12 else 0.5 * (m1 - p1) / dd
    dy = sub(r[j - 1, i], r[j, i], r[(j + 1) % h, i]); dx = sub(r[j, i - 1], r[j, i], r[j, (i + 1) % w])
    jj = j if j < h // 2 else j - h; ii = i if i < w // 2 else i - w
    return ii + dx, jj + dy
prev = L(files[0]); S = []
for f in files[1:]:
    cur = L(f); S.append(shift(cur, prev)); prev = cur
S = np.array(S); m = np.hypot(S[:, 0], S[:, 1])
print(f'{d}: pairs {len(S)}  |shift| rms {np.sqrt((m**2).mean()):.3f} px  p95 {np.percentile(m,95):.3f}  max {m.max():.3f}  mean dx {S[:,0].mean():+.3f} dy {S[:,1].mean():+.3f}')
