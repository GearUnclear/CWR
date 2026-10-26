import numpy as np
from PIL import Image
E = r"D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\B10"
for v in ["coastline_clear", "coast_oc"]:
    A = np.asarray(Image.open(rf"{E}\final2\clouds_{v}__off.png").convert("RGB")).astype(int)
    B = np.asarray(Image.open(rf"{E}\before\clouds_{v}__on.png").convert("RGB")).astype(int)
    C = np.asarray(Image.open(rf"{E}\before\clouds_{v}__off.png").convert("RGB")).astype(int)
    for name, X, Y in (("final2off-vs-beforeOn", A, B), ("beforeOff-vs-beforeOn", C, B)):
        d = np.abs(X - Y).max(axis=2)
        ys, xs = np.nonzero(d > 8)
        # row histogram in 8 bands
        h = d.shape[0]
        bands = [int(((ys >= h * i // 12) & (ys < h * (i + 1) // 12)).sum()) for i in range(12)]
        print(v, name, ">8", len(ys), "rows/12:", bands)
