"""python frames_at.py name t1 t2 ... -> <UD_PROMO_DIR>/work/edit/check/<name>.jpg (2 columns, 960x540 tiles,
labelled): the final cut's frames at the given output seconds, for checking a lock point or a cut."""
import subprocess
import sys
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from promopaths import FF, FONT_DIR, PROMO  # noqa: E402

ROOT = PROMO
V = ROOT / "ud-lebanon80-promo-1080p.mp4"
if len(sys.argv) < 3:
    print(__doc__)
    sys.exit(2)
name, ts = sys.argv[1], [float(x) for x in sys.argv[2:]]
TW, TH, C = 960, 540, 2
f = ImageFont.truetype(str(FONT_DIR / "cwr_mono.ttf"), 22)
rows = (len(ts) + C - 1) // C
sh = Image.new("RGB", (C * TW, rows * TH), (0, 0, 0))
d = ImageDraw.Draw(sh)
for i, t in enumerate(ts):
    n = round(t * 30)
    raw = subprocess.run([FF, "-v", "error", "-i", str(V), "-vf", rf"select=eq(n\,{n}),scale={TW}:{TH}", "-frames:v", "1",
                          "-f", "rawvideo", "-pix_fmt", "rgb24", "-"], capture_output=True).stdout
    x, y = (i % C) * TW, (i // C) * TH
    sh.paste(Image.frombytes("RGB", (TW, TH), raw), (x, y))
    d.rectangle([x, y, x + 250, y + 28], fill=(0, 0, 0))
    d.text((x + 4, y + 2), f"f{n} {n / 30:.3f}s", fill=(255, 220, 0), font=f)
out = ROOT / "work" / "edit" / "check"
out.mkdir(parents=True, exist_ok=True)
sh.save(out / f"{name}.jpg", quality=85)
print(out / f"{name}.jpg")
