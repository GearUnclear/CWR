from PIL import Image, ImageDraw, ImageFont
from pathlib import Path
root=Path(r'D:\Arma_CWA\arma_CWR\tmp\hq-model-candidates')
files=sorted((root/'captures').glob('*.png'))
labels=[('Single ammo crate','HeavyReammoBoxRes | bedna_ammo.p3d'),('Ammo crate stack','ReammoBoxGuer | hromada_beden.p3d'),('Map desk','Tablemap | stulsuplikmapa.p3d'),('Small field tent','ACamp | astan.p3d'),('Open tent','CampEmpty | stan_inside.p3d'),('Large tent','CampEast | stan_east.p3d'),('Covered tent','CampEastC | stan_eastC.p3d'),('HQ flagpole','FlagCarrier | vlajkstozar.p3d')]
font=ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',23)
small=ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf',17)
for name,indices in [('cache-options',[0,1]),('hq-options',[2,3,4,5,6,7])]:
    w,h,bar=640,420,70
    sheet=Image.new('RGB',(w*2,((len(indices)+1)//2)*(h+bar)),(22,27,30))
    d=ImageDraw.Draw(sheet)
    for j,i in enumerate(indices):
        im=Image.open(files[i]).convert('RGB').crop((180,150,1420,970)).resize((w,h),Image.Resampling.LANCZOS)
        x,y=j%2*w,j//2*(h+bar)
        sheet.paste(im,(x,y))
        d.text((x+14,y+h+7),labels[i][0],font=font,fill='white')
        d.text((x+14,y+h+38),labels[i][1],font=small,fill=(176,198,210))
    sheet.save(root/(name+'.jpg'),quality=94)
print('Created cache-options.jpg and hq-options.jpg')
