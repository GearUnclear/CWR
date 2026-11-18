"""Composition gate (rule of thirds) for one promo run, from the per-frame PCF log lines of pcStep.

    python gate.py <run folder> [--clips <clips dir>] [--sheet-name <take>]

For every clip in the take's <take>.shots.json: the subject's logged screen position (pcProj: eye, target
and lens, the engine's 16:9 lens of camSetFov x 4/3 across and x 0.75 up) is compared with its gate:
x = 1/3, 2/3, 0.5 (a deliberately symmetric shot) or "any" (either vertical third line), y optional, within
tol (default 0.08 of the frame). A clip FAILS when its subject is off the gate (or off screen) for more than
1 s of the delivered clip (the 15 head frames excluded), or when the horizon line of a level lens sits in
the middle band (0.42..0.58) for more than 1 s and the gate does not set hz = False.

Writes work\\gates\\<run>.json and clips\\thirds-sheet-<take>.jpg: the first, middle and last delivered
frame of each clip with the thirds grid, the subject (red ring), the second subject (cyan) and the level
horizon (yellow), and the verdict.
"""
import argparse
import json
import os
import re
import subprocess
import sys

from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
from promopaths import FF, ROOT

ANSI = re.compile(r'\x1b\[[0-9;]*m')
SW, SH = 640, 360


def pcf_lines(run):
    rows = {}
    for dp, _, fs in os.walk(run):
        for f in fs:
            if f != 'game_stdout.log':
                continue
            for ln in open(os.path.join(dp, f), encoding='utf-8', errors='replace'):
                i = ln.find('PCF|')
                if i < 0:
                    continue
                p = ANSI.sub('', ln[i:]).strip().strip('"').split('|')
                if len(p) < 13:
                    continue
                try:
                    v = lambda s: [float(x) for x in s.split(',')]
                    rows.setdefault(p[1], []).append(dict(n=int(float(p[2])), t=float(p[3]), frame=float(p[4]), eye=v(p[5]), tgt=v(p[6]),
                                                           s=v(p[7]), s2=v(p[8]), hz=float(p[9]), gain=float(p[10]), raw=v(p[11]), fov=float(p[12])))
                except ValueError:
                    continue
    return rows


def dist_x(sx, gx):
    if gx == 'any':
        return min(abs(sx - 1 / 3), abs(sx - 2 / 3))
    return abs(sx - float(gx))


def evaluate(rows, gate, head, nframes):
    tol = gate.get('tol', 0.08)
    tol2 = gate.get('tol2', tol)
    d = [r for r in rows if head <= r['n'] < head + nframes]
    # a gate may cover part of the clip only (seconds from the delivered start)
    d = [r for r in d if gate.get('from', 0) * 30 <= r['n'] - head < gate.get('until', 1e9) * 30]
    off = off2 = mid = 0
    for r in d:
        sx, sy = r['s']
        if gate.get('x') != 'none':
            if sx < 0 or sx > 1 or sy < 0 or sy > 1 or dist_x(sx, gate.get('x', 'any')) > tol or (
                    gate.get('y') is not None and abs(sy - float(gate['y'])) > tol):
                off += 1
        if gate.get('x2') is not None:
            sx2, sy2 = r['s2']
            if sx2 < 0 or sx2 > 1 or sy2 < 0 or sy2 > 1 or dist_x(sx2, gate['x2']) > tol2 or (
                    gate.get('y2') is not None and abs(sy2 - float(gate['y2'])) > tol2):
                off2 += 1
        if gate.get('hz', True) and 0.42 < r['hz'] < 0.58:
            mid += 1
    res = dict(frames=len(d), off_s=off / 30.0, off2_s=off2 / 30.0, hz_mid_s=mid / 30.0)
    res['pass'] = len(d) > 0 and res['off_s'] <= 1.0 and res['off2_s'] <= 1.0 and res['hz_mid_s'] <= 1.0
    return res


def grab(mp4, k):
    r = subprocess.run([FF, '-v', 'error', '-i', mp4, '-vf', f"select='eq(n\\,{k})',scale={SW}:{SH}", '-frames:v', '1',
                        '-f', 'rawvideo', '-pix_fmt', 'rgb24', '-'], capture_output=True)
    if len(r.stdout) != SW * SH * 3:
        return Image.new('RGB', (SW, SH), (40, 0, 0))
    return Image.frombytes('RGB', (SW, SH), r.stdout)


def mark(im, row):
    d = ImageDraw.Draw(im)
    for x in (SW / 3, 2 * SW / 3):
        d.line([(x, 0), (x, SH)], fill=(255, 255, 255), width=1)
    for y in (SH / 3, 2 * SH / 3):
        d.line([(0, y), (SW, y)], fill=(255, 255, 255), width=1)
    if row:
        hz = row['hz'] * SH
        if 0 <= hz <= SH:
            for x in range(0, SW, 16):
                d.line([(x, hz), (x + 8, hz)], fill=(255, 220, 0), width=1)
        for (sx, sy), col in ((row['s'], (255, 40, 40)), (row['s2'], (0, 230, 255))):
            if 0 <= sx <= 1 and 0 <= sy <= 1:
                x, y = sx * SW, sy * SH
                d.ellipse([x - 7, y - 7, x + 7, y + 7], outline=col, width=2)
    return im


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('run')
    ap.add_argument('--clips', default=os.path.join(ROOT, 'clips'))
    ap.add_argument('--sheet-name')
    a = ap.parse_args()
    base = os.path.basename(a.run.rstrip('\\/'))
    take = re.sub(r'-\d{4}-\d{6}$', '', base)
    shots = json.load(open(os.path.join(HERE, take + '.shots.json'), encoding='utf-8'))
    head = shots.get('head', 15)
    rows = pcf_lines(a.run)
    if not rows:
        print('no PCF lines (aborted run?): nothing written')
        return 1
    results = {}
    tiles = []
    for c in shots['clips']:
        name = c['name']
        r = rows.get(name, [])
        nfr = int(round(c['seconds'] * 30))
        r = [x for x in r if x['n'] < head + nfr]
        res = evaluate(r, c['gate'], head, nfr) if r else dict(frames=0)
        res.setdefault('pass', False)
        res['gate'] = c['gate']
        res['sym'] = c.get('sym', False)
        results[name] = res
        mp4 = os.path.join(a.clips, name + '.mp4')
        if os.path.exists(mp4) and r:
            d = [x for x in r if x['n'] >= head]
            ks = [0, len(d) // 2, len(d) - 1]
            ims = [mark(grab(mp4, k), d[k] if k < len(d) else None) for k in ks]
            tiles.append((name, res, ims))
        print('%-4s %-52s off %.1f s%s, horizon mid %.1f s  gate %s%s' % (
            'ok' if res['pass'] else 'FAIL', name, res.get('off_s', -1),
            (', second %.1f s' % res['off2_s']) if c['gate'].get('x2') is not None else '', res.get('hz_mid_s', -1),
            json.dumps(c['gate']), '  symmetric, deliberate' if c.get('sym') else ''))
    os.makedirs(os.path.join(ROOT, 'work', 'gates'), exist_ok=True)
    json.dump(results, open(os.path.join(ROOT, 'work', 'gates', base + '.json'), 'w'), indent=1)
    if tiles:
        lab = 20
        sheet = Image.new('RGB', (3 * SW, len(tiles) * (SH + lab)), (16, 16, 16))
        y = 0
        for name, res, ims in tiles:
            ImageDraw.Draw(sheet).text((4, y + 4), '%s  %s  off %.1f s  hz-mid %.1f s  %s' % (
                name, 'PASS' if res['pass'] else 'FAIL', res.get('off_s', -1), res.get('hz_mid_s', -1), json.dumps(res['gate'])),
                fill=(120, 255, 120) if res['pass'] else (255, 90, 90))
            y += lab
            for i, im in enumerate(ims):
                sheet.paste(im, (i * SW, y))
            y += SH
        out = os.path.join(a.clips, 'thirds-sheet-%s.jpg' % (a.sheet_name or take.replace('promo_', '')))
        sheet.save(out, quality=82)
        print(out)
    return 0 if all(r['pass'] for r in results.values()) else 1


if __name__ == '__main__':
    sys.exit(main())
