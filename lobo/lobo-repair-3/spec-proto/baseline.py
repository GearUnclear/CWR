"""Count original and pre-existing replacement lines for every 'line' rule (read-only)."""
import importlib
import sys
from pathlib import Path

sys.dont_write_bytecode = True
sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo')
sys.path.insert(0, str(Path(__file__).resolve().parent))
from repair.pbo import Archive  # noqa: E402

MOD = Path(r'D:/Arma_CWA/@LoBo/addons')
TOOLS = Path(r'D:/Arma_CWA/arma_CWR/dist/x64-win-rwdi/PoseidonTools.exe')
paths = {p.stem.lower(): p for p in MOD.glob('*.pbo')}
cache = {}
for name in sys.argv[1:] or ('loops', 'identifiers', 'content', 'weapons'):
    m = importlib.import_module('proto.' + name)
    for path, items in m.RULES.items():
        stem, key = path.split('/', 1)
        if path not in cache:
            a = Archive(paths[stem].read_bytes())
            cache[path] = a.read(key, paths[stem], TOOLS).decode('latin1').replace('\r\n', '\n')
        lines = cache[path].split('\n')
        for kind, before, after, count, *base in items:
            if kind == 'line':
                olds = (before,) if isinstance(before, str) else before
                o = sum(lines.count(x) for x in olds)
                n = lines.count(after)
            else:
                o = cache[path].count(before)
                n = cache[path].count(after)
            flag = '' if (o == count and n == (base[0] if base else 0)) else '  <-- CHECK'
            print(f'{name}:{path} {kind} sites={o}/{count} existing_new={n}{flag}  {after!r}'[:240])
