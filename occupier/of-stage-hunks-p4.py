"""Stage only this session's hunks in roadmap.yaml and OCCUPIER-FORCES.md; keep other
sessions'/agents' uncommitted lines on disk (HEAD bytes + hunk staged, working bytes + hunk restored)."""
import os, subprocess, sys, time
os.chdir(r'D:\Arma_CWA')

def roadmap(b):
    s = b.decode('utf-8')
    old = '    commits: ["84fbbce", "e37e79a", "71d7396", "8f9482a", "7a143ec"]\n'
    new = '    commits: ["84fbbce", "e37e79a", "71d7396", "8f9482a", "7a143ec", "863b8aa", "c825a5f", "6e1ee27"]\n'
    assert s.count(old) == 1, 'commits line'
    s = s.replace(old, new)
    anchor = '      - "2026-10-13: phase 3 landed (8f9482a director'
    assert s.count(anchor) == 1, 'anchor'
    line = ('      - "2026-10-13: phase 4 landed (c825a5f HQ model, destruction and the lose state; 6e1ee27 waves, base '
            'recapture, standing, the director\'s base and HQ operations, plus the three phase 3 gaps: the pool refund hook '
            'for S5 pending deposits, immediate re-source with a route check, legacy records take the birth chain with the '
            'key on) and the phase 5 package catalogue (863b8aa: every ground package, crew and seat caps, foot downgrade, '
            'columns = a carrier plus one escort; the CounterAttack side follows). Gate green on build/win-x64-clang-rwdi '
            '(626 unit cases, 33/33 tri on shadow-67 after four one-off flakes, human suite OK). Problems hit: the tree was '
            'handed over uncompiled from the first session and compiled clean; agent2-rwdi had to be reconfigured with '
            '--fresh (stale objects flickered in-game); an independent review of phases 1 to 3 found two HIGH defects (a '
            'release-only TIME_MIN overflow that made a disguised player hold every town, fixed; the uninstalled pending '
            'refund hook, fixed) and eight MEDIUM items routed to a follow-up agent; a lone tank can never carry the minimum '
            'bodies, so armour escorts a truck or APC (orchestrator decision from 3.2 and the 6.5 table)."\n')
    s = s.replace(anchor, line + anchor)
    return s.encode('utf-8')

def doc(b):
    s = b.decode('utf-8')
    head_row = subprocess.run(['git', 'show', 'HEAD:arma_CWR/guerrilla-mode/OCCUPIER-FORCES.md'], capture_output=True, check=True).stdout.decode('utf-8')
    old_line = [l for l in head_row.split('\n') if l.startswith('| 4 | Pending | 4B:')][0] + '\n'
    assert s.count(old_line) == 1, 'row 4'
    new_line = old_line.replace('| 4 | Pending | 4B:', '| 4 | Landed | Commits c825a5f (4B) and 6e1ee27 (4A); gate green 2026-10-13 (626 unit cases, 33/33 tri on shadow-67, four one-off flakes). 4B:', 1)
    return s.replace(old_line, new_line).encode('utf-8')

def stage(p, edit):
    work = open(p, 'rb').read()
    head = subprocess.run(['git', 'show', 'HEAD:' + p], capture_output=True, check=True).stdout
    staged = edit(head)
    tmp = p + '.stage.tmp'
    open(tmp, 'wb').write(staged); os.replace(tmp, p)
    subprocess.run(['git', 'add', p], check=True)
    # working copy: the foreign lines plus our hunk (idempotent if already present)
    try:
        restored = edit(work)
    except AssertionError:
        restored = work  # our hunk is already in the working copy
    open(tmp, 'wb').write(restored)
    for i in range(10):
        try:
            os.replace(tmp, p); break
        except OSError as e:
            print('retry', e, file=sys.stderr); time.sleep(1)
    else:
        print('RESTORE FAILED for', p, 'foreign lines in', tmp, file=sys.stderr); sys.exit(1)
    print('staged', p, len(staged), 'work', len(restored))

stage('roadmap/roadmap.yaml', roadmap)
stage('arma_CWR/guerrilla-mode/OCCUPIER-FORCES.md', doc)
