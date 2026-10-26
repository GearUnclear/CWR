"""Print PBO member timestamps for the Mk-20/RBK-500 family (installed + .1 backup originals)."""
import datetime
import sys
from pathlib import Path

sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo')
from repair.pbo import Archive  # noqa: E402

ADDONS = Path(r'D:/Arma_CWA/@LoBo/addons')
BACKUP = Path(r'D:/Arma_CWA/@LoBo/.ud-backups/issue-69/20261011T105837Z-33d55270')
WANTED = {
    'LoBo_A4.pbo': ['scripts/firemissiles_a2.sqs', 'scripts/firemissiles_a3.sqs', 'config.cpp'],
    'LoBo_F4.pbo': ['scripts/firemissiles_ww.sqs', 'config.cpp'],
    'LoBoIL28.pbo': ['script/fire_mk20.sqs', 'script/rockeye.sqs', 'config.cpp'],
    'LoboF5.pbo': ['scripts/firemissiles2.sqs', 'scripts/rockeye.sqs', 'config.cpp'],
    'LoBoF15.pbo': ['scr/fire_e_cbu.sqs', 'scr/fire_e_mav.sqs', 'scr/cluster.sqs', 'config.cpp'],
    'LoBoF16.pbo': ['scr/firemissiles_ag.sqs', 'scr/firemissiles_f16i_cas.sqs', 'config.cpp'],
    'LoBo_Kfir.pbo': ['scripts/firemissiles_cas1.sqs'],
    'LoBoMig.pbo': ['scripts/firemissiles_cas1.sqs', 'scripts/firemissiles_cas2.sqs', 'scripts/syria/firemissiles_cas1.sqs', 'config.cpp'],
    'LoBo_airammo.pbo': ['scripts/cbu/rockeye.sqs', 'scripts/cbu/rbk500.sqs', 'lobo_mk20_d.p3d', 'lobo_mk20bomblet.p3d', 'lobo_rbk500.p3d', 'config.cpp'],
}
for name, keys in WANTED.items():
    source = BACKUP / name if (BACKUP / name).exists() else ADDONS / name
    archive = Archive(source.read_bytes())
    for key in keys:
        entry = archive.by_key.get(key)
        if entry is None:
            print(f'{name:18} {key:40} MISSING')
            continue
        stamp = datetime.datetime.fromtimestamp(entry.fields[3], datetime.timezone.utc).date()
        origin = 'backup' if source.parent == BACKUP else 'installed'
        print(f'{name:18} {key:40} {stamp} ({origin})')
