import sys, re, statistics
for p in sys.argv[1:]:
    t = open(p, encoding='utf-8', errors='replace').read()
    air = [float(x) for x in re.findall(r' air ([0-9.]+) ', t)]
    fr = [float(x) for x in re.findall(r'gpu=frame ([0-9.]+) ms', t)]
    if air:
        print("%-80s n=%d air median %.2f min %.2f | frame median %.1f min %.1f" % (p[-80:], len(air), statistics.median(air), min(air), statistics.median(fr), min(fr)))
