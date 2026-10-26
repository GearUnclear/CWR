"""Round 4 sheets: frame-by-frame convergence of the 4-tap stochastic PCSS +
4-tap AO after TAA starts from an empty history, and the same effects under a
yaw pan and a strafe, with and without TAA."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))
import sheets as S  # noqa: E402

R = "run15-abel-r4"
rows = S.collect_run(R)
v = "forest_edge_golden"
SOLDIER = (500, 380, 740, 540)
TREES = (220, 150, 460, 310)
frames = [0, 1, 2, 4, 8, 16, 32, 64]
conv = [(f"{'no TAA' if k == 0 else f'TAA frame {k}'}", R, f"{v}__conv_f{k:03d}") for k in frames]
ref = [("16-tap PCSS, no TAA (ref)", R, f"{v}__r4_ref16_noTAA"), ("current (PCF, 4x MSAA 150%)", R, f"{v}__r4_remastered")]
S.grid_sheet("final/forest_edge_golden__convergence.png",
             "forest_edge_golden: 4-tap stochastic PCSS (sun 2.5 deg) + 4-tap AO, TAA + 4x MSAA 100%, frames after TAA starts from an empty history (2x crop)",
             [conv[:4] + ref[:1], conv[4:] + ref[1:]], SOLDIER, 2)
S.grid_sheet("final/forest_edge_golden__convergence_trees.png",
             "forest_edge_golden: same convergence sequence, tree crowns and trunks (2x crop)",
             [conv[:4] + ref[:1], conv[4:] + ref[1:]], TREES, 2)
S.full_sheet("final/forest_edge_golden__convergence_full.png",
             "forest_edge_golden: no TAA (4-tap effects, noisy) | TAA frame 4 | TAA frame 64 | 16-tap no-TAA reference",
             [conv[0], conv[3], conv[7], ref[0]], 2)
for tag, title in (("panfx", "yaw pan 0.1 deg per capture"), ("strafefx", "strafe 0.06 m per capture")):
    ents = [[(f"FX no TAA f{i}", R, f"{v}__{tag}_noTAA_f{i:02d}") for i in range(0, 6, 2)],
            [(f"FX + TAA f{i}", R, f"{v}__{tag}_TAA_f{i:02d}") for i in range(0, 6, 2)]]
    if tag == "strafefx":
        ents.append([(f"current MSAA4 150% f{i}", R, f"{v}__strafemsaa_f{i:02d}") for i in range(0, 6, 2)])
    S.grid_sheet(f"final/forest_edge_golden__{tag}_motion.png",
                 f"forest_edge_golden: 4-tap PCSS + 4-tap AO under camera motion ({title}), 2x soldier crop",
                 ents, SOLDIER, 2)
    S.gif(f"final/forest_edge_golden__{tag}_motion.gif",
          [("FX no TAA", R, [f"{v}__{tag}_noTAA_f{i:02d}" for i in range(6)]),
           ("FX + TAA", R, [f"{v}__{tag}_TAA_f{i:02d}" for i in range(6)])], SOLDIER, 2)
print("rows", len(rows))
