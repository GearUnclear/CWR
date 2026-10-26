# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| perf_sunrise_ridge | off | 44.13 | 42.07 | 0.09 | 1.90 |  |  | 0.73 | 0.06 | 0.27 | 0.83 | 16.30 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | on | 37.40 | 35.64 | 0.13 | 1.60 |  |  | 0.73 | 0.06 | 0.27 | 0.54 | 21.40 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | delta | -6.73 | -6.43 | +0.04 | -0.30 |  |  | +0.00 | +0.00 | +0.00 | -0.29 |  | +0.0% | +0.0% |  |  |
| temporal_forest_edge_golden | off | 14.01 | 13.34 | 0.24 | 0.50 |  |  | 0.39 | 0.07 |  | 0.04 | 60.60 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | on | 14.17 | 13.60 | 0.21 | 0.39 |  |  | 0.28 | 0.07 |  | 0.04 | 59.90 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | delta | +0.16 | +0.26 | -0.03 | -0.11 |  |  | -0.11 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| lighttiles_town_night | off | 13.14 | 12.47 |  | 0.46 |  |  | 0.35 | 0.06 |  | 0.04 | 66.00 | 1,302 | 1,127 | OK:231 | OK |
| lighttiles_town_night | on | 15.49 | 14.83 |  | 0.47 |  |  | 0.36 | 0.06 |  | 0.04 | 59.20 | 1,288 | 1,129 | OK:36 | OK |
| lighttiles_town_night | delta | +2.35 | +2.36 |  | +0.01 |  |  | +0.01 | +0.00 |  | +0.00 |  | -1.1% | +0.2% |  |  |
| atmosphere_coastline_sunset | off | 2.22 | 0.92 | 0.00 | 1.18 |  |  | 0.41 | 0.06 | 0.43 | 0.27 | 294.10 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | on | 2.17 | 0.55 | 0.00 | 1.49 |  |  | 0.52 | 0.06 | 0.43 | 0.48 | 389.60 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | delta | -0.05 | -0.37 | +0.00 | +0.31 |  |  | +0.11 | +0.00 | +0.00 | +0.21 |  | +0.0% |  |  |  |
| shadows_woods_pine2_evening | off | 9.32 | 7.00 | 0.76 | 2.17 |  |  | 0.38 | 0.10 | 0.99 | 0.71 | 81.60 | 2,831 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | on | 9.24 | 6.89 | 0.67 | 2.19 |  |  | 0.40 | 0.07 | 1.05 | 0.68 | 94.60 | 2,831 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | delta | -0.08 | -0.11 | -0.09 | +0.02 |  |  | +0.02 | -0.03 | +0.06 | -0.03 |  | +0.0% | +0.0% |  |  |
| shade_lowsun_fp | off | 4.97 | 3.88 | 0.07 | 0.94 |  |  | 0.41 | 0.06 | 0.37 | 0.10 | 98.50 | 1,890 | 1,316 | OK:30 | OK |
| shade_lowsun_fp | on | 9.20 | 7.74 | 0.22 | 1.32 |  |  | 0.29 | 0.06 | 0.37 | 0.60 | 93.40 | 1,890 | 1,316 | OK:30 | OK |
| shade_lowsun_fp | delta | +4.23 | +3.86 | +0.15 | +0.38 |  |  | -0.12 | +0.00 | +0.00 | +0.50 |  | +0.0% | +0.0% |  |  |
| night_road_flat_drive_moonless | off | 8.39 | 7.79 |  | 0.42 |  |  | 0.33 | 0.06 |  | 0.04 | 122.40 | 1,806 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | on | 6.59 | 6.02 |  | 0.41 |  |  | 0.31 | 0.06 |  | 0.04 | 116.60 | 1,806 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | delta | -1.80 | -1.77 |  | -0.01 |  |  | -0.02 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| combat_ffh | off | 8.97 | 8.21 |  | 0.61 |  |  | 0.51 | 0.06 |  | 0.04 | 110.00 | 816 | 699 | UNSETTLED:300 | OK |
| combat_ffh | on | 13.55 | 13.02 |  | 0.38 |  |  | 0.28 | 0.06 |  | 0.04 | 75.80 | 782 | 672 | UNSETTLED:300 | OK |
| combat_ffh | delta | +4.58 | +4.81 |  | -0.23 |  |  | -0.23 | +0.00 |  | +0.00 |  | -4.2% | -3.9% |  |  |
| water_coastline_sunset | off | 1.87 | 0.69 | 0.00 | 1.05 |  |  | 0.47 | 0.06 | 0.29 | 0.23 | 329.10 | 176 | 0 | OK:30 | OK |
| water_coastline_sunset | on | 2.44 | 1.29 | 0.00 | 1.02 |  |  | 0.30 | 0.06 | 0.29 | 0.37 | 292.70 | 176 | 0 | OK:30 | OK |
| water_coastline_sunset | delta | +0.57 | +0.60 | +0.00 | -0.03 |  |  | -0.17 | +0.00 | +0.00 | +0.14 |  | +0.0% |  |  |  |
| ssr_bay_offshore | off | 20.38 | 19.84 | 0.08 | 0.39 |  |  | 0.29 | 0.07 |  | 0.04 | 42.70 | 4,559 | 2,961 | OK:30 | OK |
| ssr_bay_offshore | on | 27.33 | 26.81 | 0.03 | 0.38 |  |  | 0.27 | 0.07 |  | 0.04 | 35.50 | 4,559 | 2,961 | OK:30 | OK |
| ssr_bay_offshore | delta | +6.95 | +6.97 | -0.05 | -0.01 |  |  | -0.02 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| materials_town_night_rain | off | 10.33 | 9.67 |  | 0.47 |  |  | 0.37 | 0.06 |  | 0.04 | 83.60 | 697 | 434 | UNSETTLED:300 | OK |
| materials_town_night_rain | on | 10.20 | 9.63 |  | 0.40 |  |  | 0.29 | 0.06 |  | 0.04 | 87.70 | 695 | 433 | OK:96 | OK |
| materials_town_night_rain | delta | -0.13 | -0.04 |  | -0.07 |  |  | -0.08 | +0.00 |  | +0.00 |  | -0.3% | -0.2% |  |  |
| airbeams_pinewood_morn | off | 18.39 | 16.14 | 1.81 | 2.06 |  |  | 0.28 | 0.07 | 0.27 | 1.44 | 51.40 | 2,845 | 1,678 | OK:30 | OK |
| airbeams_pinewood_morn | on | 17.38 | 15.10 | 1.68 | 2.04 |  |  | 0.33 | 0.07 | 0.27 | 1.38 | 51.90 | 2,845 | 1,678 | OK:30 | OK |
| airbeams_pinewood_morn | delta | -1.01 | -1.04 | -0.13 | -0.02 |  |  | +0.05 | +0.00 | +0.00 | -0.06 |  | +0.0% | +0.0% |  |  |
| airhaze_valley_0515 | off | 45.13 | 42.93 | 0.06 | 2.05 |  |  | 1.08 | 0.07 | 0.31 | 0.59 | 16.60 | 11,406 | 7,966 | OK:30 | OK |
| airhaze_valley_0515 | on | 62.58 | 60.18 | 0.25 | 2.25 |  |  | 1.03 | 0.07 | 0.31 | 0.84 | 15.10 | 11,406 | 7,966 | OK:30 | OK |
| airhaze_valley_0515 | delta | +17.45 | +17.25 | +0.19 | +0.20 |  |  | -0.05 | +0.00 | +0.00 | +0.25 |  | +0.0% | +0.0% |  |  |
| hfrt_hill_1730 | off | 3.39 | 2.34 | 0.03 | 0.95 |  |  | 0.84 | 0.07 |  | 0.04 | 283.70 | 1,662 | 934 | OK:30 | OK |
| hfrt_hill_1730 | on | 2.38 | 1.65 | 0.03 | 0.62 |  |  | 0.51 | 0.07 |  | 0.04 | 281.30 | 1,662 | 934 | OK:30 | OK |
| hfrt_hill_1730 | delta | -1.01 | -0.69 | +0.00 | -0.33 |  |  | -0.33 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| clouds_coast_oc_up | off | 1.72 | 0.53 | 0.00 | 1.00 |  |  | 0.39 | 0.06 | 0.35 | 0.19 | 383.20 | 186 | 0 | OK:30 | OK |
| clouds_coast_oc_up | on | 2.30 | 0.90 | 0.00 | 1.14 |  |  | 0.41 | 0.10 | 0.51 | 0.12 | 356.70 | 186 | 0 | OK:30 | OK |
| clouds_coast_oc_up | delta | +0.58 | +0.37 | +0.00 | +0.14 |  |  | +0.02 | +0.04 | +0.16 | -0.07 |  | +0.0% |  |  |  |
| texsharp_forest_edge | off | 4.35 | 3.70 | 0.20 | 0.49 |  |  | 0.38 | 0.07 |  | 0.04 | 136.70 | 2,076 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | on | 4.38 | 3.76 | 0.21 | 0.46 |  |  | 0.35 | 0.07 |  | 0.04 | 138.40 | 2,076 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | delta | +0.03 | +0.06 | +0.01 | -0.03 |  |  | -0.03 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
