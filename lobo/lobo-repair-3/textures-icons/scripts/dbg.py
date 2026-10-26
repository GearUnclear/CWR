import struct, sys
sys.path.insert(0, sys.argv[1])
import mlodtex
d = open(sys.argv[2], 'rb').read()
ls, end, size = mlodtex.lods(d)
target = d.lower().find(b'yk_ussol')
print('first hit', target)
for l in ls:
    offs = [t[0] for t in l['textures']]
    lo = offs[0] if offs else None
    hi = offs[-1] + 104 if offs else None
    names = sorted({t[1] for t in l['textures']})
    print(l['index'], mlodtex.resname(l['resolution']), l['faces'], lo, hi, len(names))
    if lo is not None and lo <= target < hi:
        print('  target in this lod; names sample', names[:80])
