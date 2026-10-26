import numpy as np
from PIL import Image, ImageFilter
d = r'D:/Arma_CWA_worktrees/graphics-remaster/tmp/integration/A7/final/'
def hp(a, r):
    im = Image.fromarray(a.astype(np.uint8))
    bl = np.asarray(im.filter(ImageFilter.BoxBlur(r))).astype(np.float64)
    return a - bl
for v in ['noon_fields','sunrise_ridge','coastline_sunset','firefight_dusk','coast_oc_up']:
    out = []
    for m in ('off','on'):
        a = np.asarray(Image.open(d+f'atmosphere_{v}__{m}.png').convert('RGB')).astype(np.float64)[0:380]
        L = 0.2126*a[...,0]+0.7152*a[...,1]+0.0722*a[...,2]
        res = []
        for r in (6, 24):
            hL = hp(np.stack([L]*3,-1), r)[...,0].std()
            hC = np.sqrt((hp(a, r)**2).sum(-1).mean())  # RGB vector magnitude of high-pass
            res.append((r, hL, hC))
        out.append(res)
    print(v, ' '.join(f'r{r}: lum {o[1]:.2f}->{n[1]:.2f} rgb {o[2]:.2f}->{n[2]:.2f}' for o, n, r in [(out[0][i], out[1][i], out[0][i][0]) for i in range(2)]))
