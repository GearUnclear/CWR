# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge_r1 | classic | 19.69 | 19.54 |  |  |  |  |  |  |  |  | 36.80 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge_r1 | remastered | 17.48 | 16.34 | 0.08 | 0.98 | 0.46 |  | 0.13 | 0.06 | 0.27 | 0.06 | 37.70 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r1 | delta | -2.21 | -3.20 |  |  |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| forest_edge_golden_r1 | classic | 5.75 | 5.55 |  |  |  |  |  |  |  |  | 114.80 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden_r1 | remastered | 5.96 | 5.18 | 0.34 | 0.57 | 0.24 |  | 0.18 | 0.09 |  | 0.06 | 115.90 | 2,142 | 1,459 | OK:30 | OK |
| forest_edge_golden_r1 | delta | +0.21 | -0.37 |  |  |  |  |  |  |  |  |  | -14.4% | +0.0% |  |  |
| town_night_r1 | classic | 6.20 | 5.99 |  |  |  |  |  |  |  |  | 127.80 | 1,301 | 1,127 | OK:232 | OK |
| town_night_r1 | remastered | 6.51 | 5.69 |  | 0.62 | 0.32 |  | 0.18 | 0.07 |  | 0.05 | 119.60 | 1,310 | 1,134 | OK:32 | OK |
| town_night_r1 | delta | +0.31 | -0.30 |  |  |  |  |  |  |  |  |  | +0.7% | +0.6% |  |  |
| coastline_sunset_r1 | classic | 1.76 | 1.48 |  |  |  |  |  |  |  |  | 302.10 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r1 | remastered | 1.99 | 0.77 | 0.00 | 1.06 | 0.31 |  | 0.15 | 0.09 | 0.41 | 0.11 | 349.90 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r1 | delta | +0.23 | -0.71 |  |  |  |  |  |  |  |  |  | +0.0% |  |  |  |
| woods_pine2_evening_r1 | classic | 5.82 | 5.66 |  |  |  |  |  |  |  |  | 112.20 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r1 | remastered | 6.58 | 5.26 | 0.37 | 1.16 | 0.30 |  | 0.20 | 0.07 | 0.49 | 0.10 | 110.50 | 2,842 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r1 | delta | +0.76 | -0.40 |  |  |  |  |  |  |  |  |  | -12.6% | +0.0% |  |  |
| lowsun_fp_r1 | classic | 4.87 | 4.66 |  |  |  |  |  |  |  |  | 130.10 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp_r1 | remastered | 5.97 | 4.67 | 0.09 | 1.12 | 0.26 |  | 0.19 | 0.09 | 0.51 | 0.08 | 122.30 | 1,890 | 1,316 | OK:30 | OK |
| lowsun_fp_r1 | delta | +1.10 | +0.01 |  |  |  |  |  |  |  |  |  | -9.6% | +0.0% |  |  |
| road_flat_drive_moonless_r1 | classic | 4.97 | 4.70 |  |  |  |  |  |  |  |  | 126.50 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r1 | remastered | 4.09 | 3.43 |  | 0.49 | 0.25 |  | 0.14 | 0.06 |  | 0.04 | 150.20 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r1 | delta | -0.88 | -1.27 |  |  |  |  |  |  |  |  |  | +0.0% | +0.0% |  |  |
| ffh_r1 | classic | 5.99 | 5.78 |  |  |  |  |  |  |  |  | 125.60 | 821 | 699 | UNSETTLED:300 | OK |
| ffh_r1 | remastered | 6.19 | 5.27 |  | 0.73 | 0.41 |  | 0.19 | 0.08 |  | 0.06 | 128.70 | 798 | 679 | UNSETTLED:300 | OK |
| ffh_r1 | delta | +0.20 | -0.51 |  |  |  |  |  |  |  |  |  | -2.8% | -2.9% |  |  |
| bay_offshore_r1 | classic | 5.93 | 5.79 |  |  |  |  |  |  |  |  | 96.30 | 4,562 | 2,961 | OK:30 | OK |
| bay_offshore_r1 | remastered | 7.63 | 7.00 | 0.03 | 0.49 | 0.26 |  | 0.13 | 0.06 |  | 0.04 | 81.50 | 4,559 | 2,961 | OK:30 | OK |
| bay_offshore_r1 | delta | +1.70 | +1.21 |  |  |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
| town_night_rain_r1 | classic | 6.46 | 6.20 |  |  |  |  |  |  |  |  | 124.30 | 1,059 | 834 | OK:236 | OK |
| town_night_rain_r1 | remastered | 6.20 | 5.35 |  | 0.65 | 0.33 |  | 0.19 | 0.08 |  | 0.05 | 135.80 | 1,041 | 821 | OK:241 | OK |
| town_night_rain_r1 | delta | -0.26 | -0.85 |  |  |  |  |  |  |  |  |  | -1.7% | -1.6% |  |  |
| pinewood_morn_r1 | classic | 6.06 | 5.83 |  |  |  |  |  |  |  |  | 108.50 | 3,163 | 1,678 | OK:30 | OK |
| pinewood_morn_r1 | remastered | 6.21 | 5.00 | 0.63 | 1.00 | 0.27 |  | 0.19 | 0.09 | 0.36 | 0.09 | 113.50 | 2,845 | 1,678 | OK:30 | OK |
| pinewood_morn_r1 | delta | +0.15 | -0.83 |  |  |  |  |  |  |  |  |  | -10.1% | +0.0% |  |  |
| valley_0515_r1 | classic | 20.89 | 20.76 |  |  |  |  |  |  |  |  | 37.70 | 11,553 | 7,966 | OK:30 | OK |
| valley_0515_r1 | remastered | 23.97 | 22.59 | 0.04 | 1.24 | 0.65 |  | 0.06 | 0.07 | 0.31 | 0.15 | 31.20 | 11,406 | 7,966 | OK:30 | OK |
| valley_0515_r1 | delta | +3.08 | +1.83 |  |  |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| hill_1730_r1 | classic | 3.68 | 3.41 |  |  |  |  |  |  |  |  | 290.20 | 1,769 | 934 | OK:30 | OK |
| hill_1730_r1 | remastered | 2.16 | 1.64 | 0.12 | 0.39 | 0.13 |  | 0.09 | 0.10 |  | 0.06 | 288.10 | 1,662 | 934 | OK:30 | OK |
| hill_1730_r1 | delta | -1.52 | -1.77 |  |  |  |  |  |  |  |  |  | -6.0% | +0.0% |  |  |
| coast_oc_up_r1 | classic | 4.16 | 2.96 |  |  |  |  |  |  |  |  | 217.60 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r1 | remastered | 1.80 | 0.59 | 0.00 | 0.99 | 0.31 |  | 0.07 | 0.07 | 0.43 | 0.11 | 297.00 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r1 | delta | -2.36 | -2.37 |  |  |  |  |  |  |  |  |  | +0.0% |  |  |  |
| sea_noon_r1 | classic | 2.57 | 2.28 |  |  |  |  |  |  |  |  | 242.30 | 179 | 0 | OK:30 | OK |
| sea_noon_r1 | remastered | 1.52 | 0.82 | 0.00 | 0.57 | 0.30 |  | 0.13 | 0.09 |  | 0.06 | 240.40 | 179 | 0 | OK:30 | OK |
| sea_noon_r1 | delta | -1.05 | -1.46 |  |  |  |  |  |  |  |  |  | +0.0% |  |  |  |
| noon_fields_r1 | classic | 8.37 | 8.24 |  |  |  |  |  |  |  |  | 72.40 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields_r1 | remastered | 9.09 | 8.52 | 0.07 | 0.43 | 0.21 |  | 0.12 | 0.06 |  | 0.04 | 70.20 | 6,398 | 4,508 | OK:30 | OK |
| noon_fields_r1 | delta | +0.72 | +0.28 |  |  |  |  |  |  |  |  |  | -5.5% | +0.0% |  |  |
| sunrise_ridge_r2 | classic | 18.45 | 18.30 |  |  |  |  |  |  |  |  | 37.60 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge_r2 | remastered | 17.67 | 16.51 | 0.08 | 1.00 | 0.47 |  | 0.13 | 0.06 | 0.27 | 0.07 | 37.30 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r2 | delta | -0.78 | -1.79 |  |  |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| forest_edge_golden_r2 | classic | 4.81 | 4.61 |  |  |  |  |  |  |  |  | 117.70 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden_r2 | remastered | 6.03 | 5.22 | 0.29 | 0.61 | 0.27 |  | 0.18 | 0.09 |  | 0.06 | 115.70 | 2,142 | 1,459 | OK:30 | OK |
| forest_edge_golden_r2 | delta | +1.22 | +0.61 |  |  |  |  |  |  |  |  |  | -14.4% | +0.0% |  |  |
| town_night_r2 | classic | 6.45 | 6.23 |  |  |  |  |  |  |  |  | 120.90 | 1,282 | 1,117 | OK:291 | OK |
| town_night_r2 | remastered | 6.57 | 5.64 |  | 0.67 | 0.38 |  | 0.18 | 0.07 |  | 0.04 | 120.20 | 1,282 | 1,116 | OK:54 | OK |
| town_night_r2 | delta | +0.12 | -0.59 |  |  |  |  |  |  |  |  |  | +0.0% | -0.1% |  |  |
| coastline_sunset_r2 | classic | 1.96 | 1.67 |  |  |  |  |  |  |  |  | 294.70 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r2 | remastered | 1.99 | 0.80 | 0.00 | 1.04 | 0.30 |  | 0.14 | 0.08 | 0.39 | 0.13 | 336.90 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r2 | delta | +0.03 | -0.87 |  |  |  |  |  |  |  |  |  | +0.0% |  |  |  |
| woods_pine2_evening_r2 | classic | 5.35 | 5.19 |  |  |  |  |  |  |  |  | 113.80 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r2 | remastered | 6.56 | 5.28 | 0.35 | 1.12 | 0.27 |  | 0.20 | 0.07 | 0.50 | 0.08 | 110.80 | 2,842 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r2 | delta | +1.21 | +0.09 |  |  |  |  |  |  |  |  |  | -12.6% | +0.0% |  |  |
| lowsun_fp_r2 | classic | 4.35 | 4.14 |  |  |  |  |  |  |  |  | 130.90 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp_r2 | remastered | 5.93 | 4.50 | 0.09 | 1.25 | 0.26 |  | 0.19 | 0.09 | 0.51 | 0.20 | 120.70 | 1,890 | 1,316 | OK:30 | OK |
| lowsun_fp_r2 | delta | +1.58 | +0.36 |  |  |  |  |  |  |  |  |  | -9.6% | +0.0% |  |  |
| road_flat_drive_moonless_r2 | classic | 5.10 | 4.86 |  |  |  |  |  |  |  |  | 131.70 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r2 | remastered | 5.99 | 5.13 |  | 0.65 | 0.30 |  | 0.20 | 0.09 |  | 0.06 | 118.00 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r2 | delta | +0.89 | +0.27 |  |  |  |  |  |  |  |  |  | +0.0% | +0.0% |  |  |
| ffh_r2 | classic | 4.84 | 4.67 |  |  |  |  |  |  |  |  | 126.20 | 818 | 696 | UNSETTLED:300 | OK |
| ffh_r2 | remastered | 6.30 | 5.29 |  | 0.79 | 0.47 |  | 0.18 | 0.08 |  | 0.06 | 125.40 | 797 | 686 | UNSETTLED:300 | OK |
| ffh_r2 | delta | +1.46 | +0.62 |  |  |  |  |  |  |  |  |  | -2.6% | -1.4% |  |  |
| bay_offshore_r2 | classic | 5.94 | 5.80 |  |  |  |  |  |  |  |  | 94.40 | 4,562 | 2,961 | OK:30 | OK |
| bay_offshore_r2 | remastered | 7.50 | 6.87 | 0.03 | 0.47 | 0.24 |  | 0.13 | 0.06 |  | 0.04 | 82.00 | 4,559 | 2,961 | OK:30 | OK |
| bay_offshore_r2 | delta | +1.56 | +1.07 |  |  |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
| town_night_rain_r2 | classic | 6.47 | 6.25 |  |  |  |  |  |  |  |  | 122.70 | 1,091 | 853 | OK:214 | OK |
| town_night_rain_r2 | remastered | 6.46 | 5.56 |  | 0.69 | 0.37 |  | 0.19 | 0.08 |  | 0.05 | 132.40 | 1,081 | 847 | OK:178 | OK |
| town_night_rain_r2 | delta | -0.01 | -0.69 |  |  |  |  |  |  |  |  |  | -0.9% | -0.7% |  |  |
| pinewood_morn_r2 | classic | 5.61 | 5.35 |  |  |  |  |  |  |  |  | 111.20 | 3,163 | 1,678 | OK:30 | OK |
| pinewood_morn_r2 | remastered | 5.89 | 4.72 | 0.58 | 0.96 | 0.24 |  | 0.19 | 0.09 | 0.36 | 0.08 | 114.90 | 2,845 | 1,678 | OK:30 | OK |
| pinewood_morn_r2 | delta | +0.28 | -0.63 |  |  |  |  |  |  |  |  |  | -10.1% | +0.0% |  |  |
| valley_0515_r2 | classic | 20.81 | 20.67 |  |  |  |  |  |  |  |  | 37.70 | 11,553 | 7,966 | OK:30 | OK |
| valley_0515_r2 | remastered | 24.01 | 22.63 | 0.04 | 1.23 | 0.69 |  | 0.06 | 0.07 | 0.31 | 0.11 | 31.30 | 11,406 | 7,966 | OK:30 | OK |
| valley_0515_r2 | delta | +3.20 | +1.96 |  |  |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| hill_1730_r2 | classic | 3.63 | 3.31 |  |  |  |  |  |  |  |  | 282.30 | 1,769 | 934 | OK:30 | OK |
| hill_1730_r2 | remastered | 1.98 | 1.41 | 0.05 | 0.44 | 0.13 |  | 0.17 | 0.09 |  | 0.06 | 284.50 | 1,662 | 934 | OK:30 | OK |
| hill_1730_r2 | delta | -1.65 | -1.90 |  |  |  |  |  |  |  |  |  | -6.0% | +0.0% |  |  |
| coast_oc_up_r2 | classic | 2.36 | 1.83 |  |  |  |  |  |  |  |  | 252.30 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r2 | remastered | 1.87 | 0.69 | 0.00 | 0.96 | 0.26 |  | 0.08 | 0.08 | 0.46 | 0.08 | 285.80 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r2 | delta | -0.49 | -1.14 |  |  |  |  |  |  |  |  |  | +0.0% |  |  |  |
| sea_noon_r2 | classic | 2.62 | 2.33 |  |  |  |  |  |  |  |  | 237.70 | 179 | 0 | OK:30 | OK |
| sea_noon_r2 | remastered | 1.60 | 0.89 | 0.00 | 0.59 | 0.30 |  | 0.14 | 0.09 |  | 0.06 | 260.20 | 179 | 0 | OK:30 | OK |
| sea_noon_r2 | delta | -1.02 | -1.44 |  |  |  |  |  |  |  |  |  | +0.0% |  |  |  |
| noon_fields_r2 | classic | 7.94 | 7.82 |  |  |  |  |  |  |  |  | 72.50 | 6,768 | 4,508 | OK:30 | OK |
| noon_fields_r2 | remastered | 9.22 | 8.61 | 0.07 | 0.46 | 0.25 |  | 0.11 | 0.06 |  | 0.04 | 70.90 | 6,399 | 4,508 | OK:30 | OK |
| noon_fields_r2 | delta | +1.28 | +0.79 |  |  |  |  |  |  |  |  |  | -5.5% | +0.0% |  |  |
| sunrise_ridge_r3 | classic | 16.51 | 16.35 |  |  |  |  |  |  |  |  | 38.20 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge_r3 | remastered | 17.69 | 16.52 | 0.09 | 1.00 | 0.45 |  | 0.13 | 0.06 | 0.27 | 0.10 | 37.50 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r3 | delta | +1.18 | +0.17 |  |  |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| forest_edge_golden_r3 | classic | 4.61 | 4.40 |  |  |  |  |  |  |  |  | 134.40 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden_r3 | remastered | 5.87 | 5.09 | 0.32 | 0.57 | 0.24 |  | 0.18 | 0.09 |  | 0.06 | 116.60 | 2,142 | 1,459 | OK:30 | OK |
| forest_edge_golden_r3 | delta | +1.26 | +0.69 |  |  |  |  |  |  |  |  |  | -14.4% | +0.0% |  |  |
| town_night_r3 | classic | 6.41 | 6.19 |  |  |  |  |  |  |  |  | 119.70 | 1,285 | 1,134 | OK:267 | OK |
| town_night_r3 | remastered | 6.49 | 5.67 |  | 0.61 | 0.31 |  | 0.18 | 0.07 |  | 0.04 | 122.90 | 1,292 | 1,128 | OK:43 | OK |
| town_night_r3 | delta | +0.08 | -0.52 |  |  |  |  |  |  |  |  |  | +0.5% | -0.5% |  |  |
| coastline_sunset_r3 | classic | 1.80 | 1.52 |  |  |  |  |  |  |  |  | 304.80 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r3 | remastered | 2.00 | 0.75 | 0.00 | 1.09 | 0.29 |  | 0.16 | 0.09 | 0.45 | 0.10 | 310.00 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r3 | delta | +0.20 | -0.77 |  |  |  |  |  |  |  |  |  | +0.0% |  |  |  |
| woods_pine2_evening_r3 | classic | 5.09 | 4.93 |  |  |  |  |  |  |  |  | 125.50 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r3 | remastered | 6.42 | 5.15 | 0.35 | 1.11 | 0.27 |  | 0.20 | 0.07 | 0.49 | 0.08 | 110.70 | 2,842 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r3 | delta | +1.33 | +0.22 |  |  |  |  |  |  |  |  |  | -12.6% | +0.0% |  |  |
| lowsun_fp_r3 | classic | 4.10 | 3.88 |  |  |  |  |  |  |  |  | 139.60 | 2,092 | 1,316 | OK:30 | OK |
| lowsun_fp_r3 | remastered | 5.99 | 4.62 | 0.09 | 1.20 | 0.28 |  | 0.19 | 0.09 | 0.50 | 0.14 | 120.20 | 1,891 | 1,316 | OK:30 | OK |
| lowsun_fp_r3 | delta | +1.89 | +0.74 |  |  |  |  |  |  |  |  |  | -9.6% | +0.0% |  |  |
| road_flat_drive_moonless_r3 | classic | 5.21 | 4.97 |  |  |  |  |  |  |  |  | 126.50 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r3 | remastered | 4.08 | 3.40 |  | 0.51 | 0.28 |  | 0.13 | 0.06 |  | 0.04 | 149.30 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r3 | delta | -1.13 | -1.57 |  |  |  |  |  |  |  |  |  | +0.0% | +0.0% |  |  |
| ffh_r3 | classic | 6.08 | 5.86 |  |  |  |  |  |  |  |  | 123.80 | 830 | 699 | UNSETTLED:300 | OK |
| ffh_r3 | remastered | 6.29 | 5.36 |  | 0.75 | 0.43 |  | 0.18 | 0.08 |  | 0.06 | 127.10 | 796 | 693 | OK:274 | OK |
| ffh_r3 | delta | +0.21 | -0.50 |  |  |  |  |  |  |  |  |  | -4.1% | -0.9% |  |  |
| bay_offshore_r3 | classic | 5.93 | 5.79 |  |  |  |  |  |  |  |  | 95.30 | 4,562 | 2,961 | OK:30 | OK |
| bay_offshore_r3 | remastered | 7.54 | 6.92 | 0.03 | 0.47 | 0.25 |  | 0.13 | 0.06 |  | 0.04 | 81.60 | 4,559 | 2,961 | OK:30 | OK |
| bay_offshore_r3 | delta | +1.61 | +1.13 |  |  |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
| town_night_rain_r3 | classic | 6.70 | 6.45 |  |  |  |  |  |  |  |  | 122.80 | 1,070 | 839 | OK:295 | OK |
| town_night_rain_r3 | remastered | 6.20 | 5.36 |  | 0.63 | 0.31 |  | 0.20 | 0.08 |  | 0.05 | 132.80 | 1,061 | 832 | OK:58 | OK |
| town_night_rain_r3 | delta | -0.50 | -1.09 |  |  |  |  |  |  |  |  |  | -0.8% | -0.8% |  |  |
| pinewood_morn_r3 | classic | 5.27 | 5.01 |  |  |  |  |  |  |  |  | 117.20 | 3,163 | 1,678 | OK:30 | OK |
| pinewood_morn_r3 | remastered | 6.17 | 4.96 | 0.59 | 0.99 | 0.27 |  | 0.19 | 0.09 | 0.36 | 0.08 | 114.50 | 2,845 | 1,678 | OK:30 | OK |
| pinewood_morn_r3 | delta | +0.90 | -0.05 |  |  |  |  |  |  |  |  |  | -10.1% | +0.0% |  |  |
| valley_0515_r3 | classic | 20.64 | 20.50 |  |  |  |  |  |  |  |  | 38.50 | 11,553 | 7,966 | OK:30 | OK |
| valley_0515_r3 | remastered | 23.89 | 22.57 | 0.04 | 1.17 | 0.65 |  | 0.06 | 0.07 | 0.31 | 0.08 | 31.40 | 11,406 | 7,966 | OK:30 | OK |
| valley_0515_r3 | delta | +3.25 | +2.07 |  |  |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| hill_1730_r3 | classic | 3.57 | 3.04 |  |  |  |  |  |  |  |  | 295.10 | 1,769 | 934 | OK:30 | OK |
| hill_1730_r3 | remastered | 1.96 | 1.45 | 0.05 | 0.38 | 0.13 |  | 0.11 | 0.09 |  | 0.06 | 281.80 | 1,662 | 934 | OK:30 | OK |
| hill_1730_r3 | delta | -1.61 | -1.59 |  |  |  |  |  |  |  |  |  | -6.0% | +0.0% |  |  |
| coast_oc_up_r3 | classic | 2.40 | 1.77 |  |  |  |  |  |  |  |  | 252.20 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r3 | remastered | 1.94 | 0.73 | 0.00 | 0.98 | 0.29 |  | 0.08 | 0.08 | 0.45 | 0.08 | 262.70 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r3 | delta | -0.46 | -1.04 |  |  |  |  |  |  |  |  |  | +0.0% |  |  |  |
| sea_noon_r3 | classic | 2.51 | 2.22 |  |  |  |  |  |  |  |  | 246.10 | 179 | 0 | OK:30 | OK |
| sea_noon_r3 | remastered | 4.30 | 2.83 | 0.00 | 1.23 | 0.38 |  | 0.39 | 0.27 |  | 0.19 | 185.00 | 179 | 0 | OK:30 | OK |
| sea_noon_r3 | delta | +1.79 | +0.61 |  |  |  |  |  |  |  |  |  | +0.0% |  |  |  |
| noon_fields_r3 | classic | 7.58 | 7.45 |  |  |  |  |  |  |  |  | 76.80 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields_r3 | remastered | 9.12 | 8.54 | 0.07 | 0.44 | 0.22 |  | 0.11 | 0.06 |  | 0.04 | 70.90 | 6,398 | 4,508 | OK:30 | OK |
| noon_fields_r3 | delta | +1.54 | +1.09 |  |  |  |  |  |  |  |  |  | -5.5% | +0.0% |  |  |
| overcast_broken_r1 | classic | 10.62 | 10.49 |  |  |  |  |  |  |  |  | 57.40 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken_r1 | remastered | 11.77 | 11.04 | 0.17 | 0.60 | 0.38 |  | 0.11 | 0.06 |  | 0.04 | 55.00 | 8,066 | 5,692 | OK:30 | OK |
| overcast_broken_r1 | delta | +1.15 | +0.55 |  |  |  |  |  |  |  |  |  | -2.1% | +0.0% |  |  |
| broken_high_r1 | classic | 17.48 | 17.37 |  |  |  |  |  |  |  |  | 43.40 | 10,789 | 7,549 | OK:30 | OK |
| broken_high_r1 | remastered | 24.58 | 23.75 | 0.03 | 0.73 | 0.57 |  | 0.06 | 0.07 |  | 0.04 | 36.30 | 10,785 | 7,549 | OK:30 | OK |
| broken_high_r1 | delta | +7.10 | +6.38 |  |  |  |  |  |  |  |  |  | -0.0% | +0.0% |  |  |
| overcast_broken_r2 | classic | 10.59 | 10.46 |  |  |  |  |  |  |  |  | 58.30 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken_r2 | remastered | 11.86 | 11.11 | 0.21 | 0.62 | 0.41 |  | 0.11 | 0.06 |  | 0.04 | 54.50 | 8,066 | 5,692 | OK:30 | OK |
| overcast_broken_r2 | delta | +1.27 | +0.65 |  |  |  |  |  |  |  |  |  | -2.1% | +0.0% |  |  |
| broken_high_r2 | classic | 17.03 | 16.92 |  |  |  |  |  |  |  |  | 44.10 | 10,789 | 7,549 | OK:30 | OK |
| broken_high_r2 | remastered | 24.47 | 23.59 | 0.03 | 0.79 | 0.62 |  | 0.06 | 0.07 |  | 0.04 | 36.20 | 10,785 | 7,549 | OK:30 | OK |
| broken_high_r2 | delta | +7.44 | +6.67 |  |  |  |  |  |  |  |  |  | -0.0% | +0.0% |  |  |
| overcast_broken_r3 | classic | 10.23 | 10.10 |  |  |  |  |  |  |  |  | 59.30 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken_r3 | remastered | 11.75 | 11.06 | 0.18 | 0.57 | 0.36 |  | 0.11 | 0.06 |  | 0.04 | 54.40 | 8,066 | 5,692 | OK:30 | OK |
| overcast_broken_r3 | delta | +1.52 | +0.96 |  |  |  |  |  |  |  |  |  | -2.1% | +0.0% |  |  |
| broken_high_r3 | classic | 17.46 | 17.35 |  |  |  |  |  |  |  |  | 43.90 | 10,789 | 7,549 | OK:30 | OK |
| broken_high_r3 | remastered | 24.65 | 23.77 | 0.03 | 0.78 | 0.62 |  | 0.06 | 0.07 |  | 0.04 | 36.00 | 10,785 | 7,549 | OK:30 | OK |
| broken_high_r3 | delta | +7.19 | +6.42 |  |  |  |  |  |  |  |  |  | -0.0% | +0.0% |  |  |
| sinai_lowsun_fp_r1 | classic | 3.36 | 3.09 |  |  |  |  |  |  |  |  | 139.70 | 788 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r1 | remastered | 5.77 | 3.25 | 0.22 | 2.22 | 0.48 |  | 0.34 | 0.24 | 0.95 | 0.21 | 127.00 | 763 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r1 | delta | +2.41 | +0.16 |  |  |  |  |  |  |  |  |  | -3.2% | +0.0% |  |  |
| sinai_desert_noon_r1 | classic | 7.19 | 7.02 |  |  |  |  |  |  |  |  | 84.40 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r1 | remastered | 8.40 | 7.36 | 0.04 | 0.87 | 0.60 |  | 0.15 | 0.07 |  | 0.05 | 76.70 | 2,672 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r1 | delta | +1.21 | +0.34 |  |  |  |  |  |  |  |  |  | -0.6% | +0.0% |  |  |
| sinai_lowsun_fp_r2 | classic | 3.38 | 3.08 |  |  |  |  |  |  |  |  | 139.20 | 789 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r2 | remastered | 4.02 | 2.76 | 0.12 | 1.10 | 0.36 |  | 0.16 | 0.07 | 0.40 | 0.10 | 141.20 | 764 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r2 | delta | +0.64 | -0.32 |  |  |  |  |  |  |  |  |  | -3.2% | +0.0% |  |  |
| sinai_desert_noon_r2 | classic | 7.36 | 7.18 |  |  |  |  |  |  |  |  | 83.60 | 2,688 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r2 | remastered | 8.51 | 7.47 | 0.04 | 0.87 | 0.61 |  | 0.14 | 0.07 |  | 0.04 | 74.90 | 2,671 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r2 | delta | +1.15 | +0.29 |  |  |  |  |  |  |  |  |  | -0.6% | +0.0% |  |  |
| sinai_lowsun_fp_r3 | classic | 3.55 | 3.25 |  |  |  |  |  |  |  |  | 142.60 | 789 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r3 | remastered | 4.16 | 2.79 | 0.12 | 1.20 | 0.41 |  | 0.16 | 0.07 | 0.41 | 0.15 | 141.20 | 764 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r3 | delta | +0.61 | -0.46 |  |  |  |  |  |  |  |  |  | -3.2% | +0.0% |  |  |
| sinai_desert_noon_r3 | classic | 7.32 | 7.14 |  |  |  |  |  |  |  |  | 83.70 | 2,688 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r3 | remastered | 8.30 | 7.27 | 0.04 | 0.86 | 0.59 |  | 0.14 | 0.07 |  | 0.05 | 77.00 | 2,671 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r3 | delta | +0.98 | +0.13 |  |  |  |  |  |  |  |  |  | -0.6% | +0.0% |  |  |
