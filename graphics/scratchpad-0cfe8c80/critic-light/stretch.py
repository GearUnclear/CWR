import sys, numpy as np
from PIL import Image
src="D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/light/final/"
out="D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/critic-light-artefacts/"
view,mode,x0,y0,x1,y1,k,name=sys.argv[1:9]
a=np.asarray(Image.open(src+f"{view}__{mode}.png").convert("RGB")).astype(np.float32)[int(y0):int(y1),int(x0):int(x1)]
m=a.reshape(-1,3).mean(0)
b=np.clip((a-m)*float(k)+128,0,255).astype(np.uint8)
Image.fromarray(b).save(out+name); print(out+name)
# banding metric: fraction of horizontal-row-mean steps that are exactly integer jumps after runs
L=a.mean(2).mean(1)
d=np.diff(L)
print("row-mean range",L.min().round(1),L.max().round(1))
