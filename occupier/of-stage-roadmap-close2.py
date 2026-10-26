import os, subprocess, sys, time
os.chdir(r'D:\Arma_CWA')
p = 'roadmap/roadmap.yaml'
def edit(b):
    s = b.decode('utf-8')
    old = '"12f2a75", "4dac9ab"]\n'
    new = '"12f2a75", "4dac9ab", "c18b6ec", "77afbb1", "803d411", "2c23d60"]\n'
    assert s.count(old) == 1, 'commits'
    s = s.replace(old, new)
    anchor = '      - "2026-10-14: close-out after the seven phases:'
    assert s.count(anchor) == 1, 'anchor'
    line = ('      - "2026-10-14: second close-out pass: the K settlement probe returns (c18b6ec; seeded labels in the water or '
            'with under three buildings no longer count, Sinai poolStart 88 to 66), and three pre-existing test reds fixed at '
            'their roots: fire_rogue (77afbb1, target stance under the fire-visibility threshold), market_reference_mission '
            '(803d411, the debug teleport aimed at the planned spot, not the dealer body), guerrilla_assailants (2c23d60, '
            'registered bodies shot the player; tri splits a line on every top-level semicolon, recorded in agent_guidelines). '
            'Final gate green: 675 unit cases, 84/84 tri (four load flakes recovered on retry), human suite OK. Open items in '
            '#97: alarm answer timing beside the player (by design), messaging (D12), the air re-plan ledger, the Compound as '
            'an island, #76, the human session, editor_wizard_finish (needs packages/Game)."\n')
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
