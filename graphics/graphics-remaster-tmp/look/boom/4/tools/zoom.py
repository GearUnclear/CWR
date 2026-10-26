"""Frame-by-frame zoom sheet for one clip.
usage: zoom.py <run dir with tri/> <time> <clip> <out png> [x0 y0 x1 y1] [frames comma list]"""
import sys, re
from pathlib import Path
from PIL import Image, ImageDraw
run = Path(sys.argv[1]); t = sys.argv[2]; c = sys.argv[3]; out = sys.argv[4]
box = tuple(int(v) for v in sys.argv[5:9]) if len(sys.argv) >= 9 else (280, 120, 680, 420)
frames = [int(v) for v in sys.argv[9].split(",")] if len(sys.argv) >= 10 else [8,9,10,11,12,13,14,16,18,22,26,32,38,46,56,68,83,98]
fr = {}
for p in run.glob(f"tri/look_probe_mun_{t}/*.png"):
    m = re.search(rf"mun_{t}_{c}_f(\d+)\.png$", p.name)
    if m: fr[int(m.group(1))] = p
W = box[2]-box[0]; H = box[3]-box[1]; cols = 6
rows = (len(frames)+cols-1)//cols
sheet = Image.new("RGB", (W*cols, (H+0)*rows), (20,20,20))
for k, f in enumerate(frames):
    if f not in fr: continue
    im = Image.open(fr[f]).convert("RGB").crop(box)
    ms = round((f-8)*1000/30)
    ImageDraw.Draw(im).text((4,4), f"f{f} {ms:+d}ms", fill=(255,230,0))
    sheet.paste(im, ((k%cols)*W, (k//cols)*H))
sheet.save(out); print(out, sheet.size, len(fr), "frames available")
