import sys
from PIL import Image
out="D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/critic-light-artefacts/"
ims=[Image.open(out+f) for f in sys.argv[2:]]
w=sum(i.width for i in ims)+8*(len(ims)-1); h=max(i.height for i in ims)
c=Image.new("RGB",(w,h),(255,0,255)); x=0
for i in ims: c.paste(i,(x,0)); x+=i.width+8
c.save(out+sys.argv[1]); print(out+sys.argv[1])
