"""linger_strip.py <tri dir look_probe_x5_linger> <out png> [label]: frames f003 (just before) then every 2 s to 24 s, 2 rows."""
import sys, re
from pathlib import Path
from PIL import Image, ImageDraw
d, out = Path(sys.argv[1]), Path(sys.argv[2]); label = sys.argv[3] if len(sys.argv) > 3 else ""
files = {}
for p in d.glob("*.png"):
    m = re.search(r"x5_linger_f(\d+)\.png$", p.name)
    if m:
        files[int(m.group(1))] = p
# f003 shot just before the spawn; f004 + k is (k+1) x 10 frames = (k+1)/3 s after it.
picks = [3, 4, 5, 7, 9] + [3 + 3 * s for s in (2, 3, 4, 6, 8, 10, 12, 14, 16, 18, 20, 24)]
W, H = 480, 320
cells = []
for f in picks:
    if f not in files:
        continue
    t = (f - 3) / 3.0
    im = Image.open(files[f]).convert("RGB").resize((W, H))
    ImageDraw.Draw(im).text((4, 4), f"{label} +{t:.1f} s", fill=(255, 230, 0))
    cells.append(im)
cols = (len(cells) + 1) // 2
sheet = Image.new("RGB", (W * cols, H * 2), (0, 0, 0))
for i, c in enumerate(cells):
    sheet.paste(c, ((i % cols) * W, (i // cols) * H))
sheet.save(out)
print(out, len(cells))
