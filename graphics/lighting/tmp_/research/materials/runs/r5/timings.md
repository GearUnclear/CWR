# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 5.70 | 5.60 |  |  |  |  |  |  | 120.10 | OK |
| lowsun_fp | remastered | 18.77 | 16.69 | 0.10 | 1.98 | 0.18 | 0.05 | 0.22 | 1.53 | 54.00 | OK |
| lowsun_fp | delta | +13.07 | +11.09 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 20.42 | 20.31 |  |  |  |  |  |  | 31.70 | OK |
| noon_fields | remastered | 32.25 | 31.84 | 0.05 | 0.30 | 0.22 | 0.05 |  | 0.03 | 25.60 | OK |
| noon_fields | delta | +11.83 | +11.53 |  |  |  |  |  |  |  |  |
| abel_rain | classic | 4.59 | 4.48 |  |  |  |  |  |  | 140.20 | OK |
| abel_rain | remastered | 7.44 | 5.32 | 0.04 | 1.99 | 0.16 | 0.05 | 0.21 | 1.56 | 121.00 | OK |
| abel_rain | delta | +2.85 | +0.84 |  |  |  |  |  |  |  |  |
| tank_gunner_optics | classic | 12.70 | 10.94 |  |  |  |  |  |  | 71.70 | OK |
| tank_gunner_optics | remastered | 17.78 | 15.97 | 0.30 | 0.22 | 0.19 |  |  | 0.02 | 50.20 | OK |
| tank_gunner_optics | delta | +5.08 | +5.03 |  |  |  |  |  |  |  |  |
| overcast_heavy_rain | classic | 3.33 | 3.17 |  |  |  |  |  |  | 160.80 | OK |
| overcast_heavy_rain | remastered | 3.47 | 2.97 | 0.04 | 0.38 | 0.31 | 0.05 |  | 0.03 | 197.80 | OK |
| overcast_heavy_rain | delta | +0.14 | -0.20 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 6.66 | 6.54 |  |  |  |  |  |  | 81.40 | OK |
| sinai_lowsun_fp | remastered | 14.71 | 11.85 | 0.07 | 2.75 | 0.40 | 0.05 | 0.22 | 2.08 | 49.10 | OK |
| sinai_lowsun_fp | delta | +8.05 | +5.31 |  |  |  |  |  |  |  |  |
