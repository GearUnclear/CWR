"""Median / min per config across repeated rounds of UDPERF lines.

usage: summarize_rounds.py <game_stdout.log> <out.md>
Config labels look like "<regime><round>_<tag>" (e.g. A2_unlit); rows group by
(regime, tag). Columns: GPU frame / world (median over rounds, and the min),
CPU frame / draw / land:obj (median), calls (median), and with the scratch
build the ground / pass1 / rest split and the in-window min of world.
"""
import re
import statistics as st
import sys
from collections import defaultdict
from pathlib import Path

ANSI = re.compile(r"\x1b\[[0-9;]*m")


def parse(log):
    rows = []
    for raw in Path(log).read_text(errors="replace").splitlines():
        line = ANSI.sub("", raw)
        i = line.find("UDPERF;view=")
        if i < 0:
            continue
        body = line[i:].rstrip('"').rstrip()
        head, gpu = (body.split(";gpu=", 1) + [""])[:2]
        kv = dict(p.split("=", 1) for p in head.split(";")[1:] if "=" in p)
        perf = {}
        for tok in kv.get("perf", "").split():
            if "=" in tok:
                k, v = tok.split("=", 1)
                perf[k] = v.split("/")[0]
        r = {"cfg": kv.get("cfg", ""), "view": kv.get("view", "")}

        def g(pat):
            m = re.search(pat, gpu)
            return float(m.group(1)) if m else None

        r["gframe"] = g(r"frame ([\d.]+) ms")
        r["gworld"] = g(r"world ([\d.]+)")
        r["gpost"] = g(r"\| post ([\d.]+)")
        for sec in ("frame", "world", "ground", "pass1", "rest"):
            m = re.search(r" %s=([\d.]+)/([\d.]+)" % sec, gpu)
            if m:
                r["x_" + sec] = float(m.group(1))
                r["xmin_" + sec] = float(m.group(2))
        for k in ("lit", "litskip", "litKB", "vs", "vsskip"):
            m = re.search(r" %s=([\d.]+)" % k, gpu)
            if m:
                r["u_" + k] = float(m.group(1))
        for k, name in (("frame", "cframe"), ("draw", "cdraw"), ("land:obj", "cobj"), ("land:gnd", "cgnd"),
                        ("drw:land", "cland"), ("calls", "calls"), ("fps", "fps"), ("swap", "cswap")):
            try:
                r[name] = float(perf[k])
            except (KeyError, ValueError):
                r[name] = None
        rows.append(r)
    return rows


def main():
    rows = parse(sys.argv[1])
    groups = defaultdict(list)
    order = []
    for r in rows:
        m = re.match(r"([A-Za-z]+)(\d+)_(.+)", r["cfg"])
        key = (m.group(1), m.group(3)) if m else (r["cfg"], "")
        if key not in groups:
            order.append(key)
        groups[key].append(r)

    def med(vals):
        vals = [v for v in vals if v is not None]
        return st.median(vals) if vals else None

    def mn(vals):
        vals = [v for v in vals if v is not None]
        return min(vals) if vals else None

    def f(v, d=2):
        return "" if v is None else f"{v:.{d}f}"

    cols = ["regime", "config", "n", "GPU frame med", "GPU world med", "GPU world min(rounds)", "GPU post med",
            "ground med", "pass1 med", "rest med", "world in-window min (med)", "CPU frame med", "CPU draw med",
            "CPU land:obj med", "CPU swap med", "calls med", "lit flushes/frame", "lit KB/frame", "vs flushes/frame",
            "vs skips/frame"]
    out = ["| " + " | ".join(cols) + " |", "|" + "---|" * len(cols)]
    for key in order:
        rs = groups[key]
        out.append("| " + " | ".join([
            key[0], key[1], str(len(rs)),
            f(med([r["gframe"] for r in rs])), f(med([r["gworld"] for r in rs])), f(mn([r["gworld"] for r in rs])),
            f(med([r["gpost"] for r in rs])),
            f(med([r.get("x_ground") for r in rs])), f(med([r.get("x_pass1") for r in rs])),
            f(med([r.get("x_rest") for r in rs])), f(med([r.get("xmin_world") for r in rs])),
            f(med([r["cframe"] for r in rs])), f(med([r["cdraw"] for r in rs])), f(med([r["cobj"] for r in rs])),
            f(med([r["cswap"] for r in rs])),
            f(med([r["calls"] for r in rs]), 0), f(med([r.get("u_lit") for r in rs]), 0),
            f(med([r.get("u_litKB") for r in rs]), 1), f(med([r.get("u_vs") for r in rs]), 0),
            f(med([r.get("u_vsskip") for r in rs]), 0),
        ]) + " |")
    text = "\n".join(out) + "\n"
    Path(sys.argv[2]).write_text(text)
    print(text)


if __name__ == "__main__":
    main()
