import os, subprocess, sys, time
os.chdir(r'D:\Arma_CWA')
p = 'roadmap/roadmap.yaml'
def edit(b):
    s = b.decode('utf-8')
    old = '    commits: ["84fbbce", "e37e79a", "71d7396", "8f9482a", "7a143ec", "863b8aa", "c825a5f", "6e1ee27"]\n'
    new = '    commits: ["84fbbce", "e37e79a", "71d7396", "8f9482a", "7a143ec", "863b8aa", "c825a5f", "6e1ee27", "aea62c6", "37e24ba", "c5cfa44"]\n'
    assert s.count(old) == 1, 'commits'
    s = s.replace(old, new)
    anchor = '      - "2026-10-13: phase 4 landed (c825a5f'
    assert s.count(anchor) == 1, 'anchor'
    line = ('      - "2026-10-14: phase 5 landed (37e24ba: columns in CounterAttack with one escort per carrier, the launch-time foot '
            'downgrade, forcesOpsMax single source, the RoadRoute join stage for bases whose roads never reach the town net, '
            'the review fixes M1 to M8 and most LOWs, and a pre-existing engine leak: a man deleted in his seat lingered in '
            'World::_outVehicles and reached saves) and the phase 6 air spike with verdict GO (c5cfa44: OccupierAir, 18/18 '
            'insertions within 20 m on the final build, 44/45 overall, jet sortie 6/6; AIR-SPIKE.md). aea62c6 anchors the '
            'showcase recruit chapter and the outfit e2e on gmHqPos. Gate green (636 unit cases, 37/37 tri incl. the slow_air '
            'tests; the human-suite contract red only on an untracked in-flight file). Problems hit: Malden Southeast Compound '
            'is a genuine island data gap (pocket roads 1292 m from the net); the squad still stalls inside its west gate '
            '(agent in flight); a lone tank can never carry the minimum bodies, so armour escorts a carrier; Traffic.cpp has '
            'the same seated-deletion leak (agent in flight); phase 6 proper (air wired into CounterAttack, town strikes, '
            'alarm answers replacing qrf.sqs) and phase 7 in flight; a usage-limit error killed six agents at 01:10 and they '
            'were resumed from their transcripts."\n')
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
