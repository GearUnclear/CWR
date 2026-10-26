from PIL import Image
import numpy as np
base = r"D:\Arma_CWA_worktrees\lighting\tmp\research\tiling"
def L(p): return np.asarray(Image.open(base+"\\"+p).convert("RGB")).astype(np.int16)
for view, run, cands in [("high_sinai_noon","run6",["v0","v4","v6","v7"]),("sinai_desert_noon","run6",["v4","v6","v7"]),("forest_edge_golden","run6",["v4","v6"]),("lowsun_fp","run5",["v2","v3","v4","v5"]),("noon_fields","run6",["v4","v6"])]:
    B = L(f"BEST_{view}.png")
    H, W, _ = B.shape
    # find best offset match of each candidate's lower-right 300x200 patch inside the third panel
    for c in cands:
        A = L(f"{run}/cmp/{view}__{c}.png")
        patch = A[400:600, 500:900]
        best = None
        for x0 in range(W-960-20, W-960+20):
            for y0 in range(0, 60):
                sub = B[y0+400:y0+600, x0+500:x0+900]
                if sub.shape != patch.shape: continue
                d = np.abs(sub-patch).mean()
                if best is None or d < best[0]: best = (d, x0, y0)
        print(view, c, "min mean abs diff vs BEST panel 3:", round(best[0],3), best[1:])
