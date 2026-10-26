"""Final comparison sheets for the HFRT notes (labelled panels, GPU ms where a
clean in-run minimum exists)."""
import sys
from pathlib import Path
from PIL import Image, ImageDraw
sys.path.insert(0, str(Path(__file__).parent))
from collect import font, label_box

R = Path(r"D:\Arma_CWA_worktrees\lighting\tmp\research\hfrt")

def panel(path, title, note=None, crop=None, scale=1.0):
    im = Image.open(path).convert("RGB")
    if crop:
        im = im.crop(crop)
    if scale != 1.0:
        im = im.resize((int(im.width * scale), int(im.height * scale)), Image.LANCZOS)
    d = ImageDraw.Draw(im)
    label_box(d, (10, 8), title, font(20))
    if note:
        label_box(d, (10, im.height - 28), note, font(15))
    return im

def sheet(rows, title, dst, gap=6):
    w = max(sum(im.width for im in r) + gap * (len(r) - 1) for r in rows)
    h = sum(r[0].height for r in rows) + gap * (len(rows) - 1) + 40
    out = Image.new("RGB", (w, h), (24, 24, 24))
    ImageDraw.Draw(out).text((10, 8), title, font=font(22), fill=(235, 235, 235))
    y = 40
    for r in rows:
        x = 0
        for im in r:
            out.paste(im, (x, y))
            x += im.width + gap
        y += r[0].height + gap
    out.save(dst)
    print("wrote", dst)

if __name__ == "__main__":
    m, v3, p = R / "runs/main_v2", R / "runs/v3", R / "runs/p"
    # sun visibility: shipped horizon map | P | traced
    for view in ("hill_0700", "hill_1730", "sunrise_ridge", "forest_edge_golden", "overcast_broken"):
        rows = [[panel(m / f"{view}__dbgsun_rem.png", "sun vis: shipped horizon map", scale=0.75),
                 panel(p / f"{view}__dbgsun_P.png", "sun vis: P, canopy in horizon map", scale=0.75),
                 panel(m / f"{view}__dbgsun_can.png", "sun vis: traced terrain + canopy", scale=0.75)],
                [panel(m / f"{view}__dbgsky_rem.png", "sky vis: shipped (x0.6)", scale=0.75),
                 panel(p / f"{view}__dbgsky_P.png", "sky vis: P (x0.6)", scale=0.75),
                 panel(m / f"{view}__dbgsky_can.png", "sky vis: traced, normal-aware (x1)", scale=0.75)]]
        sheet(rows, f"{view}: what the light sees (debug views, white = lit / open sky)", R / f"DEBUG_{view}.png")
