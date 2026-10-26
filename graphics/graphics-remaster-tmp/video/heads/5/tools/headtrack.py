"""Frame-to-frame motion of small patches (heads, torsos, background) in a frame folder.

usage: headtrack.py <frame dir> <glob> name:x,y,w,h [name:x,y,w,h ...]
Per patch and per frame k: the shift (dx, dy) in pixels that best maps the patch
of frame k-1 (at its tracked position) into frame k (NCC, +-R px search,
parabolic sub-pixel peak). Prints per-patch jitter = RMS of the second
difference of the cumulative track (a smooth pan has ~0), and the relative
jitter of each patch against the patch named 'bg' (camera motion removed).
"""
import sys, glob, os, json
import numpy as np
from PIL import Image

R = 5

def load(p):
    a = np.asarray(Image.open(p).convert('RGB'), dtype=np.float64)
    return a @ np.array([0.2126, 0.7152, 0.0722])

def ncc_shift(prev, cur, x, y, w, h):
    t = prev[y:y + h, x:x + w]
    t = t - t.mean()
    tn = np.sqrt((t * t).sum()) + 1e-9
    S = np.full((2 * R + 1, 2 * R + 1), -1.0)
    for j in range(-R, R + 1):
        for i in range(-R, R + 1):
            yy, xx = y + j, x + i
            if yy < 0 or xx < 0 or yy + h > cur.shape[0] or xx + w > cur.shape[1]:
                continue
            c = cur[yy:yy + h, xx:xx + w]
            c = c - c.mean()
            S[j + R, i + R] = (t * c).sum() / (tn * (np.sqrt((c * c).sum()) + 1e-9))
    j, i = np.unravel_index(np.argmax(S), S.shape)
    def sub(a, b, c):
        d = a - 2 * b + c
        return 0.0 if abs(d) < 1e-12 else 0.5 * (a - c) / d
    dx = i - R + (sub(S[j, i - 1], S[j, i], S[j, i + 1]) if 0 < i < 2 * R else 0)
    dy = j - R + (sub(S[j - 1, i], S[j, i], S[j + 1, i]) if 0 < j < 2 * R else 0)
    return dx, dy, S[j, i]

def main():
    d, pat = sys.argv[1], sys.argv[2]
    patches = {}
    for a in sys.argv[3:]:
        n, v = a.split(':')
        patches[n] = [int(t) for t in v.split(',')]
    files = sorted(glob.glob(os.path.join(d, pat)))
    frames = [load(f) for f in files]
    tracks = {n: [(0.0, 0.0)] for n in patches}
    pos = {n: [float(patches[n][0]), float(patches[n][1])] for n in patches}
    corr = {n: [] for n in patches}
    for k in range(1, len(frames)):
        for n, (x, y, w, h) in patches.items():
            px, py = int(round(pos[n][0])), int(round(pos[n][1]))
            dx, dy, c = ncc_shift(frames[k - 1], frames[k], px, py, w, h)
            pos[n][0] += dx; pos[n][1] += dy
            tracks[n].append((dx, dy))
            corr[n].append(c)
    res = {}
    T = {n: np.array(tracks[n]) for n in tracks}
    for n in T:
        v = T[n][1:]                       # per-frame velocity
        acc = np.diff(v, axis=0)           # second difference of position
        r = {'jit_rms_px': float(np.sqrt((acc ** 2).sum(axis=1).mean())), 'mean_ncc': float(np.mean(corr[n]))}
        if 'bg' in T and n != 'bg':
            rel = v - T['bg'][1:]
            racc = np.diff(rel, axis=0)
            r['rel_jit_rms_px'] = float(np.sqrt((racc ** 2).sum(axis=1).mean()))
            r['rel_vel_rms_px'] = float(np.sqrt((rel ** 2).sum(axis=1).mean()))
            # sign flips of relative x/y velocity (wobble = back and forth)
            r['rel_drift_px'] = [float(x) for x in rel.sum(axis=0)]
        res[n] = r
    print(json.dumps(res, indent=1))
    np.save(os.path.join(d, '_tracks.npy'), {n: T[n] for n in T}, allow_pickle=True)

main()
