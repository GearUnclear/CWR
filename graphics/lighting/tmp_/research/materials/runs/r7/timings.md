# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 1.98 | 1.88 |  |  |  |  |  |  | 227.00 | OK |
| lowsun_fp | remastered | 9.05 | 7.91 | 0.04 | 1.03 | 0.27 | 0.05 | 0.22 | 0.49 | 108.60 | OK |
| lowsun_fp | delta | +7.07 | +6.03 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 19.59 | 19.49 |  |  |  |  |  |  | 42.10 | OK |
| noon_fields | remastered | 27.76 | 27.41 | 0.24 | 0.24 | 0.16 | 0.05 |  | 0.03 | 29.60 | OK |
| noon_fields | delta | +8.17 | +7.92 |  |  |  |  |  |  |  |  |
| abel_rain | classic | 1.86 | 1.75 |  |  |  |  |  |  | 280.80 | OK |
| abel_rain | remastered | 3.96 | 2.78 | 0.04 | 1.06 | 0.22 | 0.05 | 0.21 | 0.58 | 209.80 | OK |
| abel_rain | delta | +2.10 | +1.03 |  |  |  |  |  |  |  |  |
| town_night | classic | 4.19 | 4.05 |  |  |  |  |  |  | 158.70 | OK |
| town_night | remastered | 8.55 | 7.89 |  | 0.49 | 0.42 | 0.05 |  | 0.03 | 96.70 | OK |
| town_night | delta | +4.36 | +3.84 |  |  |  |  |  |  |  |  |
| town_night_rain | classic | 11.53 | 11.41 |  |  |  |  |  |  | 71.60 | OK |
| town_night_rain | remastered | 11.92 | 11.44 |  | 0.35 | 0.27 | 0.05 |  | 0.03 | 77.70 | OK |
| town_night_rain | delta | +0.39 | +0.03 |  |  |  |  |  |  |  |  |
| tank_gunner_optics | classic | 10.49 | 9.37 |  |  |  |  |  |  | 88.40 | OK |
| tank_gunner_optics | remastered | 9.24 | 8.39 | 0.04 | 0.23 | 0.21 |  |  | 0.02 | 86.50 | OK |
| tank_gunner_optics | delta | -1.25 | -0.98 |  |  |  |  |  |  |  |  |
| overcast_heavy_rain | classic | 1.49 | 1.36 |  |  |  |  |  |  | 259.60 | OK |
| overcast_heavy_rain | remastered | 1.56 | 0.80 | 0.04 | 0.65 | 0.57 | 0.05 |  | 0.03 | 289.50 | OK |
| overcast_heavy_rain | delta | +0.07 | -0.56 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 2.31 | 2.20 |  |  |  |  |  |  | 136.20 | OK |
| sinai_lowsun_fp | remastered | 6.42 | 5.17 | 0.15 | 1.12 | 0.51 | 0.05 | 0.22 | 0.34 | 107.10 | OK |
| sinai_lowsun_fp | delta | +4.11 | +2.97 |  |  |  |  |  |  |  |  |
