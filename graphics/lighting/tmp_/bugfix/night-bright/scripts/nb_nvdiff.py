"""python nb_nvdiff.py a.png b.png -> mean abs diff overall, in the static regions (left 40%, bottom 30%)."""
import sys
from PIL import Image, ImageChops
a = Image.open(sys.argv[1]).convert('RGB'); b = Image.open(sys.argv[2]).convert('RGB')
w, h = a.size
def region(box):
    d = ImageChops.difference(a.crop(box), b.crop(box))
    px = list(d.getdata()); n = len(px)
    return sum(sum(p) for p in px) / (3 * n), 100 * sum(1 for p in px if max(p) > 4) / n
for name, box in (("whole frame", (0, 0, w, h)), ("left 40%", (0, 0, int(0.4 * w), h)), ("bottom 30%", (0, int(0.7 * h), w, h))):
    m, big = region(box)
    print(f"{name:12s} mean abs {m:.2f}/255, pixels off by >4: {big:.2f}%")
