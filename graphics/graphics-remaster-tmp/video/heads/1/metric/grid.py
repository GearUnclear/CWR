import glob, json, os, re, sys
from PIL import Image, ImageDraw
src, boxes_path, out = sys.argv[1], sys.argv[2], sys.argv[3]
step = int(sys.argv[4]) if len(sys.argv) > 4 else 6
start = int(sys.argv[5]) if len(sys.argv) > 5 else 0
stop = int(sys.argv[6]) if len(sys.argv) > 6 else 10**9
boxes = json.load(open(boxes_path))
files = {}
for f in glob.glob(os.path.join(src, "**", "*__f[0-9][0-9][0-9][0-9].png"), recursive=True):
    files.setdefault(int(re.search(r"__f(\d{4})", f).group(1)), f)
ks = [k for k in sorted(files) if k - 1 >= start and k - 1 < stop][::step]
names = list(boxes)
w = max(b[2] - b[0] for b in boxes.values()); h = max(b[3] - b[1] for b in boxes.values())
lab = 110
sheet = Image.new("RGB", (lab + w * len(ks), 16 + h * len(names)), (20, 20, 20))
d = ImageDraw.Draw(sheet)
for j, k in enumerate(ks):
    im = Image.open(files[k]).convert("RGB")
    d.text((lab + j * w + 2, 2), str(k), fill=(255, 255, 0))
    for i, n in enumerate(names):
        sheet.paste(im.crop(tuple(boxes[n])), (lab + j * w, 16 + i * h))
for i, n in enumerate(names):
    d.text((2, 16 + i * h + h // 2), n, fill=(255, 255, 255))
sheet.save(out)
print(out, sheet.size)
