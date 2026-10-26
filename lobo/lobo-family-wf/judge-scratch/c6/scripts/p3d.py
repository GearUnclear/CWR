import sys, re
sys.path.insert(0, r'D:/Arma_CWA/tmp/lobo-family-wf/judge-scratch/c6/scripts')
from scan import entries
BS = b'\x5c'
for n, c, data in entries(sys.argv[1]):
    if n.lower().endswith('.p3d'):
        tex = set(m.group(0).lower() for m in re.finditer(rb'[A-Za-z0-9_\x5c]+\x5c[A-Za-z0-9_\x5c]+\.(?:paa|pac)', data))
        prefixes = sorted(set(t.lstrip(BS).split(BS)[0] for t in tex))
        print(n, data[:4], 'tail=', data[-24:].replace(b'\x00', b'.'), 'prefixes=', prefixes[:10])
