import json, sys, hashlib, subprocess
from pathlib import Path
V = Path(sys.argv[1])
ISSUE = Path('D:/Arma_CWA/@LoBo/.ud-backups/issue-69')
TOOL = 'D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe'
src = (V / 'verify.py').read_text().split("report = {}")[0]
ns = {}
exec(src, ns)
parse = ns['parse']
out = V / 'extract'
for b in ['20261012T034354Z-0532e837', '20261012T042613Z-b68ba5de']:
    m = json.load(open(ISSUE / b / 'manifest.json', encoding='utf8'))
    files = {a['file'][:-4].lower(): a['file'] for a in m['archives']}
    for r in m['members']:
        if r.get('added'):
            continue
        stem, key = r['member'].split('/', 1)
        f = files[stem]
        ent = parse((ISSUE / b / f).read_bytes())[0][key]
        if ent['orig'] and ent['orig'] != ent['size']:
            d = out / b[-8:] / stem
            base = key.split('/')[-1]
            subprocess.run([TOOL, 'pbo', 'extract', '-f', base, str(ISSUE / b / f), str(d)], capture_output=True)
            cands = [p for p in d.rglob('*') if p.is_file() and p.relative_to(d).as_posix().lower() == key]
            got = hashlib.sha256(cands[0].read_bytes()).hexdigest() if cands else None
            print(b[-8:], r['member'], 'compressed', 'OK' if got == r['before_sha256'] else 'MISMATCH ' + str(got),
                  'len', cands[0].stat().st_size if cands else None, r['before_bytes'])
