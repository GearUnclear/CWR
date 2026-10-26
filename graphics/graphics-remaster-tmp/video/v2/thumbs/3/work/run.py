import sys, json
sys.path.insert(0, "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2/thumbs/3/work")
from compose import compose, save_all

VARIANTS = {
    "v1_outpost_sun": dict(
        frame="outpost_ud_0428.png", crop=(120, 40, 1680),
        brand_px=116, label_px=36, align="center", x=640, y=522,
        rule_gap=20, label_gap=18, rule_len=96, rule_th=5,
        brand_track=0.15, brand_bold=1.8,
    ),
    "alt_firefight_flash": dict(
        frame="firefight_ud_0322.png", crop=(240, 30, 1440),
        brand_px=116, label_px=36, align="center", x=640, y=40,
        rule_gap=20, label_gap=18, rule_len=96, rule_th=5,
        brand_track=0.15, brand_bold=1.8,
    ),
    "v3_town_fire": dict(
        frame="town_patrol_ud_0370.png", crop=(0, 0, 1920),
        brand_px=116, label_px=36, align="left", x=60, y=516,
        rule_gap=20, label_gap=18, rule_len=96, rule_th=5,
        brand_track=0.15, brand_bold=1.8,
    ),
    "v2_dawn_ridge": dict(
        frame="valley_mist_ud_0360.png", crop=(300, 0, 1620),
        brand_px=116, label_px=36, align="left", x=60, y=56,
        rule_gap=20, label_gap=18, rule_len=96, rule_th=5,
        brand_track=0.15, brand_bold=1.8,
        shadow=(12, 0.55, 2),
    ),
}

if __name__ == "__main__":
    names = sys.argv[1:] or list(VARIANTS)
    for n in names:
        im = compose(VARIANTS[n])
        png, jpg, q = save_all(n, im)
        print(n, png, jpg, "q", q)
