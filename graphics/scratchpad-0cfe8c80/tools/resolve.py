"""resolve.py <file> <spec.py>: spec defines R = list of resolutions, one per conflict in order.
Each entry: 'ours', 'theirs', 'both' (ours then theirs), 'theirs+ours', or a literal string (replacement text, must end with newline)."""
import sys, re
path, spec = sys.argv[1], sys.argv[2]
ns = {}
exec(open(spec, encoding='utf-8').read(), ns)
R = ns['R']
src = open(path, encoding='utf-8', newline='').read()
nl = '\r\n' if '\r\n' in src else '\n'
lines = src.split(nl)
out, i, k = [], 0, 0
while i < len(lines):
    l = lines[i]
    if l.startswith('<<<<<<< '):
        j = i + 1
        ours = []
        while not lines[j].startswith('======='):
            ours.append(lines[j]); j += 1
        j += 1
        theirs = []
        while not lines[j].startswith('>>>>>>> '):
            theirs.append(lines[j]); j += 1
        r = R[k]; k += 1
        if r == 'ours': out += ours
        elif r == 'theirs': out += theirs
        elif r == 'both': out += ours + theirs
        elif r == 'theirs+ours': out += theirs + ours
        else:
            t = r[:-1] if r.endswith('\n') else r
            out += t.split('\n')
        i = j + 1
    else:
        out.append(l); i += 1
assert k == len(R), f'{k} conflicts, {len(R)} resolutions'
open(path, 'w', encoding='utf-8', newline='').write(nl.join(out))
print(path, k, 'resolved')
