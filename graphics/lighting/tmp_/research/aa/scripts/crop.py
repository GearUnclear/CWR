import sys
from PIL import Image
# usage: crop.py in out x0 y0 x1 y1 [scale]
src, dst = sys.argv[1], sys.argv[2]
x0, y0, x1, y1 = map(int, sys.argv[3:7])
sc = int(sys.argv[7]) if len(sys.argv) > 7 else 2
im = Image.open(src).convert("RGB").crop((x0, y0, x1, y1))
im = im.resize((im.width * sc, im.height * sc), Image.NEAREST)
im.save(dst)
print(dst, im.size)
