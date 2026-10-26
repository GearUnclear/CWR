"""Side-by-side: vanilla | landed | before | after, plus a 2x crop row.
python sheet.py <view> <after.png> <out.png> <cx> <cy> <cw> <ch>  (crop box in 1920x1080 pixels)
"""
import sys
from PIL import Image, ImageDraw

LOOK = "D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/"
view, after, out = sys.argv[1], sys.argv[2], sys.argv[3]
cx, cy, cw, ch = (int(v) for v in sys.argv[4:8])
srcs = [("vanilla", LOOK + f"ref/vanilla/{view}__1080.png"),
        ("landed", LOOK + f"ref/landed/{view}__1080.png"),
        ("before (director/final)", LOOK + f"director/final/{view}__1080__overhaul.png"),
        ("after (fix5 dawn)", after)]
W, H = 480, 270
ims = [(n, Image.open(p).convert("RGB").resize((1920, 1080))) for n, p in srcs]
sheet = Image.new("RGB", (W * 4, 2 * H + 40), (20, 20, 20))
d = ImageDraw.Draw(sheet)
for i, (n, im) in enumerate(ims):
    t = im.resize((W, H), Image.LANCZOS)
    dt = ImageDraw.Draw(t)
    dt.rectangle([cx * W / 1920, cy * H / 1080, (cx + cw) * W / 1920, (cy + ch) * H / 1080], outline=(255, 0, 0))
    sheet.paste(t, (i * W, 20))
    d.text((i * W + 6, 4), n, fill=(255, 255, 255))
    c = im.crop((cx, cy, cx + cw, cy + ch))
    s = 2.0 * W / 1920  # twice the panel's scale
    c = c.resize((min(int(cw * s), W), min(int(ch * s), H)), Image.LANCZOS)
    sheet.paste(c, (i * W, 40 + H))
d.text((6, 24 + H), f"crop x{cx}-{cx + cw} y{cy}-{cy + ch} of 1920x1080, at 2x the panels' scale", fill=(255, 255, 0))
sheet.save(out)
print(out, sheet.size)
