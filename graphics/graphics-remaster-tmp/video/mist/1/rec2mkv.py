#!/usr/bin/env python
"""Agent 1: turn a mist-1 recorder dump (raw RGB24 bottom-up + .txt sidecar) into an mkv
(libx264rgb CRF 4, the same codec collect_frames.py uses for the trailer clips), print the
pacing, and delete the raw file once the mkv is written.

  python rec2mkv.py <dump.raw> [--keep-raw]
"""
import json
import os
import subprocess
import sys

import numpy as np

FF = "C:/ffmpeg/bin/ffmpeg.exe"


def main():
    raw = sys.argv[1]
    keep = "--keep-raw" in sys.argv
    lines = open(raw + ".txt").read().split("\n")
    hdr = dict(kv.split("=") for kv in lines[0].split())
    w, h, n = int(hdr["w"]), int(hdr["h"]), int(hdr["frames"])
    rows = [l.split() for l in lines[1:] if l.strip()]
    wall = np.array([float(r[1]) for r in rows])
    pan = np.array([float(r[2]) for r in rows])
    shot = np.array([int(r[3]) for r in rows])
    dw = np.diff(wall)
    dp = np.diff(pan)
    info = dict(src=raw, w=w, h=h, frames=n, mode=hdr.get("mode"), half=hdr.get("half"),
                wall_ms=round(float(wall[-1] - wall[0]), 1),
                fps=round(float((n - 1) / max(1e-6, (wall[-1] - wall[0]) / 1000.0)), 2),
                dwall_ms=dict(min=round(float(dw.min()), 2), med=round(float(np.median(dw)), 2), max=round(float(dw.max()), 2)),
                dpan_ms=dict(min=round(float(dp.min() * 1000), 2), med=round(float(np.median(dp) * 1000), 2),
                             max=round(float(dp.max() * 1000), 2)),
                zero_sim_frames=int((dp == 0).sum()), shots=int(shot.sum()),
                pan_first=round(float(pan[0]), 4), pan_last=round(float(pan[-1]), 4))
    size = os.path.getsize(raw)
    assert size == w * h * 3 * n, (size, w * h * 3 * n)
    out = os.path.splitext(raw)[0] + ".mkv"
    subprocess.run([FF, "-hide_banner", "-loglevel", "error", "-y", "-f", "rawvideo", "-pix_fmt", "rgb24",
                    "-s", f"{w}x{h}", "-framerate", "60", "-i", raw, "-vf", "vflip", "-c:v", "libx264rgb",
                    "-crf", "4", "-preset", "fast", out], check=True)
    info["mkv"] = out
    json.dump(info, open(os.path.splitext(raw)[0] + ".pacing.json", "w"), indent=1)
    print(json.dumps(info))
    if not keep and os.path.exists(out) and os.path.getsize(out) > 0:
        os.remove(raw)


if __name__ == "__main__":
    main()
