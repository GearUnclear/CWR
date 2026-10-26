import re
from pathlib import Path

dump = Path(r'D:/Arma_CWA/tmp/lobo-repair-3/issues-4-5-cluster/dump')
BS = chr(92)


def norm(s):
    return s.replace(BS, '/').lower()


members = {norm(p.relative_to(dump).as_posix()) for p in dump.rglob('*') if p.is_file()}
classes = set()
for p in dump.rglob('config.cpp'):
    for m in re.finditer(r'(?im)^\s*class\s+(\w+)', p.read_text('latin1')):
        classes.add(m.group(1).lower())

exec_re = re.compile(r'exec\s*"+' + re.escape(BS) + r'([^"]+?\.sq[sf])"', re.I)
cv_re = re.compile(r'"(\w+)"\s*(?:createvehicle|camcreate)', re.I)
bad_exec = []
bad_cv = []
for p in dump.rglob('*'):
    if not p.is_file() or p.suffix.lower() not in ('.sqs', '.sqf', '.cpp'):
        continue
    rel = p.relative_to(dump).as_posix()
    for i, line in enumerate(p.read_text('latin1').splitlines(), 1):
        s = line.lstrip()
        if s.startswith(';') or s.startswith('//'):
            continue
        for m in exec_re.finditer(line):
            t = norm(m.group(1))
            if t not in members:
                bad_exec.append((rel, i, m.group(1)))
        for m in cv_re.finditer(line):
            n = m.group(1)
            if n.lower().startswith('lobo') and n.lower() not in classes:
                bad_cv.append((rel, i, n))
print('unresolved absolute exec targets:', len(bad_exec))
for r in sorted(bad_exec):
    print('  ', *r)
print('unresolved LoBo createVehicle/camCreate literals:', len(bad_cv))
for r in sorted(bad_cv):
    print('  ', *r)
