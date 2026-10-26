import sys
from PIL import Image
P = r"D:\Arma_CWA\tmp\lobo-wound-recolor\png"
parts = ["back","legsbk","front","armle","armri","legsfr","legslft","legsrgt"]
facs = ["eaf_tex","iaf_tex2","rjaf_tex"]
part = sys.argv[1]
imgs = []
for f in facs:
    for suf in ["","_d"]:
        im = Image.open(f"{P}\\{f}_{part}{suf}.png").convert("RGB")
        im = im.resize((256, int(256*im.height/im.width)))
        imgs.append(im)
w = sum(i.width for i in imgs)+10*len(imgs); h = max(i.height for i in imgs)
sheet = Image.new("RGB",(w,h),(255,0,255)); x=0
for i in imgs:
    sheet.paste(i,(x,0)); x+=i.width+10
sheet.save(rf"D:\Arma_CWA\tmp\lobo-wound-recolor\sheet_{part}.png")
print("ok", [ (i.width,i.height) for i in imgs])
