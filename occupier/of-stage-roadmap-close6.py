import os, subprocess, sys, time
os.chdir(r'D:\Arma_CWA')
p = 'roadmap/roadmap.yaml'
def edit(b):
    s = b.decode('utf-8')
    old = '"8909838", "3946b5a", "f9d0444"]\n'
    new = '"8909838", "3946b5a", "f9d0444", "fe9a3ae", "5554d85", "e267e54", "c7dc8c7", "f4c6151", "4a41951"]\n'
    assert s.count(old) == 1, 'commits'
    s = s.replace(old, new)
    old_issues = '    gh_issues: [77, 35, 72, 79, 86]\n    commits: ["84fbbce"'
    new_issues = '    gh_issues: [77, 35, 72, 79, 86, 97, 98, 99]\n    commits: ["84fbbce"'
    assert s.count(old_issues) == 1, 'issues'
    s = s.replace(old_issues, new_issues)
    anchor = '      - "2026-10-14: #76 slice landed (3946b5a, f9d0444)'
    assert s.count(anchor) == 1, 'anchor'
    line = ('      - "2026-10-14: after the owner\'s first Malden session (Cancon start): the far live-group census and the '
            'arrival and birth trace for the playtest (fe9a3ae; the abstract-token director assessment found the travel '
            'layer already abstract, tmp/of-abstract-director-assessment.md); the Arma 3 controls preset row on the '
            'Remaster through a UD-owned bin/ud-options.hpp shard (5554d85); a main-menu Options crash fixed at its root '
            '(c7dc8c7: a pointer release on a scroll-list Close row popped and freed the page under its own click '
            'handler; popped pages are parked until the next frame) with a Remaster-lane click-through (e267e54) and a '
            'headful main-menu walk (f4c6151); the forces save sequence\'s stale 15-slot asserts corrected to the documented '
            '30 (4a41951; no gate had run the sequence since phase 3). Gate 15 green: 709 unit cases, base lane 25/25, '
            'forces and detection set 25/25 after the fix, human suite OK. Owner findings: a findable vehicle dealer (#99), '
            'a vehicle ramming a sign in town (traffic, details pending), the Camp marker with a start-town HQ (by design, '
            'owner call)."\n')
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
