import sys
from PIL import Image, ImageChops
def cnt(a, b):
    A = Image.open(a).convert("RGB"); B = Image.open(b).convert("RGB")
    d = ImageChops.difference(A, B)
    r, g, bb = d.split()
    m = ImageChops.lighter(ImageChops.lighter(r, g), bb)
    h = m.histogram()
    over2 = sum(h[3:])
    mx = max(i for i, v in enumerate(h) if v)
    return over2, mx
base = "D:/Arma_CWA_worktrees/graphics-remaster/tmp/integration/B5/"
B1 = base + "20261018-071820-before/"; B2 = base + "20261018-073656-before/"; N = base + "20261018-075851final-new/"
for v in ("water_coastline_sunset", "water_sea_noon", "water_shore_midsun", "water_night"):
    print(v,
          "B5off vs before2.off", cnt(N+v+"__off.png", B2+v+"__off.png"),
          "| before1.off vs before2.off", cnt(B1+v+"__off.png", B2+v+"__off.png"),
          "| before2 off vs on (same game)", cnt(B2+v+"__off.png", B2+v+"__on.png"),
          "| classic B5 vs before2", cnt(N+v+"__classic.png", B2+v+"__classic.png"))
