"""Sharpen / render-scale sweep of TAA without MSAA on the Sinai camo net and
M113 (existing run05 shots), 2x crops."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))
import sheets as S  # noqa: E402

R = "run05-sinai2"
v = "sinai_lowsun_fp"
ents = [("current MSAA4 150%", R, f"{v}__remastered"), ("TAA 150% sharpen 0", R, f"{v}__taa15_s0"),
        ("TAA 150% sharpen 0.5", R, f"{v}__taa15_s50"), ("TAA 150% sharpen 0.8", R, f"{v}__taa15_s80"),
        ("TAA 125% sharpen 0.5", R, f"{v}__taa125_s50"), ("TAA 100% sharpen 0.5", R, f"{v}__taa10_s50")]
n = S.crop_sheet("final/sinai_lowsun_fp__sharpen_sweep.png",
                 "sinai_lowsun_fp: TAA (no MSAA) output sharpening and render scale on the camo net and M113, 2x",
                 ents, [(380, 150, 620, 310), (590, 150, 830, 310)], 2)
print(n)
