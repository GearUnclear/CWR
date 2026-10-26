# highpass.py out.png gain "label|path|x0,y0,x1,y1" ... : 12x amplified high-pass (minus 9 px box blur) of luma, plus the crop
import sys
import numpy as np
from PIL import Image, ImageDraw, ImageFilter
out=sys.argv[1]; gain=float(sys.argv[2]); tiles=[]
for it in sys.argv[3:]:
    lab,p,box=it.split('|'); b=[int(v) for v in box.split(',')]
    im=Image.open(p).convert('RGB').crop(b)
    a=np.asarray(im).astype(float)
    l=a@[0.2126,0.7152,0.0722]
    from scipy.ndimage import uniform_filter
    bl=uniform_filter(l,19,mode='nearest')
    hp=np.clip(128+gain*(l-bl),0,255).astype(np.uint8)
    t=Image.new('RGB',(im.width*2+8,im.height+20),(20,20,20))
    t.paste(im,(0,20)); t.paste(Image.fromarray(hp).convert('RGB'),(im.width+8,20))
    ImageDraw.Draw(t).text((4,4),lab+'  (crop | luma high-pass x%g)'%gain,fill=(255,255,255)); tiles.append(t)
w=max(t.width for t in tiles); H=sum(t.height+8 for t in tiles)
S=Image.new('RGB',(w,H),(20,20,20)); y=0
for t in tiles: S.paste(t,(0,y)); y+=t.height+8
S.save(out); print(out,S.size)
