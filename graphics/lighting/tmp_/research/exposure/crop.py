import sys
from PIL import Image
run, view, labs, out = sys.argv[1], sys.argv[2], sys.argv[3].split(","), sys.argv[4]
box = tuple(int(v) for v in sys.argv[5].split(",")) if len(sys.argv) > 5 else None
ims = [Image.open(f"{run}/{view}__{l}.png").convert("RGB") for l in labs]
if box: ims = [i.crop(box) for i in ims]
w, h = ims[0].size
sh = Image.new("RGB", (w*len(ims) + 6*(len(ims)-1), h), (0,0,0))
for k, i in enumerate(ims): sh.paste(i, (k*(w+6), 0))
sh.save(out)
