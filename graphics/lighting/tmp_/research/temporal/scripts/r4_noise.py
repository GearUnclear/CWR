"""High-frequency energy in the soldier-shadow region (mean |img - blur(r=1.5)|
of luma, 0-1 scale) for the round-4 convergence and motion sequences. Grain
from 4-tap stochastic effects raises it; converged soft shadow lowers it.
Ground texture detail contributes to every number, so compare, don't read
absolute values."""
import json
import sys
from pathlib import Path
import numpy as np
from PIL import ImageFilter
sys.path.insert(0, str(Path(__file__).parent))
import sheets as S  # noqa: E402

R = "run15-abel-r4"
v = "forest_edge_golden"
BOX = (570, 400, 700, 500)  # the soldier's shadow on the slope


def hf(label):
    im = S.shot(R, label)
    if im is None:
        return None
    c = im.crop(BOX).convert("L")
    a = np.asarray(c, float) / 255.0
    b = np.asarray(c.filter(ImageFilter.GaussianBlur(1.5)), float) / 255.0
    return round(float(np.abs(a - b).mean()), 5)


out = {"box": BOX, "convergence": {}, "motion": {}}
for k in (0, 1, 2, 4, 8, 16, 32, 64):
    out["convergence"][f"f{k:03d}"] = hf(f"{v}__conv_f{k:03d}")
out["convergence"]["ref16_noTAA"] = hf(f"{v}__r4_ref16_noTAA")
out["convergence"]["current_remastered"] = hf(f"{v}__r4_remastered")
for tag in ("panfx_noTAA", "panfx_TAA", "strafefx_noTAA", "strafefx_TAA", "strafemsaa"):
    vals = [hf(f"{v}__{tag}_f{i:02d}") for i in range(6)]
    out["motion"][tag] = {"mean": round(float(np.mean(vals)), 5), "per_frame": vals}
(S.ROOT / "final" / "r4_noise.json").write_text(json.dumps(out, indent=1), encoding="utf-8")
print(json.dumps(out, indent=1))
