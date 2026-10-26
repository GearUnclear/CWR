import numpy as np
from PIL import Image
E = r"D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\B10\final2"
for v, (cx, cy) in [("coast_oc", (1235, 345)), ("coast_oc_up", (1615, 645))]:
    A = np.asarray(Image.open(rf"{E}\clouds_{v}__off.png").convert("RGB")).astype(int)
    B = np.asarray(Image.open(rf"{E}\clouds_{v}__on.png").convert("RGB")).astype(int)
    la = A.sum(axis=2); 
    # find sun: brightest blob near guess
    win = la[cy-80:cy+80, cx-80:cx+80]
    yy, xx = np.unravel_index(np.argmax(win), win.shape)
    sy, sx = cy - 80 + yy, cx - 80 + xx
    r = 14
    a = A[sy-r:sy+r, sx-r:sx+r].reshape(-1, 3).mean(axis=0)
    b = B[sy-r:sy+r, sx-r:sx+r].reshape(-1, 3).mean(axis=0)
    print(v, "sun at", sx, sy, "off", a.round(1), "on", b.round(1), "maxdiff", np.abs(A[sy-r:sy+r, sx-r:sx+r] - B[sy-r:sy+r, sx-r:sx+r]).max())
