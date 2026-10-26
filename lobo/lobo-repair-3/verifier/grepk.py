import sys, re
from pathlib import Path
V = Path(sys.argv[1]); ns = {}
exec((V / 'verify.py').read_text().split("report = {}")[0], ns)
pat = re.compile(sys.argv[2], re.I)
for f in sys.argv[3:]:
    ents = ns['parse'](Path('D:/Arma_CWA/@LoBo/addons/' + f).read_bytes())[0]
    for k, e in ents.items():
        if not k.endswith(('.sqs', '.sqf', '.cpp')) or (e['orig'] and e['orig'] != e['size']):
            continue
        for i, line in enumerate(e['data'].decode('latin1').splitlines(), 1):
            if pat.search(line):
                print(f, k, i, line.strip()[:140])
