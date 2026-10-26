import re, sys, glob, statistics as st, datetime as dt
rx = re.compile(r"^\[(\d{4}-\d\d-\d\d \d\d:\d\d:\d\d\.\d+)\].*\[tri\] triScreenshot( \(at present\))?:")
rows = []
for log in sorted(glob.glob("*/tri/*/game_stdout.log")):
    ts = []
    for line in open(log, encoding="utf-8", errors="replace"):
        m = rx.match(line)
        if m:
            ts.append(dt.datetime.strptime(m.group(1), "%Y-%m-%d %H:%M:%S.%f"))
    if len(ts) < 3:
        continue
    d = [(b - a).total_seconds() * 1000 for a, b in zip(ts, ts[1:])]
    d.sort()
    rows.append((log.split("/")[0], len(ts), st.median(d), d[len(d) // 10], d[int(len(d) * .9)], st.pstdev(d)))
for r in rows:
    print(f"{r[0]:22s} shots={r[1]:4d} wall/shot median={r[2]:6.0f}ms p10={r[3]:6.0f} p90={r[4]:6.0f} sd={r[5]:6.0f}")
