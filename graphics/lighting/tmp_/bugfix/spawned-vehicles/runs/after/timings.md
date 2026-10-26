# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 3.15 | 2.86 |  |  |  |  |  |  | 181.90 | OK |
| lowsun_fp | remastered | 6.93 | 5.52 | 0.10 | 1.16 | 0.60 | 0.11 | 0.36 | 0.10 | 112.20 | OK |
| lowsun_fp | delta | +3.78 | +2.66 |  |  |  |  |  |  |  |  |
| hull_close | classic | 5.11 | 4.94 |  |  |  |  |  |  | 129.60 | OK |
| hull_close | remastered | 6.65 | 5.66 | 0.06 | 0.87 | 0.41 | 0.07 | 0.34 | 0.05 | 109.20 | OK |
| hull_close | delta | +1.54 | +0.72 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 5.78 | 5.68 |  |  |  |  |  |  | 88.60 | OK |
| noon_fields | remastered | 8.35 | 7.88 | 0.04 | 0.37 | 0.30 | 0.05 |  | 0.03 | 79.20 | OK |
| noon_fields | delta | +2.57 | +2.20 |  |  |  |  |  |  |  |  |
| town_night | classic | 5.48 | 5.25 |  |  |  |  |  |  | 134.00 | OK |
| town_night | remastered | 4.84 | 4.21 |  | 0.48 | 0.40 | 0.05 |  | 0.03 | 138.80 | OK |
| town_night | delta | -0.64 | -1.04 |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 3.91 | 3.68 |  |  |  |  |  |  | 146.20 | OK |
| tank_gunner_interior | remastered | 6.75 | 6.10 | 0.03 | 0.41 |  | 0.39 |  | 0.03 | 107.00 | OK |
| tank_gunner_interior | delta | +2.84 | +2.42 |  |  |  |  |  |  |  |  |
| placed_lowsun_fp | classic | 1.43 | 1.31 |  |  |  |  |  |  | 254.20 | OK |
| placed_lowsun_fp | remastered | 3.82 | 3.08 | 0.08 | 0.65 | 0.34 | 0.05 | 0.22 | 0.04 | 165.90 | OK |
| placed_lowsun_fp | delta | +2.39 | +1.77 |  |  |  |  |  |  |  |  |
| placed_hull_close | classic | 5.61 | 5.42 |  |  |  |  |  |  | 128.70 | OK |
| placed_hull_close | remastered | 6.60 | 5.59 | 0.06 | 0.90 | 0.45 | 0.07 | 0.33 | 0.05 | 106.90 | OK |
| placed_hull_close | delta | +0.99 | +0.17 |  |  |  |  |  |  |  |  |
| placed_noon_fields | classic | 5.45 | 5.35 |  |  |  |  |  |  | 96.10 | OK |
| placed_noon_fields | remastered | 7.69 | 7.19 | 0.04 | 0.41 | 0.34 | 0.05 |  | 0.03 | 81.50 | OK |
| placed_noon_fields | delta | +2.24 | +1.84 |  |  |  |  |  |  |  |  |
| placed_town_night | classic | 6.34 | 5.94 |  |  |  |  |  |  | 128.30 | OK |
| placed_town_night | remastered | 6.32 | 5.58 |  | 0.54 | 0.43 | 0.07 |  | 0.04 | 118.40 | OK |
| placed_town_night | delta | -0.02 | -0.36 |  |  |  |  |  |  |  |  |
| placed_tank_gunner_interior | classic | 4.79 | 4.46 |  |  |  |  |  |  | 127.00 | OK |
| placed_tank_gunner_interior | remastered | 6.84 | 6.13 | 0.05 | 0.41 |  | 0.38 |  | 0.03 | 102.50 | OK |
| placed_tank_gunner_interior | delta | +2.05 | +1.67 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 3.38 | 2.93 |  |  |  |  |  |  | 164.10 | OK |
| sinai_lowsun_fp | remastered | 5.86 | 3.65 | 0.20 | 1.88 | 0.84 | 0.16 | 0.72 | 0.16 | 126.80 | OK |
| sinai_lowsun_fp | delta | +2.48 | +0.72 |  |  |  |  |  |  |  |  |
| sinai_hull_close | classic | 3.93 | 3.62 |  |  |  |  |  |  | 140.50 | OK |
| sinai_hull_close | remastered | 6.32 | 4.10 | 0.12 | 1.92 | 0.84 | 0.19 | 0.75 | 0.13 | 114.50 | OK |
| sinai_hull_close | delta | +2.39 | +0.48 |  |  |  |  |  |  |  |  |
| placed_sinai_lowsun_fp | classic | 1.70 | 1.59 |  |  |  |  |  |  | 250.10 | OK |
| placed_sinai_lowsun_fp | remastered | 5.70 | 3.56 | 0.21 | 1.68 | 0.83 | 0.17 | 0.50 | 0.18 | 156.30 | OK |
| placed_sinai_lowsun_fp | delta | +4.00 | +1.97 |  |  |  |  |  |  |  |  |
| placed_sinai_hull_close | classic | 2.82 | 2.62 |  |  |  |  |  |  | 175.70 | OK |
| placed_sinai_hull_close | remastered | 4.17 | 2.94 | 0.05 | 1.09 | 0.56 | 0.07 | 0.35 | 0.11 | 134.70 | OK |
| placed_sinai_hull_close | delta | +1.35 | +0.32 |  |  |  |  |  |  |  |  |
