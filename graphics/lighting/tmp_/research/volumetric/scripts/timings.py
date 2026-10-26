"""Collects the GPU timings of every shot of the research runs into timings.md.

timings.py <research dir> <run name> [<run name> ...]
"""
import glob
import os
import re
import sys

root = sys.argv[1]
rows = []
for run in sys.argv[2:]:
    for log in glob.glob(os.path.join(root, 'runs', run, 'tri', '**', 'game_stdout.log'), recursive=True):
        with open(log, encoding='utf-8', errors='replace') as f:
            for line in f:
                if 'UDCAP;' not in line or 'event=shot' not in line:
                    continue
                lab = re.search(r'label=([^;]+);', line).group(1)
                g = re.search(r'gpu=(.*?)"?\s*$', line).group(1)

                def num(pat):
                    m = re.search(pat, g)
                    return m.group(1) if m else ''
                rows.append((run, lab, num(r'frame ([\d.]+) ms'), num(r'world ([\d.]+)'), num(r'shadow ([\d.]+)'),
                             num(r'post ([\d.]+)'), num(r'ao ([\d.]+)'), num(r'shafts ([\d.]+)'),
                             num(r'composite ([\d.]+)')))
out = ['# Volumetric track: GPU timings per shot', '',
       'Smoothed `triGpuTimes` averages over the 60-90 settle frames before each shot, 1440x960 render target',
       '(960x640 window, 150% scale), 4x MSAA. The `shafts` column is the volumetric pass (march, temporal',
       'resolve when on, two blur passes) when a vol variant is active, else the old screen-space shafts.',
       '**Caveat:** every run shared the GPU with up to three other agents\' games (tri global slots), so the',
       'frame and world columns swing by 2-5x between shots of the same view; compare the `shafts` column.',
       '', '| run | shot | frame | world | shadow | post | ao | shafts / vol | composite |',
       '|---|---|---|---|---|---|---|---|---|']
for r in rows:
    out.append('| ' + ' | '.join(r) + ' |')
with open(os.path.join(root, 'timings.md'), 'w', encoding='utf-8') as f:
    f.write('\n'.join(out) + '\n')
print(len(rows), 'rows')
