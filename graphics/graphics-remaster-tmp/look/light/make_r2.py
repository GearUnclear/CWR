"""Critic round 2 deliverables: copy the final run into final/, build the pairs,
the contact sheets and the emitter and artefact crops.

    python make_r2.py <final run dir>
"""
import shutil
import subprocess
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

OUT = Path(__file__).resolve().parent
HERO = ["lowsun_fp", "noon_fields", "forest_edge_golden", "sunrise_ridge", "coastline_sunset", "overcast_broken",
        "town_night", "sinai_desert_noon"]
NEW = ["woods_pine2_evening", "blue_hour_coast"]


def font(size):
    for name in ("arial.ttf", "segoeui.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            continue
    return ImageFont.load_default()


def main():
    run = Path(sys.argv[1])
    final = OUT / "final"
    final.mkdir(exist_ok=True)
    for v in HERO + NEW:
        for m in ("classic", "off", "on"):
            src = run / f"{v}__{m}.png"
            if src.exists():
                shutil.copy2(src, final / src.name)
    shutil.copy2(run / "udcap.log", final / "udcap.log")
    f = font(26)
    for v in HERO + NEW:
        a, b = final / f"{v}__classic.png", final / f"{v}__on.png"
        pa = Image.open(a).convert("RGB").resize((960, 540), Image.LANCZOS)
        pb = Image.open(b).convert("RGB").resize((960, 540), Image.LANCZOS)
        s = Image.new("RGB", (1924, 540), (20, 20, 20))
        s.paste(pa, (0, 0))
        s.paste(pb, (964, 0))
        d = ImageDraw.Draw(s)
        for x, t in ((10, "Original (Classic)"), (974, "Overhaul (photoLight)")):
            d.rectangle([x - 4, 6, x + 270, 40], fill=(0, 0, 0))
            d.text((x, 8), t, fill=(255, 255, 255), font=f)
        s.save(final / f"{v}__pair.png")
    views = ",".join(HERO + NEW)
    sheet = str(OUT / "sheet.py")
    subprocess.run([sys.executable, sheet, str(final), "--modes", "classic,on", "--titles",
                    "Original (Classic),Overhaul (photoLight)", "--views", views, "--out",
                    str(OUT / "contact_sheet.png"), "--w", "480"], check=True)
    subprocess.run([sys.executable, sheet, str(final), "--modes", "classic,off,on", "--titles",
                    "Original (Classic),Branch tip (photoLight=0),Overhaul (photoLight)", "--views", views, "--out",
                    str(OUT / "contact_sheet_3col.png"), "--w", "480"], check=True)
    r1 = OUT / "round1" / "final"
    if r1.exists():
        subprocess.run([sys.executable, sheet, str(final), "--modes", f"classic,{r1}::on,on", "--titles",
                        "Original (Classic),Round 1,Round 2", "--views", views, "--out",
                        str(OUT / "contact_sheet_r1_r2.png"), "--w", "480"], check=True)
    print("ok")


if __name__ == "__main__":
    main()
