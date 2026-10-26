import numpy as np
from PIL import Image
E = r"D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\B10\final2"
for v in ["broken_up", "noon_fields_oc", "overcast_broken", "overcast_heavy", "sunrise_oc", "coast_oc_up", "coast_oc", "broken_high"]:
    A = np.asarray(Image.open(rf"{E}\clouds_{v}__off.png").convert("RGB")).astype(float) / 255
    B = np.asarray(Image.open(rf"{E}\clouds_{v}__on.png").convert("RGB")).astype(float) / 255
    la = A @ [0.2126, 0.7152, 0.0722]; lb = B @ [0.2126, 0.7152, 0.0722]
    h = la.shape[0]
    sky = slice(0, h // 3)
    d = lb[sky] - la[sky]
    print(v, "sky: brighter>2lv %.3f darker>2lv %.3f  mean d %.4f  p1 %.3f p99 %.3f" % ((d > 2/255).mean(), (d < -2/255).mean(), d.mean(), np.percentile(d, 1), np.percentile(d, 99)))
