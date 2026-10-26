import sys, colorsys
from PIL import Image, ImageStat
for p in sys.argv[2:]:
    im = Image.open(p).convert("RGB")
    out = []
    for (t, b) in [(int(x.split("-")[0]), int(x.split("-")[1])) for x in sys.argv[1].split(",")]:
        r, g, bb = ImageStat.Stat(im.crop((0, t, 1920, b))).mean
        h, l, s = colorsys.rgb_to_hls(r/255, g/255, bb/255)
        out.append(f"{t}-{b}: ({r:.0f},{g:.0f},{bb:.0f}) h{h*360:.0f} s{s:.3f}")
    print(p.split("/")[-1], " | ".join(out))
