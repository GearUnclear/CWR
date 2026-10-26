# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| perf_sunrise_ridge | off | 20.42 | 19.21 | 0.09 | 1.06 | 0.67 | 0.06 | 0.27 | 0.06 | 32.70 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | on | 18.84 | 17.58 | 0.09 | 1.10 | 0.64 | 0.06 | 0.27 | 0.13 | 34.00 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | delta | -1.58 | -1.63 | +0.00 | +0.04 | -0.03 | +0.00 | +0.00 | +0.07 |  | +0.0% | +0.0% |  |  |
| temporal_forest_edge_golden | off | 4.40 | 3.74 | 0.21 | 0.49 | 0.38 | 0.07 |  | 0.05 | 127.40 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | on | 5.09 | 4.45 | 0.21 | 0.47 | 0.36 | 0.07 |  | 0.05 | 127.80 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | delta | +0.69 | +0.71 | +0.00 | -0.02 | -0.02 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| lighttiles_town_night | off | 7.37 | 6.36 |  | 0.83 | 0.72 | 0.06 |  | 0.05 | 100.20 | 1,321 | 1,141 | OK:224 | OK |
| lighttiles_town_night | on | 6.89 | 5.96 |  | 0.74 | 0.63 | 0.06 |  | 0.04 | 108.30 | 1,298 | 1,126 | OK:31 | OK |
| lighttiles_town_night | delta | -0.48 | -0.40 |  | -0.09 | -0.09 | +0.00 |  | -0.01 |  | -1.7% | -1.3% |  |  |
| atmosphere_coastline_sunset | off | 3.22 | 1.21 | 0.00 | 1.80 | 0.58 | 0.14 | 0.94 | 0.14 | 209.60 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | on | 2.16 | 0.74 | 0.00 | 1.27 | 0.45 | 0.09 | 0.64 | 0.09 | 258.10 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | delta | -1.06 | -0.47 | +0.00 | -0.53 | -0.13 | -0.05 | -0.30 | -0.05 |  | +0.0% |  |  |  |
| shadows_woods_pine2_evening | off | 6.63 | 5.31 | 0.37 | 1.16 | 0.48 | 0.07 | 0.54 | 0.07 | 111.10 | 2,831 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | on | 6.40 | 5.11 | 0.35 | 1.14 | 0.48 | 0.07 | 0.52 | 0.07 | 111.70 | 2,831 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | delta | -0.23 | -0.20 | -0.02 | -0.02 | +0.00 | +0.00 | -0.02 | +0.00 |  | +0.0% | +0.0% |  |  |
| shade_lowsun_fp | off | 6.17 | 4.80 | 0.10 | 1.20 | 0.46 | 0.09 | 0.56 | 0.09 | 120.30 | 1,890 | 1,316 | OK:30 | OK |
| shade_lowsun_fp | on | 5.38 | 4.02 | 0.09 | 1.17 | 0.59 | 0.09 | 0.40 | 0.09 | 60.10 | 1,890 | 1,316 | OK:30 | OK |
| shade_lowsun_fp | delta | -0.79 | -0.78 | -0.01 | -0.03 | +0.13 | +0.00 | -0.16 | +0.00 |  | +0.0% | +0.0% |  |  |
| night_road_flat_drive_moonless | off | 4.23 | 3.54 |  | 0.51 | 0.41 | 0.06 |  | 0.04 | 149.30 | 1,806 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | on | 5.61 | 4.76 |  | 0.64 | 0.50 | 0.09 |  | 0.06 | 117.30 | 1,806 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | delta | +1.38 | +1.22 |  | +0.13 | +0.09 | +0.03 |  | +0.02 |  | +0.0% | +0.0% |  |  |
| combat_ffh | off | 6.80 | 5.88 |  | 0.78 | 0.68 | 0.06 |  | 0.04 | 107.20 | 821 | 693 | UNSETTLED:300 | OK |
| combat_ffh | on | 7.52 | 6.45 |  | 0.91 | 0.80 | 0.06 |  | 0.04 | 103.00 | 785 | 677 | UNSETTLED:300 | OK |
| combat_ffh | delta | +0.72 | +0.57 |  | +0.13 | +0.12 | +0.00 |  | +0.00 |  | -4.4% | -2.3% |  |  |
| water_coastline_sunset | off | 3.04 | 1.12 | 0.00 | 1.79 | 0.92 | 0.06 | 0.29 | 0.52 | 238.60 | 176 | 0 | OK:30 | OK |
| water_coastline_sunset | on | 2.91 | 1.16 | 0.00 | 1.61 | 0.71 | 0.06 | 0.29 | 0.56 | 60.10 | 176 | 0 | OK:30 | OK |
| water_coastline_sunset | delta | -0.13 | +0.04 | +0.00 | -0.18 | -0.21 | +0.00 | +0.00 | +0.04 |  | +0.0% |  |  |  |
| ssr_bay_offshore | off | 13.41 | 12.73 | 0.18 | 0.54 | 0.43 | 0.06 |  | 0.04 | 51.80 | 4,559 | 2,961 | OK:30 | OK |
| ssr_bay_offshore | on | 14.59 | 13.87 | 0.05 | 0.57 | 0.46 | 0.06 |  | 0.04 | 48.80 | 4,559 | 2,961 | OK:30 | OK |
| ssr_bay_offshore | delta | +1.18 | +1.14 | -0.13 | +0.03 | +0.03 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| materials_town_night_rain | off | 12.94 | 11.96 |  | 0.80 | 0.69 | 0.07 |  | 0.04 | 74.80 | 950 | 715 | OK:191 | OK |
| materials_town_night_rain | on | 14.00 | 13.27 |  | 0.55 | 0.44 | 0.07 |  | 0.04 | 59.70 | 954 | 708 | OK:138 | OK |
| materials_town_night_rain | delta | +1.06 | +1.31 |  | -0.25 | -0.25 | +0.00 |  | +0.00 |  | +0.4% | -1.0% |  |  |
| airbeams_pinewood_morn | off | 15.76 | 13.50 | 1.48 | 2.02 | 0.33 | 0.07 | 0.27 | 1.35 | 54.50 | 2,845 | 1,678 | OK:30 | OK |
| airbeams_pinewood_morn | on | 15.90 | 14.41 | 2.36 | 1.31 | 0.34 | 0.07 | 0.27 | 0.62 | 54.50 | 2,845 | 1,678 | OK:30 | OK |
| airbeams_pinewood_morn | delta | +0.14 | +0.91 | +0.88 | -0.71 | +0.01 | +0.00 | +0.00 | -0.73 |  | +0.0% | +0.0% |  |  |
| airhaze_valley_0515 | off | 81.45 | 77.77 | 0.17 | 3.53 | 1.79 | 0.07 | 0.31 | 1.35 | 11.00 | 11,406 | 7,966 | OK:30 | OK |
| airhaze_valley_0515 | on | 31.46 | 29.54 | 0.04 | 1.77 | 1.09 | 0.07 | 0.31 | 0.31 | 17.20 | 11,406 | 7,966 | OK:30 | OK |
| airhaze_valley_0515 | delta | -49.99 | -48.23 | -0.13 | -1.76 | -0.70 | +0.00 | +0.00 | -1.04 |  | +0.0% | +0.0% |  |  |
| hfrt_hill_1730 | off | 3.06 | 2.27 | 0.10 | 0.63 | 0.36 | 0.16 |  | 0.11 | 314.30 | 1,662 | 934 | OK:30 | OK |
| hfrt_hill_1730 | on | 5.50 | 4.10 | 0.17 | 1.23 | 0.65 | 0.47 |  | 0.11 | 288.90 | 1,662 | 934 | OK:30 | OK |
| hfrt_hill_1730 | delta | +2.44 | +1.83 | +0.07 | +0.60 | +0.29 | +0.31 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| clouds_coast_oc_up | off | 2.55 | 1.04 | 0.00 | 1.28 | 0.32 | 0.08 | 0.47 | 0.41 | 336.00 | 186 | 0 | OK:30 | OK |
| clouds_coast_oc_up | on | 1.78 | 0.72 | 0.00 | 0.85 | 0.29 | 0.07 | 0.42 | 0.07 | 347.10 | 186 | 0 | OK:30 | OK |
| clouds_coast_oc_up | delta | -0.77 | -0.32 | +0.00 | -0.43 | -0.03 | -0.01 | -0.05 | -0.34 |  | +0.0% |  |  |  |
| texsharp_forest_edge | off | 5.16 | 4.52 | 0.23 | 0.48 | 0.35 | 0.08 |  | 0.05 | 116.50 | 2,076 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | on | 4.57 | 3.95 | 0.23 | 0.47 | 0.35 | 0.07 |  | 0.05 | 126.40 | 2,076 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | delta | -0.59 | -0.57 | +0.00 | -0.01 | +0.00 | -0.01 |  | +0.00 |  | +0.0% | +0.0% |  |  |
