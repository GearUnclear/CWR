# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge_r1 | classic | 27.98 | 27.83 |  |  |  |  |  |  | 26.60 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge_r1 | remastered | 26.73 | 25.00 | 0.22 | 1.57 | 0.77 | 0.06 | 0.27 | 0.47 | 27.50 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r1 | delta | -1.25 | -2.83 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| forest_edge_golden_r1 | classic | 5.66 | 5.51 |  |  |  |  |  |  | 99.80 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden_r1 | remastered | 5.64 | 4.96 | 0.21 | 0.51 | 0.40 | 0.07 |  | 0.04 | 110.50 | 2,142 | 1,459 | OK:30 | OK |
| forest_edge_golden_r1 | delta | -0.02 | -0.55 |  |  |  |  |  |  |  | -14.4% | +0.0% |  |  |
| town_night_r1 | classic | 7.69 | 7.51 |  |  |  |  |  |  | 100.80 | 1,297 | 1,122 | OK:271 | OK |
| town_night_r1 | remastered | 9.50 | 8.49 |  | 0.83 | 0.72 | 0.07 |  | 0.04 | 85.60 | 1,281 | 1,109 | OK:48 | OK |
| town_night_r1 | delta | +1.81 | +0.98 |  |  |  |  |  |  |  | -1.2% | -1.2% |  |  |
| coastline_sunset_r1 | classic | 1.18 | 1.06 |  |  |  |  |  |  | 313.60 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r1 | remastered | 1.86 | 0.79 | 0.00 | 0.95 | 0.45 | 0.06 | 0.29 | 0.15 | 315.00 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r1 | delta | +0.68 | -0.27 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| woods_pine2_evening_r1 | classic | 7.74 | 7.59 |  |  |  |  |  |  | 83.70 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r1 | remastered | 10.97 | 9.00 | 0.70 | 1.81 | 0.81 | 0.07 | 0.54 | 0.39 | 78.20 | 2,842 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r1 | delta | +3.23 | +1.41 |  |  |  |  |  |  |  | -12.6% | +0.0% |  |  |
| lowsun_fp_r1 | classic | 4.35 | 4.22 |  |  |  |  |  |  | 120.30 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp_r1 | remastered | 6.03 | 4.76 | 0.07 | 1.12 | 0.42 | 0.06 | 0.36 | 0.27 | 115.50 | 1,890 | 1,316 | OK:30 | OK |
| lowsun_fp_r1 | delta | +1.68 | +0.54 |  |  |  |  |  |  |  | -9.6% | +0.0% |  |  |
| road_flat_drive_moonless_r1 | classic | 4.73 | 4.53 |  |  |  |  |  |  | 124.70 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r1 | remastered | 6.40 | 5.53 |  | 0.68 | 0.58 | 0.06 |  | 0.04 | 111.60 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r1 | delta | +1.67 | +1.00 |  |  |  |  |  |  |  | +0.0% | +0.0% |  |  |
| ffh_r1 | classic | 5.34 | 5.19 |  |  |  |  |  |  | 118.30 | 783 | 688 | UNSETTLED:300 | OK |
| ffh_r1 | remastered | 6.72 | 5.85 |  | 0.72 | 0.61 | 0.06 |  | 0.04 | 109.30 | 785 | 667 | UNSETTLED:300 | OK |
| ffh_r1 | delta | +1.38 | +0.66 |  |  |  |  |  |  |  | +0.3% | -3.1% |  |  |
| bay_offshore_r1 | classic | 7.73 | 7.59 |  |  |  |  |  |  | 60.10 | 4,562 | 2,961 | OK:30 | OK |
| bay_offshore_r1 | remastered | 13.11 | 12.29 | 0.03 | 0.68 | 0.57 | 0.06 |  | 0.04 | 56.80 | 4,559 | 2,961 | OK:30 | OK |
| bay_offshore_r1 | delta | +5.38 | +4.70 |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
| town_night_rain_r1 | classic | 6.30 | 6.14 |  |  |  |  |  |  | 120.60 | 1,065 | 838 | OK:220 | OK |
| town_night_rain_r1 | remastered | 7.01 | 6.08 |  | 0.75 | 0.65 | 0.07 |  | 0.04 | 109.70 | 1,048 | 811 | OK:50 | OK |
| town_night_rain_r1 | delta | +0.71 | -0.06 |  |  |  |  |  |  |  | -1.6% | -3.2% |  |  |
| pinewood_morn_r1 | classic | 6.41 | 6.24 |  |  |  |  |  |  | 97.60 | 3,163 | 1,678 | OK:30 | OK |
| pinewood_morn_r1 | remastered | 7.51 | 5.98 | 0.65 | 1.34 | 0.56 | 0.07 | 0.27 | 0.44 | 100.00 | 2,845 | 1,678 | OK:30 | OK |
| pinewood_morn_r1 | delta | +1.10 | -0.26 |  |  |  |  |  |  |  | -10.1% | +0.0% |  |  |
| valley_0515_r1 | classic | 34.15 | 34.01 |  |  |  |  |  |  | 24.80 | 11,553 | 7,966 | OK:30 | OK |
| valley_0515_r1 | remastered | 44.33 | 42.16 | 0.04 | 2.02 | 1.04 | 0.07 | 0.31 | 0.60 | 19.10 | 11,406 | 7,966 | OK:30 | OK |
| valley_0515_r1 | delta | +10.18 | +8.15 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| hill_1730_r1 | classic | 2.47 | 2.38 |  |  |  |  |  |  | 275.20 | 1,769 | 934 | OK:30 | OK |
| hill_1730_r1 | remastered | 3.27 | 2.62 | 0.03 | 0.55 | 0.43 | 0.07 |  | 0.04 | 255.70 | 1,662 | 934 | OK:30 | OK |
| hill_1730_r1 | delta | +0.80 | +0.24 |  |  |  |  |  |  |  | -6.0% | +0.0% |  |  |
| coast_oc_up_r1 | classic | 1.14 | 0.97 |  |  |  |  |  |  | 280.00 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r1 | remastered | 2.89 | 1.55 | 0.00 | 1.16 | 0.49 | 0.06 | 0.34 | 0.27 | 232.10 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r1 | delta | +1.75 | +0.58 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| sea_noon_r1 | classic | 1.28 | 1.19 |  |  |  |  |  |  | 292.20 | 179 | 0 | OK:30 | OK |
| sea_noon_r1 | remastered | 2.21 | 1.69 | 0.00 | 0.42 | 0.32 | 0.06 |  | 0.04 | 273.10 | 179 | 0 | OK:30 | OK |
| sea_noon_r1 | delta | +0.93 | +0.50 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| noon_fields_r1 | classic | 13.46 | 13.33 |  |  |  |  |  |  | 49.00 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields_r1 | remastered | 14.60 | 13.80 | 0.09 | 0.66 | 0.55 | 0.07 |  | 0.04 | 49.10 | 6,398 | 4,508 | OK:30 | OK |
| noon_fields_r1 | delta | +1.14 | +0.47 |  |  |  |  |  |  |  | -5.5% | +0.0% |  |  |
| sunrise_ridge_r2 | classic | 29.34 | 29.19 |  |  |  |  |  |  | 24.60 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge_r2 | remastered | 31.27 | 29.73 | 0.08 | 1.38 | 0.83 | 0.06 | 0.27 | 0.22 | 25.20 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r2 | delta | +1.93 | +0.54 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| forest_edge_golden_r2 | classic | 6.50 | 6.34 |  |  |  |  |  |  | 80.30 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden_r2 | remastered | 6.37 | 5.58 | 0.21 | 0.62 | 0.51 | 0.07 |  | 0.04 | 101.80 | 2,142 | 1,459 | OK:30 | OK |
| forest_edge_golden_r2 | delta | -0.13 | -0.76 |  |  |  |  |  |  |  | -14.4% | +0.0% |  |  |
| town_night_r2 | classic | 6.24 | 6.06 |  |  |  |  |  |  | 113.90 | 1,160 | 983 | OK:159 | OK |
| town_night_r2 | remastered | 8.94 | 7.70 |  | 1.04 | 0.94 | 0.07 |  | 0.04 | 100.20 | 1,180 | 994 | OK:31 | OK |
| town_night_r2 | delta | +2.70 | +1.64 |  |  |  |  |  |  |  | +1.7% | +1.1% |  |  |
| coastline_sunset_r2 | classic | 1.33 | 1.21 |  |  |  |  |  |  | 326.50 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r2 | remastered | 2.88 | 0.99 | 0.00 | 1.77 | 0.66 | 0.06 | 0.29 | 0.75 | 73.50 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r2 | delta | +1.55 | -0.22 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| woods_pine2_evening_r2 | classic | 6.02 | 5.88 |  |  |  |  |  |  | 62.50 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r2 | remastered | 13.38 | 10.44 | 1.71 | 2.78 | 0.59 | 0.08 | 0.68 | 1.44 | 70.70 | 2,842 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r2 | delta | +7.36 | +4.56 |  |  |  |  |  |  |  | -12.6% | +0.0% |  |  |
| lowsun_fp_r2 | classic | 5.71 | 5.57 |  |  |  |  |  |  | 97.10 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp_r2 | remastered | 7.62 | 6.10 | 0.22 | 1.37 | 0.45 | 0.06 | 0.37 | 0.49 | 97.90 | 1,890 | 1,316 | OK:30 | OK |
| lowsun_fp_r2 | delta | +1.91 | +0.53 |  |  |  |  |  |  |  | -9.6% | +0.0% |  |  |
| road_flat_drive_moonless_r2 | classic | 5.32 | 5.16 |  |  |  |  |  |  | 118.10 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r2 | remastered | 5.84 | 5.04 |  | 0.63 | 0.53 | 0.06 |  | 0.04 | 111.50 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r2 | delta | +0.52 | -0.12 |  |  |  |  |  |  |  | +0.0% | +0.0% |  |  |
| ffh_r2 | classic | 6.69 | 6.53 |  |  |  |  |  |  | 104.90 | 798 | 694 | UNSETTLED:300 | OK |
| ffh_r2 | remastered | 8.56 | 7.48 |  | 0.93 | 0.82 | 0.06 |  | 0.04 | 96.10 | 788 | 681 | UNSETTLED:300 | OK |
| ffh_r2 | delta | +1.87 | +0.95 |  |  |  |  |  |  |  | -1.3% | -1.9% |  |  |
| bay_offshore_r2 | classic | 9.28 | 9.14 |  |  |  |  |  |  | 69.20 | 4,562 | 2,961 | OK:30 | OK |
| bay_offshore_r2 | remastered | 12.22 | 11.50 | 0.03 | 0.58 | 0.47 | 0.06 |  | 0.04 | 59.30 | 4,559 | 2,961 | OK:30 | OK |
| bay_offshore_r2 | delta | +2.94 | +2.36 |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
| town_night_rain_r2 | classic | 4.58 | 4.41 |  |  |  |  |  |  | 136.50 | 826 | 553 | OK:163 | OK |
| town_night_rain_r2 | remastered | 5.78 | 4.91 |  | 0.70 | 0.59 | 0.07 |  | 0.04 | 132.70 | 827 | 558 | UNSETTLED:300 | OK |
| town_night_rain_r2 | delta | +1.20 | +0.50 |  |  |  |  |  |  |  | +0.1% | +0.9% |  |  |
| pinewood_morn_r2 | classic | 5.90 | 5.74 |  |  |  |  |  |  | 94.40 | 3,163 | 1,678 | OK:30 | OK |
| pinewood_morn_r2 | remastered | 7.65 | 6.40 | 0.77 | 1.07 | 0.41 | 0.07 | 0.27 | 0.32 | 91.60 | 2,845 | 1,678 | OK:30 | OK |
| pinewood_morn_r2 | delta | +1.75 | +0.66 |  |  |  |  |  |  |  | -10.1% | +0.0% |  |  |
| valley_0515_r2 | classic | 33.73 | 33.59 |  |  |  |  |  |  | 25.20 | 11,553 | 7,966 | OK:30 | OK |
| valley_0515_r2 | remastered | 37.45 | 35.36 | 0.04 | 1.94 | 1.01 | 0.07 | 0.31 | 0.55 | 21.30 | 11,406 | 7,966 | OK:30 | OK |
| valley_0515_r2 | delta | +3.72 | +1.77 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| hill_1730_r2 | classic | 1.99 | 1.90 |  |  |  |  |  |  | 262.80 | 1,769 | 934 | OK:30 | OK |
| hill_1730_r2 | remastered | 2.43 | 1.76 | 0.03 | 0.57 | 0.46 | 0.07 |  | 0.04 | 216.70 | 1,662 | 934 | OK:30 | OK |
| hill_1730_r2 | delta | +0.44 | -0.14 |  |  |  |  |  |  |  | -6.0% | +0.0% |  |  |
| coast_oc_up_r2 | classic | 1.37 | 1.20 |  |  |  |  |  |  | 291.70 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r2 | remastered | 2.83 | 1.12 | 0.00 | 1.52 | 0.38 | 0.06 | 0.34 | 0.74 | 256.30 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r2 | delta | +1.46 | -0.08 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| sea_noon_r2 | classic | 1.21 | 1.11 |  |  |  |  |  |  | 299.00 | 179 | 0 | OK:30 | OK |
| sea_noon_r2 | remastered | 1.40 | 0.78 | 0.00 | 0.52 | 0.42 | 0.06 |  | 0.04 | 298.10 | 179 | 0 | OK:30 | OK |
| sea_noon_r2 | delta | +0.19 | -0.33 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| noon_fields_r2 | classic | 11.84 | 11.72 |  |  |  |  |  |  | 53.30 | 6,768 | 4,508 | OK:30 | OK |
| noon_fields_r2 | remastered | 14.41 | 13.65 | 0.17 | 0.62 | 0.52 | 0.06 |  | 0.04 | 51.60 | 6,399 | 4,508 | OK:30 | OK |
| noon_fields_r2 | delta | +2.57 | +1.93 |  |  |  |  |  |  |  | -5.5% | +0.0% |  |  |
| sunrise_ridge_r3 | classic | 27.24 | 27.09 |  |  |  |  |  |  | 27.20 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge_r3 | remastered | 27.84 | 26.31 | 0.08 | 1.36 | 0.81 | 0.06 | 0.27 | 0.22 | 24.70 | 12,910 | 8,897 | OK:30 | OK |
| sunrise_ridge_r3 | delta | +0.60 | -0.78 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| forest_edge_golden_r3 | classic | 5.32 | 5.16 |  |  |  |  |  |  | 119.40 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden_r3 | remastered | 7.10 | 6.30 | 0.21 | 0.63 | 0.52 | 0.07 |  | 0.04 | 95.10 | 2,142 | 1,459 | OK:30 | OK |
| forest_edge_golden_r3 | delta | +1.78 | +1.14 |  |  |  |  |  |  |  | -14.4% | +0.0% |  |  |
| town_night_r3 | classic | 7.47 | 7.29 |  |  |  |  |  |  | 102.40 | 1,188 | 1,017 | OK:179 | OK |
| town_night_r3 | remastered | 7.37 | 6.46 |  | 0.72 | 0.61 | 0.07 |  | 0.04 | 91.10 | 1,179 | 1,008 | OK:52 | OK |
| town_night_r3 | delta | -0.10 | -0.83 |  |  |  |  |  |  |  | -0.8% | -0.9% |  |  |
| coastline_sunset_r3 | classic | 1.71 | 1.59 |  |  |  |  |  |  | 317.30 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r3 | remastered | 3.80 | 1.25 | 0.00 | 2.42 | 0.42 | 0.06 | 0.29 | 1.65 | 245.30 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r3 | delta | +2.09 | -0.34 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| woods_pine2_evening_r3 | classic | 8.99 | 8.84 |  |  |  |  |  |  | 66.50 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r3 | remastered | 19.56 | 16.88 | 1.50 | 2.23 | 0.65 | 0.22 | 0.50 | 0.86 | 52.50 | 2,842 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r3 | delta | +10.57 | +8.04 |  |  |  |  |  |  |  | -12.6% | +0.0% |  |  |
| lowsun_fp_r3 | classic | 4.80 | 4.67 |  |  |  |  |  |  | 115.70 | 2,092 | 1,316 | OK:30 | OK |
| lowsun_fp_r3 | remastered | 6.65 | 5.33 | 0.07 | 1.17 | 0.53 | 0.06 | 0.37 | 0.22 | 103.50 | 1,891 | 1,316 | OK:30 | OK |
| lowsun_fp_r3 | delta | +1.85 | +0.66 |  |  |  |  |  |  |  | -9.6% | +0.0% |  |  |
| road_flat_drive_moonless_r3 | classic | 5.24 | 5.08 |  |  |  |  |  |  | 111.40 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r3 | remastered | 9.74 | 8.66 |  | 0.91 | 0.80 | 0.06 |  | 0.04 | 72.60 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r3 | delta | +4.50 | +3.58 |  |  |  |  |  |  |  | +0.0% | +0.0% |  |  |
| ffh_r3 | classic | 22.09 | 21.94 |  |  |  |  |  |  | 44.70 | 808 | 700 | UNSETTLED:300 | OK |
| ffh_r3 | remastered | 6.30 | 5.27 |  | 0.89 | 0.78 | 0.06 |  | 0.04 | 135.20 | 790 | 675 | UNSETTLED:300 | OK |
| ffh_r3 | delta | -15.79 | -16.67 |  |  |  |  |  |  |  | -2.2% | -3.6% |  |  |
| bay_offshore_r3 | classic | 7.68 | 7.54 |  |  |  |  |  |  | 76.70 | 4,562 | 2,961 | OK:30 | OK |
| bay_offshore_r3 | remastered | 10.37 | 9.37 | 0.05 | 0.86 | 0.75 | 0.07 |  | 0.04 | 64.40 | 4,559 | 2,961 | OK:30 | OK |
| bay_offshore_r3 | delta | +2.69 | +1.83 |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
| town_night_rain_r3 | classic | 4.39 | 4.08 |  |  |  |  |  |  | 159.80 | 818 | 555 | OK:59 | OK |
| town_night_rain_r3 | remastered | 6.32 | 5.15 |  | 0.85 | 0.62 | 0.13 |  | 0.09 | 129.90 | 829 | 564 | OK:105 | OK |
| town_night_rain_r3 | delta | +1.93 | +1.07 |  |  |  |  |  |  |  | +1.3% | +1.6% |  |  |
| pinewood_morn_r3 | classic | 5.50 | 5.22 |  |  |  |  |  |  | 117.00 | 3,163 | 1,678 | OK:30 | OK |
| pinewood_morn_r3 | remastered | 6.47 | 5.18 | 0.65 | 1.06 | 0.47 | 0.09 | 0.41 | 0.08 | 111.30 | 2,845 | 1,678 | OK:30 | OK |
| pinewood_morn_r3 | delta | +0.97 | -0.04 |  |  |  |  |  |  |  | -10.1% | +0.0% |  |  |
| valley_0515_r3 | classic | 19.69 | 19.55 |  |  |  |  |  |  | 40.40 | 11,553 | 7,966 | OK:30 | OK |
| valley_0515_r3 | remastered | 24.46 | 23.07 | 0.05 | 1.24 | 0.74 | 0.07 | 0.31 | 0.13 | 30.80 | 11,406 | 7,966 | OK:30 | OK |
| valley_0515_r3 | delta | +4.77 | +3.52 |  |  |  |  |  |  |  | -1.3% | +0.0% |  |  |
| hill_1730_r3 | classic | 3.50 | 2.91 |  |  |  |  |  |  | 281.00 | 1,769 | 934 | OK:30 | OK |
| hill_1730_r3 | remastered | 2.18 | 1.60 | 0.05 | 0.46 | 0.31 | 0.09 |  | 0.06 | 275.70 | 1,662 | 934 | OK:30 | OK |
| hill_1730_r3 | delta | -1.32 | -1.31 |  |  |  |  |  |  |  | -6.0% | +0.0% |  |  |
| coast_oc_up_r3 | classic | 2.29 | 1.77 |  |  |  |  |  |  | 249.20 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r3 | remastered | 1.95 | 0.71 | 0.00 | 1.01 | 0.37 | 0.08 | 0.47 | 0.09 | 318.10 | 186 | 0 | OK:30 | OK |
| coast_oc_up_r3 | delta | -0.34 | -1.06 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| sea_noon_r3 | classic | 2.57 | 2.29 |  |  |  |  |  |  | 233.10 | 179 | 0 | OK:30 | OK |
| sea_noon_r3 | remastered | 1.58 | 0.86 | 0.00 | 0.59 | 0.45 | 0.08 |  | 0.06 | 202.90 | 179 | 0 | OK:30 | OK |
| sea_noon_r3 | delta | -0.99 | -1.43 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| noon_fields_r3 | classic | 7.33 | 7.21 |  |  |  |  |  |  | 78.70 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields_r3 | remastered | 9.01 | 8.47 | 0.07 | 0.43 | 0.33 | 0.06 |  | 0.04 | 72.90 | 6,398 | 4,508 | OK:30 | OK |
| noon_fields_r3 | delta | +1.68 | +1.26 |  |  |  |  |  |  |  | -5.5% | +0.0% |  |  |
| overcast_broken_r1 | classic | 10.69 | 10.56 |  |  |  |  |  |  | 57.60 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken_r1 | remastered | 11.63 | 10.90 | 0.19 | 0.61 | 0.51 | 0.06 |  | 0.04 | 56.10 | 8,066 | 5,692 | OK:30 | OK |
| overcast_broken_r1 | delta | +0.94 | +0.34 |  |  |  |  |  |  |  | -2.1% | +0.0% |  |  |
| broken_high_r1 | classic | 16.53 | 16.43 |  |  |  |  |  |  | 44.70 | 10,789 | 7,549 | OK:30 | OK |
| broken_high_r1 | remastered | 24.40 | 23.58 | 0.03 | 0.73 | 0.62 | 0.07 |  | 0.04 | 36.80 | 10,785 | 7,549 | OK:30 | OK |
| broken_high_r1 | delta | +7.87 | +7.15 |  |  |  |  |  |  |  | -0.0% | +0.0% |  |  |
| overcast_broken_r2 | classic | 10.51 | 10.38 |  |  |  |  |  |  | 58.50 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken_r2 | remastered | 11.52 | 10.77 | 0.17 | 0.63 | 0.53 | 0.06 |  | 0.04 | 55.90 | 8,066 | 5,692 | OK:30 | OK |
| overcast_broken_r2 | delta | +1.01 | +0.39 |  |  |  |  |  |  |  | -2.1% | +0.0% |  |  |
| broken_high_r2 | classic | 17.73 | 17.63 |  |  |  |  |  |  | 42.90 | 10,789 | 7,549 | OK:30 | OK |
| broken_high_r2 | remastered | 25.31 | 24.39 | 0.03 | 0.83 | 0.72 | 0.07 |  | 0.04 | 35.20 | 10,785 | 7,549 | OK:30 | OK |
| broken_high_r2 | delta | +7.58 | +6.76 |  |  |  |  |  |  |  | -0.0% | +0.0% |  |  |
| overcast_broken_r3 | classic | 10.62 | 10.48 |  |  |  |  |  |  | 58.90 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken_r3 | remastered | 12.06 | 11.31 | 0.18 | 0.62 | 0.52 | 0.06 |  | 0.04 | 53.30 | 8,066 | 5,692 | OK:30 | OK |
| overcast_broken_r3 | delta | +1.44 | +0.83 |  |  |  |  |  |  |  | -2.1% | +0.0% |  |  |
| broken_high_r3 | classic | 17.81 | 17.70 |  |  |  |  |  |  | 42.60 | 10,789 | 7,549 | OK:30 | OK |
| broken_high_r3 | remastered | 25.11 | 24.20 | 0.03 | 0.81 | 0.70 | 0.07 |  | 0.04 | 35.30 | 10,785 | 7,549 | OK:30 | OK |
| broken_high_r3 | delta | +7.30 | +6.50 |  |  |  |  |  |  |  | -0.0% | +0.0% |  |  |
| sinai_lowsun_fp_r1 | classic | 3.95 | 3.65 |  |  |  |  |  |  | 134.40 | 789 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r1 | remastered | 5.59 | 3.30 | 0.21 | 2.00 | 0.74 | 0.17 | 0.92 | 0.18 | 127.20 | 764 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r1 | delta | +1.64 | -0.35 |  |  |  |  |  |  |  | -3.2% | +0.0% |  |  |
| sinai_desert_noon_r1 | classic | 7.16 | 7.00 |  |  |  |  |  |  | 85.50 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r1 | remastered | 8.43 | 7.41 | 0.05 | 0.86 | 0.73 | 0.08 |  | 0.05 | 76.60 | 2,672 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r1 | delta | +1.27 | +0.41 |  |  |  |  |  |  |  | -0.6% | +0.0% |  |  |
| sinai_lowsun_fp_r2 | classic | 4.63 | 3.94 |  |  |  |  |  |  | 136.10 | 789 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r2 | remastered | 4.53 | 2.93 | 0.15 | 1.42 | 0.67 | 0.09 | 0.55 | 0.11 | 139.80 | 764 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r2 | delta | -0.10 | -1.01 |  |  |  |  |  |  |  | -3.2% | +0.0% |  |  |
| sinai_desert_noon_r2 | classic | 7.68 | 7.49 |  |  |  |  |  |  | 81.30 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r2 | remastered | 8.28 | 7.29 | 0.05 | 0.83 | 0.70 | 0.08 |  | 0.05 | 78.10 | 2,672 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r2 | delta | +0.60 | -0.20 |  |  |  |  |  |  |  | -0.6% | +0.0% |  |  |
| sinai_lowsun_fp_r3 | classic | 4.76 | 4.07 |  |  |  |  |  |  | 119.00 | 789 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r3 | remastered | 4.22 | 2.93 | 0.12 | 1.12 | 0.58 | 0.07 | 0.40 | 0.07 | 139.40 | 764 | 371 | OK:30 | OK |
| sinai_lowsun_fp_r3 | delta | -0.54 | -1.14 |  |  |  |  |  |  |  | -3.2% | +0.0% |  |  |
| sinai_desert_noon_r3 | classic | 7.60 | 7.43 |  |  |  |  |  |  | 80.70 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r3 | remastered | 8.54 | 7.52 | 0.04 | 0.85 | 0.74 | 0.07 |  | 0.04 | 74.40 | 2,672 | 1,363 | OK:30 | OK |
| sinai_desert_noon_r3 | delta | +0.94 | +0.09 |  |  |  |  |  |  |  | -0.6% | +0.0% |  |  |
