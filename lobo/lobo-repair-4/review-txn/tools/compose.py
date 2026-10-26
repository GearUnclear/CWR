"""Rebuild the wanted addons from each archive's first applied-batch backup, run the
current tool's plan() once, and compare the in-memory rebuilt archives to the installed bytes."""
import hashlib, json, shutil, sys
from pathlib import Path
TOOL = Path('D:/Arma_CWA/.claude/worktrees/lobo-repair-4/arma_CWR/tools/lobo')
sys.path.insert(0, str(TOOL))
import repair_lobo as r
from repair.pbo import Archive
MOD = Path('D:/Arma_CWA/@LoBo'); ISSUE = MOD/'.ud-backups'/'issue-69'
OUT = Path(sys.argv[1]); compat = sys.argv[2] if len(sys.argv) > 2 else 'ud'
ASSET = Path('D:/LoBo-Reloaded/sources/recovered-2026-10-11/derived/TheHKpack-1.75.pbo')
TOOLS = Path('D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe')
sha = lambda b: hashlib.sha256(b).hexdigest()
first = {}
for d in sorted(ISSUE.iterdir()):
    m = json.loads((d/'manifest.json').read_text(encoding='utf8'))
    if m['state'] != 'applied': continue
    for a in m['archives']:
        first.setdefault(a['file'].lower(), (d/a['file'], a['before_sha256']))
installed = {p.stem.lower(): p for p in (MOD/'addons').glob('*.pbo')}
wanted = set().union(*(set(m.ADDONS) for m in r.modules()))
addons = OUT/'addons'; addons.mkdir(parents=True, exist_ok=True)
src = {}
for stem in sorted(wanted):
    name = installed[stem].name
    if name.lower() in first:
        p, before = first[name.lower()]
        assert sha(p.read_bytes()) == before, p
        src[stem] = ('first-backup', str(p))
    else:
        p = installed[stem]; src[stem] = ('installed-untouched', str(p))
    t = addons/name
    if not t.exists() or t.stat().st_size != p.stat().st_size: shutil.copyfile(p, t)
paths, hashes, changed, records, docs, skipped = r.plan(OUT, TOOLS, compat, ASSET)
added = {rec['member']: rec['stored_name'] for rec in records if rec.get('added')}
grouped = {}
for full, data in changed.items():
    stem, key = full.split('/', 1); e = grouped.setdefault(stem, ({}, {}))
    (e[1].__setitem__(added[full], data) if full in added else e[0].__setitem__(key, data))
res = {'compat': compat, 'changed_members': len(changed), 'added': len(added), 'skipped': skipped, 'archives': {}}
bad = []
for stem in sorted(wanted):
    raw = (addons/installed[stem].name).read_bytes()
    out = Archive(raw).replace(*grouped[stem]) if stem in grouped else raw
    inst = sha(installed[stem].read_bytes())
    res['archives'][stem] = {'source': src[stem][0], 'rebuilt': sha(out), 'installed': inst, 'equal': sha(out) == inst}
    if sha(out) != inst: bad.append(stem)
res['mismatch'] = bad
res['rule_sha256'] = r.rule_fingerprint()
(OUT/('compose-%s.json' % compat)).write_text(json.dumps(res, indent=1), encoding='utf8')
print(json.dumps({k: v for k, v in res.items() if k != 'archives'}, indent=1))
