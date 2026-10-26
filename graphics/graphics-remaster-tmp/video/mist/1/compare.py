#!/usr/bin/env python
"""Agent 1: bandmetric of recorder dumps over a common pan-time window.

  python compare.py <name.mkv> [<name2.mkv> ...] [--t0 0] [--t1 2]

For each mkv, reads <name>.raw.txt (the recorder sidecar, kept next to the mkv) to get
each frame's pan time (accumulated sim seconds since the pan start), keeps the frames whose
pan time lies in [t0, t1] and runs tools/bandmetric.py on that frame range. Also prints the
metric over the whole dump.
"""
import json
import os
import subprocess
import sys

BM = "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/mist/tools/bandmetric.py"


def band(mkv, a, b):
    p = subprocess.run([sys.executable, BM, mkv, "--from", str(a), "--to", str(b)], capture_output=True, text=True)
    return json.loads(p.stdout.strip().split("\n")[-1])


def main():
    args = [x for x in sys.argv[1:]]
    t0, t1 = 0.0, 2.0
    if "--t0" in args:
        i = args.index("--t0"); t0 = float(args[i + 1]); del args[i:i + 2]
    if "--t1" in args:
        i = args.index("--t1"); t1 = float(args[i + 1]); del args[i:i + 2]
    for mkv in args:
        side = os.path.splitext(mkv)[0] + ".raw.txt"
        rows = [l.split() for l in open(side).read().split("\n")[1:] if l.strip()]
        pan = [float(r[2]) for r in rows]
        idx = [i for i, p in enumerate(pan) if t0 <= p <= t1]
        a, b = idx[0], idx[-1] + 1
        r = band(mkv, a, b)
        whole = band(mkv, 0, len(pan))
        print(json.dumps(dict(mkv=os.path.basename(mkv), window=[t0, t1], frames=[a, b], pan=[round(pan[a], 4), round(pan[b - 1], 4)],
                              band=r["band"], band_max=r["band_max"], sky=r["sky"], spikes=r["spikes"],
                              spikes_per_frame=round(r["spikes"] / max(1, r["frames"] - 2), 1),
                              whole_band=whole["band"], whole_frames=whole["frames"])))


if __name__ == "__main__":
    main()
