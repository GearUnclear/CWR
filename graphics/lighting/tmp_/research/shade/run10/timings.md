# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| forest_edge_golden | classic | 5.26 | 5.13 |  |  |  |  |  |  | 224.50 | OK |
| forest_edge_golden | remastered | 8.36 | 7.91 | 0.03 | 0.32 | 0.24 | 0.05 |  | 0.03 | 81.60 | OK |
| forest_edge_golden | delta | +3.10 | +2.78 |  |  |  |  |  |  |  |  |
| forest_edge_noon | classic | 8.43 | 8.31 |  |  |  |  |  |  | 225.30 | OK |
| forest_edge_noon | remastered | 12.87 | 12.49 | 0.03 | 0.26 | 0.18 | 0.05 |  | 0.03 | 61.70 | OK |
| forest_edge_noon | delta | +4.44 | +4.18 |  |  |  |  |  |  |  |  |
| lowsun_box | classic | 2.72 | 2.62 |  |  |  |  |  |  | 198.90 | OK |
| lowsun_box | remastered | 10.71 | 10.21 | 0.04 | 0.39 | 0.31 | 0.05 |  | 0.03 | 90.70 | OK |
| lowsun_box | delta | +7.99 | +7.59 |  |  |  |  |  |  |  |  |
| dawn_box | classic | 13.74 | 13.64 |  |  |  |  |  |  | 194.70 | OK |
| dawn_box | remastered | 21.04 | 20.67 | 0.05 | 0.26 | 0.18 | 0.05 |  | 0.03 | 45.90 | OK |
| dawn_box | delta | +7.30 | +7.03 |  |  |  |  |  |  |  |  |
| dawn_box5 | classic | 5.75 | 5.65 |  |  |  |  |  |  | 190.60 | OK |
| dawn_box5 | remastered | 8.60 | 8.09 | 0.15 | 0.40 | 0.33 | 0.05 |  | 0.03 | 82.70 | OK |
| dawn_box5 | delta | +2.85 | +2.44 |  |  |  |  |  |  |  |  |
| sinai_dusk_box | classic | 5.33 | 5.21 |  |  |  |  |  |  | 104.70 | OK |
| sinai_dusk_box | remastered | 10.64 | 10.02 | 0.05 | 0.50 | 0.42 | 0.05 |  | 0.03 | 68.30 | OK |
| sinai_dusk_box | delta | +5.31 | +4.81 |  |  |  |  |  |  |  |  |
| sinai_box | classic | 8.70 | 8.59 |  |  |  |  |  |  | 77.20 | OK |
| sinai_box | remastered | 17.27 | 16.88 | 0.51 | 0.27 | 0.19 | 0.05 |  | 0.03 | 50.30 | OK |
| sinai_box | delta | +8.57 | +8.29 |  |  |  |  |  |  |  |  |
| lowsun_fp | classic | 3.26 | 3.15 |  |  |  |  |  |  | 162.80 | OK |
| lowsun_fp | remastered | 4.27 | 3.52 | 0.07 | 0.63 | 0.34 | 0.05 | 0.22 | 0.03 | 134.70 | OK |
| lowsun_fp | delta | +1.01 | +0.37 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 15.29 | 15.19 |  |  |  |  |  |  | 44.30 | OK |
| noon_fields | remastered | 28.24 | 27.84 | 0.07 | 0.24 | 0.16 | 0.05 |  | 0.03 | 32.00 | OK |
| noon_fields | delta | +12.95 | +12.65 |  |  |  |  |  |  |  |  |
