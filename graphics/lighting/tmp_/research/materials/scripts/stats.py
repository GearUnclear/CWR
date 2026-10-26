"""Mean colour statistics of image regions: python stats.py x0,y0,x1,y1 img [img ...]
Prints mean display RGB, luma and a warmth index (R - B) for each image."""
import sys
from PIL import Image, ImageStat

box = tuple(int(v) for v in sys.argv[1].split(","))
for p in sys.argv[2:]:
    im = Image.open(p).convert("RGB").crop(box)
    r, g, b = ImageStat.Stat(im).mean
    luma = 0.2126 * r + 0.7152 * g + 0.0722 * b
    print(f"{p.split(chr(92))[-1]:40s} rgb=({r:6.1f},{g:6.1f},{b:6.1f}) luma={luma:6.1f} warm(R-B)={r - b:6.1f}")
