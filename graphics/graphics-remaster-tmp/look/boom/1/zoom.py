import sys, glob
from PIL import Image, ImageDraw
run, out = sys.argv[1], sys.argv[2]
spec = {('dusk','he'):(range(9,21),(150,250,630,420)), ('dusk','grenade'):(range(9,21),(200,280,680,440)),
        ('dusk','heat'):(range(9,21),(250,250,730,420)), ('dusk','fuel'):(list(range(12,24))+list(range(26,90,8))+[120,150],(380,180,860,420)),
        ('night','he'):(range(9,21),(150,250,630,420)), ('night','fuel'):(list(range(12,24))+list(range(26,90,8))+[120,150],(380,180,860,420))}
rows=[]
for (t,c),(fr,box) in spec.items():
    fs=sorted(glob.glob(f'{run}/tri/look_probe_mun_{t}/*mun_{t}_{c}_f*.png'))
    tiles=[]
    for f in fr:
        if f-1 < len(fs):
            im=Image.open(fs[f-1]).convert('RGB').crop(box)
            ImageDraw.Draw(im).text((3,3),f"{t} {c} f{f} {(f-8)*33:+d}ms",fill=(255,230,0))
            tiles.append(im)
    w,h=tiles[0].size; per=6
    g=Image.new('RGB',(w*per,h*((len(tiles)+per-1)//per)))
    for i,tl in enumerate(tiles): g.paste(tl,((i%per)*w,(i//per)*h))
    g.save(f'{out}_{t}_{c}.png')
