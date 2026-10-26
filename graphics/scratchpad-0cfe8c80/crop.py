import sys
from PIL import Image
v,w,x0,y0,x1,y1,name=sys.argv[1],sys.argv[2],*map(int,sys.argv[3:7]),sys.argv[7]
scale=float(sys.argv[8]) if len(sys.argv)>8 else 1
idx=sys.argv[9] if len(sys.argv)>9 else '1'
d='E:/look-lanes/look-surface/captures/t11/p1-look_surface_t_%s_%s/tri/look_surface_t_%s_%s/'%(v,w,v,w)
import glob
b=Image.open(glob.glob(d+'*_%s__base%s.png'%(v,idx))[0]).crop((x0,y0,x1,y1)); s=Image.open(glob.glob(d+'*_%s__surf%s.png'%(v,idx))[0]).crop((x0,y0,x1,y1))
if scale!=1:
  b=b.resize((int(b.width*scale),int(b.height*scale)),Image.NEAREST); s=s.resize(b.size,Image.NEAREST)
o=Image.new('RGB',(b.width*2+6,b.height),(255,0,0)); o.paste(b,(0,0)); o.paste(s,(b.width+6,0))
o.save('D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/surface/critic_art_r2/'+name+'.png')
