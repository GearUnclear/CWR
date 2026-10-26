import numpy as np
from PIL import Image
from pathlib import Path
R=Path(r"D:\Arma_CWA_worktrees\lighting\tmp\research\shadows\run8prod\shots")
def lum(p):
    a=np.asarray(Image.open(p).convert("RGB"),dtype=np.float32)/255
    lin=a**2.2
    y=(0.2126*lin[...,0]+0.7152*lin[...,1]+0.0722*lin[...,2])
    return y.mean()**(1/2.2), np.median(a.mean(-1)), a[..., 0].mean()/max(a[...,2].mean(),1e-6)
rows=["| view | classic | rem | prod | prodexp | best |","|---|---|---|---|---|---|"]
for v in ("woods_pine2_morning","woods_pine2_evening","woods_mixed_noon","town_golden","forest_edge_golden","lowsun_fp"):
    vals=[]
    for k in ("classic","rem","prod","prodexp","best"):
        m,med,rb=lum(R/f"{v}__{k}.png"); vals.append("%.3f (R/B %.2f)"%(m,rb))
    rows.append("| %s | %s |"%(v," | ".join(vals)))
out="\n".join(rows); print(out)
open(r"D:\Arma_CWA_worktrees\lighting\tmp\research\shadows\run8prod\frame_brightness.md","w").write("Mean frame brightness (display value of the mean linear luminance, 0-1) and mean R/B ratio, whole 960x640 frame.\n\n"+out+"\n")
