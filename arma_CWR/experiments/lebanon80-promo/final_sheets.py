"""Builds the delivered thirds sheets: one clips\\thirds-sheet-<take>.jpg per take from the clips that are in
clips\\ now, each marked from the run that recorded it (the latest gate result per clip, retakes included),
and removes the per-run sheets (thirds-sheet-<take>_r2.jpg and the like) gate.py wrote along the way.

    python final_sheets.py
"""
import glob
import json
import os
import re

from PIL import Image, ImageDraw

import gate

ROOT = gate.ROOT
CLIPS = os.path.join(ROOT, 'clips')
RUNS = os.path.join(ROOT, 'work', 'runs')


def take_of(run_base):
    t = re.sub(r'-\d{4}-\d{6}$', '', run_base).replace('promo_', '')
    t = re.sub(r'_r\d+$', '', t)
    if t.startswith('amb_'):
        t = 'ambush'
    return t


def main():
    latest = {}
    for f in sorted(glob.glob(os.path.join(ROOT, 'work', 'gates', '*.json')), key=os.path.getmtime):
        base = os.path.basename(f)[:-5]
        for name, res in json.load(open(f, encoding='utf-8')).items():
            if os.path.exists(os.path.join(CLIPS, name + '.mp4')) and res.get('frames'):
                latest[name] = (base, res)
    by_take = {}
    for name, (base, res) in sorted(latest.items()):
        by_take.setdefault(take_of(base), []).append((name, base, res))
    rows_cache = {}
    rn = os.path.join(ROOT, 'work', 'renames.json')
    renames = json.load(open(rn)) if os.path.exists(rn) else {}
    for f in glob.glob(os.path.join(CLIPS, 'thirds-sheet-*.jpg')):
        os.remove(f)
    for take, items in sorted(by_take.items()):
        tiles = []
        for name, base, res in items:
            if base not in rows_cache:
                rows_cache[base] = gate.pcf_lines(os.path.join(RUNS, base))
            r = sorted([x for x in (rows_cache[base].get(name) or rows_cache[base].get(renames.get(name), [])) if x['n'] >= 15], key=lambda x: x['n'])
            n = res['frames']
            r = r[:n]
            ks = [0, len(r) // 2, len(r) - 1]
            ims = [gate.mark(gate.grab(os.path.join(CLIPS, name + '.mp4'), k), r[k] if 0 <= k < len(r) else None) for k in ks]
            tiles.append((name, res, ims))
        lab = 20
        sheet = Image.new('RGB', (3 * gate.SW, len(tiles) * (gate.SH + lab)), (16, 16, 16))
        y = 0
        for name, res, ims in tiles:
            ImageDraw.Draw(sheet).text((4, y + 4), '%s  %s  off %.1f s  hz-mid %.1f s  %s%s' % (
                name, 'PASS' if res.get('pass') else 'FAIL', res.get('off_s', -1), res.get('hz_mid_s', -1), json.dumps(res.get('gate')),
                '  symmetric, deliberate' if res.get('sym') else ''), fill=(120, 255, 120) if res.get('pass') else (255, 90, 90))
            y += lab
            for i, im in enumerate(ims):
                sheet.paste(im, (i * gate.SW, y))
            y += gate.SH
        out = os.path.join(CLIPS, 'thirds-sheet-%s.jpg' % take)
        sheet.save(out, quality=82)
        print(out, len(tiles))


if __name__ == '__main__':
    main()
