"""Agent 2: the post 'air' GPU time (ms) from udcap.log / game_stdout.log gpu= entries. usage: python aircost.py <run dir> [...]"""
import glob, re, sys, statistics as st

for d in sys.argv[1:]:
    vals, frames = [], []
    for f in glob.glob(d + "/tri/*/game_stdout.log"):
        for line in open(f, encoding="utf-8", errors="replace"):
            m = re.search(r"gpu=frame ([\d.]+) ms.* air ([\d.]+)", line)
            if m:
                frames.append(float(m.group(1)))
                vals.append(float(m.group(2)))
    if vals:
        print("%-60s n %3d  air median %.2f  mean %.2f  min %.2f   frame median %.1f" % (d, len(vals), st.median(vals), st.mean(vals), min(vals), st.median(frames)))
    else:
        print(d, "no gpu entries")
