"""Final staging: status rows 6 and 7 + the roadmap entry; foreign lines preserved.
Usage: python of-stage-hunks-final.py "<gate7 summary>" "<commit hash of phase 7>" """
import os, subprocess, sys, time
os.chdir(r'D:\Arma_CWA')
GATE7, P7 = sys.argv[1], sys.argv[2]

def doc(b):
    s = b.decode('utf-8')
    old6 = 'gate phase 6: (pending; the orchestrator fills the numbers).'
    assert s.count(old6) == 1, 'row6'
    s = s.replace(old6, 'gate phase 6 green 2026-10-14 (663 unit cases, 45/45 tri on shadow-67 incl. the slow_air tests, human suite OK).')
    old7 = '| 7 | In progress | '
    assert s.count(old7) == 1, 'row7'
    s = s.replace(old7, '| 7 | Landed | Commit ' + P7 + '; gate phase 7 ' + GATE7 + '. ')
    return s.encode('utf-8')

def roadmap(b):
    s = b.decode('utf-8')
    old = '    commits: ["84fbbce", "e37e79a", "71d7396", "8f9482a", "7a143ec", "863b8aa", "c825a5f", "6e1ee27", "aea62c6", "37e24ba", "c5cfa44"]\n'
    new = '    commits: ["84fbbce", "e37e79a", "71d7396", "8f9482a", "7a143ec", "863b8aa", "c825a5f", "6e1ee27", "aea62c6", "37e24ba", "c5cfa44", "195911e", "2524c3a", "bcca80e", "' + P7 + '"]\n'
    assert s.count(old) == 1, 'commits'
    s = s.replace(old, new)
    old_status = '    status: in-progress\n    priority: 1\n    depends_on: [offscreen-contract]\n    gh_issues: [77, 35, 72, 79, 86]\n'
    assert s.count(old_status) == 1, 'status'
    s = s.replace(old_status, '    status: done\n    priority: 1\n    depends_on: [offscreen-contract]\n    gh_issues: [77, 35, 72, 79, 86]\n')
    anchor = '      - "2026-10-14: phase 5 landed (37e24ba'
    assert s.count(anchor) == 1, 'anchor'
    line = ('      - "2026-10-14: phases 6 and 7 landed (bcca80e: air insertions, the birth-chain air re-plan, strikes, alarm '
            'answers replacing the scripted QRF (old saves keep it through legacySave), town strikes with the -40/-40 and '
            '2 km -10/-10 cuts, descriptors for every faction with aircraft, and the landmass rule after finding that '
            'Malden Southeast Compound is an island (air-only source); ' + P7 + ': occupierForces defaults to 1 with the '
            'campaignEnd = 0 missions pinned off, the forces* descriptor lint, the OFFSCREEN-CONTRACT rewrite, the plan '
            'doc reconciled to the code). Gate 6 green (663 unit, 45/45 tri); gate 7 ' + GATE7 + '. Also 2524c3a (Traffic '
            'seated-man deletion leak) and 195911e (plan doc reconciled). PR #95 marked ready. Open items tracked in the '
            'follow-ups issue and the plan doc section 17 (K probe, alarm answer timing, messaging, top-up adoption cap, '
            'air re-plan ledger, LOW review items, the Compound as an island, the human session)."\n')
    return s.replace(anchor, line + anchor).encode('utf-8')

def stage(p, edit):
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
    else: print('RESTORE FAILED', p, file=sys.stderr); sys.exit(1)
    print('staged', p)

stage('arma_CWR/guerrilla-mode/OCCUPIER-FORCES.md', doc)
stage('roadmap/roadmap.yaml', roadmap)
