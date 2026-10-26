"""Parse the UD* lines a director_scenario tri run wrote to game_stdout.log
into trace.json for the renderer.

Usage: python parse_trace.py <game_stdout.log> <trace.json>

Every line kind is engine output: UDTRACE rows are the script surface
(gmOccupierForces, gmOccupierOps, gmOccupierIntel, gmCounterAttacks,
gmZone/gmZoneSim, gmOccupierAir) sampled by the scenario; UDZONE, UDGRAPH and
UDROUTE are zone rows and gmRoadRoute results; INFO lines from
OccupierForces / CounterAttack / OccupierAir are kept verbatim with their
wall-clock stamp and mapped to simulated time by the nearest UDTRACE rows.
"""
import json
import re
import sys
from datetime import datetime

ANSI = re.compile(r"\x1b\[[0-9;]*m")
STAMP = re.compile(r"^\[(\d{4}-\d\d-\d\d \d\d:\d\d:\d\d\.\d+)\]")


def parse_value(text):
    pos = 0
    n = len(text)

    def skip():
        nonlocal pos
        while pos < n and text[pos] in " \t":
            pos += 1

    def value():
        nonlocal pos
        skip()
        c = text[pos]
        if c == "[":
            pos += 1
            out = []
            skip()
            if text[pos] == "]":
                pos += 1
                return out
            while True:
                out.append(value())
                skip()
                if text[pos] == ",":
                    pos += 1
                    continue
                if text[pos] == "]":
                    pos += 1
                    return out
                raise ValueError("bad array at %d: %r" % (pos, text[pos:pos + 40]))
        if c == '"':
            pos += 1
            buf = []
            while True:
                if text[pos] == '"':
                    if pos + 1 < n and text[pos + 1] == '"':
                        buf.append('"')
                        pos += 2
                        continue
                    pos += 1
                    return "".join(buf)
                buf.append(text[pos])
                pos += 1
        start = pos
        depth = 0
        while pos < n:
            ch = text[pos]
            if depth == 0 and ch in ",]":
                break
            if ch == "<":
                depth += 1
            elif ch == ">":
                depth -= 1
            pos += 1
        word = text[start:pos].strip()
        if word == "true":
            return True
        if word == "false":
            return False
        try:
            return float(word)
        except ValueError:
            return word

    return value()


def main(src, dst):
    traces, zones, graph, routes, info = [], [], [], [], []
    for raw in open(src, encoding="utf-8", errors="replace"):
        line = ANSI.sub("", raw.rstrip("\n"))
        m = STAMP.match(line)
        wall = datetime.strptime(m.group(1), "%Y-%m-%d %H:%M:%S.%f").timestamp() if m else None
        for tag in ("UDTRACE", "UDZONE", "UDGRAPH", "UDROUTE", "UDHQ", "UDNOTE"):
            key = tag + " "
            i = line.find(key)
            if i >= 0:
                body = line[i + len(key):]
                j = body.rfind("]")
                v = parse_value(body[: j + 1])
                if tag == "UDNOTE":
                    info.append({"wall": wall, "text": v[1], "kind": "note", "sim": v[0]})
                    break
                if tag == "UDTRACE":
                    traces.append({"wall": wall, "row": v})
                elif tag == "UDZONE":
                    zones.append(v)
                elif tag == "UDGRAPH":
                    graph.append(v)
                elif tag == "UDROUTE":
                    routes.append({"wall": wall, "row": v})
                else:
                    hq = v
                break
        else:
            k = None
            for pre in ("OccupierForces: ", "CounterAttack: ", "OccupierAir: ", "GuerrillaBase: ", "CampaignEnd: "):
                k = line.find(pre)
                if k >= 0:
                    break
            if k is not None and k >= 0 and wall is not None:
                info.append({"wall": wall, "text": line[k:].strip().strip('"'), "kind": "info"})
    # map wall time to sim time with the trace rows (piecewise linear)
    pts = [(t["wall"], t["row"][0]) for t in traces]

    def sim_at(w):
        if not pts:
            return 0.0
        if w <= pts[0][0]:
            return pts[0][1]
        for (w0, s0), (w1, s1) in zip(pts, pts[1:]):
            if w0 <= w <= w1:
                return s0 + (s1 - s0) * ((w - w0) / (w1 - w0) if w1 > w0 else 0)
        return pts[-1][1]

    for e in info:
        if "sim" not in e:
            e["sim"] = sim_at(e["wall"])
    for r in routes:
        r["sim"] = sim_at(r["wall"])
    out = {
        "source": "engine",
        "zones": zones,
        "graph": graph,
        "routes": routes,
        "hq": hq if "hq" in dir() else None,
        "trace": [t["row"] for t in traces],
        "info": info,
    }
    json.dump(out, open(dst, "w"), indent=0)
    print("traces %d zones %d graph %d routes %d info %d" % (len(traces), len(zones), len(graph), len(routes), len(info)))


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
