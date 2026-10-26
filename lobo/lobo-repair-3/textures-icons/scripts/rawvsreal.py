"""Classify every raw byte occurrence of a needle in MLOD models as a live
face texture (field starts with it), dead residue after the NUL in a face
texture field, or outside any face texture field.

Usage: rawvsreal.py <scriptsdir> <needle> model.p3d [...]
"""
import sys
sys.path.insert(0, sys.argv[1])
import mlodtex

needle = sys.argv[2].lower().encode('latin-1')
total = {'live': 0, 'dead': 0, 'outside': 0}
for path in sys.argv[3:]:
    d = open(path, 'rb').read()
    low = d.lower()
    ls, end, size = mlodtex.lods(d)
    fields = []
    for l in ls:
        for off, name, raw in l['textures']:
            fields.append((off, l['index'], mlodtex.resname(l['resolution']), name))
    fields.sort()
    import bisect
    starts = [f[0] for f in fields]
    counts = {'live': 0, 'dead': 0, 'outside': 0}
    live = {}
    dead_lods = {}
    i = low.find(needle)
    while i != -1:
        k = bisect.bisect_right(starts, i) - 1
        if k >= 0 and fields[k][0] <= i < fields[k][0] + 32:
            off, li, kind, name = fields[k]
            if needle in name.lower().encode('latin-1') and name.lower().encode('latin-1').find(needle) == i - off:
                counts['live'] += 1
                live.setdefault((li, kind, name), 0)
                live[(li, kind, name)] += 1
            else:
                counts['dead'] += 1
                dead_lods.setdefault((li, kind, repr(name)), 0)
                dead_lods[(li, kind, repr(name))] += 1
        else:
            counts['outside'] += 1
        i = low.find(needle, i + 1)
    for k in total:
        total[k] += counts[k]
    print('==', path, counts)
    for (li, kind, name), n in sorted(live.items()):
        print(f'   LIVE LOD {li:2d} {kind:<14} {name} x{n}')
    for (li, kind, name), n in sorted(dead_lods.items()):
        print(f'   dead LOD {li:2d} {kind:<14} live-field={name} x{n}')
print('TOTAL', total)
