import sys, re, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from pbo import entries
from scan import lzss

BS = chr(92)
pats = ['hilux' + BS + 'rattas', 'hiluxmg' + BS + 'rattas', 'hiluxmg' + BS, 'sebnam_wp' + BS, 'm40a1' + BS, 'fm40a1' + BS, 'humr' + BS]
jobs = [
    (r'D:/Arma_CWA/@LoBo/addons/LoBoToy.pbo', ['LoBo_Ehilux.p3d', 'LoBo_Ehiluxmg.p3d', 'hiluxat.p3d']),
    (r'D:/Arma_CWA/@LoBo/addons/LoBoTer.pbo', ['LoBo_Terhilux.p3d', 'LoBo_Terhiluxmg.p3d']),
]
for pb, models in jobs:
    es, base = entries(pb)
    data = open(pb, 'rb').read()
    off = base
    for name, packing, orig, size in es:
        blob = data[off:off + size]
        off += size
        if name not in models:
            continue
        if packing:
            blob = lzss(blob, orig)
        low = blob.lower()
        print(os.path.basename(pb), name, 'magic', blob[:4], 'len', len(blob), 'packed', bool(packing))
        # LOD markers: MLOD uses P3DM per LOD, followed by resolution float at end of lod
        print('   P3DM count', low.count(b'p3dm'), 'SP3X', low.count(b'sp3x'))
        for p in pats:
            c = low.count(p.encode())
            if c:
                print('   ', p, c)
