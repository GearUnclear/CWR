import sys, numpy as np
from PIL import Image
root=sys.argv[1]; out=sys.argv[2]
for v in ["sinai_lowsun_fp","lowsun_fp","forest_edge_golden"]:
    A=np.asarray(Image.open(f"{root}/after/{v}__off.png").convert("RGB")).astype(np.int32)
    B=np.asarray(Image.open(f"{root}/after/{v}__on.png").convert("RGB")).astype(np.int32)
    h,w,_=A.shape
    sky=slice(0,int(h*0.4))
    d=(B-A)[sky]
    print(v, "sky on-off mean RGB", d.reshape(-1,3).mean(0).round(2), "max", np.abs(d).max())
    m=np.abs(d).max(axis=2)>2
    ys,xs=np.nonzero(m)
    if len(xs): print("   changed px bbox x",xs.min(),xs.max(),"y",ys.min(),ys.max(), "count",len(xs))
    # heatmap
    hm=(np.clip(np.abs(B-A).max(axis=2)*8,0,255)).astype(np.uint8)
    Image.fromarray(hm).resize((w//2,h//2)).save(f"{out}/diff_{v}.png")
