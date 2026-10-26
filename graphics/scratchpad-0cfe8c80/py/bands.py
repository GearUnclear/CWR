import sys, colorsys
from PIL import Image, ImageStat, ImageChops
def band(p, box):
    im = Image.open(p).convert("RGB").crop(box)
    m = ImageStat.Stat(im).mean
    h, l, s = colorsys.rgb_to_hls(*(x/255 for x in m))
    return "(%5.1f,%5.1f,%5.1f) hue %5.1f sat %.3f" % (m[0], m[1], m[2], h*360, s)
d = sys.argv[1]
for view, bands in [("water_sea_noon", [("far", (0,330,1920,600)), ("near", (0,800,1920,1080)), ("all", (0,330,1920,1080))]),
                    ("water_shore_midsun", [("all", (0,120,1920,1080))]),
                    ("water_coastline_sunset", [("sea", (0,520,1920,1080))]),
                    ("water_night", [("sea", (0,520,1920,1080))])]:
    for tag in ("classic", "off", "on"):
        for name, box in bands:
            print(view, tag, name, band(f"{d}/{view}__{tag}.png", box))
