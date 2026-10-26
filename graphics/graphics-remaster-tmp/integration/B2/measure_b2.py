"""B2 lane measurements on the 1920x1080 captures (the shade track's method).

    python measure_b2.py <after run> <before run> [--boxes]

For each view with an albedo shot (<view>_alb__remastered.png, the debug=6
view with post off, from the after run) and each named box:

  F    on-screen fraction of the texture brightness, luma(shot) / luma(albedo)
  L    colour of the light on the surface, (mean(shot) / mean(albedo)) ** 1.2 per
       channel (undoes the 1.2 display gamma), reported as g:r and r:b (lower g:r
       and higher r:b are warmer)

for classic, off, on (after run) and before (the before run's __on). Hull
shade/lit is F(shaded face) / F(lit face) of the same paint.
"""
import json
import sys
from pathlib import Path

from PIL import Image, ImageDraw

BOXES = {
    "lowsun_box": {
        "m113_lit": (985, 452, 1130, 498),
        "m113_shade": (1250, 452, 1290, 488),
        "jeep_lit": (470, 545, 520, 590),
        "jeep_shade": (575, 500, 625, 545),
        "ground_lit": (300, 800, 700, 900),
        "ground_shadow": (1300, 590, 1500, 610),
    },
}


def luma(c):
    return 0.2126 * c[0] + 0.7152 * c[1] + 0.0722 * c[2]


def mean_rgb(im, box, mask=None):
    c = im.crop(box[:4])
    px = list(c.getdata())
    if mask is not None:
        px = [p for p, keep in zip(px, mask) if keep]
    n = max(len(px), 1)
    return tuple(sum(p[i] for p in px) / n / 255.0 for i in range(3))


def foliage_mask(alb, box):
    # Foliage pixels of a crown box: the albedo is greener than it is blue
    # (the sky between the leaves is not) and not near-black.
    px = list(alb.crop(box[:4]).getdata())
    return [p[1] >= p[2] + 8 and p[1] > 25 for p in px]


def shot(run, view, label):
    p = Path(run) / f"{view}__{label}.png"
    return Image.open(p).convert("RGB") if p.exists() else None


def measure(after, before, view, boxes):
    alb = Image.open(Path(after) / f"{view}_alb__remastered.png").convert("RGB")
    shots = {"classic": shot(after, view, "classic"), "before": shot(before, view, "on"),
             "off": shot(after, view, "off"), "on": shot(after, view, "on")}
    out = {}
    for lab, im in shots.items():
        if im is None:
            continue
        row = {}
        for name, box in boxes.items():
            mask = foliage_mask(alb, box) if len(box) > 4 and box[4] == "foliage" else None
            a = mean_rgb(alb, box, mask)
            s = mean_rgb(im, box, mask)
            F = luma(s) / max(luma(a), 1e-6)
            L = tuple((s[i] / max(a[i], 1e-6)) ** 1.2 for i in range(3))
            row[name] = {"F": F, "gr": L[1] / max(L[0], 1e-6), "rb": L[0] / max(L[2], 1e-6), "shot": s, "alb": a}
        out[lab] = row
    return out


def table(view, res):
    names = list(next(iter(res.values())).keys())
    lines = [f"\n### {view}", "", "F / g:r / r:b per box", "",
             "| shot | " + " | ".join(names) + " |", "|---|" + "---|" * len(names)]
    for lab, row in res.items():
        cells = [f"{row[n]['F']:.2f} / {row[n]['gr']:.2f} / {row[n]['rb']:.2f}" for n in names]
        lines.append(f"| {lab} | " + " | ".join(cells) + " |")
    pairs = [(n[:-4], n, n[:-4] + "_shade") for n in names if n.endswith("_lit") and n[:-4] + "_shade" in names]
    if pairs:
        lines += ["", "Hull shade/lit (F of the shaded face over F of the lit face):", "",
                  "| shot | " + " | ".join(p[0] for p in pairs) + " |", "|---|" + "---|" * len(pairs)]
        for lab, row in res.items():
            lines.append(f"| {lab} | " + " | ".join(f"{row[s]['F'] / row[l]['F']:.2f}" for _, l, s in pairs) + " |")
    return "\n".join(lines)


def draw(after, view, boxes):
    im = Image.open(Path(after) / f"{view}__on.png").convert("RGB")
    d = ImageDraw.Draw(im)
    for n, b in boxes.items():
        d.rectangle(b[:4], outline=(255, 0, 255), width=2)
        d.text((b[0] + 2, b[1] - 12), n, fill=(255, 255, 0))
    p = Path(after) / f"boxes_{view}.png"
    im.save(p)
    return p


def main():
    after, before = sys.argv[1], sys.argv[2]
    boxes_file = Path(__file__).with_name("boxes_b2.json")
    if boxes_file.exists():
        BOXES.update({k: {n: tuple(b) for n, b in v.items()} for k, v in json.loads(boxes_file.read_text()).items()})
    if "--boxes" in sys.argv:
        for v, b in BOXES.items():
            print(draw(after, v, b))
        return
    md = ["# B2 measurements", "", f"after: {after}", f"before: {before}"]
    allres = {}
    for v, b in BOXES.items():
        if not (Path(after) / f"{v}_alb__remastered.png").exists():
            continue
        r = measure(after, before, v, b)
        allres[v] = r
        md.append(table(v, r))
    text = "\n".join(md) + "\n"
    print(text)
    Path(after, "measure_b2.md").write_text(text, encoding="utf-8")
    Path(after, "measure_b2.json").write_text(json.dumps(allres, indent=1), encoding="utf-8")


if __name__ == "__main__":
    main()
