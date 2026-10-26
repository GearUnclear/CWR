import os, subprocess, sys, time
os.chdir(r'D:\Arma_CWA')
p = 'roadmap/roadmap.yaml'
def edit(b):
    s = b.decode('utf-8')
    old = '"e8ef485", "8380ab2"]\n'
    new = '"e8ef485", "8380ab2", "8909838"]\n'
    assert s.count(old) == 1, 'commits'
    s = s.replace(old, new)
    anchor = '      - "2026-10-14: third close-out pass:'
    assert s.count(anchor) == 1, 'anchor'
    line = ('      - "2026-10-14: owner decisions applied (8909838): the director and counter-attack INFO lines are dev-only '
            'behind forcesLog (D12 messaging stays deferred; journal integration filed as #98 with the how open); alarm '
            'answers stay born at their base (3.1 step 1); the Southeast Compound stays air-only until the vehicle system '
            'work. Gate green (681 unit cases, 72/72 tri, human suite OK). Owner also approved auto-detecting AIRFIELD and '
            'SEAPORT zones per island (#76 slice, two agents in flight: detection + seedBases seeding, ground truth + '
            'per-template tests)."\n')
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
