import sys
from PIL import Image
import numpy as np, colorsys
base = r"D:\Arma_CWA_worktrees\lighting\tmp\research\tiling"
box = tuple(int(x) for x in sys.argv[1].split(","))
for p in sys.argv[2:]:
    a = np.asarray(Image.open(base+"\\"+p).convert("RGB").crop(box)).astype(np.float32)/255
    l = (a*[0.2126,0.7152,0.0722]).sum(-1)
    ch = a.max(-1)-a.min(-1)
    m = a.reshape(-1,3).mean(0)
    # local std (fine) via difference from 8x8 block mean
    H,W = l.shape; H8,W8=H//8*8,W//8*8
    lb = l[:H8,:W8].reshape(H8//8,8,W8//8,8)
    fine = lb.std(axis=(1,3)).mean()
    macro = lb.mean(axis=(1,3)).std()
    print(f"{p:45s} meanRGB={np.round(m*255,1)} luma={l.mean():.3f} chroma={ch.mean():.3f} fineStd={fine:.4f} macroStd={macro:.4f}")
