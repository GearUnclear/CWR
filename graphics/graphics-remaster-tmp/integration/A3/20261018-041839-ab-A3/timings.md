# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| perf_sunrise_ridge | off | 18.65 | 17.44 | 0.09 | 1.06 | 0.50 |  | 0.13 | 0.06 | 0.27 | 0.10 | 34.30 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | on | 19.47 | 18.32 | 0.09 | 0.99 | 0.44 |  | 0.13 | 0.06 | 0.27 | 0.09 | 35.30 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | delta | +0.82 | +0.88 | +0.00 | -0.07 | -0.06 |  | +0.00 | +0.00 | +0.00 | -0.01 |  | +0.0% | +0.0% |  |  |
| temporal_forest_edge_golden | off | 5.18 | 4.56 | 0.21 | 0.45 | 0.21 |  | 0.13 | 0.07 |  | 0.05 | 131.70 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | on | 4.38 | 3.75 | 0.21 | 0.46 | 0.22 |  | 0.13 | 0.07 |  | 0.05 | 139.70 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | delta | -0.80 | -0.81 | +0.00 | +0.01 | +0.01 |  | +0.00 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| lighttiles_town_night | off | 5.88 | 5.09 |  | 0.60 | 0.34 |  | 0.15 | 0.06 |  | 0.04 | 125.20 | 1,270 | 1,090 | OK:246 | OK |
| lighttiles_town_night | on | 7.42 | 6.66 |  | 0.56 | 0.30 |  | 0.15 | 0.06 |  | 0.04 | 119.30 | 1,300 | 1,120 | OK:34 | OK |
| lighttiles_town_night | delta | +1.54 | +1.57 |  | -0.04 | -0.04 |  | +0.00 | +0.00 |  | +0.00 |  | +2.3% | +2.7% |  |  |
| atmosphere_coastline_sunset | off | 3.49 | 1.33 | 0.00 | 1.91 | 0.33 |  | 0.23 | 0.14 | 0.99 | 0.22 | 298.20 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | on | 4.39 | 1.74 | 0.00 | 2.37 | 0.35 |  | 0.29 | 0.21 | 1.33 | 0.19 | 254.10 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | delta | +0.90 | +0.41 | +0.00 | +0.46 | +0.02 |  | +0.06 | +0.07 | +0.34 | -0.03 |  | +0.0% |  |  |  |
| shadows_woods_pine2_evening | off | 6.51 | 5.16 | 0.34 | 1.20 | 0.32 |  | 0.20 | 0.07 | 0.50 | 0.12 | 111.10 | 2,831 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | on | 6.48 | 5.19 | 0.34 | 1.14 | 0.28 |  | 0.20 | 0.07 | 0.49 | 0.10 | 111.90 | 2,831 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | delta | -0.03 | +0.03 | +0.00 | -0.06 | -0.04 |  | +0.00 | +0.00 | -0.01 | -0.02 |  | +0.0% | +0.0% |  |  |
| shade_lowsun_fp | off | 5.82 | 4.54 | 0.09 | 1.11 | 0.26 |  | 0.18 | 0.08 | 0.49 | 0.09 | 127.80 | 1,890 | 1,316 | OK:30 | OK |
| shade_lowsun_fp | on | 5.88 | 4.44 | 0.09 | 1.26 | 0.23 |  | 0.19 | 0.09 | 0.51 | 0.24 | 123.30 | 1,890 | 1,316 | OK:30 | OK |
| shade_lowsun_fp | delta | +0.06 | -0.10 | +0.00 | +0.15 | -0.03 |  | +0.01 | +0.01 | +0.02 | +0.15 |  | +0.0% | +0.0% |  |  |
| night_road_flat_drive_moonless | off | 5.91 | 5.08 |  | 0.63 | 0.28 |  | 0.20 | 0.09 |  | 0.06 | 123.50 | 1,806 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | on | 5.81 | 4.99 |  | 0.61 | 0.26 |  | 0.20 | 0.09 |  | 0.06 | 121.70 | 1,806 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | delta | -0.10 | -0.09 |  | -0.02 | -0.02 |  | +0.00 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| combat_ffh | off | 6.22 | 5.28 |  | 0.76 | 0.44 |  | 0.18 | 0.08 |  | 0.06 | 126.10 | 807 | 701 | UNSETTLED:300 | OK |
| combat_ffh | on | 6.08 | 5.16 |  | 0.73 | 0.41 |  | 0.18 | 0.08 |  | 0.06 | 127.30 | 787 | 679 | UNSETTLED:300 | OK |
| combat_ffh | delta | -0.14 | -0.12 |  | -0.03 | -0.03 |  | +0.00 | +0.00 |  | +0.00 |  | -2.5% | -3.1% |  |  |
| water_coastline_sunset | off | 3.88 | 1.50 | 0.00 | 2.12 | 0.34 |  | 0.35 | 0.28 | 0.90 | 0.25 | 273.80 | 176 | 0 | OK:30 | OK |
| water_coastline_sunset | on | 4.86 | 1.97 | 0.00 | 2.59 | 0.41 |  | 0.49 | 0.23 | 1.10 | 0.37 | 245.10 | 176 | 0 | OK:30 | OK |
| water_coastline_sunset | delta | +0.98 | +0.47 | +0.00 | +0.47 | +0.07 |  | +0.14 | -0.05 | +0.20 | +0.12 |  | +0.0% |  |  |  |
| ssr_bay_offshore | off | 7.57 | 6.92 | 0.03 | 0.51 | 0.28 |  | 0.13 | 0.06 |  | 0.04 | 82.50 | 4,559 | 2,961 | OK:30 | OK |
| ssr_bay_offshore | on | 7.60 | 6.97 | 0.03 | 0.49 | 0.26 |  | 0.12 | 0.06 |  | 0.04 | 82.70 | 4,559 | 2,961 | OK:30 | OK |
| ssr_bay_offshore | delta | +0.03 | +0.05 | +0.00 | -0.02 | -0.02 |  | -0.01 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| materials_town_night_rain | off | 5.61 | 4.62 |  | 0.74 | 0.35 |  | 0.24 | 0.10 |  | 0.06 | 141.70 | 867 | 623 | OK:199 | OK |
| materials_town_night_rain | on | 5.10 | 3.98 |  | 0.82 | 0.36 |  | 0.27 | 0.11 |  | 0.08 | 149.10 | 759 | 495 | OK:221 | OK |
| materials_town_night_rain | delta | -0.51 | -0.64 |  | +0.08 | +0.01 |  | +0.03 | +0.01 |  | +0.02 |  | -12.5% | -20.5% |  |  |
| airbeams_pinewood_morn | off | 6.13 | 4.90 | 0.63 | 1.01 | 0.29 |  | 0.19 | 0.09 | 0.36 | 0.08 | 114.30 | 2,845 | 1,678 | OK:30 | OK |
| airbeams_pinewood_morn | on | 6.21 | 4.99 | 0.59 | 1.00 | 0.27 |  | 0.19 | 0.09 | 0.36 | 0.09 | 114.30 | 2,845 | 1,678 | OK:30 | OK |
| airbeams_pinewood_morn | delta | +0.08 | +0.09 | -0.04 | -0.01 | -0.02 |  | +0.00 | +0.00 | +0.00 | +0.01 |  | +0.0% | +0.0% |  |  |
| airhaze_valley_0515 | off | 23.98 | 22.57 | 0.04 | 1.26 | 0.72 |  | 0.06 | 0.07 | 0.31 | 0.11 | 31.40 | 11,406 | 7,966 | OK:30 | OK |
| airhaze_valley_0515 | on | 23.84 | 22.53 | 0.04 | 1.16 | 0.63 |  | 0.06 | 0.07 | 0.31 | 0.10 | 31.50 | 11,406 | 7,966 | OK:30 | OK |
| airhaze_valley_0515 | delta | -0.14 | -0.04 | +0.00 | -0.10 | -0.09 |  | +0.00 | +0.00 | +0.00 | -0.01 |  | +0.0% | +0.0% |  |  |
| hfrt_hill_1730 | off | 3.37 | 2.51 | 0.12 | 0.67 | 0.22 |  | 0.17 | 0.17 |  | 0.11 | 288.90 | 1,662 | 934 | OK:30 | OK |
| hfrt_hill_1730 | on | 2.25 | 1.69 | 0.08 | 0.42 | 0.16 |  | 0.09 | 0.10 |  | 0.07 | 289.70 | 1,662 | 934 | OK:30 | OK |
| hfrt_hill_1730 | delta | -1.12 | -0.82 | -0.04 | -0.25 | -0.06 |  | -0.08 | -0.07 |  | -0.04 |  | +0.0% | +0.0% |  |  |
| clouds_coast_oc_up | off | 4.12 | 1.57 | 0.00 | 2.10 | 0.33 |  | 0.19 | 0.20 | 1.16 | 0.21 | 241.70 | 186 | 0 | OK:30 | OK |
| clouds_coast_oc_up | on | 5.14 | 2.00 | 0.00 | 2.67 | 0.36 |  | 0.23 | 0.34 | 1.39 | 0.34 | 223.90 | 186 | 0 | OK:30 | OK |
| clouds_coast_oc_up | delta | +1.02 | +0.43 | +0.00 | +0.57 | +0.03 |  | +0.04 | +0.14 | +0.23 | +0.13 |  | +0.0% |  |  |  |
| texsharp_forest_edge | off | 5.68 | 4.93 | 0.27 | 0.55 | 0.22 |  | 0.19 | 0.08 |  | 0.06 | 119.50 | 2,076 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | on | 6.02 | 5.23 | 0.28 | 0.59 | 0.23 |  | 0.20 | 0.10 |  | 0.06 | 115.40 | 2,076 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | delta | +0.34 | +0.30 | +0.01 | +0.04 | +0.01 |  | +0.01 | +0.02 |  | +0.00 |  | +0.0% | +0.0% |  |  |
