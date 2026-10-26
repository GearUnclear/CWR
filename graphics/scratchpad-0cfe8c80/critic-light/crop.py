import sys
from PIL import Image
src="D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/light/final/"
out="D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/critic-light-artefacts/"
# args: view mode x0 y0 x1 y1 scale name
view,mode,x0,y0,x1,y1,scale,name=sys.argv[1:9]
im=Image.open(src+f"{view}__{mode}.png").convert("RGB")
c=im.crop((int(x0),int(y0),int(x1),int(y1)))
s=int(scale)
c=c.resize((c.width*s,c.height*s),Image.NEAREST)
c.save(out+name)
print(out+name)
