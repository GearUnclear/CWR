"""Stage only this session's roadmap edits; keep the other sessions' uncommitted
lines on disk.  Usage: python roadmap_hunk.py  (edits are hard-coded below)."""
import os, subprocess, sys, time
os.chdir(r'D:\Arma_CWA')
p = 'roadmap/roadmap.yaml'
work = open(p, 'rb').read()
head = subprocess.run(['git', 'show', 'HEAD:' + p], capture_output=True, check=True).stdout

def edit(b):
    s = b.decode('utf-8')
    old_commits = '    commits: ["84fbbce", "e37e79a"]\n'
    new_commits = '    commits: ["84fbbce", "e37e79a", "71d7396", "8f9482a", "7a143ec"]\n'
    assert s.count(old_commits) == 1
    s = s.replace(old_commits, new_commits)
    anchor = '      - "2026-10-13: phases 1 and 2 landed on the branch'
    assert s.count(anchor) == 1
    line = ('      - "2026-10-13: phase 3 landed (8f9482a director + ForcePackages, 7a143ec movement + hulls + birth chain + '
            'pool routing); gate green on build/win-x64-clang-rwdi (569 unit cases, 31/31 tri on the Remaster lane, one retry). '
            'Abel opts in (occupierForces = 1), fixtures and reference slices pin it off. Problems hit: DoGetOut at speed killed '
            '5 of 6 riders (brake first; set InCargo after moveInCargo); Malden\'s Southeast Compound road net is not joined to '
            'the island network, so forces from it stall against the compound (watched_route asserts re-source + departure); '
            'forcesOpsMax read by both services; a default-parallelism clang-cl build crashes under memory pressure (agents and '
            'the gate build with -j 5). Phase 4 in flight: waves/base recapture/standing (A) and HQ model/lose state (B)."\n')
    s = s.replace(anchor, line + anchor)
    return s.encode('utf-8')

staged = edit(head)
tmp = p + '.stage.tmp'
open(tmp, 'wb').write(staged)
os.replace(tmp, p)
subprocess.run(['git', 'add', p], check=True)
restored = edit(work)
open(tmp, 'wb').write(restored)
for i in range(10):
    try:
        os.replace(tmp, p)
        break
    except OSError as e:
        print('retry', e, file=sys.stderr)
        time.sleep(1)
else:
    print('RESTORE FAILED: working copy is HEAD + hunk; foreign lines are in', tmp, file=sys.stderr)
    sys.exit(1)
print('ok staged', len(staged), 'work', len(restored))
