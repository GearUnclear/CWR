"""Per delivered clip: the run that recorded it, its settle line (pin and lod), the pop scan, the judder
check (camera log and picture) and the thirds gate. Writes work\\final-report.json and prints a summary.

    python final_report.py
"""
import glob
import json
import os
import re

from promopaths import ROOT

CLIPS = os.path.join(ROOT, 'clips')
ANSI = re.compile(r'\x1b\[[0-9;]*m')


def latest(pattern, key=None):
    out = {}
    for f in sorted(glob.glob(pattern), key=os.path.getmtime):
        base = os.path.basename(f).split('.')[0]
        try:
            d = json.load(open(f, encoding='utf-8'))
        except ValueError:
            continue
        for k, v in d.items():
            if os.path.exists(os.path.join(CLIPS, k + '.mp4')) and (key is None or key(v)):
                out[k] = (base, v)
    return out


def settles(run):
    out = {}
    last = None
    for f in glob.glob(os.path.join(ROOT, 'work', 'runs', run, '*', 'game_stdout.log')):
        for ln in open(f, encoding='utf-8', errors='replace'):
            ln = ANSI.sub('', ln)
            if '[tri] triSettleScene:' in ln:
                last = ln.split('[tri] triSettleScene:', 1)[1].strip()
            m = re.search(r'"PROMO rec (\S+) ', ln)
            if m:
                out[m.group(1)] = last
                last = None
    return out


def main():
    gates = latest(os.path.join(ROOT, 'work', 'gates', '*.json'), key=lambda v: v.get('frames'))
    pops = latest(os.path.join(ROOT, 'work', 'post', '*.popscan.json'))
    judd = latest(os.path.join(ROOT, 'work', 'judder', '*.json'))
    cache = {}
    rep = {}
    rn = os.path.join(ROOT, 'work', 'renames.json')
    renames = json.load(open(rn)) if os.path.exists(rn) else {}
    for f in sorted(glob.glob(os.path.join(CLIPS, '*.mp4'))):
        name = os.path.basename(f)[:-4]
        run = gates.get(name, (None, None))[0]
        st = None
        if run:
            if run not in cache:
                cache[run] = settles(run)
            st = cache[run].get(name) or cache[run].get(renames.get(name))
        g = gates.get(name, (None, {}))[1]
        p = pops.get(name, (None, {}))[1]
        j = judd.get(name, (None, {}))[1]
        rep[name] = dict(run=run, settle=st, settled=bool(st and st.startswith('settled')),
                         pin_lod=bool(st and 'lod 0.0078125, pin 1' in st), gate=g, pop=p,
                         judder=dict(log=j.get('log', {}), video=dict(max_jerk=j.get('video', {}).get('max_jerk'),
                                                                      spikes=len(j.get('video', {}).get('spikes', [])))))
    json.dump(rep, open(os.path.join(ROOT, 'work', 'final-report.json'), 'w'), indent=1)
    n = len(rep)
    print('clips', n)
    print('pin 1 at the finest lod on every settle:', sum(r['pin_lod'] for r in rep.values()), 'of', n)
    print('settled:', sum(r['settled'] for r in rep.values()), 'of', n)
    for k, r in rep.items():
        if not r['settled']:
            print('   NOT settled:', k, (r['settle'] or '')[:90])
    print('gate pass:', sum(1 for r in rep.values() if r['gate'].get('pass')), 'of', n)
    for k, r in rep.items():
        if not r['gate'].get('pass'):
            print('   gate FAIL:', k, json.dumps({x: r['gate'].get(x) for x in ('off_s', 'off2_s', 'hz_mid_s')}))
    print('camera log: missed or doubled steps:', sum(r['judder']['log'].get('bad_dt', 0) + r['judder']['log'].get('bad_frame', 0) for r in rep.values()),
          ' tremor frames:', sum(r['judder']['log'].get('tremor_frames', 0) for r in rep.values()))
    big = {k: r['pop'].get('max_blocks', 0) for k, r in rep.items() if r['pop'].get('max_blocks', 0) >= 40}
    print('pop scan: clips with a frame of 40+ flagged blocks:', len(big))
    for k, v in sorted(big.items()):
        print('   ', k, v, r['pop'].get('big', [])[:3] if False else rep[k]['pop'].get('big', [])[:4])


if __name__ == '__main__':
    main()
