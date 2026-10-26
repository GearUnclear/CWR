"""fx_report.py <run dir> <out prefix>: per-shot sim dt from the UDCAP step lines,
a contact sheet of every 10th frame (fire, smoke, flare, shell burst), and the
head-jump count for the three men (boxes found from frame 1 by the caller)."""
import sys, glob, re, statistics
from PIL import Image, ImageDraw
d, outp = sys.argv[1], sys.argv[2]
log = glob.glob(f"{d}/tri/*/game_stdout.log")
dts = []
if log:
    for l in open(log[0], encoding="utf-8", errors="replace"):
        m = re.search(r"event=step;k=(\d+);dt=([-\d.e]+);time=([-\d.e]+);acc=([-\d.e]+);shell=([^;]*);shellpos=([^;]*);uaz=(.*)$", l)
        if m:
            dts.append((int(m.group(1)), float(m.group(2)), float(m.group(3)), m.group(4), m.group(5), m.group(6), m.group(7).strip()))
with open(outp + "_steps.txt", "w") as fo:
    if dts:
        v = [x[1] for x in dts]
        s = f"shots {len(v)}: sim dt per shot min {min(v):.4f} median {statistics.median(v):.4f} max {max(v):.4f} (1/60 = 0.0167)"
        print(s); fo.write(s + "\n")
        for x in dts:
            fo.write(f"k={x[0]} dt={x[1]:.4f} time={x[2]:.3f} acc={x[3]} shell={x[4]} shellpos={x[5]} uaz={x[6]}\n")
    else:
        print("no step lines in", log)
fs = sorted(glob.glob(f"{d}/tri/*/*.png"))
pick = fs[::10] + ([fs[-1]] if fs and (len(fs) - 1) % 10 else [])
W, H = 480, 270
cols = 4
sheet = Image.new("RGB", (W * cols, (H + 16) * ((len(pick) + cols - 1) // cols)), "white")
dr = ImageDraw.Draw(sheet)
for i, f in enumerate(pick):
    im = Image.open(f).convert("RGB").resize((W, H), Image.LANCZOS)
    x, y = (i % cols) * W, (i // cols) * (H + 16)
    sheet.paste(im, (x, y + 16)); dr.text((x + 4, y + 2), re.search(r"f\d+", f).group(0), fill="black")
sheet.save(outp + "_sheet.png")
print(len(fs), "frames; sheet", outp + "_sheet.png")
