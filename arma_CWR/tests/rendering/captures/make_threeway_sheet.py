"""Three-way look sheet: Original | graphics-remaster as landed | Overhaul, per view.

    python make_threeway_sheet.py --ref REF_DIR --overhaul FINAL_DIR --out SHEET.png
                                  [--strips STRIP_DIR] [--panel 320] [--strip-panel 640]
                                  [--fallback ROUND2_FINAL_DIR] [--views v1,v2,...]

REF_DIR is the reference set (look/ref: index.json, vanilla/<view>.png,
landed/<view>.png). FINAL_DIR holds the overhaul shots, <view>__overhaul.png.
Each row is one view at its own aspect, three panels of --panel px wide, each
panel labelled. A view whose vanilla shot is missing falls back to
FALLBACK/<view>__classic.png, labelled "Original (features off)". With --strips,
one strip per view (--strip-panel px a panel) is written to STRIP_DIR/<view>.png.
The default order puts the reference view town_night_rain first.
"""
import argparse
import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

# The sheet's views: the 1080p look views, the run-captures 960x640 set and the
# rifle in morning mist (the 28 views of the look-director sheet), reference view first.
DEFAULT_VIEWS = [
    "town_night_rain", "town_night", "town_fog_night", "rain_day", "overcast_heavy", "overcast_broken",
    "noon_vista", "noon_fields", "town_noon", "jeep_close_noon", "forest_morning", "pine_rays",
    "woods_pine2_evening", "forest_edge_golden", "coastline_sunset", "sunrise_ridge", "dawn_valley",
    "blue_hour_coast", "lowsun_fp", "firefight_1", "firefight_2", "firefight_3", "tank_gunner_interior",
    "tank_gunner_optics", "sinai_lowsun_fp", "sinai_desert_noon", "optics_mist_hud", "optics_mist_sight",
]

BG = (24, 24, 26)
LABEL = (230, 210, 120)
SUB = (205, 205, 205)


def font(size):
    for name in ("segoeui.ttf", "arial.ttf", "DejaVuSans.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            continue
    return ImageFont.load_default()


def panels_for(view, ref_dir, index, overhaul_dir, fallback):
    entry = index.get(view, {})
    van = Path(entry.get("vanilla", ref_dir / "vanilla" / f"{view}.png"))
    landed = Path(entry.get("landed", ref_dir / "landed" / f"{view}.png"))
    over = overhaul_dir / f"{view}__overhaul.png"
    first = (van, "Original")
    if not van.exists():
        alt = fallback / f"{view}__classic.png" if fallback else None
        if alt is None or not alt.exists():
            raise FileNotFoundError(f"{view}: no vanilla shot and no fallback")
        first = (alt, "Original (features off)")
    for p in (landed, over):
        if not p.exists():
            raise FileNotFoundError(p)
    return [first, (landed, "graphics-remaster (landed)"), (over, "Overhaul")]


def row_image(view, panels, w, f_view, f_lab, gap=6, lab=20, head=24):
    ims = [Image.open(p).convert("RGB") for p, _ in panels]
    h = round(w * ims[2].height / ims[2].width)
    width = 3 * w + 2 * gap
    row = Image.new("RGB", (width, head + lab + h), BG)
    d = ImageDraw.Draw(row)
    d.text((0, 2), view, fill=LABEL, font=f_view)
    for j, (im, (_, label)) in enumerate(zip(ims, panels)):
        x = j * (w + gap)
        d.text((x + 2, head), label, fill=SUB, font=f_lab)
        if abs(im.width / im.height - w / h) > 0.01:
            # Crop to the overhaul's aspect from the centre.
            ar = w / h
            if im.width / im.height > ar:
                nw = int(im.height * ar)
                im = im.crop(((im.width - nw) // 2, 0, (im.width - nw) // 2 + nw, im.height))
            else:
                nh = int(im.width / ar)
                im = im.crop((0, (im.height - nh) // 2, im.width, (im.height - nh) // 2 + nh))
        row.paste(im.resize((w, h), Image.LANCZOS), (x, head + lab))
    return row


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--ref", required=True)
    ap.add_argument("--overhaul", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--strips", default="")
    ap.add_argument("--strip-views", default="", help="views for strips (default: the sheet's)")
    ap.add_argument("--fallback", default="")
    ap.add_argument("--views", default="")
    ap.add_argument("--panel", type=int, default=320)
    ap.add_argument("--strip-panel", type=int, default=640)
    ap.add_argument("--columns", type=int, default=2)
    ap.add_argument("--title", default="Uslu dur! look: Original | graphics-remaster as landed | Overhaul")
    a = ap.parse_args()
    ref_dir, over_dir = Path(a.ref), Path(a.overhaul)
    fallback = Path(a.fallback) if a.fallback else None
    index = {v["view"]: v for v in json.loads((ref_dir / "index.json").read_text(encoding="utf-8"))["views"]}
    views = [v for v in a.views.split(",") if v] or DEFAULT_VIEWS
    f_view, f_lab, f_title = font(16), font(13), font(22)
    rows = [row_image(v, panels_for(v, ref_dir, index, over_dir, fallback), a.panel, f_view, f_lab) for v in views]
    # Rows in --columns columns, filled top to bottom then left to right (reading order).
    cols = max(1, a.columns)
    per_col = (len(rows) + cols - 1) // cols
    pad, top = 16, 46
    col_w = max(r.width for r in rows)
    col_h = [sum(r.height + pad for r in rows[c * per_col:(c + 1) * per_col]) for c in range(cols)]
    sheet = Image.new("RGB", (pad + cols * (col_w + 2 * pad), top + max(col_h) + pad), BG)
    d = ImageDraw.Draw(sheet)
    d.text((pad, 10), a.title, fill=(235, 235, 235), font=f_title)
    for c in range(cols):
        y = top
        x = pad + c * (col_w + 2 * pad)
        for r in rows[c * per_col:(c + 1) * per_col]:
            sheet.paste(r, (x, y))
            y += r.height + pad
    sheet.save(a.out)
    print(a.out, sheet.size, len(rows), "views")
    if a.strips:
        out = Path(a.strips)
        out.mkdir(parents=True, exist_ok=True)
        strip_views = [v for v in a.strip_views.split(",") if v] or views
        for v in strip_views:
            r = row_image(v, panels_for(v, ref_dir, index, over_dir, fallback), a.strip_panel, font(22), font(18),
                          gap=8, lab=26, head=32)
            framed = Image.new("RGB", (r.width + 2 * pad, r.height + 2 * pad), BG)
            framed.paste(r, (pad, pad))
            framed.save(out / f"{v}.png")
        print(out, len(strip_views), "strips")


if __name__ == "__main__":
    main()
