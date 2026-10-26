import os, subprocess, sys, time
os.chdir(r'D:\Arma_CWA')
p = 'roadmap/roadmap.yaml'
def edit(b):
    s = b.decode('utf-8')
    old = '"06414a4", "0c43877", "1ae2dc9"]\n'
    new = '"06414a4", "0c43877", "1ae2dc9", "64d447a", "811c5c3", "d90a942", "d5fa57b", "16acfd5", "4d0cc45", "0d5e73a", "7765904", "e371a68"]\n'
    assert s.count(old) == 1, 'commits'
    s = s.replace(old, new)
    anchor = '      - "2026-10-16: the video loop, round 3.5'
    assert s.count(anchor) == 1, 'anchor'
    line = ('      - "2026-10-17: round 4a (the owner approved decisions 1 to 15 in OCCUPIER-FORCES-NEXT.md). The director '
            'remembers repels and escalates, masses instead of trickling, weighs candidates and jitters its rests (d5fa57b); '
            'a beaten force breaks off with its hulls leaving after the infantry (16acfd5); the chooser takes the cheapest '
            'package meeting the ETA with air only across water, past forcesAirEta or with fighters reported (d90a942); '
            'spent hold groups released, shakedown under the budget (64d447a); generated occupier bases, zones stored under '
            'the density rule with conservation (811c5c3; Malden gains La Riviere Outpost and Vigny Outpost, poolStart 60 to '
            '48) and a twelve-row outpost layout (0d5e73a); the wave surge, the band on strength, stored spare hulls '
            '(7765904); two more upstream engine defects fixed, the side centre radio channel (4d0cc45) and drop rejecting a '
            'two-element position (e371a68, every LoBo helicopter engine script). PR #213 carries the Remaster-only runtime '
            'gate stacked on #95. Gate 21: 820 unit cases, 80/83 tri with the reds resolved after; the strike band sequence '
            'is held out. Handed off (tmp/occupier-forces-handoff-2.md): gate 22, the eight-lever consolidation, video 6."\n')
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
