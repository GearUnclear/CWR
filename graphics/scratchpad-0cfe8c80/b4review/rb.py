import sys
import numpy as np
from PIL import Image
d = sys.argv[1]
# view, (x0, y0, x1, y1) ground region without sprites
regions = {
    "combat_shell_k08": (0, 660, 1920, 1080),
    "combat_shell_k06": (0, 700, 1920, 1080),
    "combat_eye_k06": (0, 650, 1920, 1080),
    "combat_eye_k08": (0, 600, 1920, 1080),
    "combat_fires": (150, 560, 700, 700),
    "combat_muzzle_k06": (0, 600, 1920, 1080),
    "combat_flare_k240": (0, 600, 1920, 1080),
}
for v, (x0, y0, x1, y1) in regions.items():
    out = [v]
    for m in ("classic", "off", "on"):
        a = np.asarray(Image.open(f"{d}/{v}__{m}.png").convert("RGB")).astype(np.float64)[y0:y1, x0:x1]
        lum = 0.2126 * a[..., 0] + 0.7152 * a[..., 1] + 0.0722 * a[..., 2]
        sel = lum > 25
        if sel.sum() < 100:
            out.append(f"{m}: lum {lum.mean():5.1f} (few lit)"); continue
        r, g, b = (a[..., i][sel].mean() for i in range(3))
        out.append(f"{m}: lum {lum.mean():5.1f} lit {sel.mean():.2f} R/B {r/b:.2f} RGB {r:.0f},{g:.0f},{b:.0f}")
    print(" | ".join(out))
