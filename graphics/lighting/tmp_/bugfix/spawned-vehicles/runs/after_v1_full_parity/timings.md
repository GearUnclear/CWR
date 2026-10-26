# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 1.95 | 1.80 |  |  |  |  |  |  | 247.00 | OK |
| lowsun_fp | remastered | 5.55 | 4.61 | 0.15 | 0.84 | 0.29 | 0.05 | 0.22 | 0.28 | 138.10 | OK |
| lowsun_fp | delta | +3.60 | +2.81 |  |  |  |  |  |  |  |  |
| hull_close | classic | 5.46 | 5.37 |  |  |  |  |  |  | 129.20 | OK |
| hull_close | remastered | 9.50 | 8.43 | 0.04 | 0.98 | 0.43 | 0.05 | 0.24 | 0.26 | 90.00 | OK |
| hull_close | delta | +4.04 | +3.06 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 6.18 | 6.08 |  |  |  |  |  |  | 79.70 | OK |
| noon_fields | remastered | 10.69 | 10.18 | 0.04 | 0.40 | 0.32 | 0.05 |  | 0.03 | 66.50 | OK |
| noon_fields | delta | +4.51 | +4.10 |  |  |  |  |  |  |  |  |
| town_night | classic | 4.81 | 4.66 |  |  |  |  |  |  | 128.20 | OK |
| town_night | remastered | 7.87 | 7.27 |  | 0.44 | 0.37 | 0.05 |  | 0.03 | 105.60 | OK |
| town_night | delta | +3.06 | +2.61 |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 6.24 | 5.69 |  |  |  |  |  |  | 114.20 | OK |
| tank_gunner_interior | remastered | 8.95 | 7.86 | 0.03 | 0.63 |  | 0.60 |  | 0.02 | 79.00 | OK |
| tank_gunner_interior | delta | +2.71 | +2.17 |  |  |  |  |  |  |  |  |
| placed_lowsun_fp | classic | 4.11 | 3.99 |  |  |  |  |  |  | 120.10 | OK |
| placed_lowsun_fp | remastered | 8.32 | 7.33 | 0.09 | 0.89 | 0.56 | 0.05 | 0.22 | 0.06 | 74.50 | OK |
| placed_lowsun_fp | delta | +4.21 | +3.34 |  |  |  |  |  |  |  |  |
| placed_hull_close | classic | 5.75 | 5.55 |  |  |  |  |  |  | 118.80 | OK |
| placed_hull_close | remastered | 6.10 | 5.08 | 0.05 | 0.92 | 0.51 | 0.06 | 0.31 | 0.04 | 108.90 | OK |
| placed_hull_close | delta | +0.35 | -0.47 |  |  |  |  |  |  |  |  |
| placed_noon_fields | classic | 7.31 | 7.21 |  |  |  |  |  |  | 83.20 | OK |
| placed_noon_fields | remastered | 10.30 | 9.64 | 0.04 | 0.56 | 0.49 | 0.05 |  | 0.03 | 59.10 | OK |
| placed_noon_fields | delta | +2.99 | +2.43 |  |  |  |  |  |  |  |  |
| placed_town_night | classic | 6.30 | 5.92 |  |  |  |  |  |  | 100.60 | OK |
| placed_town_night | remastered | 6.86 | 5.99 |  | 0.68 | 0.57 | 0.07 |  | 0.04 | 106.10 | OK |
| placed_town_night | delta | +0.56 | +0.07 |  |  |  |  |  |  |  |  |
| placed_tank_gunner_interior | classic | 6.36 | 5.91 |  |  |  |  |  |  | 117.10 | OK |
| placed_tank_gunner_interior | remastered | 6.39 | 5.51 | 0.04 | 0.61 |  | 0.58 |  | 0.03 | 92.60 | OK |
| placed_tank_gunner_interior | delta | +0.03 | -0.40 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 1.57 | 1.40 |  |  |  |  |  |  | 249.30 | OK |
| sinai_lowsun_fp | remastered | 5.50 | 3.55 | 0.12 | 1.59 | 0.96 | 0.14 | 0.35 | 0.14 | 151.20 | OK |
| sinai_lowsun_fp | delta | +3.93 | +2.15 |  |  |  |  |  |  |  |  |
| sinai_hull_close | classic | 5.54 | 5.16 |  |  |  |  |  |  | 131.40 | OK |
| sinai_hull_close | remastered | 5.09 | 3.60 | 0.05 | 1.36 | 0.66 | 0.07 | 0.32 | 0.32 | 144.80 | OK |
| sinai_hull_close | delta | -0.45 | -1.56 |  |  |  |  |  |  |  |  |
| placed_sinai_lowsun_fp | classic | 1.89 | 1.77 |  |  |  |  |  |  | 219.20 | OK |
| placed_sinai_lowsun_fp | remastered | 4.27 | 3.20 | 0.10 | 0.95 | 0.50 | 0.05 | 0.22 | 0.18 | 158.90 | OK |
| placed_sinai_lowsun_fp | delta | +2.38 | +1.43 |  |  |  |  |  |  |  |  |
| placed_sinai_hull_close | classic | 2.11 | 2.01 |  |  |  |  |  |  | 183.30 | OK |
| placed_sinai_hull_close | remastered | 4.04 | 2.89 | 0.03 | 1.05 | 0.45 | 0.05 | 0.24 | 0.31 | 145.30 | OK |
| placed_sinai_hull_close | delta | +1.93 | +0.88 |  |  |  |  |  |  |  |  |
