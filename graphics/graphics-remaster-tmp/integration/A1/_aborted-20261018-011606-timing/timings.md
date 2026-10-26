# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge_r1 | classic | 19.65 | 19.50 |  |  |  |  |  |  | 36.70 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge_r1 | remastered | 17.85 | 16.67 | 0.09 | 1.02 | 0.62 | 0.06 | 0.27 | 0.07 | 37.30 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r1 | delta | -1.80 | -2.83 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| forest_edge_golden_r1 | classic | 5.72 | 5.53 |  |  |  |  |  |  | 115.10 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden_r1 | remastered | 6.32 | 5.50 | 0.31 | 0.61 | 0.46 | 0.09 |  | 0.06 | 114.80 | 2,142 | 1,459 | OK:30 | OK |
| forest_edge_golden_r1 | delta | +0.60 | -0.03 |  |  |  |  |  |  |  | -14.4% | +0.0% |  |  |
| town_night_r1 | classic | 6.40 | 6.12 |  |  |  |  |  |  | 121.60 | 1,174 | 999 | OK:187 | OK |
| town_night_r1 | remastered | 6.17 | 5.29 |  | 0.66 | 0.53 | 0.08 |  | 0.05 | 122.10 | 1,177 | 987 | OK:60 | OK |
| town_night_r1 | delta | -0.23 | -0.83 |  |  |  |  |  |  |  | +0.3% | -1.2% |  |  |
| coastline_sunset_r1 | classic | 3.11 | 2.39 |  |  |  |  |  |  | 239.90 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r1 | remastered | 1.80 | 0.68 | 0.00 | 0.98 | 0.43 | 0.08 | 0.39 | 0.08 | 278.10 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r1 | delta | -1.31 | -1.71 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| woods_pine2_evening_r1 | classic | 6.23 | 6.08 |  |  |  |  |  |  | 108.80 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r1 | remastered | 6.58 | 5.33 | 0.36 | 1.10 | 0.46 | 0.07 | 0.50 | 0.07 | 109.20 | 2,842 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r1 | delta | +0.35 | -0.75 |  |  |  |  |  |  |  | -12.6% | +0.0% |  |  |
| lowsun_fp_r1 | classic | 4.96 | 4.76 |  |  |  |  |  |  | 129.80 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp_r1 | remastered | 4.51 | 3.45 | 0.08 | 0.92 | 0.38 | 0.07 | 0.40 | 0.07 | 146.20 | 1,890 | 1,316 | OK:30 | OK |
| lowsun_fp_r1 | delta | -0.45 | -1.31 |  |  |  |  |  |  |  | -9.6% | +0.0% |  |  |
| road_flat_drive_moonless_r1 | classic | 4.48 | 4.24 |  |  |  |  |  |  | 149.90 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r1 | remastered | 6.45 | 5.53 |  | 0.66 | 0.49 | 0.11 |  | 0.07 | 114.90 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r1 | delta | +1.97 | +1.29 |  |  |  |  |  |  |  | +0.0% | +0.0% |  |  |
| ffh_r1 | classic | 5.54 | 5.33 |  |  |  |  |  |  | 129.30 | 801 | 700 | UNSETTLED:300 | OK |
| ffh_r1 | remastered | 6.31 | 5.36 |  | 0.77 | 0.64 | 0.08 |  | 0.06 | 123.00 | 775 | 672 | UNSETTLED:300 | OK |
| ffh_r1 | delta | +0.77 | +0.03 |  |  |  |  |  |  |  | -3.2% | -4.0% |  |  |
| bay_offshore_r1 | classic | 5.66 | 5.53 |  |  |  |  |  |  | 98.90 | 4,562 | 2,961 | OK:30 | OK |
| bay_offshore_r1 | remastered | 7.22 | 6.61 | 0.03 | 0.47 | 0.37 | 0.06 |  | 0.03 | 86.50 | 4,559 | 2,961 | OK:30 | OK |
| bay_offshore_r1 | delta | +1.56 | +1.08 |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
| town_night_rain_r1 | classic | 5.14 | 4.84 |  |  |  |  |  |  | 146.50 | 880 | 631 | OK:78 | OK |
| town_night_rain_r1 | remastered | 5.56 | 4.63 |  | 0.70 | 0.55 | 0.09 |  | 0.06 | 149.50 | 895 | 634 | OK:165 | OK |
| town_night_rain_r1 | delta | +0.42 | -0.21 |  |  |  |  |  |  |  | +1.7% | +0.5% |  |  |
| pinewood_morn_r1 | classic | 6.26 | 5.99 |  |  |  |  |  |  | 102.60 | 3,163 | 1,678 | OK:30 | OK |
| pinewood_morn_r1 | remastered | 5.34 | 4.27 | 0.46 | 0.88 | 0.44 | 0.07 | 0.30 | 0.07 | 120.70 | 2,845 | 1,678 | OK:30 | OK |
| pinewood_morn_r1 | delta | -0.92 | -1.72 |  |  |  |  |  |  |  | -10.1% | +0.0% |  |  |
| valley_0515_r1 | classic | 19.94 | 19.81 |  |  |  |  |  |  | 39.30 | 11,553 | 7,966 | OK:30 | OK |
| valley_0515_r1 | remastered | 22.72 | 21.38 | 0.04 | 1.21 | 0.76 | 0.07 | 0.31 | 0.07 | 32.70 | 11,406 | 7,966 | OK:30 | OK |
| valley_0515_r1 | delta | +2.78 | +1.57 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
