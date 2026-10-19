"""Contact sheet of Original | Overhaul pairs at thumbnail size (the look director).

    python make_look_sheet.py OUT.png [--panel 480] [--per-row 2] PAIR [PAIR ...]

PAIR is `name=classic.png,overhaul.png` (any image size; each panel is scaled to
--panel px wide at 3:2 or the image's own aspect). The sheet is laid out in rows
of --per-row pairs, each pair labelled with its view name over the two panels
("Original" left, "Overhaul" right).
"""
import argparse

from PIL import Image, ImageDraw, ImageFont


def font(size):
    for name in ("segoeui.ttf", "arial.ttf", "DejaVuSans.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            continue
    return ImageFont.load_default()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("out")
    ap.add_argument("pairs", nargs="+")
    ap.add_argument("--panel", type=int, default=480)
    ap.add_argument("--per-row", type=int, default=2)
    ap.add_argument("--title", default="Uslu dur! look overhaul: Original | Overhaul")
    a = ap.parse_args()
    W = a.panel
    H = W * 9 // 16
    gap, pgap, lab = 6, 18, 22
    f_lab, f_title = font(15), font(22)
    items = []
    for p in a.pairs:
        name, files = p.split("=", 1)
        c, o = files.split(",")
        items.append((name, c, o))
    rows = (len(items) + a.per_row - 1) // a.per_row
    pair_w = 2 * W + gap
    sheet_w = a.per_row * pair_w + (a.per_row - 1) * pgap + 2 * pgap
    top = 44
    sheet_h = top + rows * (H + lab + pgap) + pgap
    sheet = Image.new("RGB", (sheet_w, sheet_h), (24, 24, 26))
    d = ImageDraw.Draw(sheet)
    d.text((pgap, 10), a.title, fill=(235, 235, 235), font=f_title)
    for i, (name, c, o) in enumerate(items):
        r, k = divmod(i, a.per_row)
        x0 = pgap + k * (pair_w + pgap)
        y0 = top + r * (H + lab + pgap)
        d.text((x0, y0 + 2), f"{name}   Original | Overhaul", fill=(230, 210, 120), font=f_lab)
        for j, path in enumerate((c, o)):
            im = Image.open(path).convert("RGB")
            # Fit to W x H (crop to 16:9 from the centre when the aspect differs).
            ar = im.width / im.height
            if ar > 16 / 9:
                nw = int(im.height * 16 / 9)
                im = im.crop(((im.width - nw) // 2, 0, (im.width - nw) // 2 + nw, im.height))
            elif ar < 16 / 9:
                nh = int(im.width * 9 / 16)
                im = im.crop((0, (im.height - nh) // 2, im.width, (im.height - nh) // 2 + nh))
            im = im.resize((W, H), Image.LANCZOS)
            sheet.paste(im, (x0 + j * (W + gap), y0 + lab))
    sheet.save(a.out)
    print(a.out, sheet.size, len(items), "pairs")


if __name__ == "__main__":
    main()
