import sys, glob
from PIL import Image
for p in sorted(glob.glob(sys.argv[1] + "/*__M1.png")) + sorted(glob.glob(sys.argv[1] + "/*__NM.png")):
    im = Image.open(p).convert("RGB"); w, h = im.size
    def bar(y, ch):
        n = 0
        for x in range(w):
            px = im.getpixel((x, y))
            if px[ch] > 150 and px[(ch + 1) % 3] < 90 and px[(ch + 2) % 3] < 90: n = x + 1
        return n / w * 16 - 12
    g = bar(int(h * 0.010), 1); r = bar(int(h * 0.035), 0)
    print(f"{p.split(chr(92))[-1]:45s} metered log2 {g:6.2f}  adapted {r:6.2f}")
