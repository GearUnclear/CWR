import os, subprocess, sys, time
os.chdir(r'D:\Arma_CWA')
p = 'roadmap/roadmap.yaml'
def edit(b):
    s = b.decode('utf-8')
    old = '"f4c6151", "4a41951"]\n'
    new = '"f4c6151", "4a41951", "fe228a0", "2921bcf", "197a7d3", "acf2311"]\n'
    assert s.count(old) == 1, 'commits'
    s = s.replace(old, new)
    anchor = '      - "2026-10-14: after the owner\'s first Malden session'
    assert s.count(anchor) == 1, 'anchor'
    line = ('      - "2026-10-15: the video-driven loop, round 1. A commentated director video from a real engine trace '
            '(tmp/of-director-video) surfaced three items: the arrival census was right (a live hold squad) and its log line '
            'now names the count (fe228a0); an upstream group-id allocator leak made steady traffic churn walk a side past '
            'the 63 named slots in about 116 spawns and trip AIGroupImplHealth on every later group (2921bcf, lowest free id, '
            'ai_group_id_churn repro); the air re-plan stepped the band twice in one tick (197a7d3, BandHold). Owner decision '
            '\\"a more playable rate\\": token interval per band 600 / 1050 / 1800 s, three STRONG operations, the spare cap '
            'on retake minimums, the arrival rule in the package choice (acf2311; old values by key). Gates 16 and 17 green '
            '(721 unit cases, 56/56 tri with the whole forces set). Two read-only assessments for the owner: generated '
            'occupier bases (tmp/of-generated-bases-design.md, -engine.md: Malden has four military zones for fourteen towns, '
            'seven towns beyond 2.5 km of any base; a density rule, a base-kind catalogue, placement from the 50 m terrain '
            'cells, props the HQ way, stored zones; decisions pending) and the abstract-token director (travel already '
            'abstract)."\n')
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
