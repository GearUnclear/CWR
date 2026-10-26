# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge_r1 | classic | 19.55 | 19.40 |  |  |  |  |  |  | 37.00 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge_r1 | remastered | 17.41 | 16.26 | 0.09 | 1.01 | 0.60 | 0.06 | 0.27 | 0.07 | 37.60 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r1 | delta | -2.14 | -3.14 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| forest_edge_golden_r1 | classic | 5.54 | 5.35 |  |  |  |  |  |  | 115.10 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden_r1 | remastered | 5.58 | 4.84 | 0.27 | 0.55 | 0.42 | 0.08 |  | 0.05 | 119.30 | 2,142 | 1,459 | OK:30 | OK |
| forest_edge_golden_r1 | delta | +0.04 | -0.51 |  |  |  |  |  |  |  | -14.4% | +0.0% |  |  |
| town_night_r1 | classic | 6.56 | 6.32 |  |  |  |  |  |  | 119.70 | 1,241 | 1,077 | OK:242 | OK |
| town_night_r1 | remastered | 6.67 | 5.81 |  | 0.63 | 0.50 | 0.08 |  | 0.05 | 119.70 | 1,251 | 1,070 | OK:36 | OK |
| town_night_r1 | delta | +0.11 | -0.51 |  |  |  |  |  |  |  | +0.8% | -0.6% |  |  |
| coastline_sunset_r1 | classic | 2.14 | 1.83 |  |  |  |  |  |  | 279.00 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r1 | remastered | 3.81 | 1.60 | 0.00 | 1.95 | 0.62 | 0.21 | 0.90 | 0.23 | 244.00 | 178 | 0 | OK:30 | OK |
| coastline_sunset_r1 | delta | +1.67 | -0.23 |  |  |  |  |  |  |  | +0.6% |  |  |  |
| woods_pine2_evening_r1 | classic | 6.13 | 5.98 |  |  |  |  |  |  | 109.00 | 3,249 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r1 | remastered | 6.47 | 5.18 | 0.35 | 1.14 | 0.50 | 0.07 | 0.49 | 0.08 | 108.10 | 2,841 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r1 | delta | +0.34 | -0.80 |  |  |  |  |  |  |  | -12.6% | +0.0% |  |  |
| lowsun_fp_r1 | classic | 4.83 | 4.63 |  |  |  |  |  |  | 126.80 | 2,092 | 1,316 | OK:30 | OK |
| lowsun_fp_r1 | remastered | 5.59 | 4.34 | 0.09 | 1.08 | 0.44 | 0.08 | 0.47 | 0.09 | 120.40 | 1,891 | 1,316 | OK:30 | OK |
| lowsun_fp_r1 | delta | +0.76 | -0.29 |  |  |  |  |  |  |  | -9.6% | +0.0% |  |  |
| road_flat_drive_moonless_r1 | classic | 4.88 | 4.62 |  |  |  |  |  |  | 129.00 | 1,807 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r1 | remastered | 5.27 | 4.46 |  | 0.59 | 0.46 | 0.08 |  | 0.06 | 130.00 | 1,807 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r1 | delta | +0.39 | -0.16 |  |  |  |  |  |  |  | +0.0% | +0.0% |  |  |
| ffh_r1 | classic | 5.44 | 5.24 |  |  |  |  |  |  | 128.30 | 817 | 699 | UNSETTLED:300 | OK |
| ffh_r1 | remastered | 6.06 | 5.22 |  | 0.66 | 0.52 | 0.08 |  | 0.06 | 130.20 | 796 | 679 | UNSETTLED:300 | OK |
| ffh_r1 | delta | +0.62 | -0.02 |  |  |  |  |  |  |  | -2.6% | -2.9% |  |  |
| bay_offshore_r1 | classic | 5.78 | 5.66 |  |  |  |  |  |  | 97.90 | 4,560 | 2,961 | OK:30 | OK |
| bay_offshore_r1 | remastered | 7.72 | 7.10 | 0.03 | 0.49 | 0.39 | 0.06 |  | 0.04 | 83.40 | 4,557 | 2,961 | OK:30 | OK |
| bay_offshore_r1 | delta | +1.94 | +1.44 |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
| town_night_rain_r1 | classic | 5.01 | 4.72 |  |  |  |  |  |  | 145.40 | 871 | 616 | OK:93 | OK |
| town_night_rain_r1 | remastered | 5.23 | 4.32 |  | 0.70 | 0.56 | 0.09 |  | 0.06 | 160.80 | 871 | 620 | OK:243 | OK |
| town_night_rain_r1 | delta | +0.22 | -0.40 |  |  |  |  |  |  |  | +0.0% | +0.6% |  |  |
| pinewood_morn_r1 | classic | 6.03 | 5.81 |  |  |  |  |  |  | 107.40 | 3,163 | 1,678 | OK:30 | OK |
| pinewood_morn_r1 | remastered | 5.84 | 4.67 | 0.57 | 0.95 | 0.43 | 0.08 | 0.35 | 0.08 | 116.50 | 2,845 | 1,678 | OK:30 | OK |
| pinewood_morn_r1 | delta | -0.19 | -1.14 |  |  |  |  |  |  |  | -10.1% | +0.0% |  |  |
| valley_0515_r1 | classic | 20.30 | 20.17 |  |  |  |  |  |  | 38.80 | 11,552 | 7,966 | OK:30 | OK |
| valley_0515_r1 | remastered | 23.30 | 21.99 | 0.04 | 1.18 | 0.72 | 0.07 | 0.31 | 0.08 | 32.20 | 11,405 | 7,966 | OK:30 | OK |
| valley_0515_r1 | delta | +3.00 | +1.82 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| hill_1730_r1 | classic | 4.18 | 3.86 |  |  |  |  |  |  | 283.30 | 1,769 | 934 | OK:30 | OK |
| hill_1730_r1 | remastered | 1.99 | 1.49 | 0.05 | 0.38 | 0.22 | 0.10 |  | 0.06 | 294.20 | 1,662 | 934 | OK:30 | OK |
| hill_1730_r1 | delta | -2.19 | -2.37 |  |  |  |  |  |  |  | -6.0% | +0.0% |  |  |
| coast_oc_up_r1 | classic | 3.58 | 2.33 |  |  |  |  |  |  | 231.50 | 185 | 0 | OK:30 | OK |
| coast_oc_up_r1 | remastered | 1.73 | 0.61 | 0.00 | 0.93 | 0.36 | 0.07 | 0.42 | 0.07 | 342.00 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r1 | delta | -1.85 | -1.72 |  |  |  |  |  |  |  | +0.5% |  |  |  |
| sea_noon_r1 | classic | 3.09 | 2.82 |  |  |  |  |  |  | 239.10 | 180 | 0 | OK:30 | OK |
| sea_noon_r1 | remastered | 4.56 | 3.01 | 0.00 | 1.31 | 0.80 | 0.30 |  | 0.20 | 184.70 | 181 | 0 | OK:30 | OK |
| sea_noon_r1 | delta | +1.47 | +0.19 |  |  |  |  |  |  |  | +0.6% |  |  |  |
| noon_fields_r1 | classic | 8.40 | 8.26 |  |  |  |  |  |  | 73.40 | 6,768 | 4,508 | OK:30 | OK |
| noon_fields_r1 | remastered | 9.06 | 8.46 | 0.08 | 0.45 | 0.35 | 0.06 |  | 0.04 | 71.30 | 6,399 | 4,508 | OK:30 | OK |
| noon_fields_r1 | delta | +0.66 | +0.20 |  |  |  |  |  |  |  | -5.5% | +0.0% |  |  |
| sunrise_ridge_r2 | classic | 18.10 | 17.95 |  |  |  |  |  |  | 38.30 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge_r2 | remastered | 17.34 | 16.25 | 0.09 | 0.94 | 0.56 | 0.06 | 0.27 | 0.05 | 37.90 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r2 | delta | -0.76 | -1.70 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| forest_edge_golden_r2 | classic | 5.17 | 4.91 |  |  |  |  |  |  | 115.60 | 2,501 | 1,459 | OK:30 | OK |
| forest_edge_golden_r2 | remastered | 6.18 | 5.40 | 0.31 | 0.58 | 0.41 | 0.11 |  | 0.06 | 114.80 | 2,141 | 1,459 | OK:30 | OK |
| forest_edge_golden_r2 | delta | +1.01 | +0.49 |  |  |  |  |  |  |  | -14.4% | +0.0% |  |  |
| town_night_r2 | classic | 6.48 | 6.24 |  |  |  |  |  |  | 122.70 | 1,247 | 1,070 | OK:205 | OK |
| town_night_r2 | remastered | 6.53 | 5.64 |  | 0.67 | 0.52 | 0.09 |  | 0.05 | 122.60 | 1,228 | 1,051 | OK:44 | OK |
| town_night_r2 | delta | +0.05 | -0.60 |  |  |  |  |  |  |  | -1.5% | -1.8% |  |  |
| coastline_sunset_r2 | classic | 2.12 | 1.81 |  |  |  |  |  |  | 277.60 | 178 | 0 | OK:30 | OK |
| coastline_sunset_r2 | remastered | 1.88 | 0.71 | 0.00 | 1.03 | 0.45 | 0.08 | 0.40 | 0.11 | 288.20 | 178 | 0 | OK:30 | OK |
| coastline_sunset_r2 | delta | -0.24 | -1.10 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| woods_pine2_evening_r2 | classic | 5.52 | 5.38 |  |  |  |  |  |  | 114.30 | 3,248 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r2 | remastered | 6.45 | 5.21 | 0.35 | 1.10 | 0.46 | 0.07 | 0.50 | 0.07 | 111.00 | 2,840 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r2 | delta | +0.93 | -0.17 |  |  |  |  |  |  |  | -12.6% | +0.0% |  |  |
| lowsun_fp_r2 | classic | 4.29 | 4.08 |  |  |  |  |  |  | 132.40 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp_r2 | remastered | 6.23 | 4.85 | 0.12 | 1.19 | 0.48 | 0.09 | 0.53 | 0.09 | 118.90 | 1,890 | 1,316 | OK:30 | OK |
| lowsun_fp_r2 | delta | +1.94 | +0.77 |  |  |  |  |  |  |  | -9.6% | +0.0% |  |  |
| road_flat_drive_moonless_r2 | classic | 5.22 | 4.98 |  |  |  |  |  |  | 123.00 | 1,805 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r2 | remastered | 5.56 | 4.77 |  | 0.59 | 0.46 | 0.08 |  | 0.05 | 115.30 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r2 | delta | +0.34 | -0.21 |  |  |  |  |  |  |  | +0.1% | +0.0% |  |  |
| ffh_r2 | classic | 5.29 | 5.12 |  |  |  |  |  |  | 129.80 | 813 | 698 | UNSETTLED:300 | OK |
| ffh_r2 | remastered | 5.66 | 4.81 |  | 0.69 | 0.56 | 0.08 |  | 0.05 | 129.40 | 776 | 678 | UNSETTLED:300 | OK |
| ffh_r2 | delta | +0.37 | -0.31 |  |  |  |  |  |  |  | -4.6% | -2.9% |  |  |
| bay_offshore_r2 | classic | 6.05 | 5.93 |  |  |  |  |  |  | 92.40 | 4,560 | 2,961 | OK:30 | OK |
| bay_offshore_r2 | remastered | 7.73 | 7.11 | 0.03 | 0.49 | 0.40 | 0.06 |  | 0.04 | 80.40 | 4,557 | 2,961 | OK:30 | OK |
| bay_offshore_r2 | delta | +1.68 | +1.18 |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
| town_night_rain_r2 | classic | 4.23 | 4.00 |  |  |  |  |  |  | 151.90 | 824 | 557 | OK:34 | OK |
| town_night_rain_r2 | remastered | 5.13 | 4.10 |  | 0.79 | 0.61 | 0.11 |  | 0.07 | 140.90 | 804 | 535 | OK:122 | OK |
| town_night_rain_r2 | delta | +0.90 | +0.10 |  |  |  |  |  |  |  | -2.4% | -3.9% |  |  |
| pinewood_morn_r2 | classic | 5.30 | 5.10 |  |  |  |  |  |  | 114.50 | 3,162 | 1,678 | OK:30 | OK |
| pinewood_morn_r2 | remastered | 5.46 | 4.34 | 0.56 | 0.92 | 0.43 | 0.08 | 0.34 | 0.08 | 119.30 | 2,844 | 1,678 | OK:30 | OK |
| pinewood_morn_r2 | delta | +0.16 | -0.76 |  |  |  |  |  |  |  | -10.1% | +0.0% |  |  |
| valley_0515_r2 | classic | 19.92 | 19.78 |  |  |  |  |  |  | 39.70 | 11,551 | 7,966 | OK:30 | OK |
| valley_0515_r2 | remastered | 22.47 | 21.25 | 0.04 | 1.10 | 0.66 | 0.07 | 0.31 | 0.06 | 33.00 | 11,404 | 7,966 | OK:30 | OK |
| valley_0515_r2 | delta | +2.55 | +1.47 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| hill_1730_r2 | classic | 1.55 | 1.44 |  |  |  |  |  |  | 273.00 | 1,771 | 934 | OK:30 | OK |
| hill_1730_r2 | remastered | 1.58 | 1.13 | 0.04 | 0.34 | 0.22 | 0.07 |  | 0.04 | 271.70 | 1,664 | 934 | OK:30 | OK |
| hill_1730_r2 | delta | +0.03 | -0.31 |  |  |  |  |  |  |  | -6.0% | +0.0% |  |  |
| coast_oc_up_r2 | classic | 0.73 | 0.57 |  |  |  |  |  |  | 388.70 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r2 | remastered | 3.87 | 1.64 | 0.00 | 1.90 | 0.47 | 0.19 | 0.86 | 0.38 | 277.10 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r2 | delta | +3.14 | +1.07 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| sea_noon_r2 | classic | 3.79 | 3.15 |  |  |  |  |  |  | 188.90 | 180 | 0 | OK:30 | OK |
| sea_noon_r2 | remastered | 1.20 | 0.59 | 0.00 | 0.50 | 0.40 | 0.07 |  | 0.04 | 384.40 | 180 | 0 | OK:30 | OK |
| sea_noon_r2 | delta | -2.59 | -2.56 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| noon_fields_r2 | classic | 8.56 | 8.42 |  |  |  |  |  |  | 69.30 | 6,769 | 4,508 | OK:30 | OK |
| noon_fields_r2 | remastered | 9.83 | 9.19 | 0.08 | 0.49 | 0.38 | 0.07 |  | 0.04 | 67.40 | 6,400 | 4,508 | OK:30 | OK |
| noon_fields_r2 | delta | +1.27 | +0.77 |  |  |  |  |  |  |  | -5.5% | +0.0% |  |  |
| sunrise_ridge_r3 | classic | 17.04 | 16.89 |  |  |  |  |  |  | 38.20 | 13,077 | 8,897 | OK:30 | OK |
| sunrise_ridge_r3 | remastered | 18.43 | 17.24 | 0.08 | 1.04 | 0.58 | 0.06 | 0.27 | 0.13 | 36.40 | 12,911 | 8,897 | OK:30 | OK |
| sunrise_ridge_r3 | delta | +1.39 | +0.35 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| forest_edge_golden_r3 | classic | 5.99 | 5.76 |  |  |  |  |  |  | 114.90 | 2,500 | 1,459 | OK:30 | OK |
| forest_edge_golden_r3 | remastered | 8.46 | 7.44 | 0.48 | 0.77 | 0.53 | 0.14 |  | 0.10 | 91.30 | 2,140 | 1,459 | OK:30 | OK |
| forest_edge_golden_r3 | delta | +2.47 | +1.68 |  |  |  |  |  |  |  | -14.4% | +0.0% |  |  |
| town_night_r3 | classic | 9.11 | 8.75 |  |  |  |  |  |  | 92.70 | 1,182 | 1,007 | OK:158 | OK |
| town_night_r3 | remastered | 5.42 | 4.60 |  | 0.62 | 0.52 | 0.07 |  | 0.04 | 136.40 | 1,202 | 1,024 | OK:30 | OK |
| town_night_r3 | delta | -3.69 | -4.15 |  |  |  |  |  |  |  | +1.7% | +1.7% |  |  |
| coastline_sunset_r3 | classic | 2.41 | 2.04 |  |  |  |  |  |  | 243.50 | 179 | 0 | OK:30 | OK |
| coastline_sunset_r3 | remastered | 7.51 | 3.34 | 0.00 | 3.59 | 1.01 | 0.42 | 1.73 | 0.43 | 168.30 | 179 | 0 | OK:30 | OK |
| coastline_sunset_r3 | delta | +5.10 | +1.30 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| woods_pine2_evening_r3 | classic | 6.66 | 6.48 |  |  |  |  |  |  | 104.50 | 3,247 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r3 | remastered | 8.73 | 7.05 | 0.49 | 1.53 | 0.59 | 0.10 | 0.69 | 0.15 | 87.50 | 2,839 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r3 | delta | +2.07 | +0.57 |  |  |  |  |  |  |  | -12.6% | +0.0% |  |  |
| lowsun_fp_r3 | classic | 6.12 | 5.83 |  |  |  |  |  |  | 110.00 | 2,094 | 1,316 | OK:30 | OK |
| lowsun_fp_r3 | remastered | 4.60 | 3.53 | 0.08 | 0.92 | 0.38 | 0.07 | 0.40 | 0.07 | 134.00 | 1,893 | 1,316 | OK:30 | OK |
| lowsun_fp_r3 | delta | -1.52 | -2.30 |  |  |  |  |  |  |  | -9.6% | +0.0% |  |  |
| road_flat_drive_moonless_r3 | classic | 8.84 | 8.08 |  |  |  |  |  |  | 94.00 | 1,805 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r3 | remastered | 4.52 | 3.84 |  | 0.52 | 0.41 | 0.07 |  | 0.04 | 144.10 | 1,805 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r3 | delta | -4.32 | -4.24 |  |  |  |  |  |  |  | +0.0% | +0.0% |  |  |
| ffh_r3 | classic | 9.67 | 9.14 |  |  |  |  |  |  | 82.70 | 797 | 703 | UNSETTLED:300 | OK |
| ffh_r3 | remastered | 10.88 | 8.39 |  | 2.10 | 1.54 | 0.26 |  | 0.30 | 99.50 | 796 | 696 | UNSETTLED:300 | OK |
| ffh_r3 | delta | +1.21 | -0.75 |  |  |  |  |  |  |  | -0.1% | -1.0% |  |  |
| bay_offshore_r3 | classic | 7.85 | 7.67 |  |  |  |  |  |  | 83.40 | 4,561 | 2,961 | OK:30 | OK |
| bay_offshore_r3 | remastered | 9.85 | 9.15 | 0.05 | 0.55 | 0.41 | 0.09 |  | 0.06 | 69.50 | 4,558 | 2,961 | OK:30 | OK |
| bay_offshore_r3 | delta | +2.00 | +1.48 |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
| town_night_rain_r3 | classic | 6.13 | 5.78 |  |  |  |  |  |  | 119.60 | 799 | 535 | OK:56 | OK |
| town_night_rain_r3 | remastered | 8.89 | 7.44 |  | 1.13 | 0.82 | 0.18 |  | 0.13 | 97.40 | 809 | 539 | OK:79 | OK |
| town_night_rain_r3 | delta | +2.76 | +1.66 |  |  |  |  |  |  |  | +1.2% | +0.7% |  |  |
| pinewood_morn_r3 | classic | 7.42 | 7.12 |  |  |  |  |  |  | 97.00 | 3,161 | 1,678 | OK:30 | OK |
| pinewood_morn_r3 | remastered | 9.16 | 7.44 | 1.07 | 1.44 | 0.53 | 0.14 | 0.56 | 0.20 | 86.80 | 2,843 | 1,678 | OK:30 | OK |
| pinewood_morn_r3 | delta | +1.74 | +0.32 |  |  |  |  |  |  |  | -10.1% | +0.0% |  |  |
| valley_0515_r3 | classic | 20.50 | 20.38 |  |  |  |  |  |  | 39.20 | 11,551 | 7,966 | OK:30 | OK |
| valley_0515_r3 | remastered | 23.91 | 22.63 | 0.04 | 1.16 | 0.72 | 0.07 | 0.31 | 0.06 | 31.50 | 11,404 | 7,966 | OK:30 | OK |
| valley_0515_r3 | delta | +3.41 | +2.25 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| hill_1730_r3 | classic | 2.44 | 2.18 |  |  |  |  |  |  | 290.20 | 1,770 | 934 | OK:30 | OK |
| hill_1730_r3 | remastered | 8.36 | 4.57 | 0.11 | 3.02 | 1.99 | 0.53 |  | 0.50 | 284.90 | 1,663 | 934 | OK:30 | OK |
| hill_1730_r3 | delta | +5.92 | +2.39 |  |  |  |  |  |  |  | -6.0% | +0.0% |  |  |
| coast_oc_up_r3 | classic | 3.79 | 2.48 |  |  |  |  |  |  | 167.40 | 187 | 0 | OK:30 | OK |
| coast_oc_up_r3 | remastered | 8.58 | 3.15 | 0.00 | 3.93 | 2.00 | 0.40 | 0.69 | 0.84 | 144.80 | 187 | 0 | OK:30 | OK |
| coast_oc_up_r3 | delta | +4.79 | +0.67 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| sea_noon_r3 | classic | 4.36 | 3.74 |  |  |  |  |  |  | 144.80 | 178 | 0 | OK:30 | OK |
| sea_noon_r3 | remastered | 8.60 | 4.43 | 0.00 | 3.47 | 2.58 | 0.39 |  | 0.50 | 144.10 | 178 | 0 | OK:30 | OK |
| sea_noon_r3 | delta | +4.24 | +0.69 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| noon_fields_r3 | classic | 8.42 | 8.27 |  |  |  |  |  |  | 72.90 | 6,768 | 4,508 | OK:30 | OK |
| noon_fields_r3 | remastered | 9.79 | 9.16 | 0.08 | 0.49 | 0.38 | 0.07 |  | 0.04 | 67.60 | 6,399 | 4,508 | OK:30 | OK |
| noon_fields_r3 | delta | +1.37 | +0.89 |  |  |  |  |  |  |  | -5.5% | +0.0% |  |  |
| overcast_broken_r1 | classic | 10.89 | 10.75 |  |  |  |  |  |  | 57.20 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken_r1 | remastered | 12.10 | 11.29 | 0.18 | 0.68 | 0.58 | 0.06 |  | 0.04 | 54.00 | 8,066 | 5,692 | OK:30 | OK |
| overcast_broken_r1 | delta | +1.21 | +0.54 |  |  |  |  |  |  |  | -2.1% | +0.0% |  |  |
| broken_high_r1 | classic | 16.86 | 16.75 |  |  |  |  |  |  | 44.20 | 10,789 | 7,549 | OK:30 | OK |
| broken_high_r1 | remastered | 24.12 | 23.34 | 0.03 | 0.69 | 0.58 | 0.07 |  | 0.04 | 36.70 | 10,784 | 7,549 | OK:30 | OK |
| broken_high_r1 | delta | +7.26 | +6.59 |  |  |  |  |  |  |  | -0.0% | +0.0% |  |  |
| overcast_broken_r2 | classic | 10.98 | 10.85 |  |  |  |  |  |  | 56.60 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken_r2 | remastered | 11.85 | 11.10 | 0.18 | 0.63 | 0.52 | 0.06 |  | 0.04 | 53.90 | 8,066 | 5,692 | OK:30 | OK |
| overcast_broken_r2 | delta | +0.87 | +0.25 |  |  |  |  |  |  |  | -2.1% | +0.0% |  |  |
| broken_high_r2 | classic | 17.64 | 17.54 |  |  |  |  |  |  | 42.70 | 10,786 | 7,549 | OK:30 | OK |
| broken_high_r2 | remastered | 24.31 | 23.42 | 0.03 | 0.79 | 0.69 | 0.07 |  | 0.04 | 35.70 | 10,783 | 7,549 | OK:30 | OK |
| broken_high_r2 | delta | +6.67 | +5.88 |  |  |  |  |  |  |  | -0.0% | +0.0% |  |  |
| overcast_broken_r3 | classic | 10.42 | 10.28 |  |  |  |  |  |  | 58.90 | 8,242 | 5,692 | OK:30 | OK |
| overcast_broken_r3 | remastered | 11.77 | 11.00 | 0.18 | 0.65 | 0.55 | 0.06 |  | 0.04 | 55.40 | 8,065 | 5,692 | OK:30 | OK |
| overcast_broken_r3 | delta | +1.35 | +0.72 |  |  |  |  |  |  |  | -2.1% | +0.0% |  |  |
| broken_high_r3 | classic | 17.27 | 17.17 |  |  |  |  |  |  | 43.60 | 10,787 | 7,549 | OK:30 | OK |
| broken_high_r3 | remastered | 24.51 | 23.62 | 0.03 | 0.79 | 0.68 | 0.07 |  | 0.04 | 36.60 | 10,784 | 7,549 | OK:30 | OK |
| broken_high_r3 | delta | +7.24 | +6.45 |  |  |  |  |  |  |  | -0.0% | +0.0% |  |  |
| sinai_lowsun_fp_r1 | classic | 8.09 | 6.64 |  |  |  |  |  |  | 101.50 | 789 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r1 | remastered | 5.06 | 3.75 | 0.12 | 1.14 | 0.59 | 0.07 | 0.40 | 0.08 | 137.10 | 764 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r1 | delta | -3.03 | -2.89 |  |  |  |  |  |  |  | -3.2% | +0.0% |  |  |
| sinai_desert_noon_r1 | classic | 8.86 | 8.43 |  |  |  |  |  |  | 71.00 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r1 | remastered | 9.31 | 8.04 | 0.07 | 1.00 | 0.81 | 0.12 |  | 0.07 | 72.60 | 2,672 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r1 | delta | +0.45 | -0.39 |  |  |  |  |  |  |  | -0.6% | +0.0% |  |  |
| sinai_lowsun_fp_r2 | classic | 4.33 | 3.84 |  |  |  |  |  |  | 123.90 | 789 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r2 | remastered | 8.00 | 4.30 | 0.38 | 3.23 | 0.96 | 0.24 | 1.61 | 0.41 | 110.40 | 764 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r2 | delta | +3.67 | +0.46 |  |  |  |  |  |  |  | -3.2% | +0.0% |  |  |
| sinai_desert_noon_r2 | classic | 7.97 | 7.73 |  |  |  |  |  |  | 79.50 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r2 | remastered | 9.70 | 8.36 | 0.07 | 1.08 | 0.89 | 0.12 |  | 0.07 | 68.10 | 2,672 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r2 | delta | +1.73 | +0.63 |  |  |  |  |  |  |  | -0.6% | +0.0% |  |  |
| sinai_lowsun_fp_r3 | classic | 4.52 | 4.13 |  |  |  |  |  |  | 99.10 | 789 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r3 | remastered | 4.24 | 2.88 | 0.12 | 1.21 | 0.58 | 0.07 | 0.40 | 0.15 | 134.00 | 764 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r3 | delta | -0.28 | -1.25 |  |  |  |  |  |  |  | -3.2% | +0.0% |  |  |
| sinai_desert_noon_r3 | classic | 8.81 | 8.53 |  |  |  |  |  |  | 76.50 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r3 | remastered | 10.06 | 8.70 | 0.07 | 1.09 | 0.88 | 0.13 |  | 0.08 | 69.40 | 2,672 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r3 | delta | +1.25 | +0.17 |  |  |  |  |  |  |  | -0.6% | +0.0% |  |  |
