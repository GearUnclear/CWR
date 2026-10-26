"""Final compare sheets: round 2 (old, with seam) | round 3 (shiny) | this fix.

python compare.py <shots dir> <out dir>
"""
import sys
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

sys.path.insert(0, str(Path(__file__).parent))
import cmp  # noqa: E402

# (label, x0, y0) of 320 x 180 regions shown at 2x.
CROPS = {
    "town_night_rain": [("tree base", 780, 610), ("open street", 330, 790)],
    "seam_b": [("tree base", 110, 560), ("open street", 640, 760)],
    "seam_day": [("tree base", 770, 640), ("open ground", 180, 820)],
    "rain_day": [("puddles", 200, 700), ("open grass", 980, 840)],
}
OUT = {
    "town_night_rain": "compare_town_night_rain.png",
    "seam_b": "compare_second_angle_seam_b.png",
    "seam_day": "compare_day_town_rain.png",
    "rain_day": "compare_day_rain_eden.png",
}
HEADS = ["round 2 (old look, seam)", "round 3 (shiny)", "this fix"]


def font(size):
    for f in ("arial.ttf", "segoeui.ttf"):
        try:
            return ImageFont.truetype(f, size)
        except OSError:
            pass
    return ImageFont.load_default()


def main():
    shots = Path(sys.argv[1]) / "tri"
    out = Path(sys.argv[2])
    W, HF = 640, 360
    f, fs = font(20), font(16)
    for v, crops in CROPS.items():
        ims = [Image.open(cmp.R2 / cmp.OLD[v]).convert("RGB"), Image.open(cmp.R3 / cmp.OLD[v]).convert("RGB"),
               Image.open(shots / cmp.NEW[v]).convert("RGB")]
        top = 34
        sheet = Image.new("RGB", (W * 3, top + HF * (1 + len(crops))), (16, 16, 16))
        d = ImageDraw.Draw(sheet)
        d.text((8, 6), f"{v}: round 2 | round 3 | this fix (full frame, then 2x crops)", fill=(240, 240, 240), font=f)
        for i, im in enumerate(ims):
            sheet.paste(im.resize((W, HF), Image.LANCZOS), (i * W, top))
            d.rectangle((i * W, top, i * W + 300, top + 22), fill=(0, 0, 0))
            d.text((i * W + 6, top + 2), HEADS[i], fill=(255, 210, 0), font=fs)
            for j, (label, x0, y0) in enumerate(crops):
                c = im.crop((x0, y0, x0 + 320, y0 + 180)).resize((W, HF), Image.LANCZOS)
                y = top + HF * (1 + j)
                sheet.paste(c, (i * W, y))
                d.rectangle((i * W, y, i * W + 200, y + 22), fill=(0, 0, 0))
                d.text((i * W + 6, y + 2), f"2x {label}", fill=(255, 210, 0), font=fs)
                # where the crop sits in the full frame
                s = W / 1920.0
                d.rectangle((i * W + x0 * s, top + y0 * s, i * W + (x0 + 320) * s, top + (y0 + 180) * s),
                            outline=(255, 210, 0) if j == 0 else (0, 200, 255))
        sheet.save(out / OUT[v])
        print(out / OUT[v])


if __name__ == "__main__":
    main()
