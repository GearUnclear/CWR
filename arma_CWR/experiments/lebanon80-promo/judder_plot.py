"""Draws the judder proof: frame-to-frame whole-picture shift (phase correlation, 640-wide pixels) of the
same set-up with the kaiju tremor on and off, plus the logged tremor gain at the camera.
    python judder_plot.py <run folder> <clip prefix> <out.png>
"""
import os
import sys

import numpy as np
from PIL import Image, ImageDraw

from gate import pcf_lines
from judder import ROOT, frames, shift

run, pre, out = sys.argv[1:4]
rows = pcf_lines(run)
series = []
for tag, col in (('on', (230, 60, 60)), ('off', (40, 140, 230))):
    name = pre + tag
    prev = None
    s = []
    for f in frames(os.path.join(ROOT, 'clips', name + '.mp4')):
        if prev is not None:
            s.append(shift(f, prev))
        prev = f
    s = np.array(s)
    j = np.hypot(*(s[2:] - 2 * s[1:-1] + s[:-2]).T)
    g = [r['gain'] for r in sorted(rows.get(name, []), key=lambda r: r['n']) if r['n'] >= 15]
    series.append((name, col, j, g))
W, H, M = 1200, 420, 50
im = Image.new('RGB', (W, H), (255, 255, 255))
d = ImageDraw.Draw(im)
top = max(max(x[2].max() for x in series), 1.0)
n = max(len(x[2]) for x in series)
d.line([(M, H - M), (W - 10, H - M)], fill=(0, 0, 0))
d.line([(M, 10), (M, H - M)], fill=(0, 0, 0))
for name, col, j, g in series:
    pts = [(M + i * (W - M - 10) / n, H - M - v / top * (H - M - 20)) for i, v in enumerate(j)]
    d.line(pts, fill=col, width=2)
    gp = [(M + i * (W - M - 10) / n, H - M - v / 1.0 * 60) for i, v in enumerate(g)]
    d.line(gp, fill=tuple(c // 2 + 120 for c in col), width=1)
d.text((M + 5, 12), 'jerk of the whole-picture shift per frame (px at 640 wide), red: tremor on, blue: tremor off; thin: logged tremor gain (0..1, bottom 60 px)', fill=(0, 0, 0))
d.text((M + 5, 26), '%s: max %.1f px on, %.1f px off' % (pre, series[0][2].max(), series[1][2].max()), fill=(0, 0, 0))
d.text((W // 2, H - M + 8), 'delivered frame', fill=(0, 0, 0))
im.save(out)
print(out)
