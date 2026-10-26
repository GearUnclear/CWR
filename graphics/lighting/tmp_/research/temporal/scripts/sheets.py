"""Small CLI-free helpers to build labelled comparison sheets for the temporal
track from collected shots (<run>/shots/<label>.png) and timings
(<run>/timings.json, written by make_compare.py / collect_run)."""

import json
import sys
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).parent))
import taalib as T  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent


def collect_run(run):
    run = ROOT / run
    T.collect(run / "tri", run / "shots")
    rows, udcap = T.timings(run / "tri")
    T.save_json(run / "timings.json", rows)
    (run / "udcap.log").write_text("\n".join(udcap), encoding="utf-8")
    return rows


def shot(run, label):
    p = ROOT / run / "shots" / f"{label}.png"
    return Image.open(p).convert("RGB") if p.exists() else None


def timing(run, label):
    p = ROOT / run / "timings.json"
    if not p.exists():
        return None
    return json.loads(p.read_text(encoding="utf-8")).get(label)


def full_sheet(out, header, entries, cols=None):
    """entries: [(title, run, label)] -> full frames with GPU frame ms."""
    panels = []
    for title, run, label in entries:
        im = shot(run, label)
        if im is None:
            print("missing", run, label)
            continue
        t = timing(run, label)
        sub = f"GPU frame {t['frame']:.2f} ms (single sample, GPU shared by 4 games: see NOTES)" if t and t.get("frame") is not None else ""
        panels.append(T.label_panel(im, title, sub))
    if panels:
        T.sheet(panels, cols or len(panels), header).save(ROOT / out)
    return len(panels)


def crop_sheet(out, header, entries, boxes, scale=2):
    """Rows = boxes, columns = entries; nearest-neighbour enlargement."""
    panels = []
    cols = 0
    for box in boxes:
        n = 0
        for title, run, label in entries:
            im = shot(run, label)
            if im is None:
                print("missing", run, label)
                continue
            c = im.crop(box)
            c = c.resize((c.width * scale, c.height * scale), Image.NEAREST)
            panels.append(T.label_panel(c, title, "", 0.8))
            n += 1
        cols = max(cols, n)
    if panels:
        T.sheet(panels, cols, header).save(ROOT / out)
    return len(panels)


def gif(out, rows, box=None, scale=1, duration=250):
    """rows: [(title, run, [labels...])] played side by side, frame by frame."""
    frames = []
    n = min(len(r[2]) for r in rows)
    for i in range(n):
        panels = []
        for title, run, labels in rows:
            im = shot(run, labels[i])
            if im is None:
                return 0
            if box:
                im = im.crop(box)
            if scale != 1:
                im = im.resize((int(im.width * scale), int(im.height * scale)), Image.NEAREST if scale > 1 else Image.LANCZOS)
            panels.append(T.label_panel(im, f"{title} f{i}", "", 0.8))
        frames.append(T.sheet(panels, len(panels)))
    frames[0].save(ROOT / out, save_all=True, append_images=frames[1:], duration=duration, loop=0)
    return n


def grid_sheet(out, header, rows, box, scale=2):
    """rows: [[(title, run, label), ...], ...] all cropped with the same box."""
    panels = []
    cols = max(len(r) for r in rows)
    for r in rows:
        for title, run, label in r:
            im = shot(run, label)
            if im is None:
                print("missing", run, label)
                continue
            c = im.crop(box)
            c = c.resize((c.width * scale, c.height * scale), Image.NEAREST)
            panels.append(T.label_panel(c, title, "", 0.7))
    if panels:
        T.sheet(panels, cols, header).save(ROOT / out)
    return len(panels)