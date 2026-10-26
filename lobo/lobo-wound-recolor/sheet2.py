import sys
from PIL import Image
O = r"D:\Arma_CWA\tmp\lobo-wound-recolor\out"
P = r"D:\Arma_CWA\tmp\lobo-wound-recolor\png"
part = sys.argv[1]
names = [f"{O}\\mask_{part}.png", f"{O}\\blood_on_grey_{part}.png", f"{P}\\rjaf_tex_{part}_d.png",
         f"{O}\\eaf_{part}_d_new_rt.png", f"{O}\\iaf2_{part}_d_new_rt.png", f"{O}\\rjaf_{part}_d_new_rt.png"]
imgs = []
for n in names:
    im = Image.open(n).convert("RGB")
    im = im.resize((256, int(256*im.height/im.width)), Image.NEAREST)
    imgs.append(im)
w = sum(i.width for i in imgs)+10*len(imgs); h = max(i.height for i in imgs)
sheet = Image.new("RGB",(w,h),(255,0,255)); x=0
for i in imgs:
    sheet.paste(i,(x,0)); x+=i.width+10
sheet.save(rf"D:\Arma_CWA\tmp\lobo-wound-recolor\sheet2_{part}.png")
