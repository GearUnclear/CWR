import sys, os
from PIL import Image, ImageDraw
src = sys.argv[1]; out = sys.argv[2]
views = sys.argv[3:]
for v in views:
    ims = []
    for k in ("classic", "off", "on"):
        f = os.path.join(src, f"{v}__{k}.png")
        im = Image.open(f).convert("RGB")
        ims.append((k, im))
    w, h = ims[0][1].size
    print(v, [ (k, im.size) for k, im in ims])
    sc = 0.5
    tw, th = int(w*sc), int(h*sc)
    strip = Image.new("RGB", (tw, th*3), (0,0,0))
    for i,(k,im) in enumerate(ims):
        s = im.resize((tw, th), Image.LANCZOS)
        d = ImageDraw.Draw(s); d.rectangle((0,0,90,22), fill=(0,0,0)); d.text((5,5), k, fill=(255,255,255))
        strip.paste(s, (0, th*i))
    strip.save(os.path.join(out, f"{v}_strip.png"))
