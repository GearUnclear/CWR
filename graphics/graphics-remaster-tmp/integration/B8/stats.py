"""B8 lane image statistics over the tri tree of a run.

    python stats.py noise <run>                  shots that must match on the base build
    python stats.py gate5 <new run> <before run> the new build's off against the base build's
    python stats.py gate6 <run>                  off against on (and the split shots) on one build
    python stats.py diff <a.png> <b.png>

Bands at 1080 rows: sky 0-422, horizon 422-591, ground 591-1080 (banddiff.py's).
Per band: max channel difference / pixels over 2 levels / pixels over 8 levels.
gate6 also gives mean luma (display, 0-255) per band, its change, and the R/B
and R/G ratios of the whole frame.
"""
import sys
from pathlib import Path

import numpy as np
from PIL import Image

BANDS = (("sky", 0, 422), ("horizon", 422, 591), ("ground", 591, 1080))
LUMA = np.array([0.2126, 0.7152, 0.0722])


def shots(run):
    out = {}
    for p in Path(run).glob("tri/**/*.png"):
        stem = p.stem
        if "_" in stem and stem[:3].isdigit():
            out[stem[4:]] = p
    return out


def load(p):
    return np.asarray(Image.open(p).convert("RGB")).astype(np.int16)


def diff(a, b):
    m = np.abs(a - b).max(axis=2)
    cells = []
    for _, lo, hi in BANDS:
        s = m[lo:hi]
        cells.append(f"{int(s.max())} / {int((s > 2).sum()):,} / {int((s > 8).sum()):,}")
    cells.append(f"{int(m.max())} / {int((m > 2).sum()):,} / {int((m > 8).sum()):,}")
    return cells


def luma(a):
    return (a[..., :3].astype(np.float64) * LUMA).sum(axis=2)


def table(rows, head):
    print("| " + " | ".join(head) + " |")
    print("|" + "---|" * len(head))
    for r in rows:
        print("| " + " | ".join(r) + " |")


def views(names):
    return sorted({n.split("__")[0] for n in names})


def main():
    cmd = sys.argv[1]
    if cmd == "diff":
        print(diff(load(sys.argv[2]), load(sys.argv[3])))
        return
    if cmd == "noise":
        s = shots(sys.argv[2])
        rows = []
        for v in views(s):
            for a, b in (("off", "on"), ("offnotaa", "onnotaa"), ("off", "beams"), ("off", "haze")):
                if f"{v}__{a}" in s and f"{v}__{b}" in s:
                    rows.append([v, f"{a} vs {b}"] + diff(load(s[f"{v}__{a}"]), load(s[f"{v}__{b}"])))
        table(rows, ["view", "pair", "sky", "horizon", "ground", "whole"])
        return
    if cmd == "gate5":
        new, old = shots(sys.argv[2]), shots(sys.argv[3])
        rows = []
        for v in views(new):
            for a, b in (("off", "on"), ("offnotaa", "onnotaa"), ("offnotaa", "offnotaa")):
                ka, kb = f"{v}__{a}", f"{v}__{b}"
                if ka in new and kb in old:
                    rows.append([v, f"new {a} vs before {b}"] + diff(load(new[ka]), load(old[kb])))
        table(rows, ["view", "pair", "sky", "horizon", "ground", "whole"])
        return
    if cmd == "gate6":
        s = shots(sys.argv[2])
        rows = []
        for v in views(s):
            base = s.get(f"{v}__off")
            if base is None:
                continue
            a = load(base)
            la = luma(a)
            for b_label in ("on", "beams", "haze", "nomist", "onnotaa"):
                kb = f"{v}__{b_label}"
                if kb not in s:
                    continue
                b = load(s[kb])
                ref = a if b_label != "onnotaa" else load(s.get(f"{v}__offnotaa", base))
                lr = luma(ref)
                lb = luma(b)
                cells = []
                for _, lo, hi in BANDS:
                    m0, m1 = lr[lo:hi].mean(), lb[lo:hi].mean()
                    cells.append(f"{m0:.1f} -> {m1:.1f} ({(m1 - m0) / max(m0, 1e-3) * 100:+.1f}%)")
                rb0 = ref[..., 0].mean() / max(ref[..., 2].mean(), 1e-3)
                rb1 = b[..., 0].mean() / max(b[..., 2].mean(), 1e-3)
                rg0 = ref[..., 0].mean() / max(ref[..., 1].mean(), 1e-3)
                rg1 = b[..., 0].mean() / max(b[..., 1].mean(), 1e-3)
                d = np.abs(ref - b).max(axis=2)
                rows.append([v, b_label] + cells + [f"{rb0:.3f} -> {rb1:.3f}", f"{rg0:.3f} -> {rg1:.3f}",
                                                    f"{int((d > 2).sum()):,}"])
        table(rows, ["view", "shot vs off", "sky luma", "horizon luma", "ground luma", "R/B", "R/G", "px > 2"])
        return
    raise SystemExit(__doc__)


if __name__ == "__main__":
    main()
