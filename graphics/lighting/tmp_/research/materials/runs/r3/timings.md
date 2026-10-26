# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 3.50 | 3.39 |  |  |  |  |  |  | 133.00 | OK |
| lowsun_fp | remastered | 11.32 | 10.31 | 0.04 | 0.91 | 0.26 | 0.05 | 0.22 | 0.37 | 86.30 | OK |
| lowsun_fp | delta | +7.82 | +6.92 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 11.57 | 11.47 |  |  |  |  |  |  | 46.10 | OK |
| noon_fields | remastered | 14.70 | 14.18 | 0.12 | 0.41 | 0.33 | 0.05 |  | 0.03 | 39.60 | OK |
| noon_fields | delta | +3.13 | +2.71 |  |  |  |  |  |  |  |  |
| tank_gunner_optics | classic | 11.10 | 9.78 |  |  |  |  |  |  | 74.40 | OK |
| tank_gunner_optics | remastered | 13.31 | 11.72 | 0.18 | 0.46 | 0.44 |  |  | 0.02 | 57.50 | OK |
| tank_gunner_optics | delta | +2.21 | +1.94 |  |  |  |  |  |  |  |  |
| overcast_heavy_rain | classic | 3.83 | 3.67 |  |  |  |  |  |  | 118.80 | OK |
| overcast_heavy_rain | remastered | 3.43 | 2.80 | 0.04 | 0.50 | 0.43 | 0.05 |  | 0.03 | 172.50 | OK |
| overcast_heavy_rain | delta | -0.40 | -0.87 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 7.17 | 7.06 |  |  |  |  |  |  | 89.60 | OK |
| sinai_lowsun_fp | remastered | 13.20 | 11.26 | 0.34 | 1.83 | 0.28 | 0.05 | 0.22 | 1.28 | 74.30 | OK |
| sinai_lowsun_fp | delta | +6.03 | +4.20 |  |  |  |  |  |  |  |  |
