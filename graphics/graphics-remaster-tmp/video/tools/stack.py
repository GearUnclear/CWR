"""Stack the check frames of a clip (check/<clip>_<t>.png) vertically."""
import sys
from pathlib import Path
from PIL import Image
d = Path(r"D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\check")
for clip in sys.argv[1:]:
    fs = sorted(d.glob(f"{clip}_*.png"), key=lambda p: float(p.stem.rsplit("_", 1)[1]))
    ims = [Image.open(p).convert("RGB").resize((1600, 450)) for p in fs]
    out = Image.new("RGB", (1600, 450 * len(ims) + 6 * (len(ims) - 1)), (255, 0, 255))
    for i, im in enumerate(ims):
        out.paste(im, (0, i * 456))
    out.save(d / f"stack_{clip}.png")
    print(d / f"stack_{clip}.png", [p.name for p in fs])
