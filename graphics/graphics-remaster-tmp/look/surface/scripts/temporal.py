"""Round-to-round change on a static camera (the timing captures).

  python temporal.py <run dir> [--md out.md]

The timing tests shoot every view in interleaved rounds (base1 surf1 base2
surf2 base3 surf3) on a fixed camera in one game; any change between the
rounds of one mode is temporal noise (the TAA jitter phase, the turned
occlusion and shadow patterns). Reports the mean absolute 8-bit luma change
between rounds 1 and 2 (and 2 and 3) per mode, over the whole frame and its
bottom third (the near, sunlit ground).
"""
import argparse
import re
from pathlib import Path

import numpy as np
from PIL import Image

PAT = re.compile(r"^\d+_(?P<view>.+?)__(?P<mode>[a-z]+)(?P<round>\d)\.png$")


def luma(p):
    a = np.asarray(Image.open(p).convert("RGB")).astype(np.float64)
    return a[..., 0] * 0.2126 + a[..., 1] * 0.7152 + a[..., 2] * 0.0722


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("--md")
    a = ap.parse_args()
    f = {}
    for p in Path(a.run).rglob("*.png"):
        m = PAT.match(p.name)
        if m:
            f[(m.group("view"), m.group("mode"), int(m.group("round")))] = p
    views = sorted({v for (v, _m, _r) in f})
    out = ["| view | mode | rounds 1-2 whole | 1-2 bottom third | rounds 2-3 whole | 2-3 bottom third |",
           "|---|---|---|---|---|---|"]
    for v in views:
        for mode in ("base", "surf"):
            cells = []
            for r0, r1 in ((1, 2), (2, 3)):
                if (v, mode, r0) in f and (v, mode, r1) in f:
                    d = np.abs(luma(f[(v, mode, r0)]) - luma(f[(v, mode, r1)]))
                    h = d.shape[0]
                    cells += [f"{d.mean():.2f}", f"{d[2 * h // 3:].mean():.2f}"]
                else:
                    cells += ["-", "-"]
            out.append(f"| {v} | {mode} | " + " | ".join(cells) + " |")
    text = "\n".join(out)
    print(text)
    if a.md:
        Path(a.md).write_text(text + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
