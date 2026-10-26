# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| perf_sunrise_ridge | off | 25.02 | 23.81 | 0.20 | 1.04 | 0.60 | 0.06 | 0.27 | 0.12 | 29.70 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | on | 34.31 | 32.70 | 0.11 | 1.45 | 0.67 | 0.06 | 0.27 | 0.45 | 24.60 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | delta | +9.29 | +8.89 | -0.09 | +0.41 | +0.07 | +0.00 | +0.00 | +0.33 |  | +0.0% | +0.0% |  |  |
| temporal_forest_edge_golden | off | 7.38 | 6.75 | 0.21 | 0.46 | 0.35 | 0.07 |  | 0.04 | 97.40 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | on | 7.44 | 6.79 | 0.21 | 0.48 | 0.37 | 0.07 |  | 0.04 | 96.80 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | delta | +0.06 | +0.04 | +0.00 | +0.02 | +0.02 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| lighttiles_town_night | off | 10.70 | 9.96 |  | 0.55 | 0.44 | 0.06 |  | 0.04 | 92.20 | 1,295 | 1,125 | OK:240 | OK |
| lighttiles_town_night | on | 6.02 | 5.28 |  | 0.55 | 0.45 | 0.06 |  | 0.04 | 124.90 | 1,294 | 1,125 | OK:30 | OK |
| lighttiles_town_night | delta | -4.68 | -4.68 |  | +0.00 | +0.01 | +0.00 |  | +0.00 |  | -0.1% | +0.0% |  |  |
| atmosphere_coastline_sunset | off | 2.04 | 0.70 | 0.00 | 1.20 | 0.43 | 0.08 | 0.54 | 0.15 | 372.00 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | on | 2.70 | 0.99 | 0.00 | 1.51 | 0.48 | 0.12 | 0.71 | 0.20 | 324.10 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | delta | +0.66 | +0.29 | +0.00 | +0.31 | +0.05 | +0.04 | +0.17 | +0.05 |  | +0.0% |  |  |  |
| shadows_woods_pine2_evening | off | 9.70 | 8.45 | 0.37 | 1.10 | 0.47 | 0.07 | 0.49 | 0.07 | 95.50 | 2,831 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | on | 7.83 | 6.12 | 0.49 | 1.56 | 0.42 | 0.13 | 0.64 | 0.37 | 100.20 | 2,831 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | delta | -1.87 | -2.33 | +0.12 | +0.46 | -0.05 | +0.06 | +0.15 | +0.30 |  | +0.0% | +0.0% |  |  |
| shade_lowsun_fp | off | 6.06 | 4.89 | 0.07 | 1.04 | 0.37 | 0.06 | 0.36 | 0.24 | 126.70 | 1,890 | 1,316 | OK:30 | OK |
| shade_lowsun_fp | on | 4.55 | 3.53 | 0.07 | 0.89 | 0.36 | 0.06 | 0.36 | 0.10 | 141.50 | 1,890 | 1,316 | OK:30 | OK |
| shade_lowsun_fp | delta | -1.51 | -1.36 | +0.00 | -0.15 | -0.01 | +0.00 | +0.00 | -0.14 |  | +0.0% | +0.0% |  |  |
| night_road_flat_drive_moonless | off | 4.24 | 3.56 |  | 0.50 | 0.40 | 0.06 |  | 0.04 | 145.20 | 1,806 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | on | 5.01 | 4.34 |  | 0.50 | 0.40 | 0.06 |  | 0.04 | 132.80 | 1,806 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | delta | +0.77 | +0.78 |  | +0.00 | +0.00 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| combat_ffh | off | 5.52 | 4.75 |  | 0.56 | 0.46 | 0.06 |  | 0.04 | 152.00 | 807 | 682 | UNSETTLED:300 | OK |
| combat_ffh | on | 4.94 | 4.14 |  | 0.64 | 0.54 | 0.06 |  | 0.04 | 148.90 | 792 | 675 | UNSETTLED:300 | OK |
| combat_ffh | delta | -0.58 | -0.61 |  | +0.08 | +0.08 | +0.00 |  | +0.00 |  | -1.9% | -1.0% |  |  |
| water_coastline_sunset | off | 2.40 | 0.80 | 0.00 | 1.40 | 0.53 | 0.11 | 0.50 | 0.26 | 327.40 | 176 | 0 | OK:30 | OK |
| water_coastline_sunset | on | 2.08 | 0.96 | 0.00 | 1.00 | 0.33 | 0.07 | 0.29 | 0.32 | 317.10 | 176 | 0 | OK:30 | OK |
| water_coastline_sunset | delta | -0.32 | +0.16 | +0.00 | -0.40 | -0.20 | -0.04 | -0.21 | +0.06 |  | +0.0% |  |  |  |
| ssr_bay_offshore | off | 13.11 | 12.48 | 0.08 | 0.48 | 0.38 | 0.06 |  | 0.04 | 64.40 | 4,559 | 2,961 | OK:30 | OK |
| ssr_bay_offshore | on | 11.53 | 10.94 | 0.05 | 0.45 | 0.35 | 0.06 |  | 0.04 | 59.20 | 4,559 | 2,961 | OK:30 | OK |
| ssr_bay_offshore | delta | -1.58 | -1.54 | -0.03 | -0.03 | -0.03 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| materials_town_night_rain | off | 5.87 | 5.06 |  | 0.63 | 0.53 | 0.06 |  | 0.04 | 137.20 | 1,122 | 894 | OK:257 | OK |
| materials_town_night_rain | on | 7.07 | 6.36 |  | 0.54 | 0.43 | 0.06 |  | 0.04 | 128.30 | 1,127 | 904 | OK:186 | OK |
| materials_town_night_rain | delta | +1.20 | +1.30 |  | -0.09 | -0.10 | +0.00 |  | +0.00 |  | +0.4% | +1.1% |  |  |
| airbeams_pinewood_morn | off | 5.11 | 4.10 | 0.57 | 0.83 | 0.43 | 0.07 | 0.27 | 0.06 | 129.20 | 2,845 | 1,678 | OK:30 | OK |
| airbeams_pinewood_morn | on | 4.87 | 3.91 | 0.45 | 0.80 | 0.38 | 0.07 | 0.27 | 0.08 | 121.90 | 2,845 | 1,678 | OK:30 | OK |
| airbeams_pinewood_morn | delta | -0.24 | -0.19 | -0.12 | -0.03 | -0.05 | +0.00 | +0.00 | +0.02 |  | +0.0% | +0.0% |  |  |
| airhaze_valley_0515 | off | 23.81 | 22.47 | 0.04 | 1.19 | 0.69 | 0.07 | 0.31 | 0.12 | 29.60 | 11,406 | 7,966 | OK:30 | OK |
| airhaze_valley_0515 | on | 23.85 | 22.51 | 0.04 | 1.19 | 0.75 | 0.07 | 0.31 | 0.07 | 31.60 | 11,406 | 7,966 | OK:30 | OK |
| airhaze_valley_0515 | delta | +0.04 | +0.04 | +0.00 | +0.00 | +0.06 | +0.00 | +0.00 | -0.05 |  | +0.0% | +0.0% |  |  |
| hfrt_hill_1730 | off | 3.55 | 2.69 | 0.11 | 0.68 | 0.40 | 0.17 |  | 0.12 | 289.20 | 1,662 | 934 | OK:30 | OK |
| hfrt_hill_1730 | on | 2.23 | 1.54 | 0.05 | 0.53 | 0.33 | 0.11 |  | 0.08 | 283.70 | 1,662 | 934 | OK:30 | OK |
| hfrt_hill_1730 | delta | -1.32 | -1.15 | -0.06 | -0.15 | -0.07 | -0.06 |  | -0.04 |  | +0.0% | +0.0% |  |  |
| clouds_coast_oc_up | off | 4.32 | 1.56 | 0.00 | 2.28 | 0.52 | 0.19 | 1.12 | 0.45 | 244.60 | 186 | 0 | OK:30 | OK |
| clouds_coast_oc_up | on | 4.94 | 2.03 | 0.00 | 2.43 | 0.58 | 0.22 | 1.30 | 0.33 | 213.20 | 186 | 0 | OK:30 | OK |
| clouds_coast_oc_up | delta | +0.62 | +0.47 | +0.00 | +0.15 | +0.06 | +0.03 | +0.18 | -0.12 |  | +0.0% |  |  |  |
| texsharp_forest_edge | off | 5.58 | 4.84 | 0.28 | 0.54 | 0.41 | 0.08 |  | 0.05 | 121.20 | 2,076 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | on | 5.96 | 5.18 | 0.31 | 0.58 | 0.42 | 0.10 |  | 0.06 | 113.80 | 2,076 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | delta | +0.38 | +0.34 | +0.03 | +0.04 | +0.01 | +0.02 |  | +0.01 |  | +0.0% | +0.0% |  |  |
