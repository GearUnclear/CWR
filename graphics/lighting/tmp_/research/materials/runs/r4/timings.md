# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 6.36 | 6.26 |  |  |  |  |  |  | 108.90 | OK |
| lowsun_fp | remastered | 17.65 | 15.72 | 0.04 | 1.82 | 0.19 | 0.05 | 0.22 | 1.36 | 61.00 | OK |
| lowsun_fp | delta | +11.29 | +9.46 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 11.31 | 11.20 |  |  |  |  |  |  | 69.30 | OK |
| noon_fields | remastered | 17.42 | 16.80 | 0.07 | 0.51 | 0.44 | 0.05 |  | 0.03 | 40.80 | OK |
| noon_fields | delta | +6.11 | +5.60 |  |  |  |  |  |  |  |  |
| tank_gunner_optics | classic | 7.86 | 6.96 |  |  |  |  |  |  | 75.20 | OK |
| tank_gunner_optics | remastered | 13.36 | 11.84 | 0.14 | 0.27 | 0.25 |  |  | 0.02 | 68.40 | OK |
| tank_gunner_optics | delta | +5.50 | +4.88 |  |  |  |  |  |  |  |  |
| overcast_heavy_rain | classic | 4.22 | 4.08 |  |  |  |  |  |  | 145.00 | OK |
| overcast_heavy_rain | remastered | 3.68 | 3.23 | 0.04 | 0.32 | 0.25 | 0.05 |  | 0.03 | 165.60 | OK |
| overcast_heavy_rain | delta | -0.54 | -0.85 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 5.83 | 5.71 |  |  |  |  |  |  | 101.60 | OK |
| sinai_lowsun_fp | remastered | 15.31 | 13.12 | 0.33 | 2.08 | 0.19 | 0.05 | 0.22 | 1.62 | 70.00 | OK |
| sinai_lowsun_fp | delta | +9.48 | +7.41 |  |  |  |  |  |  |  |  |
