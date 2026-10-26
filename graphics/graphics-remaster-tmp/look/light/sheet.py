"""Contact sheet at thumbnail size: one row per view, one column per mode.

    python sheet.py <run dir> --modes classic,on --titles "Original (Classic),Overhaul" --out sheet.png [--w 480]
                    [--views a,b] [--runs dir2:mode2 ...]

A column can come from another run: give the mode as <run dir>::<mode>.
"""
import argparse
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

VIEWS = ["lowsun_fp", "noon_fields", "forest_edge_golden", "sunrise_ridge", "coastline_sunset", "overcast_broken",
         "town_night", "sinai_desert_noon"]


def font(size):
    for name in ("arial.ttf", "segoeui.ttf", "DejaVuSans.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            continue
    return ImageFont.load_default()


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("run")
    ap.add_argument("--modes", required=True)
    ap.add_argument("--titles", default="")
    ap.add_argument("--views", default=",".join(VIEWS))
    ap.add_argument("--w", type=int, default=480)
    ap.add_argument("--out", required=True)
    a = ap.parse_args()
    run = Path(a.run)
    modes = [m.strip() for m in a.modes.split(",")]
    titles = [t.strip() for t in a.titles.split(",")] if a.titles else modes
    views = [v.strip() for v in a.views.split(",") if v.strip()]
    tw = a.w
    th = tw * 9 // 16
    head = 26
    lab = 18
    W = tw * len(modes) + 4 * (len(modes) + 1)
    H = head + len(views) * (th + lab + 4) + 4
    sheet = Image.new("RGB", (W, H), (24, 24, 24))
    d = ImageDraw.Draw(sheet)
    f = font(15)
    fs = font(12)
    for i, t in enumerate(titles):
        d.text((4 + i * (tw + 4) + 6, 5), t, fill=(240, 240, 240), font=f)
    for r, v in enumerate(views):
        y = head + r * (th + lab + 4)
        d.text((8, y), v, fill=(200, 200, 200), font=fs)
        for i, m in enumerate(modes):
            src_run = run
            mode = m
            if "::" in m:
                rd, mode = m.split("::", 1)
                src_run = Path(rd)
            p = src_run / f"{v}__{mode}.png"
            x = 4 + i * (tw + 4)
            if not p.exists():
                d.rectangle([x, y + lab, x + tw, y + lab + th], outline=(90, 90, 90))
                d.text((x + 8, y + lab + 8), f"missing {p.name}", fill=(220, 80, 80), font=fs)
                continue
            im = Image.open(p).convert("RGB")
            im = im.resize((tw, im.height * tw // im.width), Image.LANCZOS)
            sheet.paste(im.crop((0, 0, tw, min(th, im.height))), (x, y + lab))
    sheet.save(a.out)
    print(a.out, sheet.size)


if __name__ == "__main__":
    main()
