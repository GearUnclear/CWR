import sys
from PIL import Image
base = r"D:\Arma_CWA_worktrees\lighting\tmp\research\tiling"
out, scale = sys.argv[1], int(sys.argv[2])
box = tuple(int(x) for x in sys.argv[3].split(","))
paths = sys.argv[4:]
ims = [Image.open(base + "\\" + p).convert("RGB").crop(box) for p in paths]
w, h = ims[0].size
W = Image.new("RGB", (w*scale*len(ims) + 6*(len(ims)-1), h*scale), (0,0,0))
for i, im in enumerate(ims):
    W.paste(im.resize((w*scale, h*scale), Image.NEAREST), (i*(w*scale+6), 0))
W.save(out)
print(W.size)
