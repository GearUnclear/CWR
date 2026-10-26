"""Median GPU times of the alternating A/B/C timing shots (tstockN / tshadeN / tbestN).

    python timing_summary.py <run_dir>  -> prints a table and writes <run_dir>/timing_summary.md

The machine runs up to four games at once for other research agents, so a
single shot's GPU time is not trustworthy; the three variants are shot in
turn, four times each, 200 frames apiece, and the medians and ranges compared.
"""
import re
import statistics
import sys
from pathlib import Path

UDCAP_RE = re.compile(r"UDCAP;(.*)$")
ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")
RX = {k: re.compile(p) for k, p in {
    "frame": r"\bframe ([\d.]+) ms", "world": r"\bworld ([\d.]+)", "post": r"\bpost ([\d.]+):",
    "composite": r"\bcomposite ([\d.]+)"}.items()}


def main():
    run = Path(sys.argv[1])
    data = {}
    for p in sorted((run / "tri").rglob("game_stdout.log")):
        if "timing" not in str(p):
            continue
        for raw in p.read_text(encoding="utf-8", errors="replace").splitlines():
            m = UDCAP_RE.search(ANSI_RE.sub("", raw))
            if not m or "event=shot" not in m.group(1):
                continue
            body = m.group(1).rstrip('"')
            lab = re.search(r"label=([^;]+)", body).group(1)
            view, l = lab.rsplit("__", 1)
            mm = re.match(r"t(stock|shade|best|final)(\d)", l)
            if not mm:
                continue
            g = body.split(";gpu=", 1)[1] if ";gpu=" in body else ""
            row = {k: float(r.search(g).group(1)) if r.search(g) else None for k, r in RX.items()}
            row["i"] = int(mm.group(2))
            data.setdefault((view, mm.group(1)), []).append(row)
    lines = ["| view | variant | n | GPU frame median (min-max) | world median | post median | composite median |",
             "|---|---|---|---|---|---|---|"]
    for (view, var), rows in sorted(data.items()):
        def med(k):
            vals = [r[k] for r in rows if r[k] is not None]
            return (statistics.median(vals), min(vals), max(vals)) if vals else (None, None, None)
        f, w, po, c = med("frame"), med("world"), med("post"), med("composite")
        fmt = lambda t: "" if t[0] is None else f"{t[0]:.2f}"
        lines.append(f"| {view} | {var} | {len(rows)} | {f[0]:.2f} ({f[1]:.2f}-{f[2]:.2f}) | {fmt(w)} | {fmt(po)} | {fmt(c)} |")
    # paired deltas: variant shot i minus the stock shot i taken right before it
    lines += ["", "| view | variant | paired delta vs stock, GPU frame: median (each pair) | world delta median |",
              "|---|---|---|---|"]
    for (view, var), rows in sorted(data.items()):
        if var == "stock" or (view, "stock") not in data:
            continue
        st = {r["i"]: r for r in data[(view, "stock")]}
        d = [r["frame"] - st[r["i"]]["frame"] for r in rows if r["i"] in st and r["frame"] is not None]
        dw = [r["world"] - st[r["i"]]["world"] for r in rows if r["i"] in st and r["world"] is not None]
        if d:
            each = ", ".join(f"{x:+.1f}" for x in d)
            lines.append(f"| {view} | {var} | {statistics.median(d):+.2f} ({each}) | {statistics.median(dw):+.2f} |")
    out = "\n".join(lines) + "\n"
    (run / "timing_summary.md").write_text(out, encoding="utf-8")
    print(out)


if __name__ == "__main__":
    main()
