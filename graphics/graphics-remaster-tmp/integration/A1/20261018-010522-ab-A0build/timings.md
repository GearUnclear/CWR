# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| perf_sunrise_ridge | off | 17.85 | 16.67 | 0.08 | 1.02 | 0.59 | 0.06 | 0.27 | 0.10 | 37.20 | 12,910 | 8,897 | OK:30 | FAIL:unknown knob drawGrouping |
| perf_sunrise_ridge | on | 17.51 | 16.35 | 0.09 | 1.00 | 0.60 | 0.06 | 0.27 | 0.07 | 38.10 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | delta | -0.34 | -0.32 | +0.01 | -0.02 | +0.01 | +0.00 | +0.00 | -0.03 |  | +0.0% | +0.0% |  |  |
| temporal_forest_edge_golden | off | 5.58 | 4.86 | 0.26 | 0.52 | 0.38 | 0.08 |  | 0.06 | 124.10 | 2,142 | 1,459 | OK:30 | FAIL:unknown knob taa |
| temporal_forest_edge_golden | on | 5.90 | 5.13 | 0.31 | 0.56 | 0.41 | 0.09 |  | 0.06 | 118.60 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | delta | +0.32 | +0.27 | +0.05 | +0.04 | +0.03 | +0.01 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| lighttiles_town_night | off | 6.18 | 5.31 |  | 0.65 | 0.52 | 0.08 |  | 0.05 | 123.80 | 1,225 | 1,046 | OK:225 | FAIL:unknown knob lightTiles |
| lighttiles_town_night | on | 6.25 | 5.37 |  | 0.66 | 0.53 | 0.08 |  | 0.05 | 123.20 | 1,204 | 1,042 | OK:30 | OK |
| lighttiles_town_night | delta | +0.07 | +0.06 |  | +0.01 | +0.01 | +0.00 |  | +0.00 |  | -1.7% | -0.4% |  |  |
| atmosphere_coastline_sunset | off | 4.20 | 1.81 | 0.00 | 2.13 | 0.57 | 0.16 | 1.19 | 0.21 | 269.50 | 175 | 0 | OK:30 | FAIL:unknown knob atmosphere |
| atmosphere_coastline_sunset | on | 4.66 | 1.73 | 0.00 | 2.63 | 0.72 | 0.19 | 1.43 | 0.30 | 246.80 | 176 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | delta | +0.46 | -0.08 | +0.00 | +0.50 | +0.15 | +0.03 | +0.24 | +0.09 |  | +0.6% |  |  |  |
| shadows_woods_pine2_evening | off | 6.50 | 5.27 | 0.34 | 1.09 | 0.45 | 0.07 | 0.49 | 0.08 | 112.50 | 2,830 | 1,810 | OK:30 | FAIL:unknown knob treeShadows |
| shadows_woods_pine2_evening | on | 6.45 | 5.18 | 0.34 | 1.13 | 0.46 | 0.07 | 0.50 | 0.10 | 112.90 | 2,830 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | delta | -0.05 | -0.09 | +0.00 | +0.04 | +0.01 | +0.00 | +0.01 | +0.02 |  | +0.0% | +0.0% |  |  |
| shade_lowsun_fp | off | 5.69 | 4.42 | 0.09 | 1.09 | 0.44 | 0.08 | 0.49 | 0.08 | 127.40 | 1,891 | 1,316 | OK:30 | FAIL:unknown knob lowSunRig |
| shade_lowsun_fp | on | 6.00 | 4.70 | 0.09 | 1.12 | 0.44 | 0.09 | 0.50 | 0.08 | 122.70 | 1,891 | 1,316 | OK:30 | OK |
| shade_lowsun_fp | delta | +0.31 | +0.28 | +0.00 | +0.03 | +0.00 | +0.01 | +0.01 | +0.00 |  | +0.0% | +0.0% |  |  |
| night_road_flat_drive_moonless | off | 5.98 | 5.10 |  | 0.65 | 0.48 | 0.11 |  | 0.06 | 122.60 | 1,807 | 914 | OK:30 | FAIL:unknown knob nightLights |
| night_road_flat_drive_moonless | on | 5.74 | 4.88 |  | 0.63 | 0.48 | 0.09 |  | 0.06 | 123.50 | 1,807 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | delta | -0.24 | -0.22 |  | -0.02 | +0.00 | -0.02 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| combat_ffh | off | 6.24 | 5.33 |  | 0.73 | 0.59 | 0.08 |  | 0.06 | 126.70 | 797 | 675 | UNSETTLED:300 | FAIL:unknown knob combatLights |
| combat_ffh | on | 6.04 | 5.14 |  | 0.71 | 0.56 | 0.09 |  | 0.06 | 128.60 | 751 | 648 | UNSETTLED:300 | OK |
| combat_ffh | delta | -0.20 | -0.19 |  | -0.02 | -0.03 | +0.01 |  | +0.00 |  | -5.8% | -4.0% |  |  |
| water_coastline_sunset | off | 3.74 | 1.40 | 0.00 | 2.08 | 0.66 | 0.20 | 0.88 | 0.34 | 271.60 | 177 | 0 | OK:30 | FAIL:unknown knob seaWaves |
| water_coastline_sunset | on | 2.02 | 0.71 | 0.00 | 1.14 | 0.48 | 0.11 | 0.44 | 0.11 | 337.70 | 177 | 0 | OK:30 | OK |
| water_coastline_sunset | delta | -1.72 | -0.69 | +0.00 | -0.94 | -0.18 | -0.09 | -0.44 | -0.23 |  | +0.0% |  |  |  |
| ssr_bay_offshore | off | 7.57 | 6.96 | 0.03 | 0.47 | 0.37 | 0.06 |  | 0.04 | 82.30 | 4,557 | 2,961 | OK:30 | FAIL:unknown knob ssr |
| ssr_bay_offshore | on | 7.58 | 6.96 | 0.03 | 0.48 | 0.38 | 0.06 |  | 0.04 | 82.70 | 4,557 | 2,961 | OK:30 | OK |
| ssr_bay_offshore | delta | +0.01 | +0.00 | +0.00 | +0.01 | +0.01 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| materials_town_night_rain | off | 5.98 | 5.13 |  | 0.65 | 0.51 | 0.08 |  | 0.05 | 137.80 | 1,008 | 772 | OK:263 | FAIL:unknown knob wetWeather |
| materials_town_night_rain | on | 5.95 | 5.08 |  | 0.66 | 0.53 | 0.08 |  | 0.05 | 137.20 | 1,015 | 770 | OK:119 | OK |
| materials_town_night_rain | delta | -0.03 | -0.05 |  | +0.01 | +0.02 | +0.00 |  | +0.00 |  | +0.7% | -0.3% |  |  |
| airbeams_pinewood_morn | off | 5.95 | 4.73 | 0.57 | 1.00 | 0.47 | 0.08 | 0.35 | 0.10 | 116.90 | 2,845 | 1,678 | OK:30 | FAIL:unknown knob airBeams |
| airbeams_pinewood_morn | on | 6.02 | 4.76 | 0.57 | 1.04 | 0.45 | 0.09 | 0.36 | 0.14 | 114.60 | 2,845 | 1,678 | OK:30 | OK |
| airbeams_pinewood_morn | delta | +0.07 | +0.03 | +0.00 | +0.04 | -0.02 | +0.01 | +0.01 | +0.04 |  | +0.0% | +0.0% |  |  |
| airhaze_valley_0515 | off | 24.09 | 22.71 | 0.04 | 1.23 | 0.77 | 0.07 | 0.31 | 0.08 | 31.50 | 11,405 | 7,966 | OK:30 | FAIL:unknown knob airHaze |
| airhaze_valley_0515 | on | 23.82 | 22.45 | 0.06 | 1.23 | 0.76 | 0.07 | 0.31 | 0.09 | 31.60 | 11,405 | 7,966 | OK:30 | OK |
| airhaze_valley_0515 | delta | -0.27 | -0.26 | +0.02 | +0.00 | -0.01 | +0.00 | +0.00 | +0.01 |  | +0.0% | +0.0% |  |  |
| hfrt_hill_1730 | off | 3.20 | 2.37 | 0.10 | 0.66 | 0.37 | 0.18 |  | 0.11 | 294.90 | 1,662 | 934 | OK:30 | FAIL:unknown knob canopyShadows |
| hfrt_hill_1730 | on | 1.93 | 1.45 | 0.05 | 0.36 | 0.22 | 0.09 |  | 0.05 | 294.70 | 1,662 | 934 | OK:30 | OK |
| hfrt_hill_1730 | delta | -1.27 | -0.92 | -0.05 | -0.30 | -0.15 | -0.09 |  | -0.06 |  | +0.0% | +0.0% |  |  |
| clouds_coast_oc_up | off | 4.67 | 1.85 | 0.00 | 2.39 | 0.59 | 0.20 | 1.19 | 0.41 | 239.50 | 186 | 0 | OK:30 | FAIL:unknown knob litClouds |
| clouds_coast_oc_up | on | 4.95 | 1.86 | 0.00 | 2.56 | 0.64 | 0.29 | 1.37 | 0.26 | 222.00 | 185 | 0 | OK:30 | OK |
| clouds_coast_oc_up | delta | +0.28 | +0.01 | +0.00 | +0.17 | +0.05 | +0.09 | +0.18 | -0.15 |  | -0.5% |  |  |  |
| texsharp_forest_edge | off | 5.65 | 4.91 | 0.28 | 0.56 | 0.42 | 0.09 |  | 0.05 | 117.60 | 2,075 | 1,447 | OK:30 | FAIL:unknown knob terrainDetail |
| texsharp_forest_edge | on | 6.14 | 5.39 | 0.29 | 0.56 | 0.41 | 0.10 |  | 0.06 | 115.70 | 2,075 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | delta | +0.49 | +0.48 | +0.01 | +0.00 | -0.01 | +0.01 |  | +0.01 |  | +0.0% | +0.0% |  |  |
