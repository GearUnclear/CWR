# sbs.py out.png view after.png cx cy cw ch [suffix]  (crop centre and size as fractions of the frame)
import sys
from PIL import Image, ImageDraw
L='D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/'
out,view,after=sys.argv[1:4]; cx,cy,cw,ch=map(float,sys.argv[4:8]); suf=sys.argv[8] if len(sys.argv)>8 else '__1080'
srcs=[('vanilla',L+'ref/vanilla/%s%s.png'%(view,suf)),('landed',L+'ref/landed/%s%s.png'%(view,suf)),('before (round 3)',L+'director/final/%s%s__overhaul.png'%(view,suf)),('after (round 4 sun)',after)]
import os
srcs=[(a,b if os.path.exists(b) else b.replace(suf+'.png','.png').replace(suf+'__overhaul','__overhaul')) for a,b in srcs]
W=480
ims=[(lab,Image.open(p).convert('RGB')) for lab,p in srcs]
fulls=[(lab,im.resize((W,int(im.height*W/im.width)),Image.LANCZOS)) for lab,im in ims]
crops=[]
for lab,im in ims:
    w,h=im.size; bw,bh=int(cw*w),int(ch*h); x0=int(cx*w-bw/2); y0=int(cy*h-bh/2)
    c=im.crop((x0,y0,x0+bw,y0+bh)); s=W/bw
    crops.append(c.resize((W,int(bh*s)),Image.LANCZOS if s<1 else Image.BICUBIC))
H1=max(i.height for _,i in fulls); H2=max(c.height for c in crops)
o=Image.new('RGB',(4*(W+4),18+H1+18+H2),(20,20,20)); d=ImageDraw.Draw(o)
for k,((lab,f),c) in enumerate(zip(fulls,crops)):
    x=k*(W+4); d.text((x+3,3),view+': '+lab,fill=(255,255,0)); o.paste(f,(x,18))
    d.text((x+3,18+H1+3),'crop %.1fx'%(W/(cw*ims[k][1].width)),fill=(255,255,0)); o.paste(c,(x,18+H1+18))
o.save(out); print(out,o.size)
