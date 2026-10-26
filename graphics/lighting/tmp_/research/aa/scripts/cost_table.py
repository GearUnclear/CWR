"""Present-pass cost table from a v4 run (scratch build appends
"| present X.XXX" to the triGpuTimes summary).

    python cost_table.py <run_dir>   -> <run_dir>/present_cost.md
"""
import csv
import re
import statistics
import sys
from collections import defaultdict
from pathlib import Path

ANSI = re.compile(r"\x1b\[[0-9;]*m")
UDCAP = re.compile(r"UDCAP;(.*)$")
NUM = {
    "frame": re.compile(r"\bframe ([\d.]+) ms"),
    "world": re.compile(r"\bworld ([\d.]+)"),
    "post": re.compile(r"\bpost ([\d.]+):"),
    "composite": re.compile(r"\bcomposite ([\d.]+)"),
    "present": re.compile(r"\bpresent ([\d.]+)"),
    "pmin": re.compile(r"\bpresent [\d.]+ min ([\d.]+)"),
}
NAMES = {
    "classic": "Classic: stock MSAA resolve + GL_LINEAR blit",
    "p0": "Remastered p0: resolve to texture + bilinear shader",
    "lz": "Remastered p1: Lanczos2 2D",
    "lzrcas05": "Remastered p2: Lanczos2 2D + RCAS 0.5",
    "seplz": "Remastered p7: separable Lanczos2 (2 passes)",
    "seplzrcas05": "Remastered p6: separable Lanczos2 + RCAS 0.5",
}


def main():
    run = Path(sys.argv[1])
    rows = []
    for log in sorted((run / "tri").rglob("game_stdout.log")):
        test = log.parent.name
        for raw in log.read_text(encoding="utf-8", errors="replace").splitlines():
            m = UDCAP.search(ANSI.sub("", raw))
            if not m or "event=shot" not in m.group(1):
                continue
            body = m.group(1).strip().rstrip('"')
            gpu = body.split(";gpu=", 1)[1] if ";gpu=" in body else ""
            label = re.search(r"label=([^;]+)", body).group(1)
            r = {"test": test, "label": label}
            for k, rx in NUM.items():
                mm = rx.search(gpu)
                r[k] = float(mm.group(1)) if mm else None
            rows.append(r)
    with open(run / "present_cost.csv", "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=["test", "label", "frame", "world", "post", "composite", "present", "pmin"])
        w.writeheader()
        w.writerows(rows)
    groups = defaultdict(list)
    for r in rows:
        m = re.match(r".*__H-(.+)-[abc]$", r["label"])
        if m and r["present"] is not None:
            groups[(r["test"], m.group(1))].append(r)
    out = ["| test (window) | variant | present ms EMA (shots) | median | present min ms (shots) | median min | GPU frame ms (shots) |",
           "|---|---|---|---|---|---|---|"]
    for (test, var), rs in sorted(groups.items(), key=lambda kv: (kv[0][0], list(NAMES).index(kv[0][1]) if kv[0][1] in NAMES else 99)):
        pres = [r["present"] for r in rs]
        fr = [r["frame"] for r in rs if r["frame"] is not None]
        mins = [r["pmin"] for r in rs if r["pmin"] is not None]
        mtxt = ', '.join(f'{x:.3f}' for x in mins) if mins else 'n/a'
        mmed = f"{statistics.median(mins):.3f}" if mins else 'n/a'
        out.append(f"| {test} | {NAMES.get(var, var)} | {', '.join(f'{p:.3f}' for p in pres)} | {statistics.median(pres):.3f} | "
                   f"{mtxt} | {mmed} | {', '.join(f'{x:.2f}' for x in fr)} |")
    (run / "present_cost.md").write_text("\n".join(out) + "\n", encoding="utf-8")
    print("\n".join(out))


if __name__ == "__main__":
    main()
