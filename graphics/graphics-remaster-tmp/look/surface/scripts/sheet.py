r"""Contact sheet for the look-surface lane.

  python sheet.py <out.png> --run <dir> [--run <dir> ...] --cols classic,base,surf
        [--labels "Original,Remastered,Overhaul"] [--width 480] [--views a,b] [--crop x0,y0,x1,y1]

Finds <seq>_<view>__<suffix>.png under every run's tri\ tree (later runs win),
lays one row per view, one column per suffix, each panel `width` px wide.
A column given as run_index:suffix takes that suffix from that run only.
"""
import argparse
import re
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

PAT = re.compile(r"^\d+_(?P<view>.+?)__(?P<suf>[^_].*)\.png$")


def collect(runs):
    found = []  # per run: {(view, suf): path}
    for r in runs:
        d = {}
        for p in Path(r).rglob("*.png"):
            m = PAT.match(p.name)
            if m:
                d[(m.group("view"), m.group("suf"))] = p
        found.append(d)
    return found


def font(size):
    for f in ("arialbd.ttf", "arial.ttf", "DejaVuSans-Bold.ttf"):
        try:
            return ImageFont.truetype(f, size)
        except OSError:
            pass
    return ImageFont.load_default()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("--run", action="append", required=True)
    ap.add_argument("--cols", required=True)
    ap.add_argument("--labels", default="")
    ap.add_argument("--width", type=int, default=480)
    ap.add_argument("--views", default="")
    ap.add_argument("--crop", default="")
    ap.add_argument("--title", default="")
    a = ap.parse_args()
    found = collect(a.run)
    cols = a.cols.split(",")
    labels = a.labels.split(",") if a.labels else cols
    views = [v for v in a.views.split(",") if v]
    if not views:
        seen = []
        for d in found:
            for (v, _s) in d:
                if v not in seen:
                    seen.append(v)
        views = seen
    crop = tuple(int(x) for x in a.crop.split(",")) if a.crop else None

    def lookup(view, col):
        if ":" in col:
            i, suf = col.split(":", 1)
            return found[int(i)].get((view, suf))
        for d in reversed(found):
            if (view, col) in d:
                return d[(view, col)]
        return None

    W = a.width
    rows = []
    for v in views:
        ims = []
        for c in cols:
            p = lookup(v, c)
            if p is None:
                ims.append(None)
                continue
            im = Image.open(p).convert("RGB")
            if crop:
                im = im.crop(crop)
            h = round(im.height * W / im.width)
            ims.append(im.resize((W, h), Image.LANCZOS))
        if all(i is None for i in ims):
            continue
        H = max(i.height for i in ims if i is not None)
        rows.append((v, ims, H))
    head = 34 if a.title else 0
    lab = 22
    total_h = head + sum(lab + H for _, _, H in rows)
    sheet = Image.new("RGB", (W * len(cols) + 4 * (len(cols) - 1), total_h), (20, 20, 20))
    dr = ImageDraw.Draw(sheet)
    f1, f2 = font(16), font(22)
    if a.title:
        dr.text((8, 5), a.title, fill=(235, 235, 235), font=f2)
    y = head
    for v, ims, H in rows:
        for i, im in enumerate(ims):
            x = i * (W + 4)
            dr.text((x + 6, y + 2), f"{labels[i]}  |  {v}", fill=(230, 230, 230), font=f1)
            if im is not None:
                sheet.paste(im, (x, y + lab))
        y += lab + H
    sheet.save(a.out)
    print(a.out, sheet.size)


if __name__ == "__main__":
    main()
