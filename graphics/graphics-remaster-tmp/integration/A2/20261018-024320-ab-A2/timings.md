# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| perf_sunrise_ridge | off | 19.61 | 18.44 | 0.10 | 1.01 | 0.62 | 0.06 | 0.27 | 0.06 | 36.10 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | on | 18.21 | 17.02 | 0.08 | 1.03 | 0.60 | 0.06 | 0.27 | 0.10 | 37.30 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | delta | -1.40 | -1.42 | -0.02 | +0.02 | -0.02 | +0.00 | +0.00 | +0.04 |  | +0.0% | +0.0% |  |  |
| temporal_forest_edge_golden | off | 5.95 | 5.18 | 0.26 | 0.58 | 0.44 | 0.08 |  | 0.05 | 118.50 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | on | 6.02 | 5.26 | 0.29 | 0.55 | 0.41 | 0.09 |  | 0.06 | 115.70 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | delta | +0.07 | +0.08 | +0.03 | -0.03 | -0.03 | +0.01 |  | +0.01 |  | +0.0% | +0.0% |  |  |
| lighttiles_town_night | off | 6.42 | 5.59 |  | 0.62 | 0.50 | 0.07 |  | 0.05 | 120.60 | 1,290 | 1,126 | OK:209 | OK |
| lighttiles_town_night | on | 6.48 | 5.66 |  | 0.61 | 0.49 | 0.07 |  | 0.05 | 124.30 | 1,290 | 1,128 | OK:85 | OK |
| lighttiles_town_night | delta | +0.06 | +0.07 |  | -0.01 | -0.01 | +0.00 |  | +0.00 |  | +0.0% | +0.2% |  |  |
| atmosphere_coastline_sunset | off | 4.15 | 1.35 | 0.00 | 2.54 | 0.66 | 0.16 | 1.31 | 0.41 | 278.90 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | on | 4.63 | 1.81 | 0.00 | 2.53 | 0.64 | 0.21 | 1.40 | 0.28 | 248.00 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | delta | +0.48 | +0.46 | +0.00 | -0.01 | -0.02 | +0.05 | +0.09 | -0.13 |  | +0.0% |  |  |  |
| shadows_woods_pine2_evening | off | 6.45 | 5.20 | 0.36 | 1.10 | 0.46 | 0.07 | 0.50 | 0.07 | 111.40 | 2,831 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | on | 6.38 | 5.14 | 0.34 | 1.08 | 0.44 | 0.07 | 0.50 | 0.07 | 111.30 | 2,831 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | delta | -0.07 | -0.06 | -0.02 | -0.02 | -0.02 | +0.00 | +0.00 | +0.00 |  | +0.0% | +0.0% |  |  |
| shade_lowsun_fp | off | 5.78 | 4.48 | 0.09 | 1.12 | 0.44 | 0.08 | 0.48 | 0.12 | 126.30 | 1,890 | 1,316 | OK:30 | OK |
| shade_lowsun_fp | on | 6.02 | 4.69 | 0.09 | 1.15 | 0.44 | 0.09 | 0.51 | 0.10 | 121.20 | 1,890 | 1,316 | OK:30 | OK |
| shade_lowsun_fp | delta | +0.24 | +0.21 | +0.00 | +0.03 | +0.00 | +0.01 | +0.03 | -0.02 |  | +0.0% | +0.0% |  |  |
| night_road_flat_drive_moonless | off | 5.95 | 5.11 |  | 0.63 | 0.48 | 0.09 |  | 0.06 | 123.10 | 1,806 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | on | 5.89 | 5.06 |  | 0.63 | 0.48 | 0.09 |  | 0.06 | 121.70 | 1,806 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | delta | -0.06 | -0.05 |  | +0.00 | +0.00 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| combat_ffh | off | 6.25 | 5.36 |  | 0.71 | 0.57 | 0.08 |  | 0.05 | 128.40 | 812 | 708 | UNSETTLED:300 | OK |
| combat_ffh | on | 6.15 | 5.26 |  | 0.71 | 0.57 | 0.08 |  | 0.06 | 127.90 | 795 | 683 | UNSETTLED:300 | OK |
| combat_ffh | delta | -0.10 | -0.10 |  | +0.00 | +0.00 | +0.00 |  | +0.01 |  | -2.1% | -3.5% |  |  |
| water_coastline_sunset | off | 3.73 | 1.31 | 0.00 | 2.16 | 0.65 | 0.31 | 0.85 | 0.35 | 277.10 | 176 | 0 | OK:30 | OK |
| water_coastline_sunset | on | 4.48 | 1.80 | 0.00 | 2.39 | 0.75 | 0.23 | 1.07 | 0.35 | 250.40 | 176 | 0 | OK:30 | OK |
| water_coastline_sunset | delta | +0.75 | +0.49 | +0.00 | +0.23 | +0.10 | -0.08 | +0.22 | +0.00 |  | +0.0% |  |  |  |
| ssr_bay_offshore | off | 7.58 | 6.94 | 0.03 | 0.50 | 0.39 | 0.06 |  | 0.04 | 82.10 | 4,559 | 2,961 | OK:30 | OK |
| ssr_bay_offshore | on | 7.61 | 6.98 | 0.03 | 0.47 | 0.37 | 0.06 |  | 0.04 | 82.80 | 4,559 | 2,961 | OK:30 | OK |
| ssr_bay_offshore | delta | +0.03 | +0.04 | +0.00 | -0.03 | -0.02 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| materials_town_night_rain | off | 6.36 | 5.45 |  | 0.70 | 0.56 | 0.08 |  | 0.06 | 135.40 | 1,038 | 802 | UNSETTLED:300 | OK |
| materials_town_night_rain | on | 6.47 | 5.65 |  | 0.62 | 0.49 | 0.08 |  | 0.05 | 132.20 | 1,112 | 889 | OK:212 | OK |
| materials_town_night_rain | delta | +0.11 | +0.20 |  | -0.08 | -0.07 | +0.00 |  | -0.01 |  | +6.7% | +9.8% |  |  |
| airbeams_pinewood_morn | off | 6.10 | 4.87 | 0.61 | 1.02 | 0.49 | 0.09 | 0.36 | 0.08 | 116.00 | 2,845 | 1,678 | OK:30 | OK |
| airbeams_pinewood_morn | on | 5.98 | 4.80 | 0.58 | 0.97 | 0.43 | 0.09 | 0.36 | 0.09 | 115.30 | 2,845 | 1,678 | OK:30 | OK |
| airbeams_pinewood_morn | delta | -0.12 | -0.07 | -0.03 | -0.05 | -0.06 | +0.00 | +0.00 | +0.01 |  | +0.0% | +0.0% |  |  |
| airhaze_valley_0515 | off | 23.99 | 22.64 | 0.04 | 1.21 | 0.74 | 0.07 | 0.31 | 0.09 | 31.20 | 11,406 | 7,966 | OK:30 | OK |
| airhaze_valley_0515 | on | 23.89 | 22.59 | 0.04 | 1.15 | 0.69 | 0.07 | 0.31 | 0.08 | 31.50 | 11,406 | 7,966 | OK:30 | OK |
| airhaze_valley_0515 | delta | -0.10 | -0.05 | +0.00 | -0.06 | -0.05 | +0.00 | +0.00 | -0.01 |  | +0.0% | +0.0% |  |  |
| hfrt_hill_1730 | off | 3.49 | 2.61 | 0.11 | 0.69 | 0.41 | 0.17 |  | 0.12 | 277.80 | 1,662 | 934 | OK:30 | OK |
| hfrt_hill_1730 | on | 5.14 | 3.52 | 0.13 | 1.27 | 0.66 | 0.41 |  | 0.21 | 287.40 | 1,662 | 934 | OK:30 | OK |
| hfrt_hill_1730 | delta | +1.65 | +0.91 | +0.02 | +0.58 | +0.25 | +0.24 |  | +0.09 |  | +0.0% | +0.0% |  |  |
| clouds_coast_oc_up | off | 3.84 | 1.52 | 0.00 | 1.91 | 0.47 | 0.19 | 1.04 | 0.21 | 253.70 | 186 | 0 | OK:30 | OK |
| clouds_coast_oc_up | on | 4.77 | 1.77 | 0.00 | 2.53 | 0.52 | 0.21 | 1.33 | 0.47 | 229.50 | 186 | 0 | OK:30 | OK |
| clouds_coast_oc_up | delta | +0.93 | +0.25 | +0.00 | +0.62 | +0.05 | +0.02 | +0.29 | +0.26 |  | +0.0% |  |  |  |
| texsharp_forest_edge | off | 5.96 | 5.16 | 0.33 | 0.59 | 0.44 | 0.09 |  | 0.06 | 116.90 | 2,076 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | on | 6.00 | 5.24 | 0.29 | 0.56 | 0.41 | 0.09 |  | 0.06 | 113.90 | 2,076 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | delta | +0.04 | +0.08 | -0.04 | -0.03 | -0.03 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
