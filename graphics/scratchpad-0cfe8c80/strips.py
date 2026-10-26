"""classic | off | on strips (each panel 960x540) for the look check.

    python strips.py <run dir> <out dir> view [view ...]
"""
import sys
from pathlib import Path

from PIL import Image, ImageDraw

run, out = Path(sys.argv[1]), Path(sys.argv[2])
out.mkdir(parents=True, exist_ok=True)
W, H = 960, 540
for view in sys.argv[3:]:
    panels = []
    for s in ("classic", "off", "on"):
        p = run / f"{view}__{s}.png"
        if p.exists():
            panels.append((s, Image.open(p).convert("RGB").resize((W, H), Image.LANCZOS)))
    if not panels:
        continue
    strip = Image.new("RGB", (W * len(panels), H + 24), (20, 20, 20))
    d = ImageDraw.Draw(strip)
    for i, (s, im) in enumerate(panels):
        strip.paste(im, (i * W, 24))
        d.text((i * W + 6, 5), f"{view} {s}", fill=(230, 230, 230))
    strip.save(out / f"{view}__strip.png")
    print("wrote", out / f"{view}__strip.png")
