import json, sys, difflib
from pathlib import Path
V = Path(sys.argv[1])
ns = {}
exec((V / 'verify.py').read_text().split("report = {}")[0], ns)
parse = ns['parse']
ISSUE = Path('D:/Arma_CWA/@LoBo/.ud-backups/issue-69')
ADDONS = Path('D:/Arma_CWA/@LoBo/addons')
B1 = '20261012T034354Z-0532e837'
B2 = '20261012T042613Z-b68ba5de'
want = sys.argv[2:]  # member keys like lobo_a4/scripts/canopy.sqs
m1 = json.load(open(ISSUE / B1 / 'manifest.json', encoding='utf8'))
files = {a['file'][:-4].lower(): a['file'] for a in m1['archives']}
for full in want:
    stem, key = full.split('/', 1)
    f = files[stem]
    before = parse((ISSUE / B1 / f).read_bytes())[0].get(key)
    after = parse((ADDONS / f).read_bytes())[0].get(key)
    bt = before['data'].decode('latin1').splitlines() if before else []
    at = after['data'].decode('latin1').splitlines()
    print('#' * 10, full, 'before-compressed' if before and before['orig'] and before['orig'] != before['size'] else '')
    for line in difflib.unified_diff(bt, at, 'pre-.3', 'installed', n=1, lineterm=''):
        print(line)
