"""Parse event=timing UDCAP lines (rounds of alternating modes) into min/median
GPU frame and world times per view and mode. The GPU is shared with other
agents' games, so the minimum over rounds is the best estimate of the
uncontended cost; the median shows the spread."""
import re
import statistics
import sys
from pathlib import Path

ANSI = re.compile(r"\x1b\[[0-9;]*m")
FRAME = re.compile(r"\bframe ([\d.]+) ms")
WORLD = re.compile(r"\bworld ([\d.]+)")

def main(run):
    run = Path(run)
    data = {}
    for p in sorted((run / "tri").rglob("game_stdout.log")):
        for line in p.read_text(encoding="utf-8", errors="replace").splitlines():
            line = ANSI.sub("", line)
            if "event=timing" not in line:
                continue
            body = line.split("UDCAP;", 1)[1].rstrip('"')
            head, gpu = body.split(";gpu=", 1)
            kv = dict(x.split("=", 1) for x in head.split(";") if "=" in x)
            f, w = FRAME.search(gpu), WORLD.search(gpu)
            if not f:
                continue
            data.setdefault((kv["view"], kv["mode"]), []).append((float(f.group(1)), float(w.group(1)) if w else None))
    out = ["| view | mode | rounds | GPU frame min | frame median | world min | world median |", "|---|---|---|---|---|---|---|"]
    for (view, mode), vals in sorted(data.items(), key=lambda kv: (kv[0][0], kv[0][1])):
        fr = [v[0] for v in vals]
        wo = [v[1] for v in vals if v[1] is not None]
        out.append(f"| {view} | {mode} | {len(vals)} | {min(fr):.2f} | {statistics.median(fr):.2f} | {min(wo):.2f} | {statistics.median(wo):.2f} |")
    text = "\n".join(out) + "\n"
    (run / "timing_rounds.md").write_text(text, encoding="utf-8")
    print(text)

if __name__ == "__main__":
    main(sys.argv[1])
