import sys, re, glob
from PIL import Image, ImageStat, ImageDraw, ImageFont
run = sys.argv[1]
times = {}
for p in glob.glob(run + "/tri/**/game_stdout.log", recursive=True):
    for l in open(p, encoding="utf-8", errors="replace"):
        m = re.search(r"label=adapt_seq__(\w+);.*?;t=([\d.]+)", l)
        if m: times[m.group(1)] = float(m.group(2))
def bar(im, y, ch):
    w = im.width; n = 0
    for x in range(w):
        px = im.getpixel((x, y))
        if px[ch] > 150 and px[(ch + 1) % 3] < 90 and px[(ch + 2) % 3] < 90: n = x + 1
    return n / w * 16 - 12
labs = ["remastered_down", "up_settled", "f001", "f011", "f031", "f071", "f151", "f311"]
t0 = times.get("f001", 0)
f = ImageFont.truetype("C:/Windows/Fonts/arialbd.ttf", 24)
ims = []
for l in labs:
    im = Image.open(f"{run}/adapt_seq__{l}.png").convert("RGB"); h = im.height
    lo = ImageStat.Stat(im.crop((0, h//2, im.width, h))).mean
    meter = adapted = None
    if l != "remastered_down":
        meter = bar(im, int(h*0.010), 1); adapted = bar(im, int(h*0.035), 0)
    t = times.get(l)
    desc = {"remastered_down": "Remastered (current), looking down", "up_settled": "Prototype, settled on the sky (pitch +20)"}.get(l, f"after the cut down: t = {t - t0:+.1f} s")
    print(f"{l:16s} t={t} lowerMean={sum(lo)/3:6.1f} meter={meter} adapted={adapted}")
    d = ImageDraw.Draw(im)
    txt = desc + (f" | meter {meter:.1f}, adapted {adapted:.1f} (log2)" if meter is not None else "")
    b = d.textbbox((12, 46), txt, font=f); d.rectangle([b[0]-6, b[1]-4, b[2]+6, b[3]+4], fill=(0,0,0)); d.text((12, 46), txt, font=f, fill=(255,255,255))
    ims.append(im)
w, h = ims[0].size; cols = 4
sheet = Image.new("RGB", (cols*w + (cols-1)*8, 2*h + 8 + 50), (20,20,20))
ImageDraw.Draw(sheet).text((10, 10), "Eye adaptation over time, sunrise_ridge: settled on the bright sky, then the camera tips down to the dark ground. Speed 0.4/s (run_r6_seq), brighten-only by day, dead band 0.75 EV, max +1 EV. Top bars: green = this frame's meter, red = adapted (log2, -12..4, ticks 1 EV).", font=ImageFont.truetype("C:/Windows/Fonts/arialbd.ttf", 26), fill=(235,235,235))
for i, im in enumerate(ims):
    r, c = divmod(i, cols); sheet.paste(im, (c*(w+8), 50 + r*(h+8)))
sheet.resize((sheet.width//2, sheet.height//2), Image.LANCZOS).save(sys.argv[2])
