# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.72 | 4.61 |  |  |  |  |  |  | 141.40 | OK |
| lowsun_fp | remastered | 10.98 | 9.92 | 0.04 | 0.95 | 0.43 | 0.05 | 0.22 | 0.25 | 77.60 | OK |
| lowsun_fp | delta | +6.26 | +5.31 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 11.03 | 10.93 |  |  |  |  |  |  | 49.60 | OK |
| noon_fields | remastered | 16.41 | 15.69 | 0.19 | 0.60 | 0.52 | 0.05 |  | 0.03 | 40.40 | OK |
| noon_fields | delta | +5.38 | +4.76 |  |  |  |  |  |  |  |  |
| abel_rain | classic | 2.01 | 1.89 |  |  |  |  |  |  | 198.60 | OK |
| abel_rain | remastered | 3.11 | 2.05 | 0.04 | 0.93 | 0.38 | 0.05 | 0.21 | 0.29 | 148.50 | OK |
| abel_rain | delta | +1.10 | +0.16 |  |  |  |  |  |  |  |  |
| tank_gunner_optics | classic | 6.58 | 5.79 |  |  |  |  |  |  | 110.10 | OK |
| tank_gunner_optics | remastered | 11.19 | 10.24 | 0.04 | 0.36 | 0.33 |  |  | 0.02 | 72.60 | OK |
| tank_gunner_optics | delta | +4.61 | +4.45 |  |  |  |  |  |  |  |  |
| overcast_heavy_rain | classic | 3.16 | 3.05 |  |  |  |  |  |  | 191.50 | OK |
| overcast_heavy_rain | remastered | 3.02 | 2.16 | 0.04 | 0.74 | 0.66 | 0.05 |  | 0.03 | 187.00 | OK |
| overcast_heavy_rain | delta | -0.14 | -0.89 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 5.21 | 5.10 |  |  |  |  |  |  | 85.60 | OK |
| sinai_lowsun_fp | remastered | 13.90 | 11.83 | 0.05 | 1.95 | 0.24 | 0.05 | 0.22 | 1.43 | 67.20 | OK |
| sinai_lowsun_fp | delta | +8.69 | +6.73 |  |  |  |  |  |  |  |  |
