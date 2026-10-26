import sys
import numpy as np
from PIL import Image
E = r"D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\B10"
OUT = r"C:\Users\Dane\AppData\Local\Temp\claude\D--Arma-CWA\0cfe8c80-fc33-4f8e-a86e-52fa1175350d\scratchpad\rev"
names = sys.argv[1].split(',')
tiles = []
for n in names:
    for f in ("final2", "final3"):
        A = np.asarray(Image.open(rf"{E}\{f}\tri\b10_sweep\{n}").convert("RGB")).astype(int)
        tiles.append(A)
# locate diff bbox across first pair
d = np.abs(tiles[0] - tiles[1]).max(axis=2)
ys, xs = np.nonzero(d > 8)
print("bbox", xs.min(), xs.max(), ys.min(), ys.max(), "size", tiles[0].shape)
cx, cy = int(xs.mean()), int(ys.mean())
r = 160
crops = [Image.fromarray(t[max(cy-r,0):cy+r, max(cx-r,0):cx+r].astype(np.uint8)) for t in tiles]
w, h = crops[0].size
out = Image.new("RGB", (w * 2 + 8, (h + 8) * len(names)), (0, 0, 0))
for i, c in enumerate(crops):
    out.paste(c, ((i % 2) * (w + 8), (i // 2) * (h + 8)))
out.save(rf"{OUT}\sun_sweep_f2_f3.png")
