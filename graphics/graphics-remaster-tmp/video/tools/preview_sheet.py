"""Contact sheet of a preview run: per clip, off and on of every frame, small."""
import re, sys
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
runs = Path(sys.argv[1]); prefix = sys.argv[2]; out = Path(sys.argv[3])
RE = re.compile(r"^\d+_(.+)__(off|on)__f(\d{4})\.png$")
rows = []
for d in sorted(runs.glob(prefix + "-*")):
    if not d.is_dir():
        continue
    fr = {}
    for p in (d / "tri").rglob("*.png"):
        m = RE.match(p.name)
        if m:
            fr[(m.group(2), int(m.group(3)))] = p
    if fr:
        rows.append((d.name[len(prefix) + 1:], fr))
TW, TH = 384, 216
fnt = ImageFont.truetype("C:/Windows/Fonts/arial.ttf", 16)
CH = 5
for c in range(0, len(rows), CH):
    part = rows[c:c + CH]
    sheet = Image.new("RGB", (TW * 4, (TH + 22) * len(part)), (30, 30, 30))
    d = ImageDraw.Draw(sheet)
    for r, (name, fr) in enumerate(part):
        y = r * (TH + 22)
        nf = max(k[1] for k in fr)
        d.text((4, y + 2), f"{name}  (f1 off, f1 on, f{nf} off, f{nf} on)", font=fnt, fill=(255, 255, 0))
        for i, key in enumerate((("off", 1), ("on", 1), ("off", nf), ("on", nf))):
            p = fr.get(key)
            if p:
                sheet.paste(Image.open(p).convert("RGB").resize((TW, TH)), (i * TW, y + 22))
    o = out.with_name(f"{out.stem}_{c // CH + 1}.png")
    sheet.save(o)
    print(o, [n for n, _ in part])
