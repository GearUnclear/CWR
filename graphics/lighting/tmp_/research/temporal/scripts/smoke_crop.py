"""Crop the town_night fire/smoke plume: does TAA smear animated particles?"""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))
import sheets as S  # noqa: E402

R = "run10-abel-r2"
v = "town_night"
ents = [("Classic", R, f"{v}__classic"), ("MSAA4 150% (current)", R, f"{v}__remastered"),
        ("TAA 150% (no MSAA)", R, f"{v}__taa15"), ("TAA+MSAA4 100%", R, f"{v}__best10msaa4"),
        ("TAA+MSAA4 100% + FX", R, f"{v}__best10msaa4fx")]
n = S.crop_sheet("final/town_night__smoke_particles.png",
                 "town_night fire + smoke particles, 2x: TAA with camera-only motion vectors and no reactive mask",
                 ents, [(560, 40, 800, 380)], 2)
print("panels", n)
