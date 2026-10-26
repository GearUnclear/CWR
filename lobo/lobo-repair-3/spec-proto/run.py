"""SPEC PROTOTYPE harness (read-only on @LoBo): composes the shipped .1/.2 rules
with the proposed .3 rules exactly like repair_lobo.plan(), checks the fixed
point, stages every changed archive in memory and reports sizes and hashes.
Usage: python -B run.py [ud|classic] [--asset-source PATH] [--out report.json]
"""
import hashlib
import importlib
import json
import sys
from pathlib import Path

sys.dont_write_bytecode = True
sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo')
sys.path.insert(0, str(Path(__file__).resolve().parent))
from repair.pbo import Archive  # noqa: E402
from proto import archive_ext  # noqa: E402

MOD = Path(r'D:/Arma_CWA/@LoBo')
TOOLS = Path(r'D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe')
OLD = ('ammo', 'scripts', 'models', 'followups')
NEW = ('loops', 'mines', 'clusters', 'numbers', 'effects', 'identifiers', 'content', 'weapons', 'textures')


def sha(b):
    return hashlib.sha256(b).hexdigest()


def main():
    args = sys.argv[1:]
    compat = args[0] if args and args[0] in ('ud', 'classic') else 'ud'
    asset = Path(args[args.index('--asset-source') + 1]) if '--asset-source' in args else None
    out = Path(args[args.index('--out') + 1]) if '--out' in args else None
    mods = [importlib.import_module('repair.' + n) for n in OLD] + [importlib.import_module('proto.' + n) for n in NEW]
    wanted = set().union(*(set(m.ADDONS) for m in mods))
    model_keys = set().union(*(set(getattr(m, 'MODEL_MEMBERS', ())) for m in mods))
    added_keys = set().union(*(set(getattr(m, 'ADDED_MEMBERS', ())) for m in mods))
    paths = {p.stem.lower(): p for p in (MOD / 'addons').glob('*.pbo')}
    if '--pristine' in args:
        # Reinstall scenario: the earliest verified backup of every archive the
        # .1/.2 batches changed, the installed bytes for everything else.
        for batch in sorted((MOD / '.ud-backups' / 'issue-69').iterdir(), reverse=True):
            for f in batch.glob('*.pbo'):
                paths[f.stem.lower()] = f
    missing = wanted - paths.keys()
    assert not missing, missing
    docs, binary, original, existing_added, archives = {}, {}, {}, {}, {}
    for stem in sorted(wanted):
        raw = paths[stem].read_bytes()
        a = Archive(raw)
        archives[stem] = a
        for key in a.by_key:
            full = stem + '/' + key
            if key == 'config.cpp' or key.endswith(('.sqs', '.sqf')):
                data = a.read(key, paths[stem], TOOLS)
                docs[full] = data.decode('latin1')
                original[full] = data
            elif full in model_keys:
                data = a.read(key, paths[stem], TOOLS)
                binary[full] = data
                original[full] = data
            elif full in added_keys:
                existing_added[full] = a.read(key, paths[stem], TOOLS)
    changed_by = {}

    def run(m):
        opts = {'compatibility': compat} if m.__name__ == 'repair.scripts' else {}
        edits = m.repair(dict(docs), **opts)
        assert not set(edits) - docs.keys(), m.__name__
        return edits

    for m in mods:
        edits = run(m)
        for k, v in edits.items():
            if v != docs[k]:
                changed_by.setdefault(k, []).append(m.__name__)
        docs.update(edits)
        if hasattr(m, 'repair_models'):
            e2 = m.repair_models(dict(binary))
            for k, v in e2.items():
                if v != binary[k]:
                    changed_by.setdefault(k, []).append(m.__name__)
            binary.update(e2)
    for m in mods:
        edits = run(m)
        bad = [k for k, v in edits.items() if docs.get(k) != v]
        assert not bad, (m.__name__, bad)
        if hasattr(m, 'repair_models'):
            e2 = m.repair_models(dict(binary))
            assert not [k for k, v in e2.items() if binary.get(k) != v], m.__name__
    additions, skipped = {}, []
    for m in mods:
        if hasattr(m, 'add_members'):
            mine = {k: v for k, v in existing_added.items() if k in m.ADDED_MEMBERS}
            add, skip = m.add_members(mine, asset)
            additions.update(add)
            skipped += skip
            # fixed point: with the additions present, nothing more to add
            again, skip2 = m.add_members({**mine, **{k: v[1] for k, v in add.items()}}, asset)
            assert not again and (skip2 == skip if not add else not skip2)
    changed = {k: v.encode('latin1') for k, v in docs.items() if v.encode('latin1') != original[k]}
    changed.update({k: v for k, v in binary.items() if v != original[k]})
    members = []
    for k, v in sorted(changed.items()):
        members.append({'member': k, 'by': changed_by.get(k), 'before_bytes': len(original[k]), 'after_bytes': len(v),
                        'before_sha256': sha(original[k]), 'after_sha256': sha(v)})
    for k, (stored, v) in sorted(additions.items()):
        members.append({'member': k, 'by': ['added'], 'stored_name': stored, 'before_bytes': 0, 'after_bytes': len(v),
                        'before_sha256': None, 'after_sha256': sha(v)})
    grouped = {}
    for k, v in changed.items():
        stem, key = k.split('/', 1)
        grouped.setdefault(stem, ({}, {}))[0][key] = v
    for k, v in additions.items():
        grouped.setdefault(k.split('/', 1)[0], ({}, {}))[1][k] = v
    staged = []
    for stem, (repl, add) in sorted(grouped.items()):
        a = archives[stem]
        result = archive_ext.replace(a, repl, add)
        b = Archive(result)
        for key, v in repl.items():
            assert bytes(b.stored(b.by_key[key])) == v
        staged.append({'archive': paths[stem].name, 'before_sha256': sha(a.data), 'after_sha256': sha(result),
                       'before_bytes': len(a.data), 'after_bytes': len(result), 'members_changed': len(repl),
                       'members_added': len(add), 'stuffpbo': a.stuffpbo, 'checksummed': a.checksummed,
                       'untouched_members': len(a.entries) - len(repl)})
    report = {'compatibility': compat, 'asset_source': str(asset) if asset else None, 'skipped': skipped,
              'changed_members': len(changed) + len(additions), 'archives': staged, 'members': members}
    if '--dump' in args:
        dump = Path(args[args.index('--dump') + 1])
        for k, v in changed.items():
            for side, data in (('before', original[k]), ('after', v)):
                target = dump / side / k
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(data)
    report['final_sha256'] = {k: sha(v.encode('latin1')) for k, v in docs.items() if k in changed}
    report['final_sha256'].update({k: sha(v) for k, v in binary.items() if k in changed})
    text = json.dumps(report, indent=1)
    if out:
        out.write_text(text + '\n', encoding='utf8')
    print(json.dumps({k: v for k, v in report.items() if k not in ('members', 'archives')}, indent=1))
    print('archives', len(staged), 'members', len(members))
    for s in staged:
        print(' ', s['archive'], s['members_changed'], '+', s['members_added'], s['before_bytes'], '->', s['after_bytes'])


if __name__ == '__main__':
    main()
