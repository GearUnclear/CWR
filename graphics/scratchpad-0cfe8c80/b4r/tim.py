"""Frame, world and combatmask GPU ms of the B4 timing shots.

    python tim.py <run dir>
"""
import glob, os, re, sys
d = sys.argv[1]
log = glob.glob(os.path.join(d, "tri", "*", "game_stdout.log"))[0]
for line in open(log, encoding="utf-8", errors="replace"):
    m = re.search(r"event=shot;label=(combat_(?:flash)?timing_r\d)__(off|on);", line)
    if not m:
        continue
    g = re.search(r"gpu=frame ([0-9.]+) ms \| world ([0-9.]+)", line)
    cm = re.search(r"combatmask ([0-9.]+)", line)
    mn = re.search(r"min frame=([0-9.]+)", line)
    print(f"{m.group(1):22s} {m.group(2):3s} frame {g.group(1) if g else '?':>6s} world {g.group(2) if g else '?':>6s} "
          f"combatmask {cm.group(1) if cm else '-':>5s} min frame {mn.group(1) if mn else '?'}")
