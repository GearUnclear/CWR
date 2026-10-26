"""compare.png for puddle approach 5: rows = views, cols = round 3 | first rework | this one,
then 2x crops of the tree base and the open street in town_night_rain.

python compare5.py <mine dir with <view>.png> <out png> [label for mine]
"""
import sys
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

LOOK = Path("D:/Arma_CWA_worktrees/graphics-remaster/tmp/look")
R3 = LOOK / "fix/seam/shots_4/tri"
R4 = LOOK / "fix4/shine/shots_3/tri"
VIEWS = ["town_night_rain", "seam_b", "seam_d", "seam_day", "rain_day"]
R3F = {
    "town_night_rain": "look_probe_f2_abel/000_town_night_rain__ovh.png",
    "seam_b": "look_probe_f2_abel/001_seam_b__ovh.png",
    "seam_d": "look_probe_f2_abel/003_seam_d__ovh.png",
    "seam_day": "look_probe_f2_abel/004_seam_day__ovh.png",
    "rain_day": "look_probe_f2_eden/000_rain_day__ovh.png",
}
R4F = {
    "town_night_rain": "look_shine_abel/000_town_night_rain__ovh.png",
    "seam_b": "look_shine_abel/001_seam_b__ovh.png",
    "seam_d": "look_shine_abel/002_seam_d__ovh.png",
    "seam_day": "look_shine_abel/003_seam_day__ovh.png",
    "rain_day": "look_shine_eden/000_rain_day__ovh.png",
}
CROPS = [("town_night_rain", "tree base", 780, 610), ("town_night_rain", "open street", 330, 790)]


def font(size):
    for f in ("arial.ttf", "segoeui.ttf"):
        try:
            return ImageFont.truetype(f, size)
        except OSError:
            pass
    return ImageFont.load_default()


def main():
    mine = Path(sys.argv[1])
    out = Path(sys.argv[2])
    label = sys.argv[3] if len(sys.argv) > 3 else "approach 5 (art direction)"
    heads = ["round 3 (shiny)", "first rework", label]
    W, H = 640, 360
    top = 34
    rows = len(VIEWS) + len(CROPS)
    sheet = Image.new("RGB", (W * 3, top + H * rows), (16, 16, 16))
    d = ImageDraw.Draw(sheet)
    f, fs = font(20), font(16)
    d.text((8, 6), "rain and puddles: round 3 | first rework | " + label + "   (last two rows: 2x crops of town_night_rain)",
           fill=(240, 240, 240), font=f)
    cache = {}
    for r, v in enumerate(VIEWS):
        ims = [Image.open(R3 / R3F[v]).convert("RGB"), Image.open(R4 / R4F[v]).convert("RGB"),
               Image.open(mine / f"{v}.png").convert("RGB")]
        cache[v] = ims
        y = top + H * r
        for i, im in enumerate(ims):
            sheet.paste(im.resize((W, H), Image.LANCZOS), (i * W, y))
            t = f"{v}: {heads[i]}"
            d.rectangle((i * W, y, i * W + 9 * len(t) + 10, y + 22), fill=(0, 0, 0))
            d.text((i * W + 6, y + 2), t, fill=(255, 210, 0), font=fs)
            if v == "town_night_rain":
                s = W / 1920.0
                for j, (_, _, x0, y0) in enumerate(CROPS):
                    d.rectangle((i * W + x0 * s, y + y0 * s, i * W + (x0 + 320) * s, y + (y0 + 180) * s),
                                outline=(255, 210, 0) if j == 0 else (0, 200, 255))
    for j, (v, name, x0, y0) in enumerate(CROPS):
        y = top + H * (len(VIEWS) + j)
        for i, im in enumerate(cache[v]):
            c = im.crop((x0, y0, x0 + 320, y0 + 180)).resize((W, H), Image.LANCZOS)
            sheet.paste(c, (i * W, y))
            t = f"2x {name} ({v})"
            d.rectangle((i * W, y, i * W + 9 * len(t) + 10, y + 22), fill=(0, 0, 0))
            d.text((i * W + 6, y + 2), t, fill=(255, 210, 0) if j == 0 else (0, 200, 255), font=fs)
    sheet.save(out)
    print(out, sheet.size)


if __name__ == "__main__":
    main()
