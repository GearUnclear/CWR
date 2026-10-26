"""python collect5.py <run-captures out dir> <dest dir> [--sheet <png>]
Copies tri/*/NNN_<view>__ovh.png to <dest>/<view>.png; optional quick sheet of all views (960x540 tiles)."""
import shutil
import sys
from pathlib import Path
from PIL import Image

VIEWS = ["town_night_rain", "seam_b", "seam_d", "seam_day", "rain_day"]
src = Path(sys.argv[1]) / "tri"
dst = Path(sys.argv[2])
dst.mkdir(parents=True, exist_ok=True)
found = []
for v in VIEWS:
    m = sorted(src.glob(f"*/*_{v}__ovh.png"))
    if m:
        shutil.copyfile(m[-1], dst / f"{v}.png")
        found.append(v)
print("copied", found)
if "--sheet" in sys.argv:
    out = Path(sys.argv[sys.argv.index("--sheet") + 1])
    tiles = [Image.open(dst / f"{v}.png").convert("RGB").resize((960, 540), Image.LANCZOS) for v in found]
    sheet = Image.new("RGB", (1920, 540 * ((len(tiles) + 1) // 2)), (0, 0, 0))
    for i, t in enumerate(tiles):
        sheet.paste(t, ((i % 2) * 960, (i // 2) * 540))
    sheet.save(out)
    print(out)
