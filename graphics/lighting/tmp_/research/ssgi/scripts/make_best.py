"""BEST_<view>.png: Classic | Remastered | Remastered + SSGI variant | change x8.

python make_best.py <out dir> <view> <run dir> <variant label> <caption>

The fourth panel is |variant - Remastered| per pixel, amplified 8x (8-bit levels),
so a change too small to see in the third panel still shows where it lands.
GPU frame ms per shot come from the UDCAP shot lines of that run.
"""
import re
import sys
from pathlib import Path

import numpy as np
from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
from compare import font, hstack, panel, parse_gpu  # noqa: E402


def find(run, view, label):
    for png in run.glob("tri/**/*.png"):
        m = re.match(r"\d+_(.+?)__(.+)\.png$", png.name)
        if m and m.group(1) == view and m.group(2) == label:
            return png
    raise SystemExit(f"missing {view}__{label} in {run}")


def main():
    out, view, run, label, caption = Path(sys.argv[1]), sys.argv[2], Path(sys.argv[3]), sys.argv[4], sys.argv[5]
    gpu = {}
    for log in run.glob("tri/**/game_stdout.log"):
        gpu.update(parse_gpu(log))
    imgs = {k: Image.open(find(run, view, k)).convert("RGB") for k in ("classic", "remastered", label)}
    a = np.asarray(imgs["remastered"]).astype(float)
    b = np.asarray(imgs[label]).astype(float)
    d = np.abs(b - a).mean(axis=2) * 8.0
    heat = np.clip(np.stack([d, d * 0.6, d * 0.25], axis=2), 0, 255).astype(np.uint8)
    stats = f"mean {np.abs(b - a).mean():.2f}, p99 {np.percentile(np.abs(b - a).mean(axis=2), 99):.1f} levels"
    f1, f2 = font(18), font(16)
    ms = lambda k: gpu.get(f"{view}__{k}", {}).get("frame")
    ps = [
        panel(imgs["classic"], "Classic", ms("classic"), f1, f2),
        panel(imgs["remastered"], "Remastered (current)", ms("remastered"), f1, f2),
        panel(imgs[label], caption, ms(label), f1, f2),
        panel(Image.fromarray(heat), f"|change| x8 ({stats})", None, f1, f2),
    ]
    out.mkdir(parents=True, exist_ok=True)
    hstack(ps).save(out / f"BEST_{view}.png")
    print("BEST", view, label, stats)


if __name__ == "__main__":
    main()
