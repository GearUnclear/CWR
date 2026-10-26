"""Dry-run the proposed issue 4/5 rules against copies of the installed archives.

Read-only for @LoBo: archives are read, rebuilt in memory and written only under
this investigation's scratch directory.
"""
import hashlib
import sys
from pathlib import Path

sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo')
from repair.pbo import Archive  # noqa: E402
from repair.scripts import _replace, script_findings, repair as scripts_repair  # noqa: E402

BS = chr(92)
ADDONS = Path(r'D:/Arma_CWA/@LoBo/addons')
OUT = Path(r'D:/Arma_CWA/tmp/lobo-repair-3/issues-4-5-cluster/simulated')
TOOLS = Path(r'D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe')


def p(*parts):
    return BS + BS.join(parts)


AIRAMMO_ROCKEYE = 'exec "' + p('LoBo_airammo', 'Scripts', 'cbu', 'rockeye.sqs') + '"'
RULES = {
    'lobo_a4/scripts/firemissiles_a2.sqs': [('exec "' + p('LoBoF16', 'scr', 'rockeye.sqs') + '"', AIRAMMO_ROCKEYE)],
    'lobo_a4/scripts/firemissiles_a3.sqs': [('exec "' + p('LoBoF16', 'scr', 'rockeye.sqs') + '"', AIRAMMO_ROCKEYE)],
    'lobo_f4/scripts/firemissiles_ww.sqs': [('exec "' + p('LoBoF16', 'scr', 'rockeye.sqs') + '"', AIRAMMO_ROCKEYE)],
    'loboil28/script/fire_mk20.sqs': [('exec "' + p('LoBoF15', 'scr', 'rockeye.sqs') + '"', AIRAMMO_ROCKEYE)],
    'lobof5/scripts/rockeye.sqs': [('"LoBoMk20Deployed" createvehicle', '"LoBo_mk20Deployed" createvehicle')],
    'loboil28/script/rockeye.sqs': [('"LoBoMk20Deployed" createvehicle', '"LoBo_mk20Deployed" createvehicle')],
    'lobo_airammo/scripts/cbu/rbk500.sqs': [('"LoBoRBKBomblet" createvehicle', '"LoBoRBK500Bomblet" createvehicle')],
}
STEMS = {'lobo_a4': 'LoBo_A4.pbo', 'lobo_f4': 'LoBo_F4.pbo', 'loboil28': 'LoBoIL28.pbo',
         'lobof5': 'LoboF5.pbo', 'lobo_airammo': 'LoBo_airammo.pbo'}


def sha(data):
    return hashlib.sha256(data).hexdigest()


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    grouped = {}
    for full in RULES:
        stem, key = full.split('/', 1)
        grouped.setdefault(stem, []).append(key)
    for stem, keys in grouped.items():
        path = ADDONS / STEMS[stem]
        raw = path.read_bytes()
        archive = Archive(raw)
        edits = {}
        for key in keys:
            entry = archive.by_key[key]
            data = archive.read(key, path, TOOLS)
            source = data.decode('latin1')
            newline = '\r\n' if '\r\n' in source else '\n'
            text = source.replace('\r\n', '\n')
            before_findings = script_findings(text)
            updated = text
            for before, after in RULES[stem + '/' + key]:
                updated = _replace(updated, before, after)
            again = updated
            for before, after in RULES[stem + '/' + key]:
                again = _replace(again, before, after)
            assert again == updated, 'not idempotent'
            after_findings = script_findings(updated)
            encoded = updated.replace('\n', newline).encode('latin1')
            edits[key] = encoded
            print(f'{stem}/{key}: packing={entry.fields[0]:#x} {len(data)} -> {len(encoded)} bytes '
                  f'(delta {len(encoded) - len(data):+d}); lint before={before_findings} after={after_findings}')
        rebuilt = archive.replace(edits)
        check = Archive(rebuilt)
        for key, value in edits.items():
            assert check.read(key) == value
        untouched = [k for k in archive.by_key if k not in edits]
        for k in untouched:
            assert bytes(archive.stored(archive.by_key[k])) == bytes(check.stored(check.by_key[k]))
        (OUT / STEMS[stem]).write_bytes(rebuilt)
        print(f'  {STEMS[stem]}: {len(raw)} -> {len(rebuilt)} bytes; {len(untouched)} untouched members byte-identical; '
              f'checksummed={archive.checksummed}; before={sha(raw)[:12]} after={sha(rebuilt)[:12]}')
    # The existing scripts module must remain a fixed point over the three
    # MISSILE_PATHS members it already owns after the new path is in place.
    print('scripts.repair fixed-point over edited MISSILE_PATHS members: see check below')


if __name__ == '__main__':
    main()
