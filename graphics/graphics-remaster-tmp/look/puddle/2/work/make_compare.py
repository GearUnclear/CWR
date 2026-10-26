"""Owner sheet: rows = the five views, columns = round 3 (shiny) | first rework | this approach,
then 2x crops of the tree base and the open street in town_night_rain.

python make_compare.py <final dir> <out.png>
"""
import sys
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

LOOK = Path(r"D:/Arma_CWA_worktrees/graphics-remaster/tmp/look")
R3 = LOOK / "fix/seam/shots_4/tri"
RW = LOOK / "fix4/shine/shots_3/tri"
R3F = {
    "town_night_rain": "look_probe_f2_abel/000_town_night_rain__ovh.png",
    "seam_b": "look_probe_f2_abel/001_seam_b__ovh.png",
    "seam_d": "look_probe_f2_abel/003_seam_d__ovh.png",
    "seam_day": "look_probe_f2_abel/004_seam_day__ovh.png",
    "rain_day": "look_probe_f2_eden/000_rain_day__ovh.png",
}
RWF = {
    "town_night_rain": "look_shine_abel/000_town_night_rain__ovh.png",
    "seam_b": "look_shine_abel/001_seam_b__ovh.png",
    "seam_d": "look_shine_abel/002_seam_d__ovh.png",
    "seam_day": "look_shine_abel/003_seam_day__ovh.png",
    "rain_day": "look_shine_eden/000_rain_day__ovh.png",
}
VIEWS = ["town_night_rain", "seam_b", "seam_d", "seam_day", "rain_day"]
CROPS = [("tree base", 780, 610), ("open street", 330, 790)]  # 320 x 180 in town_night_rain
HEADS = ["round 3 (shiny)", "first rework", "puddles from the ground's shape"]


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
    top = 40
    f, fs = font(22), font(16)
    rows = len(VIEWS) + len(CROPS)
    sheet = Image.new("RGB", (W * 3, top + H * rows), (16, 16, 16))
    d = ImageDraw.Draw(sheet)
    d.text((8, 8), "Rain and puddles: round 3 | first rework | puddles placed by the ground's shape (2 of 6)",
           fill=(240, 240, 240), font=f)
    for r, v in enumerate(VIEWS):
        ims = [Image.open(R3 / R3F[v]).convert("RGB"), Image.open(RW / RWF[v]).convert("RGB"),
               Image.open(final / f"{v}.png").convert("RGB")]
        y = top + r * H
        for i, im in enumerate(ims):
            sheet.paste(im.resize((W, H), Image.LANCZOS), (i * W, y))
            label = f"{v}: {HEADS[i]}"
            d.rectangle((i * W, y, i * W + 8 + 8 * len(label), y + 22), fill=(0, 0, 0))
            d.text((i * W + 6, y + 2), label, fill=(255, 210, 0), font=fs)
            if v == "town_night_rain":
                s = W / 1920.0
                for j, (_, x0, y0) in enumerate(CROPS):
                    d.rectangle((i * W + x0 * s, y + y0 * s, i * W + (x0 + 320) * s, y + (y0 + 180) * s),
                                outline=(255, 210, 0) if j == 0 else (0, 200, 255))
        if v == "town_night_rain":
            tnr = ims
    for j, (label, x0, y0) in enumerate(CROPS):
        y = top + (len(VIEWS) + j) * H
        for i, im in enumerate(tnr):
            c = im.crop((x0, y0, x0 + 320, y0 + 180)).resize((W, H), Image.LANCZOS)
            sheet.paste(c, (i * W, y))
            t = f"town_night_rain 2x {label}: {HEADS[i]}"
            d.rectangle((i * W, y, i * W + 8 + 8 * len(t), y + 22), fill=(0, 0, 0))
            d.text((i * W + 6, y + 2), t, fill=(255, 210, 0) if j == 0 else (0, 200, 255), font=fs)
    sheet.save(out)
    print(out, sheet.size)


if __name__ == "__main__":
    main()
