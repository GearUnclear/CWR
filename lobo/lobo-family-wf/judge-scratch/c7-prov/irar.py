import sys, re
sys.path.insert(0, r'D:/Arma_CWA/tmp/lobo-family-wf/judge-scratch/c7-prov')
from pbo import entries
BS = chr(92)
p = sys.argv[1]
es = entries(p)
for n, comp, body in es:
    print(n, len(body), 'comp' if comp else '')
for n, comp, body in es:
    if n.lower().startswith('config'):
        print('=== ', n)
        print(body[:120])
        strs = set(m.group().decode('latin1') for m in re.finditer(rb'[\x20-\x7e]{4,}', body))
        for s in sorted(strs):
            sl = s.lower()
            if BS in s or 'requ' in sl or 'imuc' in sl or sl.startswith(('irar', 'bis', 'jam', 'imuc')):
                print('  ', s)
for n, comp, body in es:
    if n.endswith('.p3d') and b'IMUC' in body:
        i = body.find(b'IMUC')
        seg = body[max(0, i - 600):i + 40]
        print('---', n, [s.decode('latin1') for s in re.findall(rb'[\x20-\x7e]{3,}', seg)])
prefixes = {}
for n, comp, body in es:
    if n.endswith('.p3d'):
        for m in re.finditer(rb'[A-Za-z0-9_\\]+\.(?:pa[ac]|p3d)', body):
            s = m.group().decode('latin1').lower().lstrip(BS)
            pre = s.split(BS)[0] if BS in s else '(local)'
            prefixes.setdefault(pre, set()).add(n)
for k, v in sorted(prefixes.items()):
    print('prefix', k, len(v), sorted(v)[:5])
