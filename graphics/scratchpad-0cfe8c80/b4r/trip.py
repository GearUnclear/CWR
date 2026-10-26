"""Side-by-side strips of a B4 capture folder: classic | off | on (or off |
on when there is no classic shot), each panel 960x540 with its label.

    python trip.py <dir> <outdir> [views,comma,separated]
"""
import sys
from pathlib import Path
from PIL import Image, ImageDraw

d, out = Path(sys.argv[1]), Path(sys.argv[2])
out.mkdir(parents=True, exist_ok=True)
views = sys.argv[3].split(",") if len(sys.argv) > 3 else sorted({p.name.rsplit("__", 1)[0] for p in d.glob("*__on.png")})
W, H = 960, 540
for v in views:
    panels = []
    for mode in ("classic", "off", "on"):
        p = d / f"{v}__{mode}.png"
        if p.exists():
            panels.append((mode, Image.open(p).convert("RGB").resize((W, H), Image.LANCZOS)))
    if not panels:
        continue
    strip = Image.new("RGB", (W * len(panels) + 4 * (len(panels) - 1), H + 24), (20, 20, 20))
    dr = ImageDraw.Draw(strip)
    dr.text((6, 5), v, fill=(230, 230, 230))
    for i, (mode, im) in enumerate(panels):
        x = i * (W + 4)
        strip.paste(im, (x, 24))
        label = {"classic": "Classic", "off": "combat off", "on": "combat on"}[mode]
        dr.rectangle([x + 4, 28, x + 8 + 7 * len(label), 44], fill=(0, 0, 0))
        dr.text((x + 6, 30), label, fill=(255, 255, 255))
    strip.save(out / f"{v}__strip.png")
    print(out / f"{v}__strip.png")
