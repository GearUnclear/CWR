"""Run the scratch prototype's plan() (read-only) and print the changed members."""
import difflib
import sys
from pathlib import Path

sys.dont_write_bytecode = True
sys.path.insert(0, r'D:/Arma_CWA/tmp/lobo-repair-3/issues-4-5-cluster/tooling')
import repair_lobo  # noqa: E402

MOD = Path(r'D:/Arma_CWA/@LoBo')
TOOLS = Path(r'D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe')

for profile in ('ud', 'classic'):
    paths, hashes, changed, records, docs = repair_lobo.plan(MOD, TOOLS, profile)
    print(f'== profile {profile}: {len(changed)} changed members')
    for record in records:
        print('  ', record['member'], record['before_bytes'], '->', record['after_bytes'])
    if profile == 'ud':
        from repair.pbo import Archive
        for member in sorted(changed):
            stem, key = member.split('/', 1)
            archive = Archive(paths[stem].read_bytes())
            before = archive.read(key, paths[stem], TOOLS).decode('latin1').splitlines()
            after = changed[member].decode('latin1').splitlines()
            print(''.join(f'    {line}\n' for line in difflib.unified_diff(before, after, member, member, n=0, lineterm='')))
