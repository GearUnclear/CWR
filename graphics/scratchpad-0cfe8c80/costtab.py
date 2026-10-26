"""costtab.py RUN: tip (look lanes off) vs overhaul GPU cost per view, mean of two interleaved rounds."""
import re, sys, glob, os, statistics as st
d = {}
for log in glob.glob(os.path.join(sys.argv[1], 'tri', '*', 'game_stdout.log')):
    for line in open(log, encoding='utf-8', errors='replace'):
        if 'event=shot' not in line:
            continue
        lab = re.search(r'label=([^;]+)', line).group(1)
        view, mode = lab.split('__')
        g = re.search(r'gpu=frame ([0-9.]+) ms \| world ([0-9.]+)(?:.*?\| post ([0-9.]+): ([^|]*))?', line)
        if not g:
            continue
        secs = dict((k, float(v)) for k, v in re.findall(r'([a-z]+) ([0-9.]+)', g.group(4) or ''))
        rec = dict(frame=float(g.group(1)), world=float(g.group(2)), post=float(g.group(3) or 0), **secs)
        d.setdefault(view, {}).setdefault(mode[:-1], []).append(rec)
print('| view | GPU frame tip -> overhaul (ms) | delta | world delta | post delta | ao | air | composite |')
print('|---|---|---|---|---|---|---|---|')
for view, m in d.items():
    if 'tip' not in m or 'ov' not in m:
        continue
    def mean(mode, k):
        return st.mean(r.get(k, 0.0) for r in m[mode])
    ft, fo = mean('tip', 'frame'), mean('ov', 'frame')
    print(f"| {view} | {ft:.2f} -> {fo:.2f} | {fo - ft:+.2f} | {mean('ov','world') - mean('tip','world'):+.2f} | "
          f"{mean('ov','post') - mean('tip','post'):+.2f} | {mean('tip','ao'):.2f} -> {mean('ov','ao'):.2f} | "
          f"{mean('tip','air'):.2f} -> {mean('ov','air'):.2f} | {mean('tip','composite'):.2f} -> {mean('ov','composite'):.2f} |")
