import os, subprocess, sys, time
os.chdir(r'D:\Arma_CWA')
p = 'roadmap/roadmap.yaml'
def edit(b):
    s = b.decode('utf-8')
    old = '"8380ab2", "8909838"]\n'
    new = '"8380ab2", "8909838", "3946b5a", "f9d0444"]\n'
    assert s.count(old) == 1, 'commits'
    s = s.replace(old, new)
    anchor = '      - "2026-10-14: owner decisions applied (8909838)'
    assert s.count(anchor) == 1, 'anchor'
    line = ('      - "2026-10-14: #76 slice landed (3946b5a, f9d0444): AIRFIELD and SEAPORT zones are auto-detected per island '
            '(the ILS table and hangar clusters; pier / quay / crane clusters touching water; authored zones win) and '
            'seeded at load behind seedBases inside playRegion[] (an authored region bounds everything, the default box '
            'bounds the base seed only); guerrilla scaffold emits candidates, guerrilla lint --bases reports coverage; four '
            'per-template detection tests plus an Eden save sequence. Eden gains Everon Airfield and Chotain Harbour, Abel '
            'is covered by its authored hubs, Lebanon80 seeds nothing by design, Sinai seeds 7 (a hand-authored region is '
            '#75). Fixtures and reference slices pin seedBases off. Gate green (707 unit cases, 83/83 tri, human suite OK). '
            'Problems hit: the default region first also bounded the city seed (Sinai K 26 to 13, the names fixture 14 to '
            '12), the Egyptian Airbase was seeded twice (fenced strips now fold within 1.5 km), the crane token matched the '
            'rowan tree, and default-on seeding leaked into fixtures that author every zone."\n')
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
