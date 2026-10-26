# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| atmosphere_coastline_sunset | off | 5.08 | 2.10 | 0.00 |  |  | 2.69 | 0.39 |  | 0.39 | 0.26 | 1.43 | 0.23 | 240.70 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | on | 5.15 | 2.23 | 0.00 | 0.29 |  | 2.40 | 0.33 |  | 0.29 | 0.18 | 1.37 | 0.23 | 207.00 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | delta | +0.07 | +0.13 | +0.00 |  |  | -0.29 | -0.06 |  | -0.10 | -0.08 | -0.06 | +0.00 |  | +0.0% |  |  |  |
| atmosphere_firefight_dusk | off | 1.25 | 0.54 |  |  |  | 0.51 | 0.26 |  | 0.14 | 0.07 |  | 0.04 | 330.40 | 127 | 70 | OK:30 | OK |
| atmosphere_firefight_dusk | on | 1.72 | 1.01 |  | 0.10 |  | 0.46 | 0.21 |  | 0.14 | 0.07 |  | 0.05 | 391.90 | 127 | 70 | OK:30 | OK |
| atmosphere_firefight_dusk | delta | +0.47 | +0.47 |  |  |  | -0.05 | -0.05 |  | +0.00 | +0.00 |  | +0.01 |  | +0.0% | +0.0% |  |  |
| atmosphere_noon_fields | off | 16.79 | 16.22 | 0.07 |  |  | 0.43 | 0.22 |  | 0.11 | 0.06 |  | 0.04 | 47.40 | 6,347 | 4,506 | OK:30 | OK |
| atmosphere_noon_fields | on | 21.11 | 20.49 | 0.08 | 0.08 |  | 0.44 | 0.23 |  | 0.11 | 0.07 |  | 0.04 | 41.10 | 6,347 | 4,506 | OK:30 | OK |
| atmosphere_noon_fields | delta | +4.32 | +4.27 | +0.01 |  |  | +0.01 | +0.01 |  | +0.00 | +0.01 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| atmosphere_sunrise_ridge | off | 24.60 | 23.46 | 0.08 |  |  | 0.97 | 0.46 |  | 0.13 | 0.06 | 0.27 | 0.06 | 37.10 | 12,910 | 8,897 | OK:30 | OK |
| atmosphere_sunrise_ridge | on | 24.86 | 23.61 | 0.11 | 0.10 |  | 1.05 | 0.49 |  | 0.13 | 0.06 | 0.27 | 0.10 | 37.00 | 12,910 | 8,897 | OK:30 | OK |
| atmosphere_sunrise_ridge | delta | +0.26 | +0.15 | +0.03 |  |  | +0.08 | +0.03 |  | +0.00 | +0.00 | +0.00 | +0.04 |  | +0.0% | +0.0% |  |  |
| atmosphere_coast_oc_up | off | 3.29 | 1.30 | 0.00 |  |  | 1.62 | 0.29 |  | 0.15 | 0.15 | 0.87 | 0.17 | 271.40 | 186 | 0 | OK:30 | OK |
| atmosphere_coast_oc_up | on | 5.27 | 2.27 | 0.00 | 0.50 |  | 2.22 | 0.39 |  | 0.18 | 0.22 | 1.10 | 0.34 | 201.10 | 186 | 0 | OK:30 | OK |
| atmosphere_coast_oc_up | delta | +1.98 | +0.97 | +0.00 |  |  | +0.60 | +0.10 |  | +0.03 | +0.07 | +0.23 | +0.17 |  | +0.0% |  |  |  |
| atmosphere_town_night | off | 5.03 | 3.60 |  |  |  | 1.07 | 0.37 |  | 0.34 | 0.24 |  | 0.11 | 140.70 | 777 | 546 | OK:30 | OK |
| atmosphere_town_night | on | 5.30 | 3.97 |  |  |  | 0.93 | 0.29 |  | 0.36 | 0.16 |  | 0.12 | 135.30 | 777 | 546 | OK:30 | OK |
| atmosphere_town_night | delta | +0.27 | +0.37 |  |  |  | -0.14 | -0.08 |  | +0.02 | -0.08 |  | +0.01 |  | +0.0% | +0.0% |  |  |
