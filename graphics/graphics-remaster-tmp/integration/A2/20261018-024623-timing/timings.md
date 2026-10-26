# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge_r1 | classic | 19.74 | 19.59 |  |  |  |  |  |  | 37.00 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge_r1 | remastered | 17.74 | 16.55 | 0.08 | 1.03 | 0.59 | 0.06 | 0.27 | 0.11 | 37.50 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r1 | delta | -2.00 | -3.04 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| forest_edge_golden_r1 | classic | 5.67 | 5.48 |  |  |  |  |  |  | 114.40 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden_r1 | remastered | 6.02 | 5.23 | 0.36 | 0.58 | 0.43 | 0.09 |  | 0.06 | 115.40 | 2,142 | 1,459 | OK:30 | OK |
| forest_edge_golden_r1 | delta | +0.35 | -0.25 |  |  |  |  |  |  |  | -14.4% | +0.0% |  |  |
| town_night_r1 | classic | 5.41 | 5.20 |  |  |  |  |  |  | 139.00 | 1,176 | 999 | OK:205 | OK |
| town_night_r1 | remastered | 6.24 | 5.33 |  | 0.69 | 0.55 | 0.08 |  | 0.05 | 121.50 | 1,193 | 1,008 | OK:30 | OK |
| town_night_r1 | delta | +0.83 | +0.13 |  |  |  |  |  |  |  | +1.4% | +0.9% |  |  |
| coastline_sunset_r1 | classic | 2.06 | 1.74 |  |  |  |  |  |  | 277.40 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r1 | remastered | 1.85 | 0.70 | 0.00 | 1.01 | 0.42 | 0.08 | 0.36 | 0.15 | 313.70 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r1 | delta | -0.21 | -1.04 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| woods_pine2_evening_r1 | classic | 5.96 | 5.80 |  |  |  |  |  |  | 111.40 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r1 | remastered | 6.43 | 5.07 | 0.35 | 1.21 | 0.47 | 0.07 | 0.49 | 0.17 | 110.70 | 2,842 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r1 | delta | +0.47 | -0.73 |  |  |  |  |  |  |  | -12.6% | +0.0% |  |  |
| lowsun_fp_r1 | classic | 5.04 | 4.84 |  |  |  |  |  |  | 129.90 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp_r1 | remastered | 5.91 | 4.47 | 0.09 | 1.26 | 0.46 | 0.09 | 0.51 | 0.20 | 120.30 | 1,890 | 1,316 | OK:30 | OK |
| lowsun_fp_r1 | delta | +0.87 | -0.37 |  |  |  |  |  |  |  | -9.6% | +0.0% |  |  |
| road_flat_drive_moonless_r1 | classic | 5.02 | 4.75 |  |  |  |  |  |  | 127.70 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r1 | remastered | 6.05 | 5.25 |  | 0.59 | 0.44 | 0.09 |  | 0.06 | 119.60 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r1 | delta | +1.03 | +0.50 |  |  |  |  |  |  |  | +0.0% | +0.0% |  |  |
| ffh_r1 | classic | 7.14 | 6.82 |  |  |  |  |  |  | 117.10 | 804 | 689 | UNSETTLED:300 | OK |
| ffh_r1 | remastered | 5.93 | 5.03 |  | 0.72 | 0.58 | 0.08 |  | 0.06 | 131.20 | 800 | 680 | UNSETTLED:300 | OK |
| ffh_r1 | delta | -1.21 | -1.79 |  |  |  |  |  |  |  | -0.5% | -1.3% |  |  |
| bay_offshore_r1 | classic | 5.88 | 5.74 |  |  |  |  |  |  | 97.30 | 4,562 | 2,961 | OK:30 | OK |
| bay_offshore_r1 | remastered | 7.55 | 6.91 | 0.03 | 0.49 | 0.39 | 0.06 |  | 0.04 | 82.30 | 4,559 | 2,961 | OK:30 | OK |
| bay_offshore_r1 | delta | +1.67 | +1.17 |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
| town_night_rain_r1 | classic | 4.26 | 3.97 |  |  |  |  |  |  | 154.80 | 845 | 573 | OK:63 | OK |
| town_night_rain_r1 | remastered | 5.88 | 4.78 |  | 0.81 | 0.62 | 0.11 |  | 0.08 | 136.00 | 834 | 573 | OK:178 | OK |
| town_night_rain_r1 | delta | +1.62 | +0.81 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| pinewood_morn_r1 | classic | 6.08 | 5.85 |  |  |  |  |  |  | 107.80 | 3,163 | 1,678 | OK:30 | OK |
| pinewood_morn_r1 | remastered | 6.19 | 4.98 | 0.63 | 1.00 | 0.44 | 0.09 | 0.37 | 0.10 | 114.70 | 2,845 | 1,678 | OK:30 | OK |
| pinewood_morn_r1 | delta | +0.11 | -0.87 |  |  |  |  |  |  |  | -10.1% | +0.0% |  |  |
| valley_0515_r1 | classic | 20.76 | 20.62 |  |  |  |  |  |  | 38.10 | 11,553 | 7,966 | OK:30 | OK |
| valley_0515_r1 | remastered | 23.80 | 22.53 | 0.06 | 1.12 | 0.68 | 0.07 | 0.31 | 0.07 | 31.50 | 11,406 | 7,966 | OK:30 | OK |
| valley_0515_r1 | delta | +3.04 | +1.91 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| hill_1730_r1 | classic | 3.24 | 3.04 |  |  |  |  |  |  | 288.00 | 1,769 | 934 | OK:30 | OK |
| hill_1730_r1 | remastered | 1.96 | 1.46 | 0.05 | 0.38 | 0.22 | 0.09 |  | 0.06 | 286.60 | 1,662 | 934 | OK:30 | OK |
| hill_1730_r1 | delta | -1.28 | -1.58 |  |  |  |  |  |  |  | -6.0% | +0.0% |  |  |
| coast_oc_up_r1 | classic | 2.48 | 1.78 |  |  |  |  |  |  | 253.20 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r1 | remastered | 2.01 | 0.71 | 0.00 | 1.07 | 0.38 | 0.09 | 0.48 | 0.12 | 297.20 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r1 | delta | -0.47 | -1.07 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| sea_noon_r1 | classic | 2.77 | 2.48 |  |  |  |  |  |  | 235.00 | 179 | 0 | OK:30 | OK |
| sea_noon_r1 | remastered | 1.61 | 0.91 | 0.00 | 0.58 | 0.43 | 0.09 |  | 0.06 | 270.80 | 179 | 0 | OK:30 | OK |
| sea_noon_r1 | delta | -1.16 | -1.57 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| noon_fields_r1 | classic | 8.34 | 8.21 |  |  |  |  |  |  | 72.80 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields_r1 | remastered | 9.17 | 8.57 | 0.08 | 0.46 | 0.36 | 0.06 |  | 0.04 | 70.70 | 6,398 | 4,508 | OK:30 | OK |
| noon_fields_r1 | delta | +0.83 | +0.36 |  |  |  |  |  |  |  | -5.5% | +0.0% |  |  |
| sunrise_ridge_r2 | classic | 18.30 | 18.15 |  |  |  |  |  |  | 37.90 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge_r2 | remastered | 17.49 | 16.37 | 0.08 | 0.96 | 0.55 | 0.06 | 0.27 | 0.08 | 37.80 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r2 | delta | -0.81 | -1.78 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| forest_edge_golden_r2 | classic | 5.16 | 4.96 |  |  |  |  |  |  | 116.90 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden_r2 | remastered | 5.91 | 5.15 | 0.29 | 0.55 | 0.40 | 0.09 |  | 0.06 | 116.50 | 2,142 | 1,459 | OK:30 | OK |
| forest_edge_golden_r2 | delta | +0.75 | +0.19 |  |  |  |  |  |  |  | -14.4% | +0.0% |  |  |
| town_night_r2 | classic | 6.01 | 5.78 |  |  |  |  |  |  | 130.10 | 1,214 | 1,036 | OK:188 | OK |
| town_night_r2 | remastered | 6.30 | 5.40 |  | 0.68 | 0.55 | 0.08 |  | 0.05 | 123.00 | 1,229 | 1,060 | OK:39 | OK |
| town_night_r2 | delta | +0.29 | -0.38 |  |  |  |  |  |  |  | +1.2% | +2.3% |  |  |
| coastline_sunset_r2 | classic | 3.09 | 2.42 |  |  |  |  |  |  | 272.00 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r2 | remastered | 1.99 | 0.82 | 0.00 | 1.01 | 0.41 | 0.09 | 0.40 | 0.11 | 292.20 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r2 | delta | -1.10 | -1.60 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| woods_pine2_evening_r2 | classic | 5.21 | 5.04 |  |  |  |  |  |  | 114.50 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r2 | remastered | 6.52 | 5.24 | 0.35 | 1.12 | 0.47 | 0.07 | 0.50 | 0.08 | 110.70 | 2,842 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r2 | delta | +1.31 | +0.20 |  |  |  |  |  |  |  | -12.6% | +0.0% |  |  |
| lowsun_fp_r2 | classic | 4.42 | 4.21 |  |  |  |  |  |  | 132.00 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp_r2 | remastered | 6.00 | 4.68 | 0.10 | 1.15 | 0.44 | 0.09 | 0.52 | 0.10 | 119.90 | 1,890 | 1,316 | OK:30 | OK |
| lowsun_fp_r2 | delta | +1.58 | +0.47 |  |  |  |  |  |  |  | -9.6% | +0.0% |  |  |
| road_flat_drive_moonless_r2 | classic | 5.24 | 5.00 |  |  |  |  |  |  | 126.30 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r2 | remastered | 5.29 | 4.53 |  | 0.57 | 0.44 | 0.08 |  | 0.05 | 133.10 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r2 | delta | +0.05 | -0.47 |  |  |  |  |  |  |  | +0.0% | +0.0% |  |  |
| ffh_r2 | classic | 5.90 | 5.69 |  |  |  |  |  |  | 126.70 | 793 | 684 | UNSETTLED:300 | OK |
| ffh_r2 | remastered | 6.31 | 5.32 |  | 0.77 | 0.63 | 0.08 |  | 0.06 | 128.60 | 790 | 685 | UNSETTLED:300 | OK |
| ffh_r2 | delta | +0.41 | -0.37 |  |  |  |  |  |  |  | -0.4% | +0.1% |  |  |
| bay_offshore_r2 | classic | 5.79 | 5.65 |  |  |  |  |  |  | 97.70 | 4,562 | 2,961 | OK:30 | OK |
| bay_offshore_r2 | remastered | 7.53 | 6.90 | 0.03 | 0.48 | 0.38 | 0.06 |  | 0.04 | 82.90 | 4,559 | 2,961 | OK:30 | OK |
| bay_offshore_r2 | delta | +1.74 | +1.25 |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
| town_night_rain_r2 | classic | 5.06 | 4.77 |  |  |  |  |  |  | 141.10 | 893 | 635 | OK:133 | OK |
| town_night_rain_r2 | remastered | 5.45 | 4.50 |  | 0.73 | 0.59 | 0.09 |  | 0.06 | 157.10 | 872 | 627 | OK:108 | OK |
| town_night_rain_r2 | delta | +0.39 | -0.27 |  |  |  |  |  |  |  | -2.4% | -1.3% |  |  |
| pinewood_morn_r2 | classic | 5.92 | 5.66 |  |  |  |  |  |  | 110.20 | 3,163 | 1,678 | OK:30 | OK |
| pinewood_morn_r2 | remastered | 6.20 | 4.99 | 0.64 | 1.00 | 0.46 | 0.09 | 0.36 | 0.08 | 113.50 | 2,845 | 1,678 | OK:30 | OK |
| pinewood_morn_r2 | delta | +0.28 | -0.67 |  |  |  |  |  |  |  | -10.1% | +0.0% |  |  |
| valley_0515_r2 | classic | 20.68 | 20.55 |  |  |  |  |  |  | 38.20 | 11,553 | 7,966 | OK:30 | OK |
| valley_0515_r2 | remastered | 23.84 | 22.54 | 0.04 | 1.14 | 0.70 | 0.07 | 0.31 | 0.07 | 31.60 | 11,406 | 7,966 | OK:30 | OK |
| valley_0515_r2 | delta | +3.16 | +1.99 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| hill_1730_r2 | classic | 3.88 | 3.52 |  |  |  |  |  |  | 282.90 | 1,769 | 934 | OK:30 | OK |
| hill_1730_r2 | remastered | 2.05 | 1.52 | 0.05 | 0.40 | 0.25 | 0.10 |  | 0.06 | 281.60 | 1,662 | 934 | OK:30 | OK |
| hill_1730_r2 | delta | -1.83 | -2.00 |  |  |  |  |  |  |  | -6.0% | +0.0% |  |  |
| coast_oc_up_r2 | classic | 2.73 | 2.00 |  |  |  |  |  |  | 249.60 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r2 | remastered | 2.00 | 0.76 | 0.00 | 1.02 | 0.35 | 0.08 | 0.45 | 0.14 | 299.90 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r2 | delta | -0.73 | -1.24 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| sea_noon_r2 | classic | 2.80 | 2.51 |  |  |  |  |  |  | 237.70 | 179 | 0 | OK:30 | OK |
| sea_noon_r2 | remastered | 1.58 | 0.87 | 0.00 | 0.58 | 0.43 | 0.09 |  | 0.06 | 215.70 | 179 | 0 | OK:30 | OK |
| sea_noon_r2 | delta | -1.22 | -1.64 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| noon_fields_r2 | classic | 8.02 | 7.89 |  |  |  |  |  |  | 70.60 | 6,768 | 4,508 | OK:30 | OK |
| noon_fields_r2 | remastered | 9.03 | 8.45 | 0.08 | 0.44 | 0.34 | 0.06 |  | 0.04 | 71.30 | 6,399 | 4,508 | OK:30 | OK |
| noon_fields_r2 | delta | +1.01 | +0.56 |  |  |  |  |  |  |  | -5.5% | +0.0% |  |  |
| sunrise_ridge_r3 | classic | 16.52 | 16.37 |  |  |  |  |  |  | 38.40 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge_r3 | remastered | 17.44 | 16.32 | 0.10 | 0.96 | 0.55 | 0.06 | 0.27 | 0.08 | 37.50 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r3 | delta | +0.92 | -0.05 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| forest_edge_golden_r3 | classic | 4.54 | 4.33 |  |  |  |  |  |  | 134.30 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden_r3 | remastered | 5.95 | 5.18 | 0.29 | 0.56 | 0.42 | 0.09 |  | 0.06 | 116.10 | 2,142 | 1,459 | OK:30 | OK |
| forest_edge_golden_r3 | delta | +1.41 | +0.85 |  |  |  |  |  |  |  | -14.4% | +0.0% |  |  |
| town_night_r3 | classic | 5.59 | 5.33 |  |  |  |  |  |  | 131.50 | 1,197 | 1,025 | OK:136 | OK |
| town_night_r3 | remastered | 6.18 | 5.24 |  | 0.72 | 0.58 | 0.08 |  | 0.05 | 122.80 | 1,199 | 1,003 | OK:56 | OK |
| town_night_r3 | delta | +0.59 | -0.09 |  |  |  |  |  |  |  | +0.2% | -2.1% |  |  |
| coastline_sunset_r3 | classic | 2.04 | 1.73 |  |  |  |  |  |  | 287.30 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r3 | remastered | 1.93 | 0.68 | 0.00 | 1.10 | 0.42 | 0.08 | 0.39 | 0.19 | 312.30 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r3 | delta | -0.11 | -1.05 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| woods_pine2_evening_r3 | classic | 5.16 | 5.00 |  |  |  |  |  |  | 125.00 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r3 | remastered | 6.34 | 5.07 | 0.35 | 1.11 | 0.47 | 0.07 | 0.49 | 0.08 | 111.30 | 2,842 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r3 | delta | +1.18 | +0.07 |  |  |  |  |  |  |  | -12.6% | +0.0% |  |  |
| lowsun_fp_r3 | classic | 4.24 | 4.03 |  |  |  |  |  |  | 140.60 | 2,092 | 1,316 | OK:30 | OK |
| lowsun_fp_r3 | remastered | 5.98 | 4.66 | 0.09 | 1.14 | 0.45 | 0.09 | 0.51 | 0.09 | 120.10 | 1,891 | 1,316 | OK:30 | OK |
| lowsun_fp_r3 | delta | +1.74 | +0.63 |  |  |  |  |  |  |  | -9.6% | +0.0% |  |  |
| road_flat_drive_moonless_r3 | classic | 5.16 | 4.91 |  |  |  |  |  |  | 130.10 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r3 | remastered | 6.03 | 5.21 |  | 0.62 | 0.46 | 0.09 |  | 0.06 | 118.50 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r3 | delta | +0.87 | +0.30 |  |  |  |  |  |  |  | +0.0% | +0.0% |  |  |
| ffh_r3 | classic | 4.54 | 4.27 |  |  |  |  |  |  | 151.10 | 797 | 693 | UNSETTLED:300 | OK |
| ffh_r3 | remastered | 6.17 | 5.31 |  | 0.68 | 0.54 | 0.08 |  | 0.06 | 126.90 | 802 | 683 | UNSETTLED:300 | OK |
| ffh_r3 | delta | +1.63 | +1.04 |  |  |  |  |  |  |  | +0.6% | -1.4% |  |  |
| bay_offshore_r3 | classic | 5.74 | 5.60 |  |  |  |  |  |  | 96.90 | 4,562 | 2,961 | OK:30 | OK |
| bay_offshore_r3 | remastered | 7.54 | 6.92 | 0.03 | 0.47 | 0.37 | 0.06 |  | 0.04 | 82.20 | 4,559 | 2,961 | OK:30 | OK |
| bay_offshore_r3 | delta | +1.80 | +1.32 |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
| town_night_rain_r3 | classic | 5.65 | 5.36 |  |  |  |  |  |  | 139.40 | 922 | 669 | OK:115 | OK |
| town_night_rain_r3 | remastered | 5.91 | 4.93 |  | 0.75 | 0.60 | 0.09 |  | 0.06 | 140.90 | 936 | 675 | OK:54 | OK |
| town_night_rain_r3 | delta | +0.26 | -0.43 |  |  |  |  |  |  |  | +1.5% | +0.9% |  |  |
| pinewood_morn_r3 | classic | 5.34 | 5.09 |  |  |  |  |  |  | 119.60 | 3,163 | 1,678 | OK:30 | OK |
| pinewood_morn_r3 | remastered | 6.02 | 4.72 | 0.58 | 1.09 | 0.48 | 0.09 | 0.36 | 0.16 | 115.60 | 2,845 | 1,678 | OK:30 | OK |
| pinewood_morn_r3 | delta | +0.68 | -0.37 |  |  |  |  |  |  |  | -10.1% | +0.0% |  |  |
| valley_0515_r3 | classic | 20.79 | 20.65 |  |  |  |  |  |  | 38.70 | 11,553 | 7,966 | OK:30 | OK |
| valley_0515_r3 | remastered | 23.82 | 22.49 | 0.04 | 1.18 | 0.73 | 0.07 | 0.31 | 0.07 | 31.50 | 11,406 | 7,966 | OK:30 | OK |
| valley_0515_r3 | delta | +3.03 | +1.84 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| hill_1730_r3 | classic | 3.62 | 3.26 |  |  |  |  |  |  | 286.80 | 1,769 | 934 | OK:30 | OK |
| hill_1730_r3 | remastered | 1.99 | 1.49 | 0.05 | 0.38 | 0.23 | 0.10 |  | 0.06 | 285.40 | 1,662 | 934 | OK:30 | OK |
| hill_1730_r3 | delta | -1.63 | -1.77 |  |  |  |  |  |  |  | -6.0% | +0.0% |  |  |
| coast_oc_up_r3 | classic | 2.53 | 1.96 |  |  |  |  |  |  | 246.90 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r3 | remastered | 1.57 | 0.48 | 0.00 | 0.90 | 0.36 | 0.06 | 0.37 | 0.10 | 372.60 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r3 | delta | -0.96 | -1.48 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| sea_noon_r3 | classic | 3.26 | 2.80 |  |  |  |  |  |  | 238.70 | 179 | 0 | OK:30 | OK |
| sea_noon_r3 | remastered | 1.42 | 0.77 | 0.00 | 0.54 | 0.41 | 0.08 |  | 0.05 | 237.00 | 179 | 0 | OK:30 | OK |
| sea_noon_r3 | delta | -1.84 | -2.03 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| noon_fields_r3 | classic | 7.55 | 7.43 |  |  |  |  |  |  | 77.10 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields_r3 | remastered | 9.10 | 8.51 | 0.08 | 0.46 | 0.36 | 0.06 |  | 0.04 | 71.80 | 6,398 | 4,508 | OK:30 | OK |
| noon_fields_r3 | delta | +1.55 | +1.08 |  |  |  |  |  |  |  | -5.5% | +0.0% |  |  |
| overcast_broken_r1 | classic | 10.50 | 10.37 |  |  |  |  |  |  | 58.80 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken_r1 | remastered | 11.99 | 11.19 | 0.18 | 0.67 | 0.57 | 0.06 |  | 0.04 | 54.50 | 8,066 | 5,692 | OK:30 | OK |
| overcast_broken_r1 | delta | +1.49 | +0.82 |  |  |  |  |  |  |  | -2.1% | +0.0% |  |  |
| broken_high_r1 | classic | 17.12 | 17.01 |  |  |  |  |  |  | 43.50 | 10,789 | 7,549 | OK:30 | OK |
| broken_high_r1 | remastered | 24.78 | 23.88 | 0.03 | 0.79 | 0.69 | 0.07 |  | 0.04 | 36.00 | 10,785 | 7,549 | OK:30 | OK |
| broken_high_r1 | delta | +7.66 | +6.87 |  |  |  |  |  |  |  | -0.0% | +0.0% |  |  |
| overcast_broken_r2 | classic | 10.32 | 10.19 |  |  |  |  |  |  | 58.20 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken_r2 | remastered | 12.96 | 12.19 | 0.20 | 0.64 | 0.54 | 0.06 |  | 0.04 | 53.80 | 8,066 | 5,692 | OK:30 | OK |
| overcast_broken_r2 | delta | +2.64 | +2.00 |  |  |  |  |  |  |  | -2.1% | +0.0% |  |  |
| broken_high_r2 | classic | 17.26 | 17.15 |  |  |  |  |  |  | 43.60 | 10,789 | 7,549 | OK:30 | OK |
| broken_high_r2 | remastered | 24.53 | 23.64 | 0.03 | 0.79 | 0.68 | 0.07 |  | 0.04 | 36.20 | 10,785 | 7,549 | OK:30 | OK |
| broken_high_r2 | delta | +7.27 | +6.49 |  |  |  |  |  |  |  | -0.0% | +0.0% |  |  |
| overcast_broken_r3 | classic | 10.07 | 9.94 |  |  |  |  |  |  | 60.60 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken_r3 | remastered | 11.91 | 11.18 | 0.20 | 0.61 | 0.50 | 0.06 |  | 0.04 | 54.80 | 8,066 | 5,692 | OK:30 | OK |
| overcast_broken_r3 | delta | +1.84 | +1.24 |  |  |  |  |  |  |  | -2.1% | +0.0% |  |  |
| broken_high_r3 | classic | 16.91 | 16.80 |  |  |  |  |  |  | 44.10 | 10,789 | 7,549 | OK:30 | OK |
| broken_high_r3 | remastered | 24.74 | 23.83 | 0.03 | 0.81 | 0.71 | 0.07 |  | 0.04 | 36.00 | 10,785 | 7,549 | OK:30 | OK |
| broken_high_r3 | delta | +7.83 | +7.03 |  |  |  |  |  |  |  | -0.0% | +0.0% |  |  |
| sinai_lowsun_fp_r1 | classic | 3.68 | 3.38 |  |  |  |  |  |  | 135.40 | 789 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r1 | remastered | 6.09 | 3.55 | 0.28 | 2.24 | 0.84 | 0.21 | 0.98 | 0.22 | 127.30 | 764 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r1 | delta | +2.41 | +0.17 |  |  |  |  |  |  |  | -3.2% | +0.0% |  |  |
| sinai_desert_noon_r1 | classic | 7.37 | 7.21 |  |  |  |  |  |  | 84.10 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r1 | remastered | 8.35 | 7.31 | 0.04 | 0.87 | 0.75 | 0.07 |  | 0.05 | 76.40 | 2,672 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r1 | delta | +0.98 | +0.10 |  |  |  |  |  |  |  | -0.6% | +0.0% |  |  |
| sinai_lowsun_fp_r2 | classic | 3.61 | 3.31 |  |  |  |  |  |  | 140.30 | 789 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r2 | remastered | 4.41 | 2.90 | 0.13 | 1.33 | 0.59 | 0.08 | 0.49 | 0.16 | 140.00 | 764 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r2 | delta | +0.80 | -0.41 |  |  |  |  |  |  |  | -3.2% | +0.0% |  |  |
| sinai_desert_noon_r2 | classic | 7.15 | 6.98 |  |  |  |  |  |  | 84.00 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r2 | remastered | 8.33 | 7.32 | 0.04 | 0.84 | 0.72 | 0.07 |  | 0.05 | 77.40 | 2,672 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r2 | delta | +1.18 | +0.34 |  |  |  |  |  |  |  | -0.6% | +0.0% |  |  |
| sinai_lowsun_fp_r3 | classic | 3.47 | 3.16 |  |  |  |  |  |  | 141.70 | 788 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r3 | remastered | 4.07 | 2.80 | 0.12 | 1.09 | 0.52 | 0.07 | 0.42 | 0.07 | 141.10 | 763 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r3 | delta | +0.60 | -0.36 |  |  |  |  |  |  |  | -3.2% | +0.0% |  |  |
| sinai_desert_noon_r3 | classic | 7.26 | 7.08 |  |  |  |  |  |  | 83.70 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r3 | remastered | 8.38 | 7.34 | 0.05 | 0.86 | 0.74 | 0.07 |  | 0.05 | 77.00 | 2,672 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r3 | delta | +1.12 | +0.26 |  |  |  |  |  |  |  | -0.6% | +0.0% |  |  |
