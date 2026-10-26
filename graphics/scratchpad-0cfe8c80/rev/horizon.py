import numpy as np
from PIL import Image
E = r"D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\B10"
OUT = r"C:\Users\Dane\AppData\Local\Temp\claude\D--Arma-CWA\0cfe8c80-fc33-4f8e-a86e-52fa1175350d\scratchpad\rev"
ims = [Image.open(p).convert("RGB") for p in (rf"{E}\before\clouds_coast_oc__on.png", rf"{E}\before\clouds_coast_oc__off.png", rf"{E}\final2\clouds_coast_oc__off.png")]
A = np.asarray(ims[0]).astype(int); C = np.asarray(ims[2]).astype(int)
d = np.abs(A - C).max(axis=2); ys, xs = np.nonzero(d > 8)
print("x range", xs.min(), xs.max(), "y range", ys.min(), ys.max())
box = (max(xs.min() - 60, 0), ys.min() - 30, min(xs.max() + 60, 1920), ys.max() + 30)
crops = [im.crop(box).resize(((box[2] - box[0]), (box[3] - box[1]) * 3)) for im in ims]
w, h = crops[0].size
out = Image.new("RGB", (w, (h + 6) * 3), (0, 0, 0))
for i, c in enumerate(crops):
    out.paste(c, (0, i * (h + 6)))
out.save(rf"{OUT}\horizon_coast_oc.png")
