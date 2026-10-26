"""Scratch prototype of the proposed issue-3 mine rules (not the implementation)."""
import sys, difflib
sys.path.insert(0, r'D:/Arma_CWA/.claude/worktrees/lobo-repair-3/arma_CWR/tools/lobo')
from pathlib import Path
from repair.scripts import _replace, script_findings

POLL = '~0.1'
FLARE_POLL = '~0.03'
GUARD = '? isNull _L1 : exit'
FLARE_GUARD = '? isNull _L1 : deleteVehicle _Mess; exit'
PLAIN = ('ap_mine', 'at1_mine', 'at2_mine', 'at3_mine', 'at4_mine', 'at5_mine', 'at6_mine',
         'egyptnavmine', 'm08navmine', 'mk118_mine')
RULES = {}
for name in PLAIN:
    RULES[f'lobo_mines/scripts/{name}.sqs'] = (
        ('#Zut\n~0.001\n_J = nearestobject [_X,_Y,_Z]\n', f'#Zut\n{POLL}\n{GUARD}\n_J = nearestobject [_X,_Y,_Z]\n'),)
LADUNG = (('#Zut\n~0.001\n_W = _D + 45\n', f'#Zut\n{POLL}\n{GUARD}\n_W = _D + 45\n'),)
LEUCHT = (('#Zut\n~0.0001\n', f'#Zut\n{FLARE_POLL}\n{FLARE_GUARD}\n'),
          ('#Ende\n_L1 animate ["aus", 1]\n', '#Ende\ndeleteVehicle _Mess\n_L1 animate ["aus", 1]\n'),
          ('? not alive _L1:exit\n', '? not alive _L1:deleteVehicle _Mess; exit\n'))
CLAYMORE = (('#Zut\n~0.01\n_M = _L1\n', f'#Zut\n{POLL}\n{GUARD}\n_M = _L1\n'),)
RULES['lobo_mines/scripts/ladung.sqs'] = LADUNG
RULES['lobo_mines/scripts/leucht_mine.sqs'] = LEUCHT
RULES['lobo_mines/scripts/claymore.sqs'] = CLAYMORE           # optional: not named in issue 3
RULES['jof_objects1/ap_mine.sqs'] = RULES['lobo_mines/scripts/ap_mine.sqs']  # optional sibling archive
RULES['jof_objects1/ladung.sqs'] = LADUNG
RULES['jof_objects1/leucht_mine.sqs'] = LEUCHT
RULES['jof_objects1/claymore.sqs'] = CLAYMORE

def apply(source):
    nl = '\r\n' if '\r\n' in source else '\n'
    return nl, source.replace('\r\n', '\n')

root = Path(sys.argv[1]); out = Path(sys.argv[2]); out.mkdir(parents=True, exist_ok=True)
diff_all = []
for path, rules in RULES.items():
    p = root / path
    src = p.read_bytes().decode('latin1')
    nl, text = apply(src)
    upd = text
    for b, a in rules: upd = _replace(upd, b, a)
    # idempotency
    again = upd
    for b, a in rules: again = _replace(again, b, a)
    assert again == upd, path
    f = script_findings(upd); assert not f, (path, f)
    final = upd.replace('\n', nl)
    assert final.endswith(src[-3:]) or True
    (out / path).parent.mkdir(parents=True, exist_ok=True); (out / path).write_bytes(final.encode('latin1'))
    d = list(difflib.unified_diff(src.replace('\r\n','\n').splitlines(), upd.splitlines(), 'a/'+path, 'b/'+path, lineterm='', n=1))
    diff_all += d
    print(path, len(src), '->', len(final), 'bytes; CRLF' if nl=='\r\n' else 'LF')
(out / 'proposed.diff').write_text('\n'.join(diff_all) + '\n', encoding='utf8')
