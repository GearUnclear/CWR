"""Crown statistics for the shade track: mean F, light hue and the spread of
on-screen brightness inside a crown (p10 / p90 of luma over foliage pixels).

Foliage pixels: the albedo shot (debug 3) is greener than it is blue, so the
sky showing through the crown is left out.

    python crownstats.py <run_dir> <view> [label ...]
"""
import json
import sys
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).parent))
import measure as m  # noqa: E402

m.load_boxes()


def luma(p):
    return 0.2126 * p[0] + 0.7152 * p[1] + 0.0722 * p[2]


def stats(run, view, label, box):
    alb = Image.open(m.find_shot(run, view, "albedo")).convert("RGB").crop(box)
    im = Image.open(m.find_shot(run, view, label)).convert("RGB").crop(box)
    a_px, s_px = list(alb.getdata()), list(im.getdata())
    keep = [(a, s) for a, s in zip(a_px, s_px) if a[1] > a[2] + 5]
    if len(keep) < 20:
        return None
    sa = [0.0, 0.0, 0.0]
    ss = [0.0, 0.0, 0.0]
    for a, s in keep:
        for i in range(3):
            sa[i] += a[i]
            ss[i] += s[i]
    n = len(keep)
    sa = [v / n / 255 for v in sa]
    ss = [v / n / 255 for v in ss]
    F = luma(ss) / max(luma(sa), 1e-6)
    L = [(ss[i] / max(sa[i], 1e-6)) ** 1.2 for i in range(3)]
    ls = sorted(luma(s) for _, s in keep)
    p10, p50, p90 = ls[int(0.1 * n)], ls[int(0.5 * n)], ls[int(0.9 * n)]
    # saturation of the shot inside the crown (display space, mean of (max-min)/max)
    sat = sum((max(s) - min(s)) / max(max(s), 1) for _, s in keep) / n
    return {"n": n, "F": F, "gr": L[1] / L[0], "rb": L[0] / L[2], "p10": p10, "p50": p50, "p90": p90,
            "spread": p10 / max(p90, 1e-6), "sat": sat}


def main():
    run, view = sys.argv[1], sys.argv[2]
    labels = sys.argv[3:] or [l for l in m.labels_for(run, view) if l != "albedo"]
    boxes = {k: v for k, v in m.BOXES[view].items() if k.startswith(("crown", "bush"))}
    out = {}
    print(f"\n## {view} crowns: F / g:r / r:b / p10:p90 / sat")
    print("| label | " + " | ".join(boxes) + " |")
    print("|---|" + "---|" * len(boxes))
    for lab in labels:
        if not m.find_shot(run, view, lab):
            continue
        row = {}
        cells = []
        for name, box in boxes.items():
            s = stats(run, view, lab, box)
            row[name] = s
            cells.append("-" if s is None else f"{s['F']:.2f} / {s['gr']:.2f} / {s['rb']:.2f} / {s['spread']:.2f} / {s['sat']:.2f}")
        out[lab] = row
        print(f"| {lab} | " + " | ".join(cells) + " |")
    Path(run, f"crowns_{view}.json").write_text(json.dumps(out, indent=1))


if __name__ == "__main__":
    main()
