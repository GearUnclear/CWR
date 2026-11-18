"""Renames the delivered clips shot between 16:53 and 17:55 (sun 0 to 12 degrees under: dusk by
night-diagnosis.md) from "bluehour" or "night" to "dusk", and carries the rename through the per-run results
(work\\gates, work\\post, work\\judder), the takes' shots.json and work\\renames.json (new name -> old name,
read by final_sheets.py and final_report.py to find a clip's PCF log lines). Rule 8 of README.md: a clip
that is not true night is named and cut as dusk. EXTRA holds renames the pattern does not catch.

    python rename_dusk.py [--dry]
"""
import glob
import json
import os
import sys

from promopaths import ROOT

HERE = os.path.dirname(os.path.abspath(__file__))
CLIPS = os.path.join(ROOT, 'clips')
EXTRA = {'c23_night_idf_convoy_aerial_headlights': 'c23c_dusk_idf_convoy_aerial_headlights'}  # 17:35, sun 9 under


def plan():
    m = {}
    for f in glob.glob(os.path.join(CLIPS, '*_bluehour_*.mp4')):
        old = os.path.basename(f)[:-4]
        m[old] = old.replace('_bluehour_', '_dusk_')
    for old, new in EXTRA.items():
        if os.path.exists(os.path.join(CLIPS, old + '.mp4')):
            m[old] = new
    return m


def rekey(path, m):
    try:
        d = json.load(open(path, encoding='utf-8'))
    except ValueError:
        return
    if isinstance(d, dict) and 'clips' in d:
        for c in d['clips']:
            c['name'] = m.get(c['name'], c['name'])
    elif isinstance(d, dict):
        d = {m.get(k, k): v for k, v in d.items()}
    json.dump(d, open(path, 'w', encoding='utf-8'), indent=1)


def main():
    m = plan()
    for old, new in sorted(m.items()):
        print(old, '->', new)
    if '--dry' in sys.argv:
        return
    for old, new in m.items():
        os.replace(os.path.join(CLIPS, old + '.mp4'), os.path.join(CLIPS, new + '.mp4'))
    for sub, pat in (('gates', '*.json'), ('post', '*.popscan.json'), ('judder', '*.json')):
        for f in glob.glob(os.path.join(ROOT, 'work', sub, pat)):
            rekey(f, m)
    for f in glob.glob(os.path.join(HERE, 'promo_*.shots.json')):
        rekey(f, m)
    rn = os.path.join(ROOT, 'work', 'renames.json')
    old = json.load(open(rn)) if os.path.exists(rn) else {}
    old.update({v: k for k, v in m.items()})
    json.dump(old, open(rn, 'w'), indent=1)


if __name__ == '__main__':
    main()
