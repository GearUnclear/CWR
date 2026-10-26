import os, subprocess, sys, time
os.chdir(r'D:\Arma_CWA')
p = 'roadmap/roadmap.yaml'
def edit(b):
    s = b.decode('utf-8')
    old = '"c8fe6a8", "df9457e"]\n'
    new = '"c8fe6a8", "df9457e", "06414a4", "0c43877", "1ae2dc9"]\n'
    assert s.count(old) == 1, 'commits'
    s = s.replace(old, new)
    anchor = '      - "2026-10-16: the video loop, rounds 2 and 3'
    assert s.count(anchor) == 1, 'anchor'
    line = ('      - "2026-10-16: the video loop, round 3.5 (video 4: one strict-mode run of 2 h 18 min game time, 0 errors, '
            '16 operations, the pool never above its start outside the wave). A defended town fell out of the director\'s reach '
            'once its repel heat decayed inside the rest: re-arm at the rest\'s end (forcesRearmHeat 40, 1ae2dc9), with the '
            'strike keeping STRONG after both debits, push-outs under the arrival rule and honest HQ and launch lines. '
            'Arrival-live columns are born with their armour (06414a4). The group budget counts only groups with a living '
            'member and idle empty groups are released (0c43877). Gate 20: 767 unit cases, 67/67 tri. Video 5 decides whether '
            'the loop stops; round 4 waits on the owner\'s base-generation decisions in OCCUPIER-FORCES-NEXT.md."\n')
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
