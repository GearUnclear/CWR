"""Before | after comparison sheets for the clear-sky-clouds fix.

usage: make_cmp.py <before_dir> <after_dir> <out_dir> <measure_before.json> <measure_after.json>
Each sheet: one row per mode (remastered, dbg1 = sun visibility), columns
BEFORE (branch tip exe) | AFTER (fix exe), same view, same test script."""
import json
import pathlib
import sys

from PIL import Image, ImageDraw, ImageFont


def font(size):
    for f in ("C:/Windows/Fonts/segoeui.ttf", "C:/Windows/Fonts/arial.ttf"):
        try:
            return ImageFont.truetype(f, size)
        except OSError:
            pass
    return ImageFont.load_default()


F = font(18)
FT = font(22)


def label(draw, xy, text):
    x, y = xy
    l, t, r, b = draw.textbbox((x, y), text, font=F)
    draw.rectangle((l - 4, t - 3, r + 4, b + 3), fill=(0, 0, 0))
    draw.text((x, y), text, font=F, fill=(255, 255, 255))


def main():
    before, after, out = (pathlib.Path(p) for p in sys.argv[1:4])
    mb = json.loads(pathlib.Path(sys.argv[4]).read_text())
    ma = json.loads(pathlib.Path(sys.argv[5]).read_text())
    out.mkdir(parents=True, exist_ok=True)
    titles = {
        "clear_high_n": "Abel, clear sky (setOvercast 0), noon, 700 m up looking north",
        "clear_high_s": "Abel, clear sky (setOvercast 0), noon, 700 m up looking south",
        "forest_edge_noon": "Abel, clear sky (setOvercast 0), noon, eye level",
        "coastline_sunset": "Abel, clear sky (setOvercast 0), sun 8 deg, eye level",
        "light_high_n": "Abel, light cloud (setOvercast 0.1), noon, 700 m up",
        "broken_high": "Eden, broken cloud (setOvercast 0.5), 250 m up",
        "overcast_broken": "Eden, broken cloud (setOvercast 0.5), eye level",
        "overcast_heavy": "Eden, heavy overcast (setOvercast 0.9), eye level",
    }
    for view, title in titles.items():
        rows = [m for m in ("remastered", "dbg1") if (before / f"{view}__{m}.png").exists() and (after / f"{view}__{m}.png").exists()]
        if not rows:
            continue
        w, h = Image.open(before / f"{view}__{rows[0]}.png").size
        gap = 8
        head = 40
        sheet = Image.new("RGB", (2 * w + gap, head + len(rows) * (h + gap)), (18, 18, 18))
        d = ImageDraw.Draw(sheet)
        d.text((10, 8), title, font=FT, fill=(235, 235, 235))
        for i, mode in enumerate(rows):
            y = head + i * (h + gap)
            for j, (src, m, tag) in enumerate(((before, mb, "BEFORE (branch tip)"), (after, ma, "AFTER (fix)"))):
                x = j * (w + gap)
                sheet.paste(Image.open(src / f"{view}__{mode}.png").convert("RGB"), (x, y))
                share = m.get(view, {}).get("cloud_shadow_share_of_frame")
                what = "remastered" if mode == "remastered" else "sun visibility (debug 1)"
                text = f"{tag}: {what}"
                if mode == "dbg1" and share is not None:
                    text += f", cloud-shadowed {100 * share:.1f}% of frame"
                label(d, (x + 10, y + 10), text)
        sheet.save(out / f"cmp_{view}.png")
        print("wrote", out / f"cmp_{view}.png")


if __name__ == "__main__":
    main()
