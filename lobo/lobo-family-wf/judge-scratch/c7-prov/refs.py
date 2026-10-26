import sys, re
sys.path.insert(0, r'D:/Arma_CWA/tmp/lobo-family-wf/judge-scratch/c7-prov')
from pbo import entries
BS = chr(92)
want = [w.lower() for w in sys.argv[1].split(',')]
for p in sys.argv[2:]:
    try:
        es = entries(p)
    except Exception as e:
        print(p, 'PARSE FAIL', e)
        continue
    ncomp = sum(1 for e in es if e[1])
    found = {}
    for n, comp, body in es:
        low = body.lower()
        for w in want:
            for m in re.finditer(re.escape(w.encode()), low):
                s = m.start()
                a = s
                while a > 0 and 32 <= body[a - 1] < 127 and body[a - 1] not in b'"\x00':
                    a -= 1
                b = m.end()
                while b < len(body) and 32 <= body[b] < 127 and body[b] not in b'"\x00;':
                    b += 1
                found.setdefault((n, body[a:b].decode('latin1')), 0)
                found[(n, body[a:b].decode('latin1'))] += 1
    print('##', p, 'entries', len(es), 'compressed', ncomp, 'distinct hits', len(found))
    for (n, s), c in sorted(found.items())[:60]:
        print('   ', n, '|', s[:200], 'x%d' % c)
