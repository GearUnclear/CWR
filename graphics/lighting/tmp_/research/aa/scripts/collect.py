"""Collect a track-aa capture run: copy NNN_<view>__<label>.png out of the
tri tree as <view>__<label>.png and write timings.csv / timings.md from the
UDCAP shot lines.

    python collect.py <run_dir>
"""
import csv
import re
import shutil
import sys
from pathlib import Path

SHOT_RE = re.compile(r"^(\d{3})_(.+?)__(.+)\.png$")
ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")
UDCAP_RE = re.compile(r"UDCAP;(.*)$")
GPU_RES = {
    "frame": re.compile(r"\bframe ([\d.]+) ms"),
    "world": re.compile(r"\bworld ([\d.]+)"),
    "post": re.compile(r"\bpost ([\d.]+):"),
    "ao": re.compile(r"\bao ([\d.]+)"),
    "bloom": re.compile(r"\bbloom ([\d.]+)"),
    "shafts": re.compile(r"\bshafts ([\d.]+)"),
    "composite": re.compile(r"\bcomposite ([\d.]+)"),
}


def main():
    run = Path(sys.argv[1])
    tri = run / "tri"
    best = {}
    for p in tri.rglob("*.png"):
        m = SHOT_RE.match(p.name)
        if not m:
            continue
        key = (m.group(2), m.group(3))
        if key not in best or p.stat().st_mtime > best[key].stat().st_mtime:
            best[key] = p
    for (view, label), src in best.items():
        shutil.copy2(src, run / f"{view}__{label}.png")
    rows = []
    for log in sorted(tri.rglob("game_stdout.log")):
        for raw in log.read_text(encoding="utf-8", errors="replace").splitlines():
            line = ANSI_RE.sub("", raw)
            m = UDCAP_RE.search(line)
            if not m:
                continue
            body = m.group(1).strip().rstrip('"')
            gpu = ""
            if ";gpu=" in body:
                body, gpu = body.split(";gpu=", 1)
            kv = dict(part.split("=", 1) for part in body.split(";") if "=" in part)
            if kv.get("event") != "shot":
                continue
            row = {"label": kv.get("label", ""), "mode": kv.get("mode", "")}
            for f, rx in GPU_RES.items():
                mm = rx.search(gpu)
                row[f] = mm.group(1) if mm else ""
            row["gpu_raw"] = gpu
            rows.append(row)
    with open(run / "timings.csv", "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=["label", "mode", "frame", "world", "post", "ao", "bloom", "shafts", "composite", "gpu_raw"])
        w.writeheader()
        w.writerows(rows)
    with open(run / "timings.md", "w", encoding="utf-8") as f:
        f.write("| shot | GPU frame ms | world | post | composite |\n|---|---|---|---|---|\n")
        for r in rows:
            if "-pan" in r["label"] or "__pan" in r["label"]:
                continue
            f.write(f"| {r['label']} | {r['frame']} | {r['world']} | {r['post']} | {r['composite']} |\n")
    print(f"copied {len(best)} images, {len(rows)} shot rows -> {run}")


if __name__ == "__main__":
    main()
