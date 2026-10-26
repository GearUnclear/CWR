import re, glob, sys
from PIL import Image, ImageDraw, ImageFont, ImageStat
run = sys.argv[1]; out = sys.argv[2]; title = sys.argv[3]
labs = ["remastered_down", "up_settled", "f001", "f009", "f025", "f057", "f121", "f281"]
fps = {}
for p in glob.glob(run + "/tri/**/game_stdout.log", recursive=True):
    for l in open(p, encoding="utf-8", errors="replace"):
        m = re.search(r"label=adapt_seq__(\w+);.*?;fps=([\d.]+)", l)
        if m: fps[m.group(1)] = float(m.group(2))
f = ImageFont.truetype("C:/Windows/Fonts/arialbd.ttf", 26)
names = {"remastered_down": "Remastered (current), looking down", "up_settled": "Prototype: settled on the sky (pitch +20)"}
ims = []
for l in labs:
    im = Image.open(f"{run}/adapt_seq__{l}.png").convert("RGB")
    d = ImageDraw.Draw(im)
    t = names.get(l) or f"after the cut down: frame {int(l[1:])}"
    b = d.textbbox((12, 10), t, font=f); d.rectangle([b[0]-6, b[1]-4, b[2]+6, b[3]+4], fill=(0,0,0)); d.text((12, 10), t, font=f, fill=(255,255,255))
    ims.append(im)
w, h = ims[0].size
cols = 4
sheet = Image.new("RGB", (cols*w + (cols-1)*8, 2*h + 8 + 44), (20,20,20))
ImageDraw.Draw(sheet).text((10, 8), title, font=ImageFont.truetype("C:/Windows/Fonts/arialbd.ttf", 28), fill=(235,235,235))
for i, im in enumerate(ims):
    r, c = divmod(i, cols); sheet.paste(im, (c*(w+8), 44 + r*(h+8)))
sheet.resize((sheet.width//2, sheet.height//2), Image.LANCZOS).save(out)
for l, im in zip(labs, ims): print(l, "lower-half mean", round(sum(ImageStat.Stat(im.crop((0, h//2, w, h))).mean)/3, 1), "fps", fps.get(l))
