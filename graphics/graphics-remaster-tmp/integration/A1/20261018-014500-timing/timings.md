# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge_r1 | classic | 19.77 | 19.62 |  |  |  |  |  |  | 36.70 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge_r1 | remastered | 17.65 | 16.48 | 0.08 | 1.01 | 0.59 | 0.06 | 0.26 | 0.09 | 37.60 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r1 | delta | -2.12 | -3.14 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| forest_edge_golden_r1 | classic | 5.69 | 5.50 |  |  |  |  |  |  | 116.30 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden_r1 | remastered | 5.90 | 5.10 | 0.29 | 0.60 | 0.45 | 0.09 |  | 0.06 | 116.10 | 2,142 | 1,459 | OK:30 | OK |
| forest_edge_golden_r1 | delta | +0.21 | -0.40 |  |  |  |  |  |  |  | -14.4% | +0.0% |  |  |
| town_night_r1 | classic | 6.37 | 6.14 |  |  |  |  |  |  | 124.00 | 1,286 | 1,121 | OK:246 | OK |
| town_night_r1 | remastered | 6.78 | 5.94 |  | 0.62 | 0.48 | 0.08 |  | 0.05 | 119.20 | 1,287 | 1,126 | OK:73 | OK |
| town_night_r1 | delta | +0.41 | -0.20 |  |  |  |  |  |  |  | +0.1% | +0.4% |  |  |
| coastline_sunset_r1 | classic | 1.54 | 1.32 |  |  |  |  |  |  | 57.10 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r1 | remastered | 1.51 | 0.55 | 0.00 | 0.84 | 0.39 | 0.07 | 0.31 | 0.06 | 388.20 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r1 | delta | -0.03 | -0.77 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| woods_pine2_evening_r1 | classic | 6.12 | 5.96 |  |  |  |  |  |  | 108.50 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r1 | remastered | 6.49 | 5.22 | 0.35 | 1.12 | 0.48 | 0.07 | 0.50 | 0.08 | 107.60 | 2,842 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r1 | delta | +0.37 | -0.74 |  |  |  |  |  |  |  | -12.6% | +0.0% |  |  |
| lowsun_fp_r1 | classic | 4.76 | 4.56 |  |  |  |  |  |  | 126.70 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp_r1 | remastered | 5.49 | 4.20 | 0.09 | 1.12 | 0.45 | 0.08 | 0.49 | 0.10 | 128.60 | 1,890 | 1,316 | OK:30 | OK |
| lowsun_fp_r1 | delta | +0.73 | -0.36 |  |  |  |  |  |  |  | -9.6% | +0.0% |  |  |
| road_flat_drive_moonless_r1 | classic | 5.85 | 5.43 |  |  |  |  |  |  | 128.20 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r1 | remastered | 5.56 | 4.70 |  | 0.65 | 0.51 | 0.08 |  | 0.06 | 129.30 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r1 | delta | -0.29 | -0.73 |  |  |  |  |  |  |  | +0.0% | +0.0% |  |  |
| ffh_r1 | classic | 5.57 | 5.37 |  |  |  |  |  |  | 125.20 | 825 | 709 | UNSETTLED:300 | OK |
| ffh_r1 | remastered | 5.67 | 4.84 |  | 0.66 | 0.52 | 0.08 |  | 0.06 | 143.40 | 789 | 681 | UNSETTLED:300 | OK |
| ffh_r1 | delta | +0.10 | -0.53 |  |  |  |  |  |  |  | -4.4% | -3.9% |  |  |
| bay_offshore_r1 | classic | 5.52 | 5.39 |  |  |  |  |  |  | 94.40 | 4,562 | 2,961 | OK:30 | OK |
| bay_offshore_r1 | remastered | 7.39 | 6.77 | 0.03 | 0.48 | 0.37 | 0.06 |  | 0.04 | 81.30 | 4,559 | 2,961 | OK:30 | OK |
| bay_offshore_r1 | delta | +1.87 | +1.38 |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
| town_night_rain_r1 | classic | 4.54 | 4.30 |  |  |  |  |  |  | 129.90 | 949 | 696 | OK:101 | OK |
| town_night_rain_r1 | remastered | 6.30 | 5.26 |  | 0.76 | 0.58 | 0.11 |  | 0.08 | 139.00 | 933 | 688 | OK:69 | OK |
| town_night_rain_r1 | delta | +1.76 | +0.96 |  |  |  |  |  |  |  | -1.7% | -1.1% |  |  |
| pinewood_morn_r1 | classic | 6.09 | 5.82 |  |  |  |  |  |  | 108.80 | 3,163 | 1,678 | OK:30 | OK |
| pinewood_morn_r1 | remastered | 6.36 | 5.10 | 0.64 | 1.03 | 0.44 | 0.09 | 0.41 | 0.09 | 112.00 | 2,845 | 1,678 | OK:30 | OK |
| pinewood_morn_r1 | delta | +0.27 | -0.72 |  |  |  |  |  |  |  | -10.1% | +0.0% |  |  |
| valley_0515_r1 | classic | 19.65 | 19.51 |  |  |  |  |  |  | 39.20 | 11,553 | 7,966 | OK:30 | OK |
| valley_0515_r1 | remastered | 22.53 | 21.24 | 0.04 | 1.15 | 0.71 | 0.07 | 0.31 | 0.06 | 33.00 | 11,406 | 7,966 | OK:30 | OK |
| valley_0515_r1 | delta | +2.88 | +1.73 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| hill_1730_r1 | classic | 3.36 | 2.87 |  |  |  |  |  |  | 292.00 | 1,769 | 934 | OK:30 | OK |
| hill_1730_r1 | remastered | 3.17 | 2.36 | 0.13 | 0.64 | 0.37 | 0.17 |  | 0.11 | 297.00 | 1,662 | 934 | OK:30 | OK |
| hill_1730_r1 | delta | -0.19 | -0.51 |  |  |  |  |  |  |  | -6.0% | +0.0% |  |  |
| coast_oc_up_r1 | classic | 3.08 | 1.89 |  |  |  |  |  |  | 199.80 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r1 | remastered | 6.15 | 1.74 | 0.00 | 3.02 | 1.71 | 0.25 | 0.56 | 0.49 | 180.40 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r1 | delta | +3.07 | -0.15 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| sea_noon_r1 | classic | 3.18 | 2.77 |  |  |  |  |  |  | 192.50 | 179 | 0 | OK:30 | OK |
| sea_noon_r1 | remastered | 5.93 | 2.78 | 0.00 | 2.72 | 1.97 | 0.34 |  | 0.41 | 179.70 | 179 | 0 | OK:30 | OK |
| sea_noon_r1 | delta | +2.75 | +0.01 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| noon_fields_r1 | classic | 7.97 | 7.85 |  |  |  |  |  |  | 75.90 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields_r1 | remastered | 8.67 | 8.11 | 0.07 | 0.43 | 0.33 | 0.06 |  | 0.04 | 74.30 | 6,398 | 4,508 | OK:30 | OK |
| noon_fields_r1 | delta | +0.70 | +0.26 |  |  |  |  |  |  |  | -5.5% | +0.0% |  |  |
| sunrise_ridge_r2 | classic | 17.58 | 17.43 |  |  |  |  |  |  | 39.50 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge_r2 | remastered | 17.31 | 16.16 | 0.08 | 1.00 | 0.60 | 0.06 | 0.27 | 0.07 | 38.10 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r2 | delta | -0.27 | -1.27 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| forest_edge_golden_r2 | classic | 4.86 | 4.66 |  |  |  |  |  |  | 117.10 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden_r2 | remastered | 5.31 | 4.58 | 0.29 | 0.54 | 0.40 | 0.08 |  | 0.05 | 117.10 | 2,142 | 1,459 | OK:30 | OK |
| forest_edge_golden_r2 | delta | +0.45 | -0.08 |  |  |  |  |  |  |  | -14.4% | +0.0% |  |  |
| town_night_r2 | classic | 6.61 | 6.32 |  |  |  |  |  |  | 118.60 | 1,239 | 1,069 | OK:212 | OK |
| town_night_r2 | remastered | 5.84 | 5.03 |  | 0.60 | 0.48 | 0.07 |  | 0.05 | 135.10 | 1,254 | 1,071 | OK:43 | OK |
| town_night_r2 | delta | -0.77 | -1.29 |  |  |  |  |  |  |  | +1.2% | +0.2% |  |  |
| coastline_sunset_r2 | classic | 3.07 | 2.32 |  |  |  |  |  |  | 248.50 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r2 | remastered | 6.12 | 2.05 | 0.00 | 3.30 | 2.00 | 0.31 | 0.45 | 0.53 | 183.60 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r2 | delta | +3.05 | -0.27 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| woods_pine2_evening_r2 | classic | 5.52 | 5.36 |  |  |  |  |  |  | 107.00 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r2 | remastered | 6.48 | 5.19 | 0.35 | 1.13 | 0.47 | 0.07 | 0.50 | 0.10 | 109.00 | 2,842 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r2 | delta | +0.96 | -0.17 |  |  |  |  |  |  |  | -12.6% | +0.0% |  |  |
| lowsun_fp_r2 | classic | 4.40 | 4.19 |  |  |  |  |  |  | 128.00 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp_r2 | remastered | 4.66 | 3.63 | 0.07 | 0.89 | 0.39 | 0.06 | 0.37 | 0.07 | 138.10 | 1,890 | 1,316 | OK:30 | OK |
| lowsun_fp_r2 | delta | +0.26 | -0.56 |  |  |  |  |  |  |  | -9.6% | +0.0% |  |  |
| road_flat_drive_moonless_r2 | classic | 5.02 | 4.77 |  |  |  |  |  |  | 128.00 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r2 | remastered | 4.46 | 3.74 |  | 0.55 | 0.44 | 0.07 |  | 0.05 | 144.40 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r2 | delta | -0.56 | -1.03 |  |  |  |  |  |  |  | +0.0% | +0.0% |  |  |
| ffh_r2 | classic | 5.45 | 5.26 |  |  |  |  |  |  | 138.00 | 806 | 692 | UNSETTLED:300 | OK |
| ffh_r2 | remastered | 6.37 | 5.40 |  | 0.77 | 0.62 | 0.09 |  | 0.06 | 124.20 | 800 | 689 | UNSETTLED:300 | OK |
| ffh_r2 | delta | +0.92 | +0.14 |  |  |  |  |  |  |  | -0.7% | -0.4% |  |  |
| bay_offshore_r2 | classic | 6.22 | 6.08 |  |  |  |  |  |  | 96.50 | 4,562 | 2,961 | OK:30 | OK |
| bay_offshore_r2 | remastered | 7.35 | 6.70 | 0.03 | 0.51 | 0.40 | 0.06 |  | 0.04 | 81.20 | 4,559 | 2,961 | OK:30 | OK |
| bay_offshore_r2 | delta | +1.13 | +0.62 |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
| town_night_rain_r2 | classic | 5.42 | 5.22 |  |  |  |  |  |  | 137.60 | 1,023 | 794 | OK:239 | OK |
| town_night_rain_r2 | remastered | 5.65 | 4.80 |  | 0.66 | 0.54 | 0.07 |  | 0.05 | 144.60 | 1,013 | 793 | OK:251 | OK |
| town_night_rain_r2 | delta | +0.23 | -0.42 |  |  |  |  |  |  |  | -1.0% | -0.1% |  |  |
| pinewood_morn_r2 | classic | 5.46 | 5.20 |  |  |  |  |  |  | 113.70 | 3,163 | 1,678 | OK:30 | OK |
| pinewood_morn_r2 | remastered | 6.41 | 5.13 | 0.66 | 1.06 | 0.47 | 0.09 | 0.41 | 0.08 | 112.80 | 2,845 | 1,678 | OK:30 | OK |
| pinewood_morn_r2 | delta | +0.95 | -0.07 |  |  |  |  |  |  |  | -10.1% | +0.0% |  |  |
| valley_0515_r2 | classic | 20.19 | 20.05 |  |  |  |  |  |  | 39.80 | 11,553 | 7,966 | OK:30 | OK |
| valley_0515_r2 | remastered | 26.00 | 24.56 | 0.04 | 1.30 | 0.78 | 0.07 | 0.31 | 0.14 | 29.50 | 11,406 | 7,966 | OK:30 | OK |
| valley_0515_r2 | delta | +5.81 | +4.51 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| hill_1730_r2 | classic | 2.27 | 2.10 |  |  |  |  |  |  | 273.80 | 1,769 | 934 | OK:30 | OK |
| hill_1730_r2 | remastered | 3.94 | 2.98 | 0.12 | 0.77 | 0.49 | 0.17 |  | 0.12 | 272.60 | 1,662 | 934 | OK:30 | OK |
| hill_1730_r2 | delta | +1.67 | +0.88 |  |  |  |  |  |  |  | -6.0% | +0.0% |  |  |
| coast_oc_up_r2 | classic | 2.11 | 1.55 |  |  |  |  |  |  | 254.50 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r2 | remastered | 4.17 | 1.63 | 0.00 | 1.95 | 0.49 | 0.21 | 1.05 | 0.21 | 228.70 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r2 | delta | +2.06 | +0.08 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| sea_noon_r2 | classic | 2.75 | 2.49 |  |  |  |  |  |  | 228.60 | 179 | 0 | OK:30 | OK |
| sea_noon_r2 | remastered | 3.50 | 2.24 | 0.00 | 1.05 | 0.67 | 0.21 |  | 0.16 | 226.20 | 179 | 0 | OK:30 | OK |
| sea_noon_r2 | delta | +0.75 | -0.25 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| noon_fields_r2 | classic | 7.73 | 7.61 |  |  |  |  |  |  | 75.80 | 6,768 | 4,508 | OK:30 | OK |
| noon_fields_r2 | remastered | 8.92 | 8.34 | 0.07 | 0.44 | 0.34 | 0.06 |  | 0.04 | 72.40 | 6,399 | 4,508 | OK:30 | OK |
| noon_fields_r2 | delta | +1.19 | +0.73 |  |  |  |  |  |  |  | -5.5% | +0.0% |  |  |
| sunrise_ridge_r3 | classic | 15.97 | 15.82 |  |  |  |  |  |  | 39.40 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge_r3 | remastered | 17.00 | 15.89 | 0.08 | 0.96 | 0.57 | 0.06 | 0.27 | 0.06 | 37.60 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r3 | delta | +1.03 | +0.07 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| forest_edge_golden_r3 | classic | 5.11 | 4.87 |  |  |  |  |  |  | 126.70 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden_r3 | remastered | 6.11 | 5.30 | 0.30 | 0.60 | 0.46 | 0.09 |  | 0.06 | 114.00 | 2,142 | 1,459 | OK:30 | OK |
| forest_edge_golden_r3 | delta | +1.00 | +0.43 |  |  |  |  |  |  |  | -14.4% | +0.0% |  |  |
| town_night_r3 | classic | 5.79 | 5.59 |  |  |  |  |  |  | 135.10 | 1,286 | 1,109 | OK:212 | OK |
| town_night_r3 | remastered | 6.51 | 5.62 |  | 0.67 | 0.55 | 0.07 |  | 0.05 | 120.50 | 1,302 | 1,122 | OK:30 | OK |
| town_night_r3 | delta | +0.72 | +0.03 |  |  |  |  |  |  |  | +1.2% | +1.2% |  |  |
| coastline_sunset_r3 | classic | 1.95 | 1.65 |  |  |  |  |  |  | 290.90 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r3 | remastered | 1.76 | 0.67 | 0.00 | 0.94 | 0.42 | 0.08 | 0.37 | 0.08 | 317.50 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r3 | delta | -0.19 | -0.98 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| woods_pine2_evening_r3 | classic | 4.99 | 4.83 |  |  |  |  |  |  | 122.60 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r3 | remastered | 6.52 | 5.25 | 0.35 | 1.12 | 0.47 | 0.07 | 0.49 | 0.09 | 110.90 | 2,842 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r3 | delta | +1.53 | +0.42 |  |  |  |  |  |  |  | -12.6% | +0.0% |  |  |
| lowsun_fp_r3 | classic | 4.00 | 3.78 |  |  |  |  |  |  | 140.80 | 2,092 | 1,316 | OK:30 | OK |
| lowsun_fp_r3 | remastered | 5.79 | 4.47 | 0.09 | 1.15 | 0.47 | 0.09 | 0.50 | 0.09 | 121.30 | 1,891 | 1,316 | OK:30 | OK |
| lowsun_fp_r3 | delta | +1.79 | +0.69 |  |  |  |  |  |  |  | -9.6% | +0.0% |  |  |
| road_flat_drive_moonless_r3 | classic | 5.16 | 4.92 |  |  |  |  |  |  | 127.40 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r3 | remastered | 4.30 | 3.61 |  | 0.52 | 0.41 | 0.06 |  | 0.04 | 149.10 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r3 | delta | -0.86 | -1.31 |  |  |  |  |  |  |  | +0.0% | +0.0% |  |  |
| ffh_r3 | classic | 5.83 | 5.63 |  |  |  |  |  |  | 129.30 | 802 | 699 | UNSETTLED:300 | OK |
| ffh_r3 | remastered | 6.15 | 5.22 |  | 0.75 | 0.61 | 0.08 |  | 0.06 | 127.50 | 795 | 673 | UNSETTLED:300 | OK |
| ffh_r3 | delta | +0.32 | -0.41 |  |  |  |  |  |  |  | -0.9% | -3.7% |  |  |
| bay_offshore_r3 | classic | 5.89 | 5.75 |  |  |  |  |  |  | 96.40 | 4,562 | 2,961 | OK:30 | OK |
| bay_offshore_r3 | remastered | 7.50 | 6.86 | 0.03 | 0.50 | 0.40 | 0.06 |  | 0.04 | 82.50 | 4,559 | 2,961 | OK:30 | OK |
| bay_offshore_r3 | delta | +1.61 | +1.11 |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
| town_night_rain_r3 | classic | 6.24 | 5.98 |  |  |  |  |  |  | 125.70 | 1,050 | 819 | OK:231 | OK |
| town_night_rain_r3 | remastered | 5.80 | 4.94 |  | 0.66 | 0.53 | 0.08 |  | 0.05 | 141.80 | 1,053 | 808 | OK:179 | OK |
| town_night_rain_r3 | delta | -0.44 | -1.04 |  |  |  |  |  |  |  | +0.3% | -1.3% |  |  |
| pinewood_morn_r3 | classic | 5.35 | 5.10 |  |  |  |  |  |  | 117.20 | 3,163 | 1,678 | OK:30 | OK |
| pinewood_morn_r3 | remastered | 6.04 | 4.84 | 0.57 | 0.99 | 0.45 | 0.10 | 0.36 | 0.08 | 113.70 | 2,845 | 1,678 | OK:30 | OK |
| pinewood_morn_r3 | delta | +0.69 | -0.26 |  |  |  |  |  |  |  | -10.1% | +0.0% |  |  |
| valley_0515_r3 | classic | 20.57 | 20.43 |  |  |  |  |  |  | 38.90 | 11,553 | 7,966 | OK:30 | OK |
| valley_0515_r3 | remastered | 23.76 | 22.43 | 0.04 | 1.19 | 0.74 | 0.07 | 0.31 | 0.07 | 31.50 | 11,406 | 7,966 | OK:30 | OK |
| valley_0515_r3 | delta | +3.19 | +2.00 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| hill_1730_r3 | classic | 3.92 | 3.47 |  |  |  |  |  |  | 289.80 | 1,769 | 934 | OK:30 | OK |
| hill_1730_r3 | remastered | 1.99 | 1.50 | 0.05 | 0.37 | 0.23 | 0.09 |  | 0.06 | 286.80 | 1,662 | 934 | OK:30 | OK |
| hill_1730_r3 | delta | -1.93 | -1.97 |  |  |  |  |  |  |  | -6.0% | +0.0% |  |  |
| coast_oc_up_r3 | classic | 3.94 | 2.43 |  |  |  |  |  |  | 229.70 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r3 | remastered | 2.03 | 0.69 | 0.00 | 1.10 | 0.38 | 0.09 | 0.50 | 0.13 | 318.50 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r3 | delta | -1.91 | -1.74 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| sea_noon_r3 | classic | 2.39 | 2.10 |  |  |  |  |  |  | 234.20 | 179 | 0 | OK:30 | OK |
| sea_noon_r3 | remastered | 1.65 | 0.99 | 0.00 | 0.54 | 0.41 | 0.08 |  | 0.05 | 242.40 | 179 | 0 | OK:30 | OK |
| sea_noon_r3 | delta | -0.74 | -1.11 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| noon_fields_r3 | classic | 7.55 | 7.42 |  |  |  |  |  |  | 77.80 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields_r3 | remastered | 9.20 | 8.57 | 0.08 | 0.49 | 0.38 | 0.06 |  | 0.04 | 71.10 | 6,398 | 4,508 | OK:30 | OK |
| noon_fields_r3 | delta | +1.65 | +1.15 |  |  |  |  |  |  |  | -5.5% | +0.0% |  |  |
| overcast_broken_r1 | classic | 10.87 | 10.73 |  |  |  |  |  |  | 58.80 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken_r1 | remastered | 12.33 | 11.57 | 0.19 | 0.63 | 0.53 | 0.06 |  | 0.04 | 54.10 | 8,066 | 5,692 | OK:30 | OK |
| overcast_broken_r1 | delta | +1.46 | +0.84 |  |  |  |  |  |  |  | -2.1% | +0.0% |  |  |
| broken_high_r1 | classic | 17.07 | 16.96 |  |  |  |  |  |  | 43.90 | 10,789 | 7,549 | OK:30 | OK |
| broken_high_r1 | remastered | 24.91 | 24.02 | 0.03 | 0.79 | 0.69 | 0.07 |  | 0.04 | 35.90 | 10,785 | 7,549 | OK:30 | OK |
| broken_high_r1 | delta | +7.84 | +7.06 |  |  |  |  |  |  |  | -0.0% | +0.0% |  |  |
| overcast_broken_r2 | classic | 10.08 | 9.95 |  |  |  |  |  |  | 59.80 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken_r2 | remastered | 12.02 | 11.27 | 0.24 | 0.62 | 0.52 | 0.06 |  | 0.04 | 54.50 | 8,066 | 5,692 | OK:30 | OK |
| overcast_broken_r2 | delta | +1.94 | +1.32 |  |  |  |  |  |  |  | -2.1% | +0.0% |  |  |
| broken_high_r2 | classic | 17.02 | 16.92 |  |  |  |  |  |  | 43.60 | 10,789 | 7,549 | OK:30 | OK |
| broken_high_r2 | remastered | 24.65 | 23.81 | 0.03 | 0.74 | 0.63 | 0.07 |  | 0.04 | 36.00 | 10,785 | 7,549 | OK:30 | OK |
| broken_high_r2 | delta | +7.63 | +6.89 |  |  |  |  |  |  |  | -0.0% | +0.0% |  |  |
| overcast_broken_r3 | classic | 10.20 | 10.07 |  |  |  |  |  |  | 59.30 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken_r3 | remastered | 11.99 | 11.21 | 0.19 | 0.66 | 0.56 | 0.06 |  | 0.04 | 54.60 | 8,066 | 5,692 | OK:30 | OK |
| overcast_broken_r3 | delta | +1.79 | +1.14 |  |  |  |  |  |  |  | -2.1% | +0.0% |  |  |
| broken_high_r3 | classic | 16.95 | 16.84 |  |  |  |  |  |  | 43.70 | 10,789 | 7,549 | OK:30 | OK |
| broken_high_r3 | remastered | 24.57 | 23.69 | 0.03 | 0.78 | 0.67 | 0.07 |  | 0.04 | 36.30 | 10,785 | 7,549 | OK:30 | OK |
| broken_high_r3 | delta | +7.62 | +6.85 |  |  |  |  |  |  |  | -0.0% | +0.0% |  |  |
| sinai_lowsun_fp_r1 | classic | 5.35 | 4.55 |  |  |  |  |  |  | 124.40 | 789 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r1 | remastered | 4.84 | 3.56 | 0.12 | 1.12 | 0.56 | 0.07 | 0.41 | 0.07 | 139.40 | 764 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r1 | delta | -0.51 | -0.99 |  |  |  |  |  |  |  | -3.2% | +0.0% |  |  |
| sinai_desert_noon_r1 | classic | 7.39 | 7.23 |  |  |  |  |  |  | 84.00 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r1 | remastered | 8.23 | 7.19 | 0.04 | 0.88 | 0.75 | 0.07 |  | 0.05 | 78.10 | 2,672 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r1 | delta | +0.84 | -0.04 |  |  |  |  |  |  |  | -0.6% | +0.0% |  |  |
| sinai_lowsun_fp_r2 | classic | 3.40 | 3.10 |  |  |  |  |  |  | 144.40 | 788 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r2 | remastered | 4.27 | 2.84 | 0.13 | 1.25 | 0.59 | 0.08 | 0.47 | 0.12 | 139.80 | 763 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r2 | delta | +0.87 | -0.26 |  |  |  |  |  |  |  | -3.2% | +0.0% |  |  |
| sinai_desert_noon_r2 | classic | 7.18 | 7.00 |  |  |  |  |  |  | 84.20 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r2 | remastered | 8.32 | 7.26 | 0.04 | 0.89 | 0.77 | 0.07 |  | 0.05 | 77.10 | 2,672 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r2 | delta | +1.14 | +0.26 |  |  |  |  |  |  |  | -0.6% | +0.0% |  |  |
| sinai_lowsun_fp_r3 | classic | 3.37 | 3.07 |  |  |  |  |  |  | 142.80 | 789 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r3 | remastered | 4.10 | 2.83 | 0.12 | 1.11 | 0.54 | 0.07 | 0.39 | 0.11 | 135.40 | 764 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r3 | delta | +0.73 | -0.24 |  |  |  |  |  |  |  | -3.2% | +0.0% |  |  |
| sinai_desert_noon_r3 | classic | 7.38 | 7.21 |  |  |  |  |  |  | 83.00 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r3 | remastered | 8.34 | 7.33 | 0.04 | 0.83 | 0.71 | 0.07 |  | 0.05 | 76.20 | 2,672 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r3 | delta | +0.96 | +0.12 |  |  |  |  |  |  |  | -0.6% | +0.0% |  |  |
