# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| perf_sunrise_ridge | off | 17.93 | 16.77 | 0.08 | 0.99 | 0.47 |  | 0.13 | 0.06 | 0.27 | 0.07 | 36.90 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | on | 17.45 | 16.30 | 0.10 | 0.99 | 0.45 |  | 0.13 | 0.06 | 0.27 | 0.09 | 37.90 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | delta | -0.48 | -0.47 | +0.02 | +0.00 | -0.02 |  | +0.00 | +0.00 | +0.00 | +0.02 |  | +0.0% | +0.0% |  |  |
| temporal_forest_edge_golden | off | 5.52 | 4.76 | 0.26 | 0.56 | 0.24 |  | 0.17 | 0.08 |  | 0.06 | 123.70 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | on | 5.97 | 5.18 | 0.32 | 0.57 | 0.25 |  | 0.18 | 0.09 |  | 0.05 | 115.80 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | delta | +0.45 | +0.42 | +0.06 | +0.01 | +0.01 |  | +0.01 | +0.01 |  | -0.01 |  | +0.0% | +0.0% |  |  |
| lighttiles_town_night | off | 6.72 | 5.78 |  | 0.67 | 0.37 |  | 0.18 | 0.07 |  | 0.05 | 121.70 | 1,281 | 1,127 | OK:230 | OK |
| lighttiles_town_night | on | 6.48 | 5.59 |  | 0.66 | 0.36 |  | 0.18 | 0.07 |  | 0.04 | 121.30 | 1,306 | 1,128 | OK:30 | OK |
| lighttiles_town_night | delta | -0.24 | -0.19 |  | -0.01 | -0.01 |  | +0.00 | +0.00 |  | -0.01 |  | +1.9% | +0.1% |  |  |
| atmosphere_coastline_sunset | off | 4.06 | 1.65 | 0.00 | 2.16 | 0.38 |  | 0.24 | 0.15 | 1.18 | 0.21 | 282.20 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | on | 4.58 | 1.86 | 0.00 | 2.44 | 0.34 |  | 0.30 | 0.20 | 1.39 | 0.21 | 247.60 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | delta | +0.52 | +0.21 | +0.00 | +0.28 | -0.04 |  | +0.06 | +0.05 | +0.21 | +0.00 |  | +0.0% |  |  |  |
| shadows_woods_pine2_evening | off | 6.44 | 5.20 | 0.34 | 1.10 | 0.26 |  | 0.20 | 0.07 | 0.50 | 0.07 | 111.80 | 2,831 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | on | 6.54 | 5.28 | 0.34 | 1.11 | 0.27 |  | 0.20 | 0.07 | 0.50 | 0.08 | 111.40 | 2,831 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | delta | +0.10 | +0.08 | +0.00 | +0.01 | +0.01 |  | +0.00 | +0.00 | +0.00 | +0.01 |  | +0.0% | +0.0% |  |  |
| shade_lowsun_fp | off | 5.70 | 4.27 | 0.09 | 1.26 | 0.31 |  | 0.18 | 0.08 | 0.48 | 0.21 | 127.80 | 1,890 | 1,316 | OK:30 | OK |
| shade_lowsun_fp | on | 5.87 | 4.43 | 0.09 | 1.26 | 0.25 |  | 0.19 | 0.09 | 0.51 | 0.23 | 122.20 | 1,890 | 1,316 | OK:30 | OK |
| shade_lowsun_fp | delta | +0.17 | +0.16 | +0.00 | +0.00 | -0.06 |  | +0.01 | +0.01 | +0.03 | +0.02 |  | +0.0% | +0.0% |  |  |
| night_road_flat_drive_moonless | off | 5.95 | 5.10 |  | 0.64 | 0.29 |  | 0.20 | 0.09 |  | 0.06 | 124.20 | 1,806 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | on | 5.94 | 5.12 |  | 0.62 | 0.27 |  | 0.20 | 0.09 |  | 0.06 | 121.90 | 1,806 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | delta | -0.01 | +0.02 |  | -0.02 | -0.02 |  | +0.00 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| combat_ffh | off | 6.11 | 5.19 |  | 0.74 | 0.42 |  | 0.18 | 0.08 |  | 0.06 | 129.00 | 813 | 695 | UNSETTLED:300 | OK |
| combat_ffh | on | 6.11 | 5.21 |  | 0.71 | 0.38 |  | 0.19 | 0.08 |  | 0.06 | 129.20 | 780 | 672 | UNSETTLED:300 | OK |
| combat_ffh | delta | +0.00 | +0.02 |  | -0.03 | -0.04 |  | +0.01 | +0.00 |  | +0.00 |  | -4.1% | -3.3% |  |  |
| water_coastline_sunset | off | 3.83 | 1.46 | 0.00 | 2.10 | 0.35 |  | 0.37 | 0.26 | 0.87 | 0.26 | 273.00 | 176 | 0 | OK:30 | OK |
| water_coastline_sunset | on | 1.92 | 0.65 | 0.00 | 1.11 | 0.28 |  | 0.19 | 0.09 | 0.43 | 0.12 | 353.80 | 176 | 0 | OK:30 | OK |
| water_coastline_sunset | delta | -1.91 | -0.81 | +0.00 | -0.99 | -0.07 |  | -0.18 | -0.17 | -0.44 | -0.14 |  | +0.0% |  |  |  |
| ssr_bay_offshore | off | 7.55 | 6.90 | 0.03 | 0.49 | 0.26 |  | 0.13 | 0.06 |  | 0.04 | 82.50 | 4,559 | 2,961 | OK:30 | OK |
| ssr_bay_offshore | on | 7.60 | 6.96 | 0.03 | 0.49 | 0.26 |  | 0.13 | 0.06 |  | 0.04 | 82.70 | 4,559 | 2,961 | OK:30 | OK |
| ssr_bay_offshore | delta | +0.05 | +0.06 | +0.00 | +0.00 | +0.00 |  | +0.00 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| materials_town_night_rain | off | 6.02 | 5.05 |  | 0.75 | 0.38 |  | 0.22 | 0.09 |  | 0.06 | 138.00 | 960 | 711 | OK:294 | OK |
| materials_town_night_rain | on | 6.36 | 5.44 |  | 0.71 | 0.36 |  | 0.21 | 0.08 |  | 0.05 | 134.50 | 1,022 | 790 | OK:81 | OK |
| materials_town_night_rain | delta | +0.34 | +0.39 |  | -0.04 | -0.02 |  | -0.01 | -0.01 |  | -0.01 |  | +6.1% | +10.0% |  |  |
| airbeams_pinewood_morn | off | 6.06 | 4.84 | 0.64 | 1.00 | 0.28 |  | 0.19 | 0.09 | 0.36 | 0.08 | 114.80 | 2,845 | 1,678 | OK:30 | OK |
| airbeams_pinewood_morn | on | 6.13 | 4.84 | 0.58 | 1.08 | 0.29 |  | 0.19 | 0.12 | 0.37 | 0.12 | 114.00 | 2,845 | 1,678 | OK:30 | OK |
| airbeams_pinewood_morn | delta | +0.07 | +0.00 | -0.06 | +0.08 | +0.01 |  | +0.00 | +0.03 | +0.01 | +0.04 |  | +0.0% | +0.0% |  |  |
| airhaze_valley_0515 | off | 24.14 | 22.78 | 0.04 | 1.21 | 0.70 |  | 0.06 | 0.07 | 0.31 | 0.07 | 31.30 | 11,406 | 7,966 | OK:30 | OK |
| airhaze_valley_0515 | on | 23.87 | 22.58 | 0.04 | 1.14 | 0.63 |  | 0.06 | 0.07 | 0.31 | 0.07 | 31.30 | 11,406 | 7,966 | OK:30 | OK |
| airhaze_valley_0515 | delta | -0.27 | -0.20 | +0.00 | -0.07 | -0.07 |  | +0.00 | +0.00 | +0.00 | +0.00 |  | +0.0% | +0.0% |  |  |
| hfrt_hill_1730 | off | 3.53 | 2.62 | 0.15 | 0.72 | 0.23 |  | 0.21 | 0.17 |  | 0.11 | 288.10 | 1,662 | 934 | OK:30 | OK |
| hfrt_hill_1730 | on | 3.87 | 2.31 | 0.07 | 1.33 | 0.72 |  | 0.12 | 0.26 |  | 0.22 | 284.50 | 1,662 | 934 | OK:30 | OK |
| hfrt_hill_1730 | delta | +0.34 | -0.31 | -0.08 | +0.61 | +0.49 |  | -0.09 | +0.09 |  | +0.11 |  | +0.0% | +0.0% |  |  |
| clouds_coast_oc_up | off | 4.33 | 1.74 | 0.00 | 2.09 | 0.31 |  | 0.19 | 0.19 | 1.15 | 0.26 | 245.40 | 186 | 0 | OK:30 | OK |
| clouds_coast_oc_up | on | 4.65 | 1.85 | 0.00 | 2.32 | 0.32 |  | 0.22 | 0.21 | 1.32 | 0.25 | 223.30 | 186 | 0 | OK:30 | OK |
| clouds_coast_oc_up | delta | +0.32 | +0.11 | +0.00 | +0.23 | +0.01 |  | +0.03 | +0.02 | +0.17 | -0.01 |  | +0.0% |  |  |  |
| texsharp_forest_edge | off | 5.59 | 4.79 | 0.27 | 0.59 | 0.25 |  | 0.19 | 0.11 |  | 0.05 | 121.00 | 2,076 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | on | 5.88 | 5.09 | 0.29 | 0.59 | 0.24 |  | 0.20 | 0.09 |  | 0.06 | 115.90 | 2,076 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | delta | +0.29 | +0.30 | +0.02 | +0.00 | -0.01 |  | +0.01 | -0.02 |  | +0.01 |  | +0.0% | +0.0% |  |  |
