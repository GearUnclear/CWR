import sys
from PIL import Image, ImageChops
a = Image.open(sys.argv[1]).convert("RGB"); b = Image.open(sys.argv[2]).convert("RGB")
d = ImageChops.difference(a, b).convert("L").point(lambda v: min(255, v * 16))
box = tuple(int(x) for x in sys.argv[4].split(",")) if len(sys.argv) > 4 else None
if box: d = d.crop(box)
d.save(sys.argv[3]); print(sys.argv[3], d.size)
