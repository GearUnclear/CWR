"""Parse UDCAP frame lines: per-shot engine frame counter delta and camera meter values."""
import re, sys, json
path = sys.argv[1]
rows = []
for line in open(path, encoding='utf-8', errors='replace'):
    if 'event=frame;' not in line:
        continue
    m = re.search(r'label=([^;]+);k=(\d+)', line)
    fr = re.search(r' frame=(\d+) ', line)
    cam = re.search(r'\| cam (.*?);gpu=', line)
    gpu = re.search(r'gpu=frame ([\d.]+) ms', line)
    ts = re.match(r'\[([\d\- :.]+)\]', line)
    d = {'k': int(m.group(2)), 'frame': int(fr.group(1)) if fr else None, 'ts': ts.group(1) if ts else None}
    if cam:
        for kv in cam.group(1).split():
            if '=' in kv:
                a, b = kv.split('=', 1)
                try:
                    d[a] = float(b.split('/')[0])
                except ValueError:
                    pass
    d['gpu'] = float(gpu.group(1)) if gpu else None
    rows.append(d)
out = sys.argv[2] if len(sys.argv) > 2 else None
import datetime
prev = None
deltas = []
walls = []
for r in rows:
    if prev and r['frame'] is not None and prev['frame'] is not None:
        r['dframe'] = r['frame'] - prev['frame']
        deltas.append(r['dframe'])
        t0 = datetime.datetime.strptime(prev['ts'], '%Y-%m-%d %H:%M:%S.%f')
        t1 = datetime.datetime.strptime(r['ts'], '%Y-%m-%d %H:%M:%S.%f')
        r['wall'] = (t1 - t0).total_seconds()
        walls.append(r['wall'])
    prev = r
from collections import Counter
print('shots', len(rows))
print('dframe histogram', sorted(Counter(deltas).items()))
if walls:
    walls_s = sorted(walls)
    print('wall per shot: median %.3f min %.3f max %.3f' % (walls_s[len(walls_s)//2], walls_s[0], walls_s[-1]))
if out:
    json.dump(rows, open(out, 'w'), indent=0)
