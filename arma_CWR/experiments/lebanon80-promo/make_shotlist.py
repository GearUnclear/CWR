"""Writes clips\\shotlist.md from the clips on disk, the takes' shots.json (what each clip shows), the latest
thirds-gate and pop-scan results per clip (work\\gates, work\\post) and the review table (review_table.py:
rating 1-5, camera, faults, composition note).

    python make_shotlist.py
"""
import glob
import json
import os
import subprocess

from promopaths import CLIPS, FP, WORK

HERE = os.path.dirname(os.path.abspath(__file__))

REVIEW = {}
HEAD = ''
PICKS = ''
exec(open(os.path.join(HERE, 'review_table.py'), encoding='utf-8').read())


def latest(pattern):
    """clip name -> result, later runs (by file time) winning"""
    out = {}
    for f in sorted(glob.glob(pattern), key=os.path.getmtime):
        try:
            d = json.load(open(f, encoding='utf-8'))
        except ValueError:
            continue
        for k, v in d.items():
            if os.path.exists(os.path.join(CLIPS, k + '.mp4')):
                out[k] = v
    return out


def notes():
    out = {}
    for f in sorted(glob.glob(os.path.join(HERE, 'promo_*.shots.json')), key=os.path.getmtime):
        for c in json.load(open(f, encoding='utf-8'))['clips']:
            if c.get('note'):
                out[c['name']] = c['note']
    return out


def probe(f):
    r = subprocess.run([FP, '-v', 'error', '-select_streams', 'v:0', '-show_entries', 'stream=width,height:format=duration',
                        '-of', 'default=nk=1:nw=1', f], capture_output=True, text=True)
    v = r.stdout.split()
    return int(v[0]), int(v[1]), float(v[2])


def tod(name):
    for k in ('dawn', 'day', 'golden', 'dusk', 'bluehour', 'night'):
        if '_%s_' % k in name:
            return {'golden': 'golden hour', 'bluehour': 'blue hour'}.get(k, k)
    return 'map screen' if 'journal' in name else 'golden hour'


def gate_text(g):
    if not g:
        return 'not gated'
    gt = g.get('gate', {})
    if gt.get('x') == 'none':
        return 'no subject gate (first person / map screen)'

    def pos(v):
        if v == 'any':
            return 'a third'
        return {0.333: 'left third', 0.667: 'right third', 0.5: 'centre'}.get(round(float(v), 3), '%.2f across' % float(v))

    def posy(v):
        return {0.333: 'upper third', 0.667: 'lower third'}.get(round(float(v), 3), '%.2f down' % float(v))
    s = 'subject on the %s' % pos(gt.get('x', 'any'))
    if gt.get('y') is not None:
        s += ', %s' % posy(gt['y'])
    s += ': %s (off %.1f s)' % ('PASS' if g.get('pass') else 'FAIL', g.get('off_s', 0))
    if gt.get('x2') is not None:
        s += '; second subject off %.1f s' % g.get('off2_s', 0)
    if gt.get('hz', True):
        s += '; horizon in the middle band %.1f s' % g.get('hz_mid_s', 0)
    if g.get('sym'):
        s += '; symmetric, deliberate'
    return s


def main():
    n = notes()
    gates = latest(os.path.join(WORK, 'gates', '*.json'))
    pops = latest(os.path.join(WORK, 'post', '*.popscan.json'))
    rows = []
    total = 0
    for f in sorted(glob.glob(os.path.join(CLIPS, '*.mp4'))):
        name = os.path.splitext(os.path.basename(f))[0]
        w, h, d = probe(f)
        total += d
        rating, move, faults, comp = REVIEW.get(name, (0, '', 'not reviewed', ''))
        g = gate_text(gates.get(name))
        if comp:
            g = comp + '. Gate: ' + g
        p = pops.get(name)
        ps = 'max %d blocks/frame' % p['max_blocks'] if p else 'not scanned'
        rows.append('| %s | %.1f s | %s | %s | %s | %s | %s | %s | %s |' % (
            os.path.basename(f), d, n.get(name, ''), tod(name), move, g, ps, rating, faults))
    head = ['# Lebanon80 promo: shot list', '',
            '%d clips, %.0f s (%.1f min) in all. 2560 x 1440, 30 fps, H.264 CRF 16, yuv420p, no audio track.' % (len(rows), total, total / 60),
            'Rating 1 to 5 (5 = lead shot). Faults are what to cut around. Shoot details: shoot-notes.md.', HEAD, '',
            '| File | Length | What it shows | Time of day | Camera | Composition | Pop scan | Rating | Faults |',
            '|---|---|---|---|---|---|---|---|---|']
    with open(os.path.join(CLIPS, 'shotlist.md'), 'w', encoding='utf-8', newline='\n') as fh:
        fh.write('\n'.join(head + rows) + '\n' + PICKS)
    print(len(rows), 'clips', round(total, 1), 's')


if __name__ == '__main__':
    main()
