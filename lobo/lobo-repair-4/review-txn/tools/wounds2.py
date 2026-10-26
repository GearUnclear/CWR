import re, sys
from pathlib import Path
sys.path.insert(0, 'D:/Arma_CWA/.claude/worktrees/lobo-repair-4/arma_CWR/tools/lobo')
from repair.pbo import Archive
BS = chr(92)
a = Archive(Path(sys.argv[1]).read_bytes())
keys = set(a.by_key)
t = a.read('config.cpp').decode('latin1').replace('\r\n', '\n')
lines = t.split('\n')
for i, l in enumerate(lines, 1):
    if 'wounds' in l.lower():
        block = lines[i-1:i+4]
        print('== line', i)
        for j, b in enumerate(block):
            print('  ', i + j, repr(b[:400]))
        text = ' '.join(block)
        seg = text[text.lower().index('wounds'):]
        seg = seg[:seg.index('}') + 1] if '}' in seg else seg
        names = re.findall(r'"([^"]+)"', seg)
        for n in names:
            k = n.lstrip(BS).replace(BS, '/').lower()
            stem, rest = k.split('/', 1) if '/' in k else ('', k)
            ok = stem == 'lobo_pilot' and rest in keys
            print('     ', n, 'OK' if ok else ('MISSING' if stem == 'lobo_pilot' else 'external'))
