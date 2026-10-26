import numpy as np
from PIL import Image
src="D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/light/final/"
views=["lowsun_fp","noon_fields","forest_edge_golden","sunrise_ridge","coastline_sunset","overcast_broken","town_night","sinai_desert_noon","woods_pine2_evening","blue_hour_coast","tank_gunner_interior","tank_gunner_optics"]
print(f"{'view':22s} {'mode':8s} {'meanL':>6s} {'p1':>4s} {'p50':>4s} {'p99':>4s} {'white%':>7s} {'clip1%':>7s} {'black%':>7s} {'sat':>5s}")
for v in views:
    for m in ["classic","off","on"]:
        try: a=np.asarray(Image.open(src+f"{v}__{m}.png").convert("RGB")).astype(np.float32)
        except Exception as e: print(v,m,"missing"); continue
        L=0.2126*a[...,0]+0.7152*a[...,1]+0.0722*a[...,2]
        mx=a.max(-1); mn=a.min(-1)
        white=(mn>=250).mean()*100
        clip1=(mx>=254).mean()*100
        black=(L<10).mean()*100
        sat=((mx-mn)/np.maximum(mx,1)).mean()
        p=np.percentile(L,[1,50,99])
        print(f"{v:22s} {m:8s} {L.mean():6.1f} {p[0]:4.0f} {p[1]:4.0f} {p[2]:4.0f} {white:7.2f} {clip1:7.2f} {black:7.2f} {sat:5.2f}")
