import sys, re, collections
exec(open(r'D:/Arma_CWA/tmp/lobo-family-wf/judge-scratch/c7/pbolib.py').read())
BS = chr(92)
path = sys.argv[1]
c = set(); prefixes = collections.Counter()
for name, status, body in read_pbo(path):
    ln = name.lower()
    if ln.endswith('.p3d') or ln.endswith('.cpp') or ln.endswith('.bin') or ln.endswith('.sqs') or ln.endswith('.rtm'):
        for s in re.findall(rb'[\x20-\x7e]{3,}\.(?:pa[ac]|jpg|tga|p3d|sqs|wss|ogg|rtm)', body, re.I):
            s = s.decode('latin1').lower().lstrip(BS)
            c.add((ln, s))
            prefixes[s.split(BS)[0] if BS in s else '(noprefix)'] += 1
for k in sorted(c): print(k)
print(prefixes)
