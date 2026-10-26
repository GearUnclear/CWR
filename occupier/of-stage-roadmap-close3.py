import os, subprocess, sys, time
os.chdir(r'D:\Arma_CWA')
p = 'roadmap/roadmap.yaml'
def edit(b):
    s = b.decode('utf-8')
    old = '"803d411", "2c23d60"]\n'
    new = '"803d411", "2c23d60", "e8ef485", "8380ab2"]\n'
    assert s.count(old) == 1, 'commits'
    s = s.replace(old, new)
    anchor = '      - "2026-10-14: second close-out pass:'
    assert s.count(anchor) == 1, 'anchor'
    line = ('      - "2026-10-14: third close-out pass: the air re-plan and the crew of every hull and airframe settle through '
            'the director ledger one for one (e8ef485, 8380ab2: SpendOnOp, CAOpCrewSettled, legacy rows reconciled on load, '
            'the forced prune never refunds crew), dealer markers follow the body, and the engine repoints the script player '
            'handles after a body-rebuilding load (playerOnVars) so older saves recover too. Final gate green: 680 unit '
            'cases, 84/84 tri (two load flakes recovered), human suite OK. Remaining in #97 are owner decisions and the '
            'human session (12.3)."\n')
    return s.replace(anchor, line + anchor).encode('utf-8')
work = open(p, 'rb').read()
head = subprocess.run(['git', 'show', 'HEAD:' + p], capture_output=True, check=True).stdout
staged = edit(head); tmp = p + '.stage.tmp'
open(tmp, 'wb').write(staged); os.replace(tmp, p)
subprocess.run(['git', 'add', p], check=True)
try: restored = edit(work)
except AssertionError: restored = work
open(tmp, 'wb').write(restored)
for i in range(10):
    try: os.replace(tmp, p); break
    except OSError as e: print('retry', e, file=sys.stderr); time.sleep(1)
else: print('RESTORE FAILED', file=sys.stderr); sys.exit(1)
print('staged roadmap hunk')
