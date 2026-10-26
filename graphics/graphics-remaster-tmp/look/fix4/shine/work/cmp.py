"""Quick compare: round 2 | round 3 | shots dir, per view, plus a 2x crop.

python cmp.py <shots dir> <out.png> [view ...]
"""
import sys
from pathlib import Path
from PIL import Image, ImageDraw

SEAM = Path(r"D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/fix/seam")
R2 = SEAM / "shots_before2/tri"
R3 = SEAM / "shots_4/tri"
OLD = {
    "town_night_rain": "look_probe_f2_abel/000_town_night_rain__ovh.png",
    "seam_b": "look_probe_f2_abel/001_seam_b__ovh.png",
    "seam_d": "look_probe_f2_abel/003_seam_d__ovh.png",
    "seam_day": "look_probe_f2_abel/004_seam_day__ovh.png",
    "rain_day": "look_probe_f2_eden/000_rain_day__ovh.png",
}
NEW = {
    "town_night_rain": "look_shine_abel/000_town_night_rain__ovh.png",
    "seam_b": "look_shine_abel/001_seam_b__ovh.png",
    "seam_d": "look_shine_abel/002_seam_d__ovh.png",
    "seam_day": "look_shine_abel/003_seam_day__ovh.png",
    "rain_day": "look_shine_eden/000_rain_day__ovh.png",
}
# 2x crop boxes (x0, y0, w, h) in the 1920x1080 frame.
CROPS = {
    "town_night_rain": (700, 600, 640, 360),
    "seam_b": (300, 600, 640, 360),
    "seam_d": (500, 600, 640, 360),
    "seam_day": (600, 700, 640, 360),
    "rain_day": (640, 650, 640, 360),
}


def main():
    shots = Path(sys.argv[1]) / "tri"
    out = sys.argv[2]
    views = sys.argv[3:] or list(NEW)
    W = 640
    rows = []
    for v in views:
        ims = [Image.open(R2 / OLD[v]).convert("RGB"), Image.open(R3 / OLD[v]).convert("RGB"),
               Image.open(shots / NEW[v]).convert("RGB")]
        x0, y0, w, h = CROPS[v]
        full = [im.resize((W, W * 1080 // 1920), Image.LANCZOS) for im in ims]
        crop = [im.crop((x0, y0, x0 + w, y0 + h)) for im in ims]
        crop = [c.resize((W, W * h // w), Image.LANCZOS) for c in crop]
        rows.append((full, crop))
    H = sum(r[0][0].height + r[1][0].height for r in rows)
    o = Image.new("RGB", (W * 3, H))
    y = 0
    d = ImageDraw.Draw(o)
    for (full, crop), v in zip(rows, views):
        for i in range(3):
            o.paste(full[i], (i * W, y))
            o.paste(crop[i], (i * W, y + full[i].height))
        d.text((6, y + 4), f"{v}: round 2 | round 3 | new", fill=(255, 220, 0))
        y += full[0].height + crop[0].height
    o.save(out)


if __name__ == "__main__":
    main()
