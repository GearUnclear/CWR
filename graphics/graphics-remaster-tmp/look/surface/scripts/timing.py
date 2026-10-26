"""GPU timing table from a capture run's UDCAP shot lines.

  python timing.py <run dir> [--md out.md]

For every shot: label, GPU frame / world / post / ao / composite (ms, the
triGpuTimes summary of the 90 frames before the shot), draws. Then per view and
mode family (the label suffix without its trailing round digits) the median.
"""
import argparse
import re
import statistics
from collections import defaultdict
from pathlib import Path

SHOT = re.compile(r'UDCAP;view=(?P<view>[^;]+);event=shot;label=(?P<label>[^;]+);.*?;draws=(?P<draws>\d+);.*;gpu=(?P<gpu>.*)"?$')


def num(pat, s):
    m = re.search(pat, s)
    return float(m.group(1)) if m else None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("--md")
    a = ap.parse_args()
    rows = []
    for log in Path(a.run).rglob("game_stdout.log"):
        for line in log.read_text(encoding="utf-8", errors="replace").splitlines():
            m = SHOT.search(line)
            if not m:
                continue
            g = m.group("gpu")
            rows.append(dict(view=m.group("view"), label=m.group("label"), draws=int(m.group("draws")),
                             frame=num(r"frame ([\d.]+) ms", g), world=num(r"world ([\d.]+)", g),
                             post=num(r"post ([\d.]+)", g), ao=num(r" ao ([\d.]+)", g),
                             resolve=num(r"resolve ([\d.]+)", g), composite=num(r"composite ([\d.]+)", g),
                             minframe=num(r"min frame=([\d.]+)", g), minworld=num(r"min .*?world=([\d.]+)", g),
                             cpu=num(r"perf=fps=[\d.]+ frame=([\d.]+)", line), drwobj=num(r"drw:obj=([\d.]+)", line)))
    out = ["| shot | frame | world | post | resolve | ao | composite | draws |", "|---|---|---|---|---|---|---|---|"]
    fam = defaultdict(list)
    for r in rows:
        out.append(f"| {r['label']} | {r['frame']} | {r['world']} | {r['post']} | {r['resolve']} | {r['ao']} | {r['composite']} | {r['draws']} |")
        mode = r["label"].split("__", 1)[1].rstrip("0123456789")
        fam[(r["view"], mode)].append(r)
    out += ["", "| view | mode | n | frame | world | post | resolve | ao | composite | min frame | min world | cpu frame | cpu drw:obj |", "|---|---|---|---|---|---|---|---|---|---|---|---|---|"]
    for (view, mode), rs in sorted(fam.items()):
        def med(k):
            v = [x[k] for x in rs if x[k] is not None]
            return f"{statistics.median(v):.2f}" if v else "-"
        def lo(k):
            v = [x[k] for x in rs if x[k] is not None]
            return f"{min(v):.2f}" if v else "-"
        out.append(f"| {view} | {mode} | {len(rs)} | {med('frame')} | {med('world')} | {med('post')} | {med('resolve')} | {med('ao')} | {med('composite')} | {lo('minframe')} | {lo('minworld')} | {med('cpu')} | {med('drwobj')} |")
    text = "\n".join(out)
    print(text)
    if a.md:
        Path(a.md).write_text(text + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
