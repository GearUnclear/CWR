"""Spot check that the footage in each thumbnail is only cropped and scaled.

For every footage region (ROI in thumbnail pixels, chosen away from type and
overlays) the source frame is rescaled and registered by a small brute-force
search over scale and offset; then the signed per-channel bias and the median
absolute difference are reported. A grade or recolour shows as a bias.
"""
import numpy as np
from PIL import Image

T = "D:/Arma_CWA_worktrees/graphics-remaster/tmp/video/v2/thumbs/"
F = T + "judge/frames/"

def load(p):
    return np.asarray(Image.open(p).convert("RGB")).astype(np.float32)

# name, thumb, src frame, approx scale, expected (dx, dy) so that T[y,x] ~ R[y+dy, x+dx], ROI (x0,y0,x1,y1)
CHECKS = [
    ("1 split1 vanilla", "1/split1_dawn_ridge.png", "valley_mist_vanilla_200.png", 1280/1920, (0, 0), (20, 220, 300, 700)),
    ("1 split1 ud",      "1/split1_dawn_ridge.png", "valley_mist_ud_200.png",      1280/1920, (0, 0), (700, 200, 1260, 700)),
    ("1 split2 vanilla", "1/split2_night_road.png", "supply_run_vanilla_400.png",  1280/1360, (round(284*1280/1360), round(200*1280/1360)), (20, 180, 600, 700)),
    ("1 split2 ud",      "1/split2_night_road.png", "supply_run_ud_400.png",       1280/1360, (round(284*1280/1360), round(200*1280/1360)), (680, 200, 1260, 700)),
    ("1 split3 vanilla", "1/split3_bay.png",        "bay_reflections_vanilla_300.png", 1280/1920, (0, 0), (20, 160, 320, 700)),
    ("1 split3 ud",      "1/split3_bay.png",        "bay_reflections_ud_300.png",  1280/1920, (0, 0), (700, 200, 1260, 700)),
    ("2a footage",       "2/2a_poster_beside_outpost.png", "outpost_ud_420.png",   720/900, (round(680*0.8) - 506, round(40*0.8)), (520, 20, 1260, 620)),
    ("2b footage",       "2/2b_poster_frame_outpost.png",  "outpost_ud_510.png",   1280/1920, (0, 0), (560, 330, 1240, 630)),
    ("2c vanilla",       "2/2c_poster_column_split.png",   "valley_mist_vanilla_450.png", 672/1080, (round(187*672/1080) - 24, -24), (40, 120, 280, 680)),
    ("2c ud",            "2/2c_poster_column_split.png",   "valley_mist_ud_450.png", 672/1080, (round(187*672/1080) - 24, -24), (450, 40, 970, 680)),
    ("3 v1",             "3/v1_outpost_sun.png",    "outpost_ud_428.png",          1280/1680, (round(120*1280/1680), round(40*1280/1680)), (20, 20, 1260, 480)),
    ("3 v2",             "3/v2_dawn_ridge.png",     "valley_mist_ud_360.png",      1280/1620, (round(300*1280/1620), 0), (300, 260, 1260, 700)),
    ("3 v3",             "3/v3_town_fire.png",      "town_patrol_ud_370.png",      1280/1920, (0, 0), (20, 20, 1260, 480)),
    ("4 v2",             "4/v2_fighter_outpost.png", "outpost_ud_300.png",         0.69, (round(100*0.69) - 20, -20), (360, 380, 1240, 680)),
    ("4 v1",             "4/v1_poster_plate.png",   "valley_mist_ud_350.png",      0.74, (round(700*0.74) - 360, round(70*0.74) - 22), (380, 40, 1240, 680)),
    ("4 v3 vanilla",     "4/v3_poster_split.png",   "outpost_vanilla_400.png",     0.72, (round(400*0.72) - 360, round(60*0.72) - 22), (380, 40, 670, 640)),
    ("4 v3 ud",          "4/v3_poster_split.png",   "outpost_ud_400.png",          0.72, (round(400*0.72) - 360, round(60*0.72) - 22), (710, 40, 1240, 640)),
]

def register(th, src, s0, d0, roi, srange=0.03, sstep=0.005, orad=24):
    x0, y0, x1, y1 = roi
    tg = th[y0:y1, x0:x1].mean(axis=2)
    best = None
    for s in np.arange(s0 * (1 - srange), s0 * (1 + srange) + 1e-9, s0 * sstep):
        W, H = int(round(src.shape[1] * s)), int(round(src.shape[0] * s))
        R = np.asarray(Image.fromarray(src.astype(np.uint8)).resize((W, H), Image.LANCZOS)).astype(np.float32)
        Rg = R.mean(axis=2)
        # expected offset scales with s
        ex = d0[0] * s / s0
        ey = d0[1] * s / s0
        for dy in range(int(ey) - orad, int(ey) + orad + 1, 2):
            for dx in range(int(ex) - orad, int(ex) + orad + 1, 2):
                ya, yb, xa, xb = y0 + dy, y1 + dy, x0 + dx, x1 + dx
                if ya < 0 or xa < 0 or yb > H or xb > W:
                    continue
                e = np.median(np.abs(Rg[ya:yb:3, xa:xb:3] - tg[::3, ::3]))
                if best is None or e < best[0]:
                    best = (e, s, dx, dy, R)
    # refine at 1 px
    e, s, dx0, dy0, R = best
    Rg = R.mean(axis=2)
    for dy in range(dy0 - 2, dy0 + 3):
        for dx in range(dx0 - 2, dx0 + 3):
            ya, yb, xa, xb = y0 + dy, y1 + dy, x0 + dx, x1 + dx
            if ya < 0 or xa < 0 or yb > R.shape[0] or xb > R.shape[1]:
                continue
            e2 = np.median(np.abs(Rg[ya:yb, xa:xb] - tg))
            if e2 < e:
                e, dx0, dy0 = e2, dx, dy
    return s, dx0, dy0, R

if __name__ == "__main__":
    out = []
    for name, tp, sp, s0, d0, roi in CHECKS:
        th = load(T + tp)
        src = load(F + sp)
        s, dx, dy, R = register(th, src, s0, d0, roi)
        x0, y0, x1, y1 = roi
        a = th[y0:y1, x0:x1]
        b = R[y0 + dy:y1 + dy, x0 + dx:x1 + dx]
        diff = a - b
        bias = diff.reshape(-1, 3).mean(axis=0)
        mad = np.median(np.abs(diff))
        big = (np.abs(diff).max(axis=2) > 16).mean() * 100
        line = (f"{name:18s} scale {s:.4f} off ({dx:4d},{dy:4d})  bias RGB {bias[0]:+5.2f} {bias[1]:+5.2f} {bias[2]:+5.2f}"
                f"  median|d| {mad:4.2f}  px>16: {big:5.2f}%")
        print(line, flush=True)
        out.append(line)
    open(T + "judge/grade_check.txt", "w").write("\n".join(out) + "\n")
