"""crops.py <imgA> <imgB> <out> x0,y0,x1,y1[:scale] ... : side by side crops (A left, B right), stacked."""
import sys
from PIL import Image, ImageDraw
a = Image.open(sys.argv[1]).convert('RGB')
b = Image.open(sys.argv[2]).convert('RGB')
rows = []
for spec in sys.argv[4:]:
    box, _, sc = spec.partition(':')
    x0, y0, x1, y1 = map(int, box.split(','))
    s = int(sc) if sc else 2
    ca = a.crop((x0, y0, x1, y1)).resize(((x1 - x0) * s, (y1 - y0) * s), Image.NEAREST)
    cb = b.crop((x0, y0, x1, y1)).resize(((x1 - x0) * s, (y1 - y0) * s), Image.NEAREST)
    r = Image.new('RGB', (ca.width * 2 + 6, ca.height), (255, 0, 255))
    r.paste(ca, (0, 0)); r.paste(cb, (ca.width + 6, 0))
    rows.append(r)
W = max(r.width for r in rows); H = sum(r.height + 6 for r in rows)
out = Image.new('RGB', (W, H), (40, 40, 40))
y = 0
for r in rows:
    out.paste(r, (0, y)); y += r.height + 6
out.save(sys.argv[3])
print(sys.argv[3], out.size)
