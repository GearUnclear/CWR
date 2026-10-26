"""For a source LOD's textures (matching a needle), list the named selections their
faces belong to, and for a reference LOD the texture distribution per selection.

Usage: selmap.py <scriptsdir> model.p3d src_lod ref_lod needle
"""
import sys
sys.path.insert(0, sys.argv[1])
from emptyfaces_lib import parse
from collections import Counter, defaultdict
lods = parse(open(sys.argv[2], 'rb').read())
src, ref = lods[int(sys.argv[3])], lods[int(sys.argv[4])]
needle = sys.argv[5].lower()


def sel_of(lod, k):
    return sorted(s for s, w in lod['sels'].items() if w[k] and not s.lower().startswith('proxy'))


bytex = defaultdict(Counter)
for k, f in enumerate(src['faces']):
    if needle in f['name'].lower():
        for s in sel_of(src, k):
            bytex[f['name']][s] += 1
refsel = defaultdict(Counter)
for k, f in enumerate(ref['faces']):
    for s in sel_of(ref, k):
        refsel[s][f['name']] += 1
for t, c in bytex.items():
    print('SRC', t)
    for s, n in c.most_common(6):
        print(f'   sel {s:<16} {n:4d}   ref-lod textures: {refsel[s].most_common(4)}')
