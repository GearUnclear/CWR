"""Evidence sheets for the spawned-vehicles fix (deliverable folder).

python make_evidence.py <deliverable_dir>
Reads runs/before, runs/after, runs/after_v1_full_parity under it.
"""
import sys
from pathlib import Path

import numpy as np
from PIL import Image, ImageDraw, ImageFont

root = Path(sys.argv[1])
B, A, V1 = root / "runs" / "before", root / "runs" / "after", root / "runs" / "after_v1_full_parity"


def font(size):
    for f in ("C:/Windows/Fonts/segoeui.ttf", "C:/Windows/Fonts/arial.ttf"):
        try:
            return ImageFont.truetype(f, size)
        except OSError:
            pass
    return ImageFont.load_default()


def grid(rows, title, out, scale=1.0, box=None):
    """rows: list of lists of (label, path)."""
    cells = []
    for row in rows:
        r = []
        for label, p in row:
            im = Image.open(p).convert("RGB")
            if box:
                im = im.crop(box)
            if scale != 1.0:
                im = im.resize((int(im.width * scale), int(im.height * scale)), Image.LANCZOS)
            r.append((label, im))
        cells.append(r)
    w = max(im.width for r in cells for _, im in r)
    h = max(im.height for r in cells for _, im in r)
    lab = 30
    top = 44
    ncol = max(len(r) for r in cells)
    sheet = Image.new("RGB", (w * ncol, top + (h + lab) * len(cells)), (18, 18, 18))
    d = ImageDraw.Draw(sheet)
    d.text((10, 8), title, fill=(255, 255, 255), font=font(24))
    for ri, r in enumerate(cells):
        for ci, (label, im) in enumerate(r):
            x, y = ci * w, top + ri * (h + lab)
            d.text((x + 8, y + 4), label, fill=(235, 235, 235), font=font(19))
            sheet.paste(im, (x, y + lab))
    sheet.save(out)
    print(out)


VIEWS = {
    "lowsun_fp": "Abel 08:00, sun 12.7 deg: Jeep, M113, tent spawned by createVehicle",
    "sinai_lowsun_fp": "Sinai + @LoBo 16:10, sun 12.2 deg: HMMWV, M113 Zelda, camnet, Golani rifleman spawned",
    "noon_fields": "Abel 12:30, sun 71.9 deg: Jeep spawned",
    "town_night": "Abel 22:00, moon: campfire (Fire) spawned and inflamed",
    "hull_close": "Abel 08:00 close-up: M113 7.5 m, Jeep 6 m spawned",
    "sinai_hull_close": "Sinai 16:10 close-up: M113 Zelda 7.5 m, HMMWV 6 m spawned",
}
for view, desc in VIEWS.items():
    rows = [
        [("BEFORE (branch tip): script-spawned", B / f"{view}__remastered.png"),
         ("AFTER (fix): script-spawned", A / f"{view}__remastered.png"),
         ("AFTER (fix): mission-placed, same classes", A / f"placed_{view}__remastered.png")],
        [("BEFORE debug: magenta = CPU T&L (pre-lit) draw", B / f"{view}__debug.png"),
         ("AFTER debug: per-pixel path shows albedo", A / f"{view}__debug.png"),
         ("AFTER debug: mission-placed", A / f"placed_{view}__debug.png")],
    ]
    grid(rows, f"{view} (Remastered): {desc}", root / f"before_after_{view}.png")

# Classic is not changed for spawned vehicles (CPU vs GPU classic T&L)
grid([[("BEFORE classic: script-spawned", B / "lowsun_fp__classic.png"),
       ("AFTER classic: script-spawned", A / "lowsun_fp__classic.png"),
       ("AFTER classic: mission-placed", A / "placed_lowsun_fp__classic.png")]],
     "lowsun_fp in Classic lighting: the spawned vehicles keep the classic look", root / "classic_lowsun_fp.png")

# crops
grid([[("BEFORE spawned", B / "sinai_hull_close__remastered.png"),
       ("AFTER spawned", A / "sinai_hull_close__remastered.png"),
       ("AFTER placed", A / "placed_sinai_hull_close__remastered.png"),
       ("albedo (debug 3, per-pixel path)", A / "sinai_hull_close__debug.png")]],
     "Zelda slat armour: grey on the CPU path (classic blue ambient on a tan texture), tan per pixel",
     root / "crop_zelda_slats.png", scale=1.6, box=(450, 90, 960, 380))
grid([[("BEFORE spawned", B / "lowsun_fp__remastered.png"),
       ("AFTER spawned", A / "lowsun_fp__remastered.png"),
       ("AFTER placed", A / "placed_lowsun_fp__remastered.png")]],
     "lowsun_fp: Jeep, tent and M113 (2x)", root / "crop_lowsun_fp_vehicles.png", scale=2.0, box=(20, 220, 860, 450))
grid([[("BEFORE spawned", B / "town_night__remastered.png"),
       ("AFTER spawned", A / "town_night__remastered.png"),
       ("AFTER placed", A / "placed_town_night__remastered.png")],
      [("BEFORE debug (fire pit magenta)", B / "town_night__debug.png"),
       ("AFTER debug", A / "town_night__debug.png"),
       ("AFTER placed debug", A / "placed_town_night__debug.png")]],
     "town_night campfire (2x): the pit is lit per pixel like a placed one; the town stays at night",
     root / "crop_town_night_fire.png", scale=2.0, box=(540, 360, 800, 520))
grid([[("BEFORE spawned (CPU path)", B / "tank_gunner_interior__remastered.png"),
       ("AFTER spawned (crew view kept on CPU path)", A / "tank_gunner_interior__remastered.png"),
       ("mission-placed (per-pixel, shipped)", A / "placed_tank_gunner_interior__remastered.png"),
       ("rejected variant: full parity", V1 / "tank_gunner_interior__remastered.png")],
      [("BEFORE debug", B / "tank_gunner_interior__debug.png"),
       ("AFTER debug", A / "tank_gunner_interior__debug.png"),
       ("placed debug", A / "placed_tank_gunner_interior__debug.png"),
       ("full-parity variant debug", V1 / "tank_gunner_interior__debug.png")]],
     "M60 gunner interior, 10:00 (half size)", root / "interior_m60_gunner.png", scale=0.5)
