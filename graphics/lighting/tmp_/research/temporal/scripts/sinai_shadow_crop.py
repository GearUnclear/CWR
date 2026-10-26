"""Sinai low sun: the long HMMWV and soldier shadows (the earlier crop boxes
missed most of them). Reads existing shots; 2x crops."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent))
import sheets as S  # noqa: E402

v = "sinai_lowsun_fp"
R1, R5 = "run03-sinai", "run05-sinai2"
ents = [("current 12-tap PCF", R5, f"{v}__remastered"),
        ("4 taps 2.5 deg, no TAA", R1, f"{v}__shadowStoch_noTAA"),
        ("4 taps 2.5 deg + TAA", R1, f"{v}__shadowStoch_TAA"),
        ("12-tap PCSS 2.5, no TAA", R1, f"{v}__shadowPcss12_noTAA"),
        ("2 taps 5 deg, no TAA", R5, f"{v}__pcss_sun5_t2_noTAA"),
        ("2 taps 5 deg + TAA", R5, f"{v}__pcss_sun5_t2_TAA")]
n = S.crop_sheet("final/sinai_lowsun_fp__pcss_longshadow.png",
                 "sinai_lowsun_fp: long low-sun HMMWV + soldier shadows, 2x: stochastic contact-hardening shadow, noisy alone, converged under TAA",
                 ents[:3], [(0, 330, 330, 560), (500, 360, 830, 590)], 2)
n2 = S.crop_sheet("final/sinai_lowsun_fp__pcss_longshadow_b.png",
                  "sinai_lowsun_fp: same crops, 12-tap PCSS reference and the 2-tap / 5 degree extreme",
                  ents[3:], [(0, 330, 330, 560), (500, 360, 830, 590)], 2)
print(n, n2)
