"""GPU sections per shot from a run's UDCAP lines.

    python gpu.py <run> [<run> ...]

Prints label, frame, world, post, air, shafts, bloom and composite (ms, the
backend's smoothed readings, triGpuTimes) and the air gates of the shot.
"""
import re
import sys
from pathlib import Path

SECTIONS = ("air", "shafts", "bloom", "composite", "resolve", "lindepth", "ao", "taa")


def parse(run):
    rows = []
    for log in Path(run).glob("tri/**/game_stdout.log"):
        for line in log.read_text(encoding="utf-8", errors="replace").splitlines():
            if "UDCAP;" not in line or "event=shot" not in line:
                continue
            label = re.search(r"label=([^;]+)", line).group(1)
            gpu = re.search(r"gpu=([^\"]*)", line)
            g = gpu.group(1) if gpu else ""
            frame = re.search(r"frame ([0-9.]+) ms", g)
            world = re.search(r"world ([0-9.]+)", g)
            post = re.search(r"post ([0-9.]+)", g)
            secs = {s: re.search(rf"\b{s} ([0-9.]+)", g.split("post", 1)[-1]) for s in SECTIONS}
            feat = re.search(r"feat=([^;]*)", line)
            f = feat.group(1) if feat else ""
            gates = " ".join(re.findall(r"(air\w+=[^ ]+|valleyMist=[^ ]+)", f))
            rows.append((label, frame and frame.group(1), world and world.group(1), post and post.group(1),
                         {k: (v.group(1) if v else "") for k, v in secs.items()}, gates))
    return rows


def main():
    for run in sys.argv[1:]:
        print(f"## {run}")
        print("| shot | frame | world | post | air | shafts | bloom | composite | gates |")
        print("|---|---|---|---|---|---|---|---|---|")
        for label, frame, world, post, s, gates in parse(run):
            print(f"| {label} | {frame} | {world} | {post} | {s['air']} | {s['shafts']} | {s['bloom']} | "
                  f"{s['composite']} | {gates} |")


if __name__ == "__main__":
    main()
