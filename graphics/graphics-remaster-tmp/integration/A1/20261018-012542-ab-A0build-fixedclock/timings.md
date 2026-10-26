# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| perf_sunrise_ridge | off | 38.58 | 36.73 | 0.33 | 1.69 | 0.61 | 0.06 | 0.27 | 0.75 | 18.70 | 12,910 | 8,897 | OK:30 | FAIL:unknown knob drawGrouping |
| perf_sunrise_ridge | on | 40.34 | 38.29 | 0.11 | 1.89 | 0.74 | 0.06 | 0.27 | 0.82 | 19.20 | 12,910 | 8,897 | OK:30 | OK |
| perf_sunrise_ridge | delta | +1.76 | +1.56 | -0.22 | +0.20 | +0.13 | +0.00 | +0.00 | +0.07 |  | +0.0% | +0.0% |  |  |
| temporal_forest_edge_golden | off | 6.68 | 5.97 | 0.21 | 0.54 | 0.43 | 0.07 |  | 0.04 | 119.60 | 2,142 | 1,459 | OK:30 | FAIL:unknown knob taa |
| temporal_forest_edge_golden | on | 4.48 | 3.84 | 0.21 | 0.47 | 0.36 | 0.07 |  | 0.04 | 136.30 | 2,142 | 1,459 | OK:30 | OK |
| temporal_forest_edge_golden | delta | -2.20 | -2.13 | +0.00 | -0.07 | -0.07 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| lighttiles_town_night | off | 13.60 | 12.92 |  | 0.49 | 0.39 | 0.06 |  | 0.04 | 55.10 | 1,276 | 1,104 | OK:290 | FAIL:unknown knob lightTiles |
| lighttiles_town_night | on | 9.32 | 8.52 |  | 0.61 | 0.50 | 0.06 |  | 0.04 | 96.00 | 1,288 | 1,123 | OK:101 | OK |
| lighttiles_town_night | delta | -4.28 | -4.40 |  | +0.12 | +0.11 | +0.00 |  | +0.00 |  | +0.9% | +1.7% |  |  |
| atmosphere_coastline_sunset | off | 1.72 | 0.53 | 0.00 | 1.05 | 0.45 | 0.07 | 0.47 | 0.06 | 332.40 | 176 | 0 | OK:30 | FAIL:unknown knob atmosphere |
| atmosphere_coastline_sunset | on | 1.86 | 0.71 | 0.00 | 1.02 | 0.40 | 0.06 | 0.43 | 0.13 | 311.20 | 176 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | delta | +0.14 | +0.18 | +0.00 | -0.03 | -0.05 | -0.01 | -0.04 | +0.07 |  | +0.0% |  |  |  |
| shadows_woods_pine2_evening | off | 20.00 | 17.04 | 1.48 | 2.72 | 0.35 | 0.27 | 1.11 | 0.98 | 54.70 | 2,830 | 1,810 | OK:30 | FAIL:unknown knob treeShadows |
| shadows_woods_pine2_evening | on | 18.26 | 15.36 | 1.22 | 2.75 | 0.37 | 0.40 | 1.00 | 0.98 | 60.50 | 2,830 | 1,810 | OK:30 | OK |
| shadows_woods_pine2_evening | delta | -1.74 | -1.68 | -0.26 | +0.03 | +0.02 | +0.13 | -0.11 | +0.00 |  | +0.0% | +0.0% |  |  |
| shade_lowsun_fp | off | 7.85 | 6.77 | 0.14 | 0.93 | 0.41 | 0.06 | 0.37 | 0.09 | 101.80 | 1,891 | 1,316 | OK:30 | FAIL:unknown knob lowSunRig |
| shade_lowsun_fp | on | 6.28 | 4.83 | 0.08 | 1.30 | 0.35 | 0.06 | 0.37 | 0.51 | 114.10 | 1,891 | 1,316 | OK:30 | OK |
| shade_lowsun_fp | delta | -1.57 | -1.94 | -0.06 | +0.37 | -0.06 | +0.00 | +0.00 | +0.42 |  | +0.0% | +0.0% |  |  |
| night_road_flat_drive_moonless | off | 6.57 | 5.89 |  | 0.51 | 0.41 | 0.06 |  | 0.04 | 115.60 | 1,807 | 914 | OK:30 | FAIL:unknown knob nightLights |
| night_road_flat_drive_moonless | on | 4.13 | 3.47 |  | 0.48 | 0.38 | 0.06 |  | 0.04 | 138.10 | 1,807 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | delta | -2.44 | -2.42 |  | -0.03 | -0.03 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| combat_ffh | off | 9.68 | 8.80 |  | 0.72 | 0.62 | 0.06 |  | 0.04 | 118.60 | 805 | 693 | UNSETTLED:300 | FAIL:unknown knob combatLights |
| combat_ffh | on | 7.30 | 6.63 |  | 0.52 | 0.42 | 0.06 |  | 0.04 | 104.60 | 786 | 678 | UNSETTLED:300 | OK |
| combat_ffh | delta | -2.38 | -2.17 |  | -0.20 | -0.20 | +0.00 |  | +0.00 |  | -2.4% | -2.2% |  |  |
| water_coastline_sunset | off | 2.16 | 0.99 | 0.00 | 1.04 | 0.36 | 0.06 | 0.29 | 0.34 | 314.20 | 177 | 0 | OK:30 | FAIL:unknown knob seaWaves |
| water_coastline_sunset | on | 3.23 | 1.73 | 0.00 | 1.38 | 0.32 | 0.06 | 0.29 | 0.71 | 311.30 | 177 | 0 | OK:30 | OK |
| water_coastline_sunset | delta | +1.07 | +0.74 | +0.00 | +0.34 | -0.04 | +0.00 | +0.00 | +0.37 |  | +0.0% |  |  |  |
| ssr_bay_offshore | off | 17.68 | 17.17 | 0.03 | 0.38 | 0.27 | 0.06 |  | 0.04 | 50.00 | 4,557 | 2,961 | OK:30 | FAIL:unknown knob ssr |
| ssr_bay_offshore | on | 10.77 | 10.19 | 0.03 | 0.45 | 0.35 | 0.06 |  | 0.04 | 64.00 | 4,557 | 2,961 | OK:30 | OK |
| ssr_bay_offshore | delta | -6.91 | -6.98 | +0.00 | +0.07 | +0.08 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| materials_town_night_rain | off | 6.87 | 6.18 |  | 0.52 | 0.41 | 0.06 |  | 0.04 | 126.90 | 825 | 548 | OK:31 | FAIL:unknown knob wetWeather |
| materials_town_night_rain | on | 6.02 | 5.41 |  | 0.44 | 0.33 | 0.06 |  | 0.04 | 138.70 | 811 | 545 | OK:51 | OK |
| materials_town_night_rain | delta | -0.85 | -0.77 |  | -0.08 | -0.08 | +0.00 |  | +0.00 |  | -1.7% | -0.5% |  |  |
| airbeams_pinewood_morn | off | 7.04 | 5.52 | 0.67 | 1.34 | 0.43 | 0.07 | 0.27 | 0.57 | 109.90 | 2,845 | 1,678 | OK:30 | FAIL:unknown knob airBeams |
| airbeams_pinewood_morn | on | 6.08 | 5.03 | 0.43 | 0.87 | 0.40 | 0.07 | 0.27 | 0.13 | 111.00 | 2,845 | 1,678 | OK:30 | OK |
| airbeams_pinewood_morn | delta | -0.96 | -0.49 | -0.24 | -0.47 | -0.03 | +0.00 | +0.00 | -0.44 |  | +0.0% | +0.0% |  |  |
| airhaze_valley_0515 | off | 45.73 | 43.30 | 0.04 | 2.28 | 1.20 | 0.07 | 0.31 | 0.69 | 17.30 | 11,405 | 7,966 | OK:30 | FAIL:unknown knob airHaze |
| airhaze_valley_0515 | on | 45.03 | 42.99 | 0.17 | 1.90 | 0.85 | 0.07 | 0.31 | 0.67 | 19.40 | 11,404 | 7,966 | OK:30 | OK |
| airhaze_valley_0515 | delta | -0.70 | -0.31 | +0.13 | -0.38 | -0.35 | +0.00 | +0.00 | -0.02 |  | -0.0% | +0.0% |  |  |
| hfrt_hill_1730 | off | 2.21 | 1.51 | 0.03 | 0.59 | 0.48 | 0.07 |  | 0.04 | 287.10 | 1,662 | 934 | OK:30 | FAIL:unknown knob canopyShadows |
| hfrt_hill_1730 | on | 4.26 | 3.08 | 0.12 | 1.04 | 0.58 | 0.37 |  | 0.09 | 283.30 | 1,662 | 934 | OK:30 | OK |
| hfrt_hill_1730 | delta | +2.05 | +1.57 | +0.09 | +0.45 | +0.10 | +0.30 |  | +0.05 |  | +0.0% | +0.0% |  |  |
| clouds_coast_oc_up | off | 2.43 | 0.93 | 0.00 | 1.22 | 0.41 | 0.11 | 0.57 | 0.12 | 271.30 | 185 | 0 | OK:30 | FAIL:unknown knob litClouds |
| clouds_coast_oc_up | on | 3.13 | 1.26 | 0.00 | 1.59 | 0.50 | 0.12 | 0.58 | 0.38 | 251.80 | 185 | 0 | OK:30 | OK |
| clouds_coast_oc_up | delta | +0.70 | +0.33 | +0.00 | +0.37 | +0.09 | +0.01 | +0.01 | +0.26 |  | +0.0% |  |  |  |
| texsharp_forest_edge | off | 5.31 | 4.62 | 0.20 | 0.53 | 0.42 | 0.07 |  | 0.04 | 99.60 | 2,075 | 1,447 | OK:30 | FAIL:unknown knob terrainDetail |
| texsharp_forest_edge | on | 8.06 | 7.50 | 0.20 | 0.40 | 0.29 | 0.07 |  | 0.04 | 116.30 | 2,075 | 1,447 | OK:30 | OK |
| texsharp_forest_edge | delta | +2.75 | +2.88 | +0.00 | -0.13 | -0.13 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
