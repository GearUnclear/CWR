r"""Reads notes back out of the strips that stems.py rendered.

    from strips import note
    x = note('cmi_wolf', 41)            # mono samples of that note (its slot, trailing silence trimmed)

    python strips.py                    # table of every note in every strip: length and what it measures like
    python strips.py cmi_wolf brs_growl # only these, and a picture of each under renders\pictures
"""
import json
import os
import sys

import numpy as np

import sndlib as S
from rjob import NOTES

STRIPS = os.path.join(NOTES, 'renders', 'strips')
_meta = None
_cache = {}


def meta():
    global _meta
    if _meta is None:
        with open(os.path.join(STRIPS, 'strips.json')) as fh:
            _meta = json.load(fh)
    return _meta


def whole(name):
    if name not in _cache:
        x, loss = S.load_mono(os.path.join(STRIPS, name + '.wav'))
        _cache[name] = (x, loss)
    return _cache[name]


def note(name, pitch, trim=True, db=-55):
    m = meta()[name]
    i = m['notes'].index(pitch)
    x, _ = whole(name)
    a = int((m['first'] + i * m['slot'] - 0.02) * S.SR)
    b = int((m['first'] + (i + 1) * m['slot'] - 0.05) * S.SR)
    seg = x[max(a, 0):b].copy()
    if trim:
        thr = 10 ** (db / 20) * (np.max(np.abs(seg)) + 1e-12)
        idx = np.where(np.abs(seg) > thr)[0]
        if len(idx):
            seg = seg[: min(len(seg), idx[-1] + int(0.05 * S.SR))]
    return seg


if __name__ == '__main__':
    names = sys.argv[1:] or sorted(meta())
    os.makedirs(os.path.join(NOTES, 'renders', 'pictures'), exist_ok=True)
    for n in names:
        if not os.path.exists(os.path.join(STRIPS, n + '.wav')):
            print(f'{n}: NOT RENDERED')
            continue
        m = meta()[n]
        x, loss = whole(n)
        print(f"== {n}: {m['instrument']} / {m['preset']}  (mono fold loss {loss:.1f} dB)")
        for p in m['notes']:
            seg = note(n, p)
            if np.max(np.abs(seg)) < 1e-4:
                print(f'   note {p}: SILENT')
                continue
            print(f'   note {p:3d} {S.describe(seg)}')
        if sys.argv[1:]:
            S.picture([(x, S.SR, f"{n}: {m['instrument']} / {m['preset']} notes {m['notes']}")],
                      os.path.join(NOTES, 'renders', 'pictures', f'strip_{n}.png'))
