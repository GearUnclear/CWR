import sys, json, hashlib
from pathlib import Path
V = Path(sys.argv[1])
ns = {}
exec((V / 'verify.py').read_text().split("report = {}")[0], ns)
parse = ns['parse']
ISSUE = Path('D:/Arma_CWA/@LoBo/.ud-backups/issue-69')
A = Path('D:/Arma_CWA/@LoBo/addons')
B1 = '20261012T034354Z-0532e837'
m = json.load(open(ISSUE / B1 / 'manifest.json', encoding='utf8'))
def tailinfo(raw):
    ents, order, props, tail = parse(raw)
    valid = len(tail) == 21 and tail[0] == 0 and hashlib.sha1(raw[:-20]).digest() == tail[1:]
    return len(tail), valid, tail
summary = {}
for a in m['archives']:
    f = a['file']
    b = (ISSUE / B1 / f).read_bytes(); x = (A / f).read_bytes()
    bt = tailinfo(b); at = tailinfo(x)
    kind = ('sha1-valid' if bt[1] else f'tail{bt[0]}') + ' -> ' + ('sha1-valid' if at[1] else f'tail{at[0]}')
    if not bt[1] and bt[0] and bt[2] != at[2]:
        kind += ' (non-sha1 tail CHANGED)'
    elif not bt[1] and bt[0]:
        kind += ' (non-sha1 tail kept)'
    summary.setdefault(kind, []).append(f + ('' if len(b) != len(x) else ' [same length]'))
for k, v in summary.items():
    print(k, len(v))
    if 'sha1-valid -> sha1-valid' not in k:
        print('   ', v)
