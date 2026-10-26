# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.35 | 4.25 |  |  |  |  |  |  | 130.30 | OK |
| lowsun_fp | remastered | 11.36 | 9.81 | 0.04 | 1.44 | 0.18 | 0.05 | 0.22 | 0.99 | 89.00 | OK |
| lowsun_fp | delta | +7.01 | +5.56 |  |  |  |  |  |  |  |  |
| lowsun_box | classic | 8.87 | 8.76 |  |  |  |  |  |  | 199.00 | OK |
| lowsun_box | remastered | 14.59 | 14.15 | 0.04 | 0.33 | 0.25 | 0.05 |  | 0.03 | 59.20 | OK |
| lowsun_box | delta | +5.72 | +5.39 |  |  |  |  |  |  |  |  |
| dawn_box | classic | 8.91 | 8.81 |  |  |  |  |  |  | 200.90 | OK |
| dawn_box | remastered | 14.77 | 14.40 | 0.04 | 0.26 | 0.19 | 0.05 |  | 0.03 | 59.70 | OK |
| dawn_box | delta | +5.86 | +5.59 |  |  |  |  |  |  |  |  |
| dawn_box5 | classic | 5.64 | 5.54 |  |  |  |  |  |  | 188.70 | OK |
| dawn_box5 | remastered | 10.56 | 10.13 | 0.04 | 0.32 | 0.24 | 0.05 |  | 0.03 | 75.50 | OK |
| dawn_box5 | delta | +4.92 | +4.59 |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 4.94 | 4.81 |  |  |  |  |  |  | 223.80 | OK |
| forest_edge_golden | remastered | 6.93 | 6.48 | 0.03 | 0.32 | 0.24 | 0.05 |  | 0.03 | 103.60 | OK |
| forest_edge_golden | delta | +1.99 | +1.67 |  |  |  |  |  |  |  |  |
| forest_edge_noon | classic | 5.52 | 5.40 |  |  |  |  |  |  | 217.80 | OK |
| forest_edge_noon | remastered | 4.73 | 4.22 | 0.03 | 0.38 | 0.31 | 0.05 |  | 0.03 | 128.70 | OK |
| forest_edge_noon | delta | -0.79 | -1.18 |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 0.76 | 0.66 |  |  |  |  |  |  | 484.30 | OK |
| coastline_sunset | remastered | 1.37 | 0.72 | 0.00 | 0.55 | 0.31 | 0.05 | 0.16 | 0.03 | 409.40 | OK |
| coastline_sunset | delta | +0.61 | +0.06 |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 4.47 | 4.35 |  |  |  |  |  |  | 96.50 | OK |
| sunrise_ridge | remastered | 27.85 | 26.97 | 0.00 | 0.76 | 0.53 | 0.05 | 0.15 | 0.03 | 37.60 | OK |
| sunrise_ridge | delta | +23.38 | +22.62 |  |  |  |  |  |  |  |  |
| noon_fields | classic | 11.42 | 11.11 |  |  |  |  |  |  | 63.10 | OK |
| noon_fields | remastered | 13.81 | 13.32 | 0.27 | 0.37 | 0.30 | 0.05 |  | 0.03 | 61.00 | OK |
| noon_fields | delta | +2.39 | +2.21 |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 3.39 | 3.28 |  |  |  |  |  |  | 172.30 | OK |
| sinai_lowsun_fp | remastered | 7.18 | 6.01 | 0.16 | 1.05 | 0.41 | 0.05 | 0.22 | 0.37 | 121.30 | OK |
| sinai_lowsun_fp | delta | +3.79 | +2.73 |  |  |  |  |  |  |  |  |
| sinai_box | classic | 10.40 | 10.29 |  |  |  |  |  |  | 66.40 | OK |
| sinai_box | remastered | 10.87 | 10.45 | 0.05 | 0.30 | 0.22 | 0.05 |  | 0.03 | 59.70 | OK |
| sinai_box | delta | +0.47 | +0.16 |  |  |  |  |  |  |  |  |
| sinai_dusk_box | classic | 12.06 | 11.94 |  |  |  |  |  |  | 61.00 | OK |
| sinai_dusk_box | remastered | 9.90 | 9.45 | 0.05 | 0.33 | 0.25 | 0.05 |  | 0.03 | 65.30 | OK |
| sinai_dusk_box | delta | -2.16 | -2.49 |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 13.52 | 13.41 |  |  |  |  |  |  | 57.80 | OK |
| sinai_desert_noon | remastered | 12.85 | 11.96 | 0.13 | 0.62 | 0.55 | 0.05 |  | 0.03 | 53.50 | OK |
| sinai_desert_noon | delta | -0.67 | -1.45 |  |  |  |  |  |  |  |  |
