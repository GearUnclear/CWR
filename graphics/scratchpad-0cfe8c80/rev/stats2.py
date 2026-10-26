import sys, glob, os
from PIL import Image
import numpy as np
def st(p):
    a=np.asarray(Image.open(p).convert('RGB')).astype(float)
    h=a.shape[0]; g=a[int(h*0.72):]
    m=g.reshape(-1,3).mean(0)
    lin=(g/255.0)**2.2; L=(0.2126*lin[...,0]+0.7152*lin[...,1]+0.0722*lin[...,2]).mean()
    mx=g.max(2); mn=g.min(2); sat=(np.where(mx>0,(mx-mn)/np.maximum(mx,1),0)).mean()
    return m, L, sat
d=sys.argv[1]
for v in ['dm1900','dm1910','dm1925','dm1940','dm1955','dm2015']:
    r={}
    for tag,cls in [('on','remastered'),('off','remastered'),('cl','classic')]:
        fs=glob.glob(os.path.join(d,'%s%s__%s.png'%(v,tag,cls)))
        if fs: r[tag]=st(fs[0])
    line=v
    for t in ['on','off','cl']:
        if t in r:
            m,L,s=r[t]; line+='  %s rgb=%s linL=%.4f sat=%.2f' % (t, np.round(m).astype(int).tolist(), L, s)
    if 'on' in r and 'off' in r: line+='  on/off EV=%+.2f' % np.log2(r['on'][1]/r['off'][1])
    print(line)
