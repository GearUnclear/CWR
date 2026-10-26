# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| perf_sunrise_ridge | off | 17.97 | 16.81 | 0.08 | 1.00 | 0.59 | 0.06 | 0.27 | 0.08 | 36.90 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | on | 17.59 | 16.45 | 0.08 | 0.98 | 0.56 | 0.06 | 0.27 | 0.10 | 37.70 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | delta | -0.38 | -0.36 | +0.00 | -0.02 | -0.03 | +0.00 | +0.00 | +0.02 |  | +0.0% | +0.0% |  |  |
| temporal_forest_edge_golden | off | 5.55 | 4.83 | 0.26 | 0.53 | 0.39 | 0.08 |  | 0.05 | 123.70 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | on | 5.87 | 5.09 | 0.30 | 0.58 | 0.43 | 0.09 |  | 0.06 | 116.40 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | delta | +0.32 | +0.26 | +0.04 | +0.05 | +0.04 | +0.01 |  | +0.01 |  | +0.0% | +0.0% |  |  |
| lighttiles_town_night | off | 6.19 | 5.31 |  | 0.66 | 0.53 | 0.08 |  | 0.05 | 123.70 | 1,204 | 1,025 | OK:163 | OK |
| lighttiles_town_night | on | 6.22 | 5.31 |  | 0.69 | 0.56 | 0.08 |  | 0.05 | 125.00 | 1,219 | 1,038 | OK:30 | OK |
| lighttiles_town_night | delta | +0.03 | +0.00 |  | +0.03 | +0.03 | +0.00 |  | +0.00 |  | +1.2% | +1.3% |  |  |
| atmosphere_coastline_sunset | off | 3.90 | 1.44 | 0.00 | 2.19 | 0.58 | 0.16 | 1.21 | 0.24 | 270.10 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | on | 5.04 | 1.91 | 0.00 | 2.84 | 0.70 | 0.27 | 1.45 | 0.42 | 240.30 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | delta | +1.14 | +0.47 | +0.00 | +0.65 | +0.12 | +0.11 | +0.24 | +0.18 |  | +0.0% |  |  |  |
| shadows_woods_pine2_evening | off | 6.47 | 5.21 | 0.34 | 1.10 | 0.46 | 0.07 | 0.49 | 0.08 | 111.10 | 2,831 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | on | 6.39 | 5.09 | 0.35 | 1.14 | 0.45 | 0.07 | 0.49 | 0.13 | 111.90 | 2,831 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | delta | -0.08 | -0.12 | +0.01 | +0.04 | -0.01 | +0.00 | +0.00 | +0.05 |  | +0.0% | +0.0% |  |  |
| shade_lowsun_fp | off | 5.84 | 4.55 | 0.09 | 1.12 | 0.42 | 0.08 | 0.52 | 0.10 | 125.40 | 1,890 | 1,316 | OK:30 | OK |
| shade_lowsun_fp | on | 6.05 | 4.68 | 0.09 | 1.19 | 0.46 | 0.09 | 0.51 | 0.13 | 119.40 | 1,890 | 1,316 | OK:30 | OK |
| shade_lowsun_fp | delta | +0.21 | +0.13 | +0.00 | +0.07 | +0.04 | +0.01 | -0.01 | +0.03 |  | +0.0% | +0.0% |  |  |
| night_road_flat_drive_moonless | off | 5.97 | 5.11 |  | 0.65 | 0.50 | 0.09 |  | 0.06 | 123.60 | 1,806 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | on | 5.87 | 4.99 |  | 0.68 | 0.50 | 0.12 |  | 0.06 | 123.50 | 1,806 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | delta | -0.10 | -0.12 |  | +0.03 | +0.00 | +0.03 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| combat_ffh | off | 6.16 | 5.28 |  | 0.70 | 0.57 | 0.08 |  | 0.06 | 127.70 | 799 | 701 | UNSETTLED:300 | OK |
| combat_ffh | on | 6.05 | 5.15 |  | 0.71 | 0.57 | 0.08 |  | 0.06 | 127.10 | 774 | 660 | UNSETTLED:300 | OK |
| combat_ffh | delta | -0.11 | -0.13 |  | +0.01 | +0.00 | +0.00 |  | +0.00 |  | -3.1% | -5.8% |  |  |
| water_coastline_sunset | off | 3.97 | 1.62 | 0.00 | 2.09 | 0.71 | 0.23 | 0.89 | 0.26 | 272.00 | 176 | 0 | OK:30 | OK |
| water_coastline_sunset | on | 4.43 | 1.76 | 0.00 | 2.38 | 0.79 | 0.22 | 1.10 | 0.28 | 251.00 | 176 | 0 | OK:30 | OK |
| water_coastline_sunset | delta | +0.46 | +0.14 | +0.00 | +0.29 | +0.08 | -0.01 | +0.21 | +0.02 |  | +0.0% |  |  |  |
| ssr_bay_offshore | off | 7.54 | 6.91 | 0.03 | 0.49 | 0.39 | 0.06 |  | 0.04 | 83.60 | 4,559 | 2,961 | OK:30 | OK |
| ssr_bay_offshore | on | 7.54 | 6.93 | 0.03 | 0.46 | 0.36 | 0.06 |  | 0.04 | 83.20 | 4,559 | 2,961 | OK:30 | OK |
| ssr_bay_offshore | delta | +0.00 | +0.02 | +0.00 | -0.03 | -0.03 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| materials_town_night_rain | off | 6.01 | 5.09 |  | 0.70 | 0.56 | 0.09 |  | 0.06 | 137.90 | 936 | 694 | OK:91 | OK |
| materials_town_night_rain | on | 6.37 | 5.43 |  | 0.72 | 0.57 | 0.09 |  | 0.06 | 134.70 | 1,035 | 819 | OK:102 | OK |
| materials_town_night_rain | delta | +0.36 | +0.34 |  | +0.02 | +0.01 | +0.00 |  | +0.00 |  | +9.6% | +15.3% |  |  |
| airbeams_pinewood_morn | off | 6.01 | 4.78 | 0.62 | 1.02 | 0.48 | 0.09 | 0.36 | 0.09 | 115.40 | 2,845 | 1,678 | OK:30 | OK |
| airbeams_pinewood_morn | on | 6.17 | 4.99 | 0.61 | 0.96 | 0.44 | 0.09 | 0.37 | 0.08 | 116.10 | 2,845 | 1,678 | OK:30 | OK |
| airbeams_pinewood_morn | delta | +0.16 | +0.21 | -0.01 | -0.06 | -0.04 | +0.00 | +0.01 | -0.01 |  | +0.0% | +0.0% |  |  |
| airhaze_valley_0515 | off | 23.88 | 22.54 | 0.04 | 1.19 | 0.74 | 0.07 | 0.31 | 0.08 | 31.30 | 11,406 | 7,966 | OK:30 | OK |
| airhaze_valley_0515 | on | 24.10 | 22.74 | 0.04 | 1.21 | 0.75 | 0.07 | 0.31 | 0.09 | 31.20 | 11,406 | 7,966 | OK:30 | OK |
| airhaze_valley_0515 | delta | +0.22 | +0.20 | +0.00 | +0.02 | +0.01 | +0.00 | +0.00 | +0.01 |  | +0.0% | +0.0% |  |  |
| hfrt_hill_1730 | off | 3.26 | 2.42 | 0.10 | 0.66 | 0.37 | 0.19 |  | 0.11 | 284.70 | 1,662 | 934 | OK:30 | OK |
| hfrt_hill_1730 | on | 1.93 | 1.40 | 0.05 | 0.42 | 0.26 | 0.10 |  | 0.06 | 289.70 | 1,662 | 934 | OK:30 | OK |
| hfrt_hill_1730 | delta | -1.33 | -1.02 | -0.05 | -0.24 | -0.11 | -0.09 |  | -0.05 |  | +0.0% | +0.0% |  |  |
| clouds_coast_oc_up | off | 4.31 | 1.57 | 0.00 | 2.30 | 0.55 | 0.19 | 1.14 | 0.42 | 240.00 | 186 | 0 | OK:30 | OK |
| clouds_coast_oc_up | on | 4.89 | 1.83 | 0.00 | 2.47 | 0.59 | 0.22 | 1.39 | 0.28 | 221.30 | 186 | 0 | OK:30 | OK |
| clouds_coast_oc_up | delta | +0.58 | +0.26 | +0.00 | +0.17 | +0.04 | +0.03 | +0.25 | -0.14 |  | +0.0% |  |  |  |
| texsharp_forest_edge | off | 5.55 | 4.81 | 0.29 | 0.54 | 0.40 | 0.08 |  | 0.05 | 120.20 | 2,076 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | on | 5.89 | 5.15 | 0.31 | 0.54 | 0.40 | 0.09 |  | 0.06 | 115.60 | 2,076 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | delta | +0.34 | +0.34 | +0.02 | +0.00 | +0.00 | +0.01 |  | +0.01 |  | +0.0% | +0.0% |  |  |
