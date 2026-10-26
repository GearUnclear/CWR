import os, subprocess, sys, time
os.chdir(r'D:\Arma_CWA')
p = 'roadmap/roadmap.yaml'
def edit(b):
    s = b.decode('utf-8')
    old = '"195911e", "2524c3a", "bcca80e", "fff2c0b"]\n'
    new = '"195911e", "2524c3a", "bcca80e", "fff2c0b", "08c6ae2", "e6bff81", "5c7a771", "12f2a75", "4dac9ab"]\n'
    assert s.count(old) == 1, 'commits'
    s = s.replace(old, new)
    anchor = '      - "2026-10-14: phases 6 and 7 landed (bcca80e'
    assert s.count(anchor) == 1, 'anchor'
    line = ('      - "2026-10-14: close-out after the seven phases: the review LOW items L6 to L15 and the top-up adoption cap '
            '(12f2a75); the mod-lane tomls follow the Remaster layout through a tri ${repo_root} token and the MP wizard '
            'tests target the Remaster templates (e6bff81); the saved player handles are rebound after a load that '
            'rebuilt the body (5c7a771); the assailants fixture pins cityGarrison off because a town garrison shot the '
            'player once the hashed spawn points made the kill deterministic (4dac9ab). Final gate green (670 unit cases, '
            '82/82 after the fixture pin incl. the main-menu lane, human suite OK). Still open, tracked in #97: the K '
            'settlement probe, alarm answer timing beside the player, messaging (D12 deferred), the air re-plan price in '
            'the ledger, the Compound as an island, the occupation network (#76), the human session, editor_wizard_finish '
            '(needs packages/Game), guerrilla_assailant_fire_rogue (pre-existing red)."\n')
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
