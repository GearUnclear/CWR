# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| perf_sunrise_ridge | off | 18.59 | 17.40 | 0.09 | 1.03 | 0.47 |  | 0.13 | 0.06 | 0.27 | 0.10 | 34.10 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | on | 19.29 | 18.11 | 0.10 | 1.02 | 0.44 |  | 0.13 | 0.06 | 0.27 | 0.13 | 35.50 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | delta | +0.70 | +0.71 | +0.01 | -0.01 | -0.03 |  | +0.00 | +0.00 | +0.00 | +0.03 |  | +0.0% | +0.0% |  |  |
| temporal_forest_edge_golden | off | 5.11 | 4.42 | 0.21 | 0.52 | 0.27 |  | 0.13 | 0.07 |  | 0.04 | 109.00 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | on | 6.43 | 5.77 | 0.21 | 0.49 | 0.25 |  | 0.13 | 0.07 |  | 0.04 | 109.60 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | delta | +1.32 | +1.35 | +0.00 | -0.03 | -0.02 |  | +0.00 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| lighttiles_town_night | off | 14.63 | 13.92 |  | 0.51 | 0.25 |  | 0.16 | 0.07 |  | 0.04 | 64.30 | 1,260 | 1,077 | OK:180 | OK |
| lighttiles_town_night | on | 13.81 | 13.12 |  | 0.45 | 0.18 |  | 0.16 | 0.07 |  | 0.04 | 61.00 | 1,176 | 1,016 | OK:81 | OK |
| lighttiles_town_night | delta | -0.82 | -0.80 |  | -0.06 | -0.07 |  | +0.00 | +0.00 |  | +0.00 |  | -6.7% | -5.7% |  |  |
| atmosphere_coastline_sunset | off | 4.20 | 2.29 | 0.00 | 1.78 | 0.15 |  | 0.11 | 0.06 | 0.43 | 1.02 | 251.40 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | on | 3.92 | 2.14 | 0.00 | 1.65 | 0.18 |  | 0.11 | 0.06 | 0.43 | 0.86 | 238.30 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | delta | -0.28 | -0.15 | +0.00 | -0.13 | +0.03 |  | +0.00 | +0.00 | +0.00 | -0.16 |  | +0.0% |  |  |  |
| shadows_woods_pine2_evening | off | 18.55 | 15.60 | 1.21 | 2.76 | 0.34 |  | 0.20 | 0.49 | 0.74 | 0.99 | 51.60 | 2,831 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | on | 18.39 | 15.52 | 1.27 | 2.69 | 0.17 |  | 0.20 | 0.54 | 0.87 | 0.92 | 51.20 | 2,831 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | delta | -0.16 | -0.08 | +0.06 | -0.07 | -0.17 |  | +0.00 | +0.05 | +0.13 | -0.07 |  | +0.0% | +0.0% |  |  |
| shade_lowsun_fp | off | 12.71 | 11.14 | 0.13 | 1.43 | 0.16 |  | 0.14 | 0.06 | 0.36 | 0.71 | 68.70 | 1,890 | 1,316 | OK:30 | OK |
| shade_lowsun_fp | on | 13.10 | 11.24 | 0.17 | 1.71 | 0.14 |  | 0.14 | 0.06 | 0.36 | 1.01 | 69.10 | 1,890 | 1,316 | OK:30 | OK |
| shade_lowsun_fp | delta | +0.39 | +0.10 | +0.04 | +0.28 | -0.02 |  | +0.00 | +0.00 | +0.00 | +0.30 |  | +0.0% | +0.0% |  |  |
| night_road_flat_drive_moonless | off | 6.15 | 5.54 |  | 0.44 | 0.21 |  | 0.13 | 0.06 |  | 0.04 | 101.90 | 1,806 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | on | 9.92 | 9.33 |  | 0.42 | 0.19 |  | 0.13 | 0.06 |  | 0.04 | 93.10 | 1,806 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | delta | +3.77 | +3.79 |  | -0.02 | -0.02 |  | +0.00 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| combat_ffh | off | 8.54 | 7.65 |  | 0.66 | 0.42 |  | 0.13 | 0.06 |  | 0.04 | 100.30 | 806 | 701 | UNSETTLED:300 | OK |
| combat_ffh | on | 8.40 | 7.82 |  | 0.42 | 0.18 |  | 0.14 | 0.06 |  | 0.04 | 87.70 | 785 | 671 | UNSETTLED:300 | OK |
| combat_ffh | delta | -0.14 | +0.17 |  | -0.24 | -0.24 |  | +0.01 | +0.00 |  | +0.00 |  | -2.6% | -4.3% |  |  |
| water_coastline_sunset | off | 3.29 | 1.74 | 0.00 | 1.43 | 0.22 |  | 0.13 | 0.06 | 0.29 | 0.73 | 293.90 | 176 | 0 | OK:30 | OK |
| water_coastline_sunset | on | 1.83 | 0.70 | 0.00 | 1.00 | 0.26 |  | 0.13 | 0.06 | 0.29 | 0.26 | 361.40 | 176 | 0 | OK:30 | OK |
| water_coastline_sunset | delta | -1.46 | -1.04 | +0.00 | -0.43 | +0.04 |  | +0.00 | +0.00 | +0.00 | -0.47 |  | +0.0% |  |  |  |
| ssr_bay_offshore | off | 10.57 | 9.97 | 0.05 | 0.46 | 0.23 |  | 0.13 | 0.06 |  | 0.04 | 66.10 | 4,559 | 2,961 | OK:30 | OK |
| ssr_bay_offshore | on | 12.13 | 11.48 | 0.03 | 0.51 | 0.27 |  | 0.13 | 0.06 |  | 0.04 | 62.70 | 4,559 | 2,961 | OK:30 | OK |
| ssr_bay_offshore | delta | +1.56 | +1.51 | -0.02 | +0.05 | +0.04 |  | +0.00 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| materials_town_night_rain | off | 7.70 | 6.96 |  | 0.57 | 0.31 |  | 0.16 | 0.07 |  | 0.04 | 140.50 | 876 | 623 | OK:55 | OK |
| materials_town_night_rain | on | 7.18 | 6.47 |  | 0.53 | 0.27 |  | 0.16 | 0.07 |  | 0.04 | 100.50 | 1,021 | 780 | UNSETTLED:300 | OK |
| materials_town_night_rain | delta | -0.52 | -0.49 |  | -0.04 | -0.04 |  | +0.00 | +0.00 |  | +0.00 |  | +14.2% | +20.1% |  |  |
| airbeams_pinewood_morn | off | 9.49 | 8.19 | 0.96 | 1.12 | 0.21 |  | 0.14 | 0.07 | 0.27 | 0.43 | 104.50 | 2,845 | 1,678 | OK:30 | OK |
| airbeams_pinewood_morn | on | 14.52 | 12.64 | 1.34 | 1.59 | 0.15 |  | 0.14 | 0.07 | 0.27 | 0.95 | 70.80 | 2,845 | 1,678 | OK:30 | OK |
| airbeams_pinewood_morn | delta | +5.03 | +4.45 | +0.38 | +0.47 | -0.06 |  | +0.00 | +0.00 | +0.00 | +0.52 |  | +0.0% | +0.0% |  |  |
| airhaze_valley_0515 | off | 99.44 | 96.76 | 0.38 | 2.53 | 0.52 |  | 0.06 | 0.07 | 0.31 | 1.57 | 10.20 | 11,406 | 7,966 | OK:30 | OK |
| airhaze_valley_0515 | on | 97.87 | 95.37 | 0.04 | 2.35 | 0.41 |  | 0.06 | 0.07 | 0.31 | 1.50 | 9.60 | 11,406 | 7,966 | OK:30 | OK |
| airhaze_valley_0515 | delta | -1.57 | -1.39 | -0.34 | -0.18 | -0.11 |  | +0.00 | +0.00 | +0.00 | -0.07 |  | +0.0% | +0.0% |  |  |
| hfrt_hill_1730 | off | 4.21 | 2.92 | 0.03 | 1.19 | 1.01 |  | 0.06 | 0.07 |  | 0.05 | 272.60 | 1,662 | 934 | OK:30 | OK |
| hfrt_hill_1730 | on | 4.15 | 2.83 | 0.03 | 1.22 | 1.04 |  | 0.06 | 0.07 |  | 0.04 | 274.80 | 1,662 | 934 | OK:30 | OK |
| hfrt_hill_1730 | delta | -0.06 | -0.09 | +0.00 | +0.03 | +0.03 |  | +0.00 | +0.00 |  | -0.01 |  | +0.0% | +0.0% |  |  |
| clouds_coast_oc_up | off | 4.30 | 2.34 | 0.00 | 1.77 | 0.18 |  | 0.06 | 0.06 | 0.34 | 1.13 | 254.40 | 186 | 0 | OK:30 | OK |
| clouds_coast_oc_up | on | 2.94 | 1.46 | 0.00 | 1.29 | 0.18 |  | 0.06 | 0.06 | 0.34 | 0.66 | 292.50 | 186 | 0 | OK:30 | OK |
| clouds_coast_oc_up | delta | -1.36 | -0.88 | +0.00 | -0.48 | +0.00 |  | +0.00 | +0.00 | +0.00 | -0.47 |  | +0.0% |  |  |  |
| texsharp_forest_edge | off | 9.28 | 8.67 | 0.20 | 0.44 | 0.19 |  | 0.14 | 0.07 |  | 0.04 | 79.00 | 2,076 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | on | 12.43 | 11.86 | 0.20 | 0.40 | 0.15 |  | 0.14 | 0.07 |  | 0.04 | 66.10 | 2,076 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | delta | +3.15 | +3.19 | +0.00 | -0.04 | -0.04 |  | +0.00 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
