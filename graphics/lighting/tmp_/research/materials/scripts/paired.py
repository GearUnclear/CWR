"""Paired GPU deltas: for each view, materials_X minus the remastered_X shot taken right
before it (same view, seconds apart), so slow drifts in the shared GPU's load cancel.
python paired.py <run>..."""
import re, sys, statistics
from pathlib import Path
UD = re.compile(r"UDCAP;(.*)$"); AN = re.compile(r"\x1b\[[0-9;]*m")
FR = re.compile(r"\bframe ([\d.]+) ms"); WO = re.compile(r"\bworld ([\d.]+)"); AO = re.compile(r"\bao ([\d.]+)")
for run in sys.argv[1:]:
    shots = {}
    for p in sorted((Path(run) / 'tri').rglob('game_stdout.log')):
        for raw in p.read_text(encoding='utf-8', errors='replace').splitlines():
            m = UD.search(AN.sub('', raw))
            if not m or 'event=shot' not in m.group(1): continue
            body = m.group(1); lab = re.search(r'label=([^;]+)', body).group(1)
            gpu = body.split(';gpu=', 1)[1] if ';gpu=' in body else ''
            f, w, a = FR.search(gpu), WO.search(gpu), AO.search(gpu)
            shots[lab] = (float(f.group(1)) if f else None, float(w.group(1)) if w else None, float(a.group(1)) if a else None)
    views = sorted({l.rsplit('__', 1)[0] for l in shots})
    print(f'## {run}')
    for v in views:
        pairs = []
        for suf in ('', '_b', '_c'):
            r, m = shots.get(f'{v}__remastered{suf}'), shots.get(f'{v}__materials{suf}')
            if r and m and r[0] and m[0]:
                pairs.append((m[0] - r[0], (m[1] or 0) - (r[1] or 0), (m[2] or 0) - (r[2] or 0)))
        if pairs:
            fd = [p[0] for p in pairs]; wd = [p[1] for p in pairs]; ad = [p[2] for p in pairs]
            print(f'{v:22s} n={len(pairs)} frame delta median {statistics.median(fd):+.2f} ms  all {[round(x,2) for x in fd]}  '
                  f'world {statistics.median(wd):+.2f}  ao(SSR) {statistics.median(ad):+.2f}')
