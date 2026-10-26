import sys, re, struct
exec(open(r'D:/Arma_CWA/tmp/lobo-family-wf/judge-scratch/c7/pbolib.py').read())
path, want = sys.argv[1], sys.argv[2].split(',')
for name, status, body in read_pbo(path):
    if name.lower() in want:
        print('==', name, status, len(body), body[:4], body[4:8].hex())
        lb = body.lower()
        for m in re.finditer(b'imuc', lb):
            s = body[max(0,m.start()-200):m.end()+200]
            print(m.start(), s)
        # list all strings with backslash
        strs = set(x.decode('latin1').lower() for x in re.findall(rb'[A-Za-z0-9_\\.\-]{4,}\[A-Za-z0-9_\\.\-]*', body))
        print(sorted(strs)[:200])
