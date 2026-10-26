import os, sys
from PIL import Image, ImageDraw
src = r"D:\Arma_CWA_worktrees\graphics-remaster\tmp\look\director\final"
out = r"D:\Arma_CWA_worktrees\graphics-remaster\tmp\look\critic-director"
views = sorted({f.split('__classic')[0] for f in os.listdir(src) if f.endswith('__classic.png')})
pairs=[]
for v in views:
    c=Image.open(os.path.join(src,v+'__classic.png')).convert('RGB')
    o=Image.open(os.path.join(src,v+'__overhaul.png')).convert('RGB')
    W=480; H=int(c.height*W/c.width)
    c=c.resize((W,H),Image.LANCZOS); o=o.resize((W,int(o.height*W/o.width)),Image.LANCZOS)
    p=Image.new('RGB',(W*2+8,H+18),(255,255,255))
    p.paste(c,(0,18)); p.paste(o,(W+8,18))
    ImageDraw.Draw(p).text((4,3),v+"   [left Original | right Overhaul]",fill=(0,0,0))
    pairs.append((v,p))
    print(v, c.size, o.size)
# group 4 pairs per page vertically
for i in range(0,len(pairs),4):
    grp=pairs[i:i+4]
    Wt=max(p.width for _,p in grp); Ht=sum(p.height for _,p in grp)
    page=Image.new('RGB',(Wt,Ht),(255,255,255)); y=0
    for _,p in grp: page.paste(p,(0,y)); y+=p.height
    page.save(os.path.join(out,f'thumbs_{i//4:02d}.png'))
