import re, sys
from PIL import Image, ImageDraw, ImageFont
log=open(sys.argv[1],encoding='utf-8',errors='replace').read().splitlines()
S={}
for l in log:
    if 'event=motion' not in l: continue
    mv=re.search(r'move=(\w+);mode=(\w+);frame=(\d+);offset=([-\d.]+)',l)
    cam=re.search(r'cam ev=([-\d.]+) target=([-\d.]+).*?mean=([-\d.]+) high=([-\d.]+)',l)
    fr=re.search(r'computed=\d+ frame=(\d+)',l)
    if not (mv and cam): continue
    S.setdefault((mv.group(1),mv.group(2)),[]).append((int(mv.group(3)),float(mv.group(4)),float(cam.group(1)),float(cam.group(2)),float(cam.group(3)),float(cam.group(4)),int(fr.group(1)) if fr else 0))
f=ImageFont.truetype('segoeui.ttf',14)
W,H=900,220
img=Image.new('RGB',(W,H*len(S)+10),(250,250,250)); d=ImageDraw.Draw(img)
for i,(k,rows) in enumerate(sorted(S.items())):
    evs=[r[2] for r in rows]; tg=[r[3] for r in rows]
    lo=min(evs+tg)-0.1; hi=max(evs+tg)+0.1
    y0=i*H
    # reversals: target derivative sign flips with |step|>0.05 while moving
    dt=[tg[j+1]-tg[j] for j in range(len(tg)-1)]
    de=[evs[j+1]-evs[j] for j in range(len(evs)-1)]
    flips=sum(1 for j in range(len(de)-1) if de[j]*de[j+1]<0 and abs(de[j])>0.03 and abs(de[j+1])>0.03)
    tflips=sum(1 for j in range(len(dt)-1) if dt[j]*dt[j+1]<0 and abs(dt[j])>0.1 and abs(dt[j+1])>0.1)
    maxstep=max(abs(x) for x in de); maxtstep=max(abs(x) for x in dt)
    txt=f"{k[0]} mode={k[1]}: ev {min(evs):.2f}..{max(evs):.2f}, target {min(tg):.2f}..{max(tg):.2f}, max ev step {maxstep:.2f}, max target step {maxtstep:.2f}, ev reversals {flips}, target reversals {tflips}, n {len(rows)}"
    print(txt)
    d.text((8,y0+4),txt,fill=(0,0,0),font=f)
    def P(j,v): return (10+j*(W-20)/max(len(rows)-1,1), y0+H-15-(v-lo)/(hi-lo)*(H-45))
    d.line([P(j,v) for j,v in enumerate(tg)],fill=(210,120,0),width=2)
    d.line([P(j,v) for j,v in enumerate(evs)],fill=(0,80,200),width=2)
img.save(sys.argv[2]); print(sys.argv[2])
