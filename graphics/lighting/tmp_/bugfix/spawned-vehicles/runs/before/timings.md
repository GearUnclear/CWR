# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 3.06 | 2.95 |  |  |  |  |  |  | 211.20 | OK |
| lowsun_fp | remastered | 7.30 | 6.41 | 0.04 | 0.79 | 0.25 | 0.05 | 0.22 | 0.27 | 127.50 | OK |
| lowsun_fp | delta | +4.24 | +3.46 |  |  |  |  |  |  |  |  |
| hull_close | classic | 7.07 | 6.98 |  |  |  |  |  |  | 102.70 | OK |
| hull_close | remastered | 9.11 | 8.18 | 0.04 | 0.84 | 0.31 | 0.05 | 0.24 | 0.25 | 85.10 | OK |
| hull_close | delta | +2.04 | +1.20 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 8.99 | 8.89 |  |  |  |  |  |  | 70.00 | OK |
| noon_fields | remastered | 10.60 | 10.02 | 0.05 | 0.47 | 0.39 | 0.05 |  | 0.03 | 49.20 | OK |
| noon_fields | delta | +1.61 | +1.13 |  |  |  |  |  |  |  |  |
| town_night | classic | 4.37 | 4.22 |  |  |  |  |  |  | 145.40 | OK |
| town_night | remastered | 8.45 | 8.02 |  | 0.27 | 0.19 | 0.05 |  | 0.03 | 112.60 | OK |
| town_night | delta | +4.08 | +3.80 |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 4.60 | 4.26 |  |  |  |  |  |  | 110.90 | OK |
| tank_gunner_interior | remastered | 7.81 | 6.94 | 0.03 | 0.31 |  | 0.29 |  | 0.02 | 82.50 | OK |
| tank_gunner_interior | delta | +3.21 | +2.68 |  |  |  |  |  |  |  |  |
| placed_lowsun_fp | classic | 1.40 | 1.30 |  |  |  |  |  |  | 239.10 | OK |
| placed_lowsun_fp | remastered | 5.99 | 5.10 | 0.18 | 0.78 | 0.25 | 0.05 | 0.22 | 0.27 | 160.20 | OK |
| placed_lowsun_fp | delta | +4.59 | +3.80 |  |  |  |  |  |  |  |  |
| placed_hull_close | classic | 3.17 | 3.07 |  |  |  |  |  |  | 154.50 | OK |
| placed_hull_close | remastered | 5.58 | 4.64 | 0.04 | 0.85 | 0.52 | 0.05 | 0.24 | 0.04 | 111.90 | OK |
| placed_hull_close | delta | +2.41 | +1.57 |  |  |  |  |  |  |  |  |
| placed_noon_fields | classic | 6.24 | 6.14 |  |  |  |  |  |  | 88.30 | OK |
| placed_noon_fields | remastered | 10.08 | 9.55 | 0.04 | 0.43 | 0.35 | 0.05 |  | 0.03 | 69.30 | OK |
| placed_noon_fields | delta | +3.84 | +3.41 |  |  |  |  |  |  |  |  |
| placed_town_night | classic | 4.42 | 4.27 |  |  |  |  |  |  | 150.80 | OK |
| placed_town_night | remastered | 6.61 | 5.89 |  | 0.56 | 0.48 | 0.05 |  | 0.03 | 128.90 | OK |
| placed_town_night | delta | +2.19 | +1.62 |  |  |  |  |  |  |  |  |
| placed_tank_gunner_interior | classic | 4.18 | 3.91 |  |  |  |  |  |  | 98.50 | OK |
| placed_tank_gunner_interior | remastered | 6.52 | 5.87 | 0.03 | 0.37 |  | 0.35 |  | 0.02 | 109.00 | OK |
| placed_tank_gunner_interior | delta | +2.34 | +1.96 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 2.55 | 2.37 |  |  |  |  |  |  | 151.20 | OK |
| sinai_lowsun_fp | remastered | 3.45 | 2.47 | 0.06 | 0.87 | 0.54 | 0.05 | 0.24 | 0.04 | 145.30 | OK |
| sinai_lowsun_fp | delta | +0.90 | +0.10 |  |  |  |  |  |  |  |  |
| sinai_hull_close | classic | 4.41 | 4.24 |  |  |  |  |  |  | 139.20 | OK |
| sinai_hull_close | remastered | 3.07 | 2.12 | 0.04 | 0.84 | 0.50 | 0.05 | 0.24 | 0.05 | 133.90 | OK |
| sinai_hull_close | delta | -1.34 | -2.12 |  |  |  |  |  |  |  |  |
| placed_sinai_lowsun_fp | classic | 1.55 | 1.43 |  |  |  |  |  |  | 246.10 | OK |
| placed_sinai_lowsun_fp | remastered | 3.66 | 2.67 | 0.05 | 0.87 | 0.50 | 0.05 | 0.22 | 0.10 | 193.50 | OK |
| placed_sinai_lowsun_fp | delta | +2.11 | +1.24 |  |  |  |  |  |  |  |  |
| placed_sinai_hull_close | classic | 2.66 | 2.50 |  |  |  |  |  |  | 157.90 | OK |
| placed_sinai_hull_close | remastered | 2.85 | 1.83 | 0.04 | 0.91 | 0.55 | 0.05 | 0.27 | 0.04 | 151.80 | OK |
| placed_sinai_hull_close | delta | +0.19 | -0.67 |  |  |  |  |  |  |  |  |
