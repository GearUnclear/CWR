"""GPU timing table from a research run: python timings.py <run_dir> [out.md]

Reads every UDCAP shot line under <run_dir>/tri, groups repeated A/B labels
(<view>__materials, <view>__materials_b, <view>__materials_c -> one variant)
and prints the median GPU frame / world / post time per view and variant.
The machine is shared with other agents' games, so single readings swing;
medians over the repeats are the honest number, and deltas against the
Remastered reading of the same view are what matter."""
import re
import statistics
import sys
from pathlib import Path

UDCAP_RE = re.compile(r"UDCAP;(.*)$")
ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")
F = {k: re.compile(p) for k, p in {
    "frame": r"\bframe ([\d.]+) ms", "world": r"\bworld ([\d.]+)", "shadow": r"\(shadow ([\d.]+)\)",
    "post": r"\bpost ([\d.]+):", "ao": r"\bao ([\d.]+)"}.items()}

run = Path(sys.argv[1])
rows = {}
for p in sorted((run / "tri").rglob("*.log")):
    for raw in p.read_text(encoding="utf-8", errors="replace").splitlines():
        m = UDCAP_RE.search(ANSI_RE.sub("", raw))
        if not m or "event=shot" not in m.group(1):
            continue
        body = m.group(1).strip().rstrip('"')
        lab = re.search(r"label=([^;]+)", body).group(1)
        if "dbg" in lab:
            continue
        gpu = body.split(";gpu=", 1)[1] if ";gpu=" in body else ""
        view, var = lab.rsplit("__", 1)
        var = re.sub(r"_[bc]$", "", var)
        vals = {k: (float(rx.search(gpu).group(1)) if rx.search(gpu) else None) for k, rx in F.items()}
        rows.setdefault((view, var), []).append(vals)

out = ["| view | variant | n | GPU frame ms (median) | world ms | post ms (ao bucket incl. SSR) | frame minus Remastered |",
       "|---|---|---|---|---|---|---|"]
views = sorted({v for v, _ in rows})
order = ["classic", "remastered", "vbfix", "materials", "norim", "nobump", "wet", "wetnossr"]
for view in views:
    base = None
    if (view, "remastered") in rows:
        base = statistics.median([r["frame"] for r in rows[(view, "remastered")] if r["frame"] is not None])
    for var in order:
        if (view, var) not in rows:
            continue
        rs = rows[(view, var)]
        def med(k):
            xs = [r[k] for r in rs if r[k] is not None]
            return statistics.median(xs) if xs else None
        fr, wo, po = med("frame"), med("world"), med("post")
        d = f"{fr - base:+.2f}" if (fr is not None and base is not None and var != "classic") else ""
        out.append(f"| {view} | {var} | {len(rs)} | {fr:.2f} | {wo:.2f} | {po if po is None else round(po, 2)} | {d} |")
text = "\n".join(out)
print(text)
if len(sys.argv) > 2:
    Path(sys.argv[2]).write_text(text + "\n", encoding="utf-8")
