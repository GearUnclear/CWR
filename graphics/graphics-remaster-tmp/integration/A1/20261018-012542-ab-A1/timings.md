# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| perf_sunrise_ridge | off | 41.97 | 40.32 | 0.29 | 1.49 | 0.65 | 0.06 | 0.27 | 0.51 | 18.50 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | on | 41.89 | 40.07 | 0.60 | 1.65 | 0.49 | 0.06 | 0.27 | 0.83 | 19.10 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | delta | -0.08 | -0.25 | +0.31 | +0.16 | -0.16 | +0.00 | +0.00 | +0.32 |  | +0.0% | +0.0% |  |  |
| temporal_forest_edge_golden | off | 4.81 | 4.20 | 0.21 | 0.45 | 0.34 | 0.07 |  | 0.04 | 121.00 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | on | 5.84 | 5.22 | 0.21 | 0.45 | 0.34 | 0.07 |  | 0.04 | 123.60 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | delta | +1.03 | +1.02 | +0.00 | +0.00 | +0.00 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| lighttiles_town_night | off | 17.79 | 17.20 |  | 0.39 | 0.29 | 0.06 |  | 0.04 | 51.80 | 1,296 | 1,122 | OK:257 | OK |
| lighttiles_town_night | on | 15.35 | 14.72 |  | 0.43 | 0.33 | 0.06 |  | 0.04 | 63.30 | 1,275 | 1,117 | OK:30 | OK |
| lighttiles_town_night | delta | -2.44 | -2.48 |  | +0.04 | +0.04 | +0.00 |  | +0.00 |  | -1.6% | -0.4% |  |  |
| atmosphere_coastline_sunset | off | 1.83 | 0.56 | 0.00 | 1.13 | 0.52 | 0.07 | 0.47 | 0.06 | 339.00 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | on | 2.22 | 0.90 | 0.00 | 1.19 | 0.40 | 0.07 | 0.47 | 0.25 | 365.80 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | delta | +0.39 | +0.34 | +0.00 | +0.06 | -0.12 | +0.00 | +0.00 | +0.19 |  | +0.0% |  |  |  |
| shadows_woods_pine2_evening | off | 17.88 | 15.11 | 1.32 | 2.45 | 0.38 | 0.42 | 0.80 | 0.86 | 71.20 | 2,831 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | on | 20.48 | 17.33 | 1.41 | 2.84 | 0.36 | 0.36 | 1.00 | 1.12 | 53.50 | 2,831 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | delta | +2.60 | +2.22 | +0.09 | +0.39 | -0.02 | -0.06 | +0.20 | +0.26 |  | +0.0% | +0.0% |  |  |
| shade_lowsun_fp | off | 8.64 | 6.94 | 0.17 | 1.55 | 0.55 | 0.06 | 0.37 | 0.58 | 104.00 | 1,890 | 1,316 | OK:30 | OK |
| shade_lowsun_fp | on | 5.39 | 4.34 | 0.09 | 0.90 | 0.39 | 0.06 | 0.37 | 0.08 | 118.70 | 1,890 | 1,316 | OK:30 | OK |
| shade_lowsun_fp | delta | -3.25 | -2.60 | -0.08 | -0.65 | -0.16 | +0.00 | +0.00 | -0.50 |  | +0.0% | +0.0% |  |  |
| night_road_flat_drive_moonless | off | 5.01 | 4.37 |  | 0.47 | 0.37 | 0.06 |  | 0.04 | 109.80 | 1,806 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | on | 7.63 | 6.94 |  | 0.53 | 0.43 | 0.06 |  | 0.04 | 116.80 | 1,806 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | delta | +2.62 | +2.57 |  | +0.06 | +0.06 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| combat_ffh | off | 8.81 | 8.00 |  | 0.61 | 0.51 | 0.06 |  | 0.04 | 97.70 | 774 | 678 | UNSETTLED:300 | OK |
| combat_ffh | on | 13.72 | 13.15 |  | 0.42 | 0.32 | 0.06 |  | 0.04 | 66.20 | 763 | 657 | UNSETTLED:300 | OK |
| combat_ffh | delta | +4.91 | +5.15 |  | -0.19 | -0.19 | +0.00 |  | +0.00 |  | -1.4% | -3.1% |  |  |
| water_coastline_sunset | off | 2.40 | 0.95 | 0.00 | 1.34 | 0.41 | 0.06 | 0.29 | 0.58 | 350.00 | 176 | 0 | OK:30 | OK |
| water_coastline_sunset | on | 1.96 | 0.85 | 0.00 | 0.98 | 0.37 | 0.06 | 0.29 | 0.26 | 363.40 | 176 | 0 | OK:30 | OK |
| water_coastline_sunset | delta | -0.44 | -0.10 | +0.00 | -0.36 | -0.04 | +0.00 | +0.00 | -0.32 |  | +0.0% |  |  |  |
| ssr_bay_offshore | off | 13.03 | 12.51 | 0.03 | 0.37 | 0.27 | 0.06 |  | 0.04 | 59.20 | 4,559 | 2,961 | OK:30 | OK |
| ssr_bay_offshore | on | 14.21 | 13.56 | 0.03 | 0.50 | 0.40 | 0.06 |  | 0.04 | 51.50 | 4,559 | 2,961 | OK:30 | OK |
| ssr_bay_offshore | delta | +1.18 | +1.05 | +0.00 | +0.13 | +0.13 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| materials_town_night_rain | off | 8.17 | 7.35 |  | 0.63 | 0.53 | 0.07 |  | 0.04 | 108.30 | 993 | 758 | OK:214 | OK |
| materials_town_night_rain | on | 6.05 | 5.24 |  | 0.63 | 0.53 | 0.06 |  | 0.04 | 132.40 | 1,115 | 886 | OK:182 | OK |
| materials_town_night_rain | delta | -2.12 | -2.11 |  | +0.00 | +0.00 | -0.01 |  | +0.00 |  | +10.9% | +14.4% |  |  |
| airbeams_pinewood_morn | off | 9.10 | 7.79 | 0.84 | 1.14 | 0.31 | 0.07 | 0.27 | 0.49 | 90.30 | 2,845 | 1,678 | OK:30 | OK |
| airbeams_pinewood_morn | on | 9.92 | 8.45 | 1.02 | 1.29 | 0.29 | 0.07 | 0.27 | 0.66 | 80.90 | 2,845 | 1,678 | OK:30 | OK |
| airbeams_pinewood_morn | delta | +0.82 | +0.66 | +0.18 | +0.15 | -0.02 | +0.00 | +0.00 | +0.17 |  | +0.0% | +0.0% |  |  |
| airhaze_valley_0515 | off | 46.04 | 44.05 | 0.04 | 1.85 | 0.83 | 0.07 | 0.31 | 0.64 | 19.50 | 11,406 | 7,966 | OK:30 | OK |
| airhaze_valley_0515 | on | 27.43 | 25.91 | 0.04 | 1.37 | 0.80 | 0.07 | 0.31 | 0.19 | 26.80 | 11,406 | 7,966 | OK:30 | OK |
| airhaze_valley_0515 | delta | -18.61 | -18.14 | +0.00 | -0.48 | -0.03 | +0.00 | +0.00 | -0.45 |  | +0.0% | +0.0% |  |  |
| hfrt_hill_1730 | off | 4.63 | 3.77 | 0.39 | 0.70 | 0.37 | 0.25 |  | 0.08 | 285.60 | 1,662 | 934 | OK:30 | OK |
| hfrt_hill_1730 | on | 2.48 | 1.81 | 0.06 | 0.50 | 0.31 | 0.12 |  | 0.07 | 284.20 | 1,662 | 934 | OK:30 | OK |
| hfrt_hill_1730 | delta | -2.15 | -1.96 | -0.33 | -0.20 | -0.06 | -0.13 |  | -0.01 |  | +0.0% | +0.0% |  |  |
| clouds_coast_oc_up | off | 3.64 | 1.61 | 0.00 | 1.72 | 0.39 | 0.28 | 0.69 | 0.37 | 285.60 | 186 | 0 | OK:30 | OK |
| clouds_coast_oc_up | on | 1.73 | 0.72 | 0.00 | 0.81 | 0.32 | 0.06 | 0.34 | 0.10 | 351.00 | 186 | 0 | OK:30 | OK |
| clouds_coast_oc_up | delta | -1.91 | -0.89 | +0.00 | -0.91 | -0.07 | -0.22 | -0.35 | -0.27 |  | +0.0% |  |  |  |
| texsharp_forest_edge | off | 5.61 | 4.98 | 0.20 | 0.46 | 0.35 | 0.07 |  | 0.04 | 105.00 | 2,076 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | on | 5.31 | 4.54 | 0.27 | 0.58 | 0.45 | 0.08 |  | 0.05 | 131.30 | 2,076 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | delta | -0.30 | -0.44 | +0.07 | +0.12 | +0.10 | +0.01 |  | +0.01 |  | +0.0% | +0.0% |  |  |
