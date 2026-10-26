"""Moving-Jeep ghosting as a grid (rows: 4x MSAA, TAA + depth rejection,
TAA without rejection; columns: frames 1, 3, 5). Reads the existing shots."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))
import sheets as S  # noqa: E402

for v, run, box in (("sinai_lowsun_fp", "run07-sinai-r2", (560, 220, 920, 390)),
                    ("forest_edge_golden", "run10-abel-r2", (300, 300, 760, 520))):
    rows = []
    for t, n in (("msaa", "4x MSAA 150%"), ("taa", "TAA + depth rejection"), ("taa_norej", "TAA, no rejection")):
        rows.append([(f"{n} f{i}", run, f"{v}__ghost{t}_f{i:02d}") for i in (1, 3, 5)])
    k = S.grid_sheet(f"final/{v}__ghosting_grid.png",
                     f"{v}: Jeep moving 0.12 m per frame, camera-only motion vectors, 2x (rows: MSAA, TAA + depth rejection, TAA no rejection)",
                     rows, box, 2)
    print(v, k)
