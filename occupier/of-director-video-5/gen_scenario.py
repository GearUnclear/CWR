"""Expand scenario.tmpl into director_scenario_5.test.sqf.

A template line
    @WAIT <max s> <label> | <statements run on every poll> | <condition>
becomes a run of triSimUntil chunks, each waiting at most CHUNK s of game
time, with a UDTRACE (call udLogNow) between them, and a UDNOTE when the
condition was still false after <max s>.  The poll statements may be empty.

Usage: python gen_scenario.py [scenario.tmpl] [out.test.sqf]
"""
import math
import os
import sys

CHUNK = 300


def expand(line, n):
    body = line[len("@WAIT"):].strip()
    head, pre, cond = [p.strip() for p in body.split("|", 2)]
    maxs, label = head.split(" ", 1)
    maxs = int(maxs)
    var = "udW%d" % n
    pre = (pre + "; ") if pre else ""
    out = ["%s = time" % var]
    k = max(1, math.ceil(maxs / CHUNK))
    for i in range(1, k + 1):
        lim = min(maxs, i * CHUNK)
        out.append("triSimUntil { call udTick; %s(%s) or (time > %s + %d) }" % (pre, cond, var, lim))
        out.append("call udLogNow")
    if cond != "false":
        out.append('if (not (%s)) then {"wait timed out after %d s: %s" call udNote}' % (cond, maxs, label.strip()))
    return out


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    src = sys.argv[1] if len(sys.argv) > 1 else os.path.join(here, "scenario.tmpl")
    dst = sys.argv[2] if len(sys.argv) > 2 else os.path.join(here, "director_scenario_5.test.sqf")
    out = []
    n = 0
    for line in open(src, encoding="utf-8"):
        line = line.rstrip("\n")
        if line.startswith("@WAIT"):
            n += 1
            out.extend(expand(line, n))
        else:
            out.append(line)
    text = "\n".join(out) + "\n"
    if chr(0x2014) in text:
        raise SystemExit("em dash in the scenario")
    open(dst, "w", encoding="utf-8", newline="\n").write(text)
    print("%s: %d lines, %d waits" % (dst, len(out), n))


if __name__ == "__main__":
    main()
