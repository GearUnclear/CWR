import numpy as np
from PIL import Image
P = r"D:\Arma_CWA\tmp\lobo-wound-recolor\png"
parts = ["back","legsbk","front","armle","armri","legsfr","legslft","legsrgt"]
def load(name):
    return np.asarray(Image.open(f"{P}\\{name}.png").convert("RGB")).astype(np.float32)
def down(a, size):
    im = Image.fromarray(a.astype(np.uint8))
    return np.asarray(im.resize(size, Image.BOX)).astype(np.float32)
for p in parts:
    d = load(f"rjaf_tex_{p}_d")
    for fac in ["eaf_tex","iaf_tex2","rjaf_tex"]:
        dd = load(f"{fac}_{p}_d")
        assert np.array_equal(d, dd), (fac, p)
    h = load(f"rjaf_tex_{p}")
    hd = down(h, (d.shape[1], d.shape[0]))
    diff = np.abs(hd - d).sum(axis=2)
    red = d[...,0] - np.maximum(d[...,1], d[...,2])
    nonred = red < 8
    q = np.percentile(diff[nonred], [50, 90, 99, 99.9])
    print(f"{p:8s} d={d.shape[1]}x{d.shape[0]} healthy={h.shape[1]}x{h.shape[0]}  diff(non-red px) p50/90/99/99.9={q}  red>8: {100*(~nonred).mean():.1f}%  red>30: {100*(red>30).mean():.1f}%")
    # olive stats of base
    print("   base mean", hd[nonred].mean(axis=0).round(1), " d mean nonred", d[nonred].mean(axis=0).round(1))
