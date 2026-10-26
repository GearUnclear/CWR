import numpy as np, sys, re
from PIL import Image, ImageDraw
from parse import load
RANGES={'sinai':(-50.0,475.0),'abel':(-100.0,521.0),'eden':(-49.8,357.8),'cain':(-60.1,688.2)}
def heights(w):
    im=np.asarray(Image.open(f'{w}.png').convert('RGB')).astype(float)
    r,g,b=im[...,0],im[...,1],im[...,2]
    t=np.zeros(r.shape)
    # band1: g in 120..180, r 30..130 -> s=(r-30)/100
    m1=(b<=50.5)&(g>=119)&(r<=130.5)&~((r==30)&(g==60))
    t[m1]=0.01+((r[m1]-30)/100)*0.29
    m2=(r>130.5)&(r<=190.5)&(b<=50.5)
    t[m2]=0.3+((r[m2]-130)/60)*0.4
    m3=(r>190.5)
    t[m3]=0.7+((r[m3]-190)/65)*0.3
    mn,mx=RANGES[w]
    H=mn+t*(mx-mn)
    return H  # image rows: top = north
def at(H,x,z,cell=50.0):
    n=H.shape[0]; i=int(x/cell); j=n-1-int(z/cell)
    return H[max(0,min(n-1,j)),max(0,min(n-1,i))]
if __name__=='__main__':
    w=sys.argv[1]; H=heights(w); n=H.shape[0]; S=4
    img=np.zeros((n,n,3),np.uint8)
    land=H>0.5
    v=np.clip(H/ H.max(),0,1)
    img[...,0]=np.where(land,60+v*180,20); img[...,1]=np.where(land,90+v*150,40); img[...,2]=np.where(land,40+v*120,120)
    I=Image.fromarray(img).resize((n*S,n*S),Image.NEAREST); d=ImageDraw.Draw(I)
    for o in load(w):
        px=o[1]/50*S; pz=(n-o[3]/50)*S
        nm=o[0]
        if 'lampa' in nm: d.ellipse([px-3,pz-3,px+3,pz+3],fill=(255,255,0))
        elif re.search(r'dum|domek|house|kostel|hospoda|barak|garaz|budova|hangar|stodola|kaple',nm): d.point((px,pz),fill=(255,60,60))
        elif re.search(r'silnice|asfalt|cesta',nm): d.point((px,pz),fill=(200,200,200))
        elif re.search(r'^data3d.(str|les|jablon|oliva|sakura|smrk|borovice|dub)',nm): d.point((px,pz),fill=(0,140,0))
    for k in range(0,n*S,S*20): d.line([k,0,k,n*S],fill=(80,80,80)); d.line([0,k,n*S,k],fill=(80,80,80))
    for k in range(0,n,20): d.text((k*S+2,2),str(k*50),fill=(255,255,255)); d.text((2,(n-k)*S-12),str(k*50),fill=(255,255,255))
    I.save(f'{w}_map.png')
