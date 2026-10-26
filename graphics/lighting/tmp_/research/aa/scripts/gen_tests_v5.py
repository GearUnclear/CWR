"""v5 for track aa: present-pass cost only (the scratch build now also logs the
minimum present time over each 90-frame window). Three ABAB rounds per
window size; forest_edge_golden at H (4x MSAA, 1.5x)."""
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import gen_tests as g  # noqa: E402
import gen_tests_v4 as v4  # noqa: E402

OUT = Path(sys.argv[1]) if len(sys.argv) > 1 else v4.OUT
OUT.mkdir(parents=True, exist_ok=True)
abel = "tests/rendering/captures/missions/udcap_abel.Abel"
for name, w, h in (("aa_cost5_1080", 1920, 1080), ("aa_cost5_960", 960, 640)):
    v4.write(name, "Abel", 4674, 10156, v4.block("forest_edge_golden", *v4.FOREST, ["a", "b", "c"], [], []),
             abel, "", w, h, 1500)
print("wrote", OUT)
