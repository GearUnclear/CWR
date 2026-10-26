import os, subprocess, sys, time
os.chdir(r'D:\Arma_CWA')
p = 'roadmap/roadmap.yaml'
def edit(b):
    s = b.decode('utf-8')
    old = '"197a7d3", "acf2311"]\n'
    new = '"197a7d3", "acf2311", "6eefc87", "973a36d", "3f4972b", "3c9947b", "98cba8a", "797173a", "c8fe6a8", "df9457e"]\n'
    assert s.count(old) == 1, 'commits'
    s = s.replace(old, new)
    anchor = '      - "2026-10-15: the video-driven loop, round 1.'
    assert s.count(anchor) == 1, 'anchor'
    line = ('      - "2026-10-16: the video loop, rounds 2 and 3 (plan: guerrilla-mode/OCCUPIER-FORCES-NEXT.md). The five '
            'reviewable_forces fixes applied after an independent review (6eefc87). The InCargo check traced to NativeMoveIn '
            '(973a36d, the #78 family, note posted on #78). The package quote travels the road (3f4972b, 98cba8a; the base_retake '
            'fixture had assumed the straight-nearest base, c8fe6a8). No opener on an empty target, a band hold per launch, a '
            'pool ceiling (3c9947b), then the owner decision to bank the excess into garrisons rather than drop it (df9457e). '
            'Traffic and the town civilians held to the live-AI budget (797173a; the video overshoot was civilians.sqs, not '
            'traffic). Gates 18 and 19: 756 unit cases, 63/63 tri. Still unheld: shakedown.sqs and garrison bubble spawns; '
            'a deferred birth can lose freed room to traffic; most CounterAttack refunds bank by most room until they name a '
            'source."\n')
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
