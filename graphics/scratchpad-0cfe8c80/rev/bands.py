import sys, numpy as np
from PIL import Image
root = sys.argv[1]
views = ["lowsun_fp","lowsun_box","dawn_box","dawn_box5","forest_edge_golden","noon_fields","sinai_lowsun_fp","sinai_box","sinai_dusk_box"]
def load(p): return np.asarray(Image.open(p).convert("RGB")).astype(np.float32)
for v in views:
    out=[]
    for a,b,lab in [("before/%s__off.png","after/%s__off.png","offX"),("before/%s__classic.png","after/%s__classic.png","clsX"),("after/%s__off.png","after/%s__on.png","onvsoff")]:
        A=load(root+"/"+(a%v)); B=load(root+"/"+(b%v))
        h=A.shape[0]
        bands={"sky":(0,int(h*0.4)),"hor":(int(h*0.4),int(h*0.55)),"gnd":(int(h*0.55),h)}
        s=[]
        for n,(y0,y1) in bands.items():
            a_=A[y0:y1]; b_=B[y0:y1]
            d=np.abs(a_-b_).max(axis=2)
            s.append("%s mean %.2f/%.2f >2:%d" % (n, a_.mean(), b_.mean(), int((d>2).sum())))
        out.append(lab+": "+"; ".join(s))
    print(v); [print("   ",o) for o in out]
