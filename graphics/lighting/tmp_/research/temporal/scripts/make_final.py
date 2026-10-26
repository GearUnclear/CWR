"""Final comparison sheets of the temporal track (reads the collected runs).

    python make_final.py

Writes into ..\final\ (and BEST_<view>.png into the research root)."""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import sheets as S  # noqa: E402

R1A, R1S, R1E = "run02-abel", "run03-sinai", "run04-eden"
R2S, R2E = "run07-sinai-r2", "run08-eden-r2"
R5S = "run05-sinai2"
R3S = "run09-sinai-r3"
R2A = "run10-abel-r2"
R3E = "run11-eden-r3"

for r in (R1A, R1S, R1E, R2S, R2E, R5S, R3S, R2A, R3E):
    if (S.ROOT / r / "tri").exists():
        S.collect_run(r)

OUT = "final"
(S.ROOT / OUT).mkdir(exist_ok=True)

VIEWS = {
    "forest_edge_golden": R2A,
    "town_night": R2A,
    "sinai_lowsun_fp": R3S,
    "overcast_broken": R3E,
}
CROPS = {
    "forest_edge_golden": [(690, 30, 930, 190), (220, 150, 460, 310), (500, 380, 740, 540)],
    "town_night": [(330, 20, 570, 180), (150, 170, 390, 330), (560, 260, 800, 420)],
    "sinai_lowsun_fp": [(380, 150, 620, 310), (590, 110, 830, 270), (560, 380, 800, 540)],
    "overcast_broken": [(600, 280, 840, 440), (110, 370, 350, 530), (60, 260, 300, 420)],
}

# 1. BEST: Classic | Remastered (current) | Remastered + TAA + 4x MSAA at 100% + stochastic PCSS + AO
for v, run in VIEWS.items():
    classic_run = run if S.shot(run, f"{v}__classic") is not None else {"sinai_lowsun_fp": R2S, "overcast_broken": R2E}.get(v, run)
    n = S.full_sheet(f"BEST_{v}.png",
                     f"{v}: Classic | Remastered (current: 4x MSAA, 150%) | Remastered + TAA prototype (TAA + 4x MSAA at 100%, stochastic contact-hardening shadows, 4-tap AO)",
                     [("Classic (4x MSAA, 150%)", classic_run, f"{v}__classic"),
                      ("Remastered (4x MSAA, 150%)", run, f"{v}__remastered"),
                      ("+ TAA (4x MSAA, 100%) + PCSS + AO", run, f"{v}__best10msaa4fx")])
    print("BEST", v, n)
    # AA-only three-way at the same settings, no effects
    S.full_sheet(f"{OUT}/{v}__3way_aa.png", f"{v}: Classic | Remastered (4x MSAA, 150%) | Remastered + TAA + 4x MSAA at 100% (AA only)",
                 [("Classic (4x MSAA, 150%)", classic_run, f"{v}__classic"),
                  ("Remastered (4x MSAA, 150%)", run, f"{v}__remastered"),
                  ("Remastered + TAA (4x MSAA, 100%)", run, f"{v}__best10msaa4")])

# 2. AA detail crops: current vs TAA configurations
for v in ("forest_edge_golden", "town_night"):
    S.crop_sheet(f"{OUT}/{v}__aa_crops.png", f"{v}: AA detail, 2x nearest (columns: current, TAA no MSAA 150%, TAA no MSAA 100%, TAA + 4x MSAA 100%, no AA 100%)",
                 [("MSAA4 150% (current)", R2A, f"{v}__remastered"), ("TAA 150%", R2A, f"{v}__best15"),
                  ("TAA 100%", R2A, f"{v}__best10"), ("TAA+MSAA4 100%", R2A, f"{v}__best10msaa4"),
                  ("no AA 100%", R1A, f"{v}__noaa10")], CROPS[v])
S.crop_sheet(f"{OUT}/sinai_lowsun_fp__aa_crops.png", "sinai_lowsun_fp: AA detail, 2x nearest",
             [("MSAA4 150% (current)", R3S, "sinai_lowsun_fp__remastered"), ("TAA 150%", R2S, "sinai_lowsun_fp__best15"),
              ("TAA 100%", R2S, "sinai_lowsun_fp__best10"), ("TAA+MSAA4 100%", R3S, "sinai_lowsun_fp__best10msaa4"),
              ("no AA 100%", R1S, "sinai_lowsun_fp__noaa10")], CROPS["sinai_lowsun_fp"])
S.crop_sheet(f"{OUT}/overcast_broken__aa_crops.png", "overcast_broken: AA detail, 2x nearest",
             [("MSAA4 150% (current)", R3E, "overcast_broken__remastered"), ("TAA 150%", R2E, "overcast_broken__best15"),
              ("TAA 100%", R2E, "overcast_broken__best10"), ("TAA+MSAA4 100%", R3E, "overcast_broken__best10msaa4"),
              ("no AA 100%", R1E, "overcast_broken__noaa10")], CROPS["overcast_broken"])

# 3. Thin features (antennas): TAA alone erodes them, TAA + MSAA keeps them
S.crop_sheet(f"{OUT}/thin_features_antennas.png", "M113 antennas (sub-pixel wide), 3x nearest: TAA without MSAA erodes them; TAA + 4x MSAA keeps them",
             [("MSAA4 150% (current)", R3S, "sinai_lowsun_fp__remastered"), ("no AA 100%", R1S, "sinai_lowsun_fp__noaa10"),
              ("TAA 150%", R2S, "sinai_lowsun_fp__best15"), ("TAA 150% clip 1.5", R3S, "sinai_lowsun_fp__best15_g15"),
              ("TAA+MSAA2 100%", R3S, "sinai_lowsun_fp__best10msaa2"), ("TAA+MSAA4 100%", R3S, "sinai_lowsun_fp__best10msaa4")],
             [(590, 110, 730, 250)], 3)

# 4. Foliage alpha: A2C (MSAA) vs alpha test under TAA vs stochastic alpha under TAA
S.crop_sheet(f"{OUT}/forest_edge_golden__foliage_alpha.png",
             "Foliage cutouts, 2x: MSAA alpha-to-coverage | TAA alpha test | TAA stochastic alpha (A2C emulation) | TAA + mip bias -0.5 | TAA best | TAA + MSAA4 100%",
             [("MSAA4 A2C 150%", R2A, "forest_edge_golden__remastered"), ("TAA alpha test", R2A, "forest_edge_golden__taa15"),
              ("TAA stochastic alpha", R2A, "forest_edge_golden__taa15_sa"), ("TAA mip -0.5", R2A, "forest_edge_golden__taa15_mip"),
              ("TAA best 150%", R2A, "forest_edge_golden__best15"), ("TAA+MSAA4 100%", R2A, "forest_edge_golden__best10msaa4")],
             CROPS["forest_edge_golden"][:2])

# 5. Stochastic soft shadows: noise without TAA, converged with TAA; sun size
S.crop_sheet(f"{OUT}/sinai_lowsun_fp__pcss_converge.png",
             "Stochastic contact-hardening sun shadow (PCSS), 2x: current 12-tap PCF | 2-tap stochastic, no TAA (noisy) | 2-tap + TAA (converged) | 12-tap PCSS, no TAA",
             [("current 12-tap PCF", R5S, "sinai_lowsun_fp__remastered"), ("2 taps 5 deg, no TAA", R5S, "sinai_lowsun_fp__pcss_sun5_t2_noTAA"),
              ("2 taps 5 deg + TAA", R5S, "sinai_lowsun_fp__pcss_sun5_t2_TAA"), ("4 taps 2.5 deg, no TAA", R1S, "sinai_lowsun_fp__shadowStoch_noTAA"),
              ("4 taps 2.5 deg + TAA", R1S, "sinai_lowsun_fp__shadowStoch_TAA"), ("12-tap PCSS 2.5, no TAA", R1S, "sinai_lowsun_fp__shadowPcss12_noTAA")],
             [(560, 380, 800, 540), (0, 330, 240, 490)])
S.crop_sheet(f"{OUT}/sinai_lowsun_fp__pcss_sunsize.png",
             "Apparent sun size of the contact-hardening shadow (all 4 taps + TAA), 2x: the long low-sun leg and hull shadows soften with distance from the caster",
             [("current PCF", R5S, "sinai_lowsun_fp__remastered"), ("sun 1 deg", R5S, "sinai_lowsun_fp__pcss_sun1_TAA"),
              ("sun 2.5 deg", R1S, "sinai_lowsun_fp__shadowStoch_TAA"), ("sun 5 deg", R5S, "sinai_lowsun_fp__pcss_sun5_TAA")],
             [(560, 380, 860, 560), (0, 330, 300, 510)])
for v, run in (("forest_edge_golden", R1A), ("overcast_broken", R1E)):
    S.crop_sheet(f"{OUT}/{v}__pcss_converge.png", f"{v}: stochastic PCSS (4 taps, sun 2.5 deg), 2x",
                 [("current PCF", run, f"{v}__remastered"), ("4 taps, no TAA", run, f"{v}__shadowStoch_noTAA"),
                  ("4 taps + TAA", run, f"{v}__shadowStoch_TAA"), ("12-tap PCSS, no TAA", run, f"{v}__shadowPcss12_noTAA")],
                 CROPS[v])

# 6. AO convergence (debug view: the AO buffer itself)
for v in ("forest_edge_golden", "town_night"):
    S.full_sheet(f"{OUT}/{v}__ao_converge.png",
                 f"{v}: ambient occlusion buffer (debug view 4): current 12 taps + bilateral blur | 4 taps r2.5 m, no blur, no TAA | the same + TAA",
                 [("current AO (12 taps + blur)", R2A, f"{v}__aodbg_current"), ("4-tap AO r2.5m, no TAA", R2A, f"{v}__aodbg_stoch_noTAA"),
                  ("4-tap AO r2.5m + TAA", R2A, f"{v}__aodbg_stoch_TAA")])
    S.crop_sheet(f"{OUT}/{v}__ao_converge_crops.png", f"{v}: AO buffer, 2x crops",
                 [("current AO", R2A, f"{v}__aodbg_current"), ("4-tap, no TAA", R2A, f"{v}__aodbg_stoch_noTAA"),
                  ("4-tap + TAA", R2A, f"{v}__aodbg_stoch_TAA")], CROPS[v][:2])

# 7. Ghosting: a Jeep teleported 0.12 m per frame (no per-object motion vectors)
for v, run, box in (("sinai_lowsun_fp", R2S, (560, 220, 920, 390)), ("forest_edge_golden", R2A, (300, 300, 760, 520))):
    ents = []
    for t, n in (("msaa", "4x MSAA"), ("taa", "TAA + depth rejection"), ("taa_norej", "TAA, no rejection")):
        for i in (1, 3, 5):
            ents.append((f"{n} f{i}", run, f"{v}__ghost{t}_f{i:02d}"))
    S.crop_sheet(f"{OUT}/{v}__ghosting.png", f"{v}: moving Jeep (0.12 m per frame, camera-only motion vectors), 2x", ents, [box], 2)
    S.gif(f"{OUT}/{v}__ghosting.gif", [("4x MSAA", run, [f"{v}__ghostmsaa_f{i:02d}" for i in range(6)]),
                                       ("TAA", run, [f"{v}__ghosttaa_f{i:02d}" for i in range(6)])], box, 1.5)

# 8. Pans: current vs TAA + MSAA4 at 100%
PANS = {"forest_edge_golden": (R2A, "panbest10msaa4"), "town_night": (R2A, "panbest10msaa4"),
        "sinai_lowsun_fp": (R3S, "panbest10msaa4"), "overcast_broken": (R3E, "panbest10msaa4")}
for v, (run, tag) in PANS.items():
    msaa_run = run if S.shot(run, f"{v}__panmsaa_f00") is not None else {"overcast_broken": R2E}.get(v, run)
    box = CROPS[v][0]
    ents = [(f"MSAA4 150% f{i}", msaa_run, f"{v}__panmsaa_f{i:02d}") for i in range(6)]
    ents += [(f"TAA+MSAA4 100% f{i}", run, f"{v}__{tag}_f{i:02d}") for i in range(6)]
    S.grid_sheet(f"{OUT}/{v}__pan_strip.png", f"{v}: slow yaw pan (0.1 deg between captures), 2x crop; top row current (4x MSAA 150%), bottom row TAA + 4x MSAA at 100%",
                 [ents[:6], ents[6:]], box, 2)
    S.gif(f"{OUT}/{v}__pan_crop.gif", [("MSAA4 150%", msaa_run, [f"{v}__panmsaa_f{i:02d}" for i in range(6)]),
                                       ("TAA+MSAA4 100%", run, [f"{v}__{tag}_f{i:02d}" for i in range(6)])], box, 2)
print("done")
