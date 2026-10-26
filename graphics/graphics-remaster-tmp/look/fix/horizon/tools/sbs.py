"""sbs.py out.png title view=before.png,after.png,x0,y0,x1,y1 [...]
Per view: full before | full after (960x640 each, the box outlined), then the
box at 2x, before above after (1920 wide at most)."""
import sys
from PIL import Image, ImageDraw

out, title = sys.argv[1], sys.argv[2]
rows = []
for spec in sys.argv[3:]:
    name, rest = spec.split("=", 1)
    b, a, x0, y0, x1, y1 = rest.split(",")
    x0, y0, x1, y1 = int(x0), int(y0), int(x1), int(y1)
    B = Image.open(b).convert("RGB").resize((960, 640), Image.LANCZOS)
    A = Image.open(a).convert("RGB").resize((960, 640), Image.LANCZOS)
    full = Image.new("RGB", (1920, 640))
    full.paste(B, (0, 0)); full.paste(A, (960, 0))
    d = ImageDraw.Draw(full)
    for ox, lab in ((0, f"{name}  BEFORE (director final)"), (960, f"{name}  AFTER (horizon fix)")):
        d.rectangle((ox + x0, y0, ox + x1, y1), outline=(255, 255, 0))
        d.rectangle((ox, 0, ox + 7 * len(lab) + 10, 16), fill=(0, 0, 0))
        d.text((ox + 4, 2), lab, fill=(255, 255, 0))
    rows.append(full)
    w, h = (x1 - x0) * 2, (y1 - y0) * 2
    for lab, im in (("before", B), ("after", A)):
        c = im.crop((x0, y0, x1, y1)).resize((w, h), Image.LANCZOS)
        r = Image.new("RGB", (1920, h + 16), (0, 0, 0))
        r.paste(c, (0, 16))
        ImageDraw.Draw(r).text((4, 2), f"{name}: the box at 2x, {lab}", fill=(255, 255, 0))
        rows.append(r)
H = 30 + sum(r.height for r in rows)
o = Image.new("RGB", (1920, H), (0, 0, 0))
ImageDraw.Draw(o).text((6, 8), title, fill=(255, 255, 255))
y = 30
for r in rows:
    o.paste(r, (0, y)); y += r.height
o.save(out)
print(out, o.size)
