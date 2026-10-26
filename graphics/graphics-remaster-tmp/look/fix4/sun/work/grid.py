import sys
from PIL import Image, ImageDraw
# usage: grid.py out.png width cols label=path ...
out=sys.argv[1]; W=int(sys.argv[2]); cols=int(sys.argv[3]); items=[a.split('=',1) for a in sys.argv[4:]]
ims=[]
for lab,p in items:
    im=Image.open(p).convert('RGB'); h=int(im.height*W/im.width); ims.append((lab,im.resize((W,h),Image.LANCZOS)))
H=max(i.height for _,i in ims)+18
rows=(len(ims)+cols-1)//cols
o=Image.new('RGB',(cols*(W+4),rows*H),(20,20,20)); d=ImageDraw.Draw(o)
for k,(lab,im) in enumerate(ims):
    x=(k%cols)*(W+4); y=(k//cols)*H
    d.text((x+3,y+3),lab,fill=(255,255,0)); o.paste(im,(x,y+18))
o.save(out)
