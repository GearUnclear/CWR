"""Pop scan: finds one-frame steps in a clip (pop-in, a whole-scene detail snap, a judder, an explosion).

    python popscan.py "<clip glob>" <out.json>

Each clip is decoded at 960 x 540 grey and cut into 20 px blocks. A block flags on frame t when its change
from t-1 to t is a one-frame step: more than 1.5 x the larger of its neighbouring changes plus 1, by over 4
grey levels. A frame with 40 or more flagged blocks (3 percent of the picture) is worth a look on the clip
itself. post_take.py runs this on every clip of a take. Prints the worst eight frames per clip and writes
every flagged frame to <out.json> as [frame, blocks, score, cx, cy, x0, y0, x1, y1] in 1920 x 1080 pixels.
"""
import glob
import json
import os
import subprocess
import sys

import numpy as np

from promopaths import FF

W, H, B = 960, 540, 20


def frames(path):
    p = subprocess.Popen([FF, '-v', 'error', '-i', path, '-vf', f'scale={W}:{H}', '-pix_fmt', 'gray', '-f', 'rawvideo', '-'],
                         stdout=subprocess.PIPE)
    while True:
        b = p.stdout.read(W * H)
        if len(b) < W * H:
            break
        yield np.frombuffer(b, np.uint8).reshape(H, W).astype(np.int16)


def blk(a):
    return a.reshape(H // B, B, W // B, B).mean(axis=(1, 3))


def scan(path):
    """every frame with flagged blocks: (frame, blocks, score, cx, cy, x0, y0, x1, y1), sorted by frame"""
    prev = None
    ds = []
    for f in frames(path):
        if prev is not None:
            ds.append(blk(np.abs(f - prev)))
        prev = f
    d = np.array(ds)  # d[t] = change from frame t to t+1
    res = []
    for t in range(1, len(d) - 1):
        s = d[t] - np.maximum(d[t - 1], d[t + 1]) * 1.5 - 1.0
        m = s > 4
        c = int(m.sum())
        if c > 0:
            ys, xs = np.nonzero(m)
            res.append((t + 1, c, float(s[m].sum()), int(xs.mean() * B * 2), int(ys.mean() * B * 2), int(xs.min() * B * 2),
                        int(ys.min() * B * 2), int((xs.max() + 1) * B * 2), int((ys.max() + 1) * B * 2)))
    return res, len(d) + 1


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        return 2
    out = {}
    for path in sorted(glob.glob(sys.argv[1])):
        res, n = scan(path)
        name = os.path.splitext(os.path.basename(path))[0]
        tot = sum(r[1] for r in res)
        out[name] = sorted(res, key=lambda r: -r[2])
        print(f"{name}: frames={n} popframes={len(res)} blocks={tot} max/frame={max([r[1] for r in res] or [0])}")
        for r in out[name][:8]:
            print(f"    f{r[0]} t={r[0] / 30:.2f}s blocks={r[1]} score={r[2]:.0f} bbox=({r[5]},{r[6]})-({r[7]},{r[8]})")
    with open(sys.argv[2], 'w') as fh:
        json.dump(out, fh)
    return 0


if __name__ == '__main__':
    sys.exit(main())
