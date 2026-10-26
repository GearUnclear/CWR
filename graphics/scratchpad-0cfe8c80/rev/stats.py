import sys
from PIL import Image
import numpy as np
base=sys.argv[1]
def st(p):
    a=np.asarray(Image.open(p).convert('RGB')).astype(float)
    h=a.shape[0]; g=a[int(h*0.72):]
    m=g.reshape(-1,3).mean(0)
    mx=g.max(2); mn=g.min(2); sat=(np.where(mx>0,(mx-mn)/np.maximum(mx,1),0)).mean()
    return 'ground=%s sat=%.2f r/b=%.2f frameLuma=%.1f' % (np.round(m).astype(int).tolist(), sat, m[0]/max(m[2],1e-3), (0.299*a[...,0]+0.587*a[...,1]+0.114*a[...,2]).mean())
for v in ['town_night','town_lamps','road_moonlit','road_moonless','dusk_town','nv_road','nv_town']:
    for d in ['before','after']:
        for m in ['classic','remastered']:
            print('%-14s %-6s %-10s %s' % (v,d,m, st(base+'/'+d+'/'+v+'__'+m+'.png')))
