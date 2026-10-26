import sys, re, glob, os
sys.path.insert(0, r'D:/Arma_CWA/tmp/lobo-family-wf/judge-scratch/c6/scripts')
from scan import entries
BS = b'\x5c'
for path in sys.argv[1:]:
    for p in sorted(glob.glob(path)):
        ents = entries(p)
        for n, c, data in ents:
            if n.lower() in ('config.cpp','config.bin'):
                low = data.lower()
                has = b'cfgfaces' in low
                refs = sorted(set(m.group(0).decode('latin1') for m in re.finditer(rb'[A-Za-z0-9_\x5c]*face[A-Za-z0-9_\x5c]*\.(?:paa|pac|jpg)', low)))
                print(os.path.basename(p), n, 'CfgFaces' if has else '', 'compressed' if c else '', len(refs), refs[:12])
