# crop helper: python crop.py out.png scale "label|path|x0,y0,x1,y1" ...  (grid of 2 columns)
import sys
from PIL import Image, ImageDraw
out=sys.argv[1]; scale=float(sys.argv[2]); items=sys.argv[3:]
cols=int(__import__('os').environ.get('COLS','2'))
tiles=[]
for it in items:
    lab,path,box=it.split('|')
    im=Image.open(path).convert('RGB')
    b=[int(v) for v in box.split(',')]
    c=im.crop(b)
    c=c.resize((int(c.width*scale),int(c.height*scale)),Image.NEAREST if scale>=2 else Image.LANCZOS)
    t=Image.new('RGB',(c.width,c.height+20),(20,20,20)); t.paste(c,(0,20))
    ImageDraw.Draw(t).text((4,4),lab,fill=(255,255,255)); tiles.append(t)
w=max(t.width for t in tiles); h=max(t.height for t in tiles)
rows=(len(tiles)+cols-1)//cols
S=Image.new('RGB',(cols*w+(cols-1)*8,rows*h+(rows-1)*8),(20,20,20))
for i,t in enumerate(tiles): S.paste(t,((i%cols)*(w+8),(i//cols)*(h+8)))
S.save(out); print(out,S.size)
