import sys, re
from pathlib import Path
from PIL import Image, ImageStat
def shots(run):
    d = {}
    for p in sorted((Path(run) / "tri").rglob("*.png")):
        m = re.match(r"^\d{3}_(.+)\.png$", p.name)
        if m: d[m.group(1)] = p
    return d
runs = sys.argv[1:]
names = [Path(r).name for r in runs]
S = [shots(r) for r in runs]
labels = sorted(set().union(*[set(s) for s in S]))
print("| label | " + " | ".join(f"{n} mean / lower-half mean (0-255 luma)" for n in names) + " |")
print("|---|" + "---|" * len(runs))
for l in labels:
    if "dbg" in l: continue
    row = []
    for s in S:
        if l not in s: row.append(""); continue
        im = Image.open(s[l]).convert("L")
        w, h = im.size
        a = ImageStat.Stat(im).mean[0]
        b = ImageStat.Stat(im.crop((0, h // 2, w, h))).mean[0]
        row.append(f"{a:.1f} / {b:.1f}")
    print(f"| {l} | " + " | ".join(row) + " |")
