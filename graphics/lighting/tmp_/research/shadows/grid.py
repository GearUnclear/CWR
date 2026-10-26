from PIL import Image, ImageDraw
import sys
import os; base=os.environ.get("GRIDBASE", r"D:\Arma_CWA_worktrees\lighting\tmp\research\shadows\run7woods\shots")
view=sys.argv[1]; labs=sys.argv[2].split(","); box=tuple(int(v) for v in sys.argv[3].split(",")); out=sys.argv[4]; sc=float(sys.argv[5]) if len(sys.argv)>5 else 1.0
names={"classic":"Classic","rem":"Remastered (current)","obj":"+ casters (stock filter, 2048)","objpcss2k":"+ casters, PCSS 1 deg, 2048","best":"+ casters, PCSS 1 deg, 4096 tight, contact","dbgsunvisbest":"debug: sun visibility (best)","prod":"+ casters, PCSS 1 deg, 2048, tight tiers (prod)","prodexp":"+ prod, exposure x1.25","dbgsunvisprod":"debug: sun visibility (prod)"}
tiles=[]
for l in labs:
    im=Image.open(f"{base}/{view}__{l}.png").convert("RGB").crop(box)
    if sc!=1.0: im=im.resize((int(im.width*sc),int(im.height*sc)),Image.LANCZOS)
    d=ImageDraw.Draw(im); t=names.get(l,l); d.rectangle([0,0,7*len(t)+8,16],fill=(0,0,0)); d.text((4,3),t,fill=(255,255,255))
    tiles.append(im)
cols=min(len(tiles),3); rows=(len(tiles)+cols-1)//cols
W=tiles[0].width; H=tiles[0].height
o=Image.new("RGB",(cols*W+(cols-1)*6,rows*H+(rows-1)*6),(30,30,30))
for i,t in enumerate(tiles): o.paste(t,((i%cols)*(W+6),(i//cols)*(H+6)))
o.save(out); print(o.size)

