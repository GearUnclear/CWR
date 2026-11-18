"""Plots the scout dump (SC road / SC house lines of promo_scout) as one map per place, for choosing stages.

    python plot_scout.py <game_stdout.log> <out folder>
"""
import os
import re
import sys

from PIL import Image, ImageDraw


def main():
    log, out = sys.argv[1], sys.argv[2]
    os.makedirs(out, exist_ok=True)
    data = {}
    for line in open(log, encoding='utf-8', errors='replace'):
        m = re.search(r'"SC (road|house) (\w+)((?: -?\d+,-?\d+)*)', line)
        if m:
            pts = [tuple(int(v) for v in p.split(',')) for p in m.group(3).split()]
            data.setdefault(m.group(2), {})[m.group(1)] = pts
    for name, d in data.items():
        allp = d.get('road', []) + d.get('house', [])
        if not allp:
            continue
        x0 = min(p[0] for p in allp) - 20
        x1 = max(p[0] for p in allp) + 20
        y0 = min(p[1] for p in allp) - 20
        y1 = max(p[1] for p in allp) + 20
        s = 1000 / max(x1 - x0, y1 - y0)
        W, H = int((x1 - x0) * s) + 1, int((y1 - y0) * s) + 1
        im = Image.new('RGB', (W, H + 20), (30, 30, 30))
        dr = ImageDraw.Draw(im)

        def P(p):
            return ((p[0] - x0) * s, H - (p[1] - y0) * s)
        for p in d.get('house', []):
            x, y = P(p)
            dr.rectangle([x - 3, y - 3, x + 3, y + 3], fill=(200, 160, 90))
        for p in d.get('road', []):
            x, y = P(p)
            dr.ellipse([x - 2, y - 2, x + 2, y + 2], fill=(120, 200, 255))
        # grid every 50 m
        g = 50 * (1 + int((x1 - x0) / 1000))
        for gx in range(int(x0 // g * g), int(x1) + 1, g):
            x, _ = P((gx, 0))
            dr.line([x, 0, x, H], fill=(60, 60, 60))
            dr.text((x + 2, H + 4), str(gx), fill=(200, 200, 200))
        for gy in range(int(y0 // g * g), int(y1) + 1, g):
            _, y = P((0, gy))
            dr.line([0, y, W, y], fill=(60, 60, 60))
            dr.text((2, y + 2), str(gy), fill=(200, 200, 200))
        im.save(os.path.join(out, 'scout_%s.png' % name))
        print(name, len(d.get('road', [])), 'road points', len(d.get('house', [])), 'houses', (x0, y0, x1, y1))


if __name__ == '__main__':
    main()
