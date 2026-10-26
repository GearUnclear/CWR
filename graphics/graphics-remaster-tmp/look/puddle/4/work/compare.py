"""compare.png for puddle agent 4: rows the five views, columns round 3 (shiny) |
first rework | this fix; then 2x crops of the tree base and the open street in
town_night_rain.

python compare.py <final dir> <out png>
"""
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

LOOK = Path(r"D:/Arma_CWA_worktrees/graphics-remaster/tmp/look")
R3 = LOOK / "fix/seam/shots_4/tri"
RW = LOOK / "fix4/shine/shots_3/tri"
OLD = {
    "town_night_rain": ("look_probe_f2_abel/000_town_night_rain__ovh.png", "look_shine_abel/000_town_night_rain__ovh.png"),
    "seam_b": ("look_probe_f2_abel/001_seam_b__ovh.png", "look_shine_abel/001_seam_b__ovh.png"),
    "seam_d": ("look_probe_f2_abel/003_seam_d__ovh.png", "look_shine_abel/002_seam_d__ovh.png"),
    "seam_day": ("look_probe_f2_abel/004_seam_day__ovh.png", "look_shine_abel/003_seam_day__ovh.png"),
    "rain_day": ("look_probe_f2_eden/000_rain_day__ovh.png", "look_shine_eden/000_rain_day__ovh.png"),
}
CROPS = [("tree base", 780, 610), ("open street", 330, 790)]
HEADS = ["round 3 (shiny)", "first rework", "this fix (puddle 4: only puddles reflect)"]


def font(size):
    for f in ("arial.ttf", "segoeui.ttf"):
        try:
            return ImageFont.truetype(f, size)
        except OSError:
            pass
    return ImageFont.load_default()


def main():
    final = Path(sys.argv[1])
    out = Path(sys.argv[2])
    W, H = 640, 360
    f, fs = font(22), font(16)
    top = 40
    rows = list(OLD) + ["crop0", "crop1"]
    sheet = Image.new("RGB", (W * 3, top + H * len(rows)), (16, 16, 16))
    d = ImageDraw.Draw(sheet)
    d.text((8, 8), "Wet ground: round 3 (shiny) | first rework | this fix. Last two rows: town_night_rain at 2x.",
           fill=(240, 240, 240), font=f)
    tnr = None
    for r, v in enumerate(OLD):
        ims = [Image.open(R3 / OLD[v][0]).convert("RGB"), Image.open(RW / OLD[v][1]).convert("RGB"),
               Image.open(final / f"{v}.png").convert("RGB")]
        if v == "town_night_rain":
            tnr = ims
        for i, im in enumerate(ims):
            x, y = i * W, top + r * H
            sheet.paste(im.resize((W, H), Image.LANCZOS), (x, y))
            d.rectangle((x, y, x + 330, y + 22), fill=(0, 0, 0))
            d.text((x + 6, y + 2), f"{v}: {HEADS[i]}", fill=(255, 210, 0), font=fs)
            if v == "town_night_rain":
                s = W / 1920.0
                for j, (_, x0, y0) in enumerate(CROPS):
                    d.rectangle((x + x0 * s, y + y0 * s, x + (x0 + 320) * s, y + (y0 + 180) * s),
                                outline=(255, 210, 0) if j == 0 else (0, 200, 255))
    for j, (label, x0, y0) in enumerate(CROPS):
        for i, im in enumerate(tnr):
            x, y = i * W, top + (len(OLD) + j) * H
            sheet.paste(im.crop((x0, y0, x0 + 320, y0 + 180)).resize((W, H), Image.LANCZOS), (x, y))
            d.rectangle((x, y, x + 330, y + 22), fill=(0, 0, 0))
            d.text((x + 6, y + 2), f"2x {label}: {HEADS[i]}", fill=(255, 210, 0) if j == 0 else (0, 200, 255), font=fs)
    sheet.save(out)
    print(out, sheet.size)


if __name__ == "__main__":
    main()
