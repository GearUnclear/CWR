import sys, re
sys.path.insert(0, r'D:/Arma_CWA/tmp/lobo-family-wf/judge-scratch/c6/scripts')
from scan import entries
path, member, pat = sys.argv[1], sys.argv[2].lower(), sys.argv[3].lower().encode()
for name, comp, data in entries(path):
    if name.lower() != member: continue
    print(name, 'compressed' if comp else 'stored', len(data))
    for m in re.finditer(re.escape(pat), data.lower()):
        s = max(0, m.start()-200); e = m.end()+120
        print(m.start(), repr(data[s:e]))
