"""Pixel difference between captures, per row band (sky / horizon / ground).

    python banddiff.py <a.png> <b.png> [--bands 0-250,250-350,350-640]
    python banddiff.py <run_a> <run_b> --label-a off --label-b on [--md out.md] [--json out.json]
    python banddiff.py --pairs <run> [--label-a off --label-b on] [--md out.md] [--json out.json]

Per band it prints the largest channel difference, the pixels over 2 levels and
the pixels over 8 levels ("max / >2 / >8"), then the whole frame. The default
bands are the capture set's rows 0-250 (sky), 250-350 (horizon band) and
350-640 (ground) of a 640-row frame, scaled to the image height (0-422,
422-591, 591-1080 at 1080 rows).

Two folders: every <view>__<label-a>.png of run A against <view>__<label-b>.png
of run B (gate item 5, "off equals baseline": the new build's __off against the
previous build's __on). --pairs: within one run, <view>__<label-a>.png against
<view>__<label-b>.png (gate item 6, or the repeat noise on a build where off and
on draw the same frame).

Judge a difference against the repeat noise of the same view (INTEGRATION-PLAN.md
5 A0): the Sinai ground band, the crew views and fire and smoke are not
repeatable between boots.
"""
import argparse
import json
import sys
from pathlib import Path

import numpy as np
from PIL import Image

# The capture set's band edges as shares of a 640-row frame.
DEFAULT_BANDS = ((0, 250), (250, 350), (350, 640))
BAND_NAMES = ("sky", "horizon", "ground")


def load(path):
    return np.asarray(Image.open(path).convert("RGB")).astype(np.int16)


def bands_for(height, spec):
    if spec:
        out = []
        for band in spec.split(","):
            lo, hi = (int(x) for x in band.split("-"))
            out.append((lo, hi))
        return out
    return [(int(round(lo * height / 640)), int(round(hi * height / 640))) for lo, hi in DEFAULT_BANDS]


def diff(a, b, spec=None):
    """Per band and whole frame: (max, pixels over 2, pixels over 8, pixels)."""
    if a.shape != b.shape:
        raise ValueError(f"sizes differ: {a.shape[1]}x{a.shape[0]} against {b.shape[1]}x{b.shape[0]}")
    m = np.abs(a - b).max(axis=2)
    out = []
    for lo, hi in bands_for(m.shape[0], spec):
        s = m[lo:hi]
        out.append({"rows": f"{lo}-{hi}", "max": int(s.max()), "over2": int((s > 2).sum()),
                    "over8": int((s > 8).sum()), "pixels": int(s.size)})
    out.append({"rows": "whole", "max": int(m.max()), "over2": int((m > 2).sum()), "over8": int((m > 8).sum()),
                "pixels": int(m.size)})
    return out


def line(cells):
    parts = [f"rows {c['rows']}: {c['max']} / {c['over2']} / {c['over8']}" for c in cells[:-1]]
    w = cells[-1]
    return " | ".join(parts) + f" | whole: {w['max']} / {w['over2']} / {w['over8']} of {w['pixels']}"


def compare_runs(dir_a, dir_b, label_a, label_b, spec):
    rows = []
    for pa in sorted(Path(dir_a).glob(f"*__{label_a}.png")):
        view = pa.name[: -len(f"__{label_a}.png")]
        pb = Path(dir_b) / f"{view}__{label_b}.png"
        if not pb.exists():
            rows.append({"view": view, "a": str(pa), "b": str(pb), "error": "missing"})
            continue
        try:
            rows.append({"view": view, "a": str(pa), "b": str(pb), "bands": diff(load(pa), load(pb), spec)})
        except ValueError as e:
            rows.append({"view": view, "a": str(pa), "b": str(pb), "error": str(e)})
    return rows


def markdown(rows, title):
    md = [f"# {title}", "",
          "max channel difference / pixels over 2 levels / pixels over 8 levels, per row band", "",
          "| view | sky | horizon | ground | whole |", "|---|---|---|---|---|"]
    for r in rows:
        if "error" in r:
            md.append(f"| {r['view']} | {r['error']} |  |  |  |")
            continue
        cells = [f"{c['max']} / {c['over2']:,} / {c['over8']:,}" for c in r["bands"]]
        while len(cells) < 4:
            cells.insert(-1, "")
        md.append(f"| {r['view']} | " + " | ".join(cells) + " |")
    return "\n".join(md) + "\n"


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("a", nargs="?")
    ap.add_argument("b", nargs="?")
    ap.add_argument("--pairs", default=None, help="one run folder: compare its label-a and label-b shots")
    ap.add_argument("--label-a", default="off")
    ap.add_argument("--label-b", default="on")
    ap.add_argument("--bands", default=None, help="explicit row bands, e.g. 0-250,250-350,350-640")
    ap.add_argument("--md", default=None)
    ap.add_argument("--json", default=None)
    ap.add_argument("--title", default=None)
    o = ap.parse_args(argv)

    if o.pairs:
        rows = compare_runs(o.pairs, o.pairs, o.label_a, o.label_b, o.bands)
        title = o.title or f"{Path(o.pairs).name}: __{o.label_a} against __{o.label_b}"
    elif o.a and o.b and Path(o.a).is_dir() and Path(o.b).is_dir():
        rows = compare_runs(o.a, o.b, o.label_a, o.label_b, o.bands)
        title = o.title or f"{Path(o.a).name} __{o.label_a} against {Path(o.b).name} __{o.label_b}"
    elif o.a and o.b:
        print(line(diff(load(o.a), load(o.b), o.bands)))
        return 0
    else:
        ap.print_help()
        return 2

    for r in rows:
        print(f"{r['view']}: " + (r["error"] if "error" in r else line(r["bands"])))
    if o.md:
        Path(o.md).write_text(markdown(rows, title), encoding="utf-8")
    if o.json:
        Path(o.json).write_text(json.dumps(rows, indent=2), encoding="utf-8")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
