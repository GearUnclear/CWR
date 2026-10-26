"""Labelled comparison of one shot across modes, larger than the sheets.

    python make_best.py <run_dir> <label with {m}> <modes> <out.png> --title T [--scale 0.75]
        [--crop x0,y0,x1,y1] [--names "c=Classic;x=..."] [--cols N] [--timing-from <label with {m}>]

Each panel carries the mode name and the GPU frame time the capture logged for
that mode (the timer average of the settle frames before the shot).
"""
import argparse
import sys
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).parent))
import collect as C  # noqa: E402
from make_gif import NAMES  # noqa: E402


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("label")
    ap.add_argument("modes")
    ap.add_argument("out")
    ap.add_argument("--title", default="")
    ap.add_argument("--scale", type=float, default=0.75)
    ap.add_argument("--crop", default="")
    ap.add_argument("--names", default="")
    ap.add_argument("--cols", type=int, default=0)
    ap.add_argument("--timing-from", default="")
    a = ap.parse_args()
    names = dict(NAMES)
    for kv in filter(None, a.names.split(";")):
        k, v = kv.split("=", 1)
        names[k] = v
    run = Path(a.run)
    t = C.load_timings(run)
    panels = []
    tmp = run / "_crop_tmp.png"
    for m in a.modes:
        label = a.label.format(m=m)
        src = run / "shots" / f"{label}.png"
        if a.crop:
            box = tuple(int(v) for v in a.crop.split(","))
            Image.open(src).convert("RGB").crop(box).save(tmp)
            src = tmp
        tl = t.get(a.timing_from.format(m=m)) if a.timing_from else t.get(label)
        panels.append(C.panel(src, names[m], C.gpu_text(tl), a.scale))
    if tmp.exists():
        tmp.unlink()
    C.sheet(panels, a.title or a.label, a.cols or len(panels), run.parent / a.out if a.out.startswith("BEST") else run / a.out)


if __name__ == "__main__":
    main()
