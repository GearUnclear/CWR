import sys
from PIL import Image, ImageChops, ImageStat
pairs = [("run_r1","run_r2"),("run_r2","run_r3"),("run_r1","run_r3")]
for view in ["lowsun_fp","sunrise_ridge","coastline_sunset"]:
    for mode in ["classic","remastered"]:
        row=[]
        for a,b in pairs:
            try:
                ia=Image.open(f"{a}/{view}__{mode}.png").convert("RGB"); ib=Image.open(f"{b}/{view}__{mode}.png").convert("RGB")
                d=ImageStat.Stat(ImageChops.difference(ia,ib)).mean
                ma=ImageStat.Stat(ia).mean; mb=ImageStat.Stat(ib).mean
                row.append(f"{a[-2:]}-{b[-2:]} diff {sum(d)/3:5.2f} mean {sum(ma)/3:5.1f}->{sum(mb)/3:5.1f}")
            except Exception as e: row.append(str(e)[:30])
        print(f"{view:18s} {mode:10s} " + " | ".join(row))
