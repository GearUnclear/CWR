# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| atmosphere_pillarbox | off | 4.06 | 1.25 | 0.00 |  |  | 2.57 | 0.40 |  | 0.29 | 0.17 | 1.24 | 0.47 | 299.10 | 145 | 0 | OK:30 | OK |
| atmosphere_pillarbox | on | 4.73 | 1.63 | 0.00 | 0.23 |  | 2.54 | 0.31 |  | 0.27 | 0.21 | 1.47 | 0.27 | 241.70 | 145 | 0 | OK:30 | OK |
| atmosphere_pillarbox | delta | +0.67 | +0.38 | +0.00 |  |  | -0.03 | -0.09 |  | -0.02 | +0.04 | +0.23 | -0.20 |  | +0.0% |  |  |  |
| atmosphere_coastline_sunset | off | 4.15 | 1.40 | 0.00 |  |  | 2.48 | 0.33 |  | 0.26 | 0.21 | 1.22 | 0.47 | 262.50 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | on | 5.18 | 2.10 | 0.00 | 0.28 |  | 2.46 | 0.34 |  | 0.31 | 0.22 | 1.34 | 0.25 | 215.20 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | delta | +1.03 | +0.70 | +0.00 |  |  | -0.02 | +0.01 |  | +0.05 | +0.01 | +0.12 | -0.22 |  | +0.0% |  |  |  |
| atmosphere_firefight_dusk | off | 3.21 | 0.96 |  |  |  | 1.61 | 0.94 |  | 0.22 | 0.18 |  | 0.27 | 234.80 | 127 | 70 | OK:30 | OK |
| atmosphere_firefight_dusk | on | 3.16 | 1.41 |  | 0.30 |  | 1.08 | 0.31 |  | 0.35 | 0.29 |  | 0.14 | 313.90 | 127 | 70 | OK:30 | OK |
| atmosphere_firefight_dusk | delta | -0.05 | +0.45 |  |  |  | -0.53 | -0.63 |  | +0.13 | +0.11 |  | -0.13 |  | +0.0% | +0.0% |  |  |
| atmosphere_noon_fields | off | 9.40 | 8.81 | 0.07 |  |  | 0.45 | 0.24 |  | 0.11 | 0.06 |  | 0.04 | 68.70 | 6,347 | 4,506 | OK:30 | OK |
| atmosphere_noon_fields | on | 9.89 | 9.28 | 0.06 | 0.08 |  | 0.42 | 0.20 |  | 0.11 | 0.06 |  | 0.04 | 66.30 | 6,347 | 4,506 | OK:30 | OK |
| atmosphere_noon_fields | delta | +0.49 | +0.47 | -0.01 |  |  | -0.03 | -0.04 |  | +0.00 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| atmosphere_sunrise_ridge | off | 24.78 | 23.64 | 0.08 |  |  | 0.99 | 0.47 |  | 0.13 | 0.06 | 0.27 | 0.06 | 37.40 | 12,910 | 8,897 | OK:30 | OK |
| atmosphere_sunrise_ridge | on | 24.67 | 23.45 | 0.08 | 0.10 |  | 0.99 | 0.46 |  | 0.13 | 0.06 | 0.27 | 0.07 | 37.20 | 12,910 | 8,897 | OK:30 | OK |
| atmosphere_sunrise_ridge | delta | -0.11 | -0.19 | +0.00 |  |  | +0.00 | -0.01 |  | +0.00 | +0.00 | +0.00 | +0.01 |  | +0.0% | +0.0% |  |  |
| atmosphere_coast_oc_up | off | 3.41 | 1.28 | 0.00 |  |  | 1.72 | 0.34 |  | 0.15 | 0.15 | 0.90 | 0.17 | 273.70 | 186 | 0 | OK:30 | OK |
| atmosphere_coast_oc_up | on | 4.98 | 1.93 | 0.00 | 0.51 |  | 2.07 | 0.34 |  | 0.18 | 0.19 | 1.13 | 0.23 | 200.40 | 186 | 0 | OK:30 | OK |
| atmosphere_coast_oc_up | delta | +1.57 | +0.65 | +0.00 |  |  | +0.35 | +0.00 |  | +0.03 | +0.04 | +0.23 | +0.06 |  | +0.0% |  |  |  |
| atmosphere_town_night | off | 4.69 | 3.29 |  |  |  | 1.06 | 0.41 |  | 0.30 | 0.17 |  | 0.19 | 153.30 | 777 | 546 | OK:30 | OK |
| atmosphere_town_night | on | 5.14 | 3.79 |  |  |  | 1.00 | 0.35 |  | 0.34 | 0.19 |  | 0.11 | 142.10 | 777 | 546 | OK:30 | OK |
| atmosphere_town_night | delta | +0.45 | +0.50 |  |  |  | -0.06 | -0.06 |  | +0.04 | +0.02 |  | -0.08 |  | +0.0% | +0.0% |  |  |
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  | 172.00 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 5.20 | 3.98 | 0.18 | 0.12 |  | 0.99 | 0.23 |  | 0.15 | 0.09 | 0.41 | 0.09 | 153.30 | 1,285 | 740 |  | OK |
| smoke_3 | classic | 3.73 | 3.38 | 0.13 | 0.12 |  | 0.97 | 0.23 |  | 0.15 | 0.09 | 0.42 | 0.08 | 140.20 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 5.61 | 4.09 | 0.11 | 0.12 |  | 1.23 | 0.33 |  | 0.15 | 0.15 | 0.40 | 0.20 | 133.70 | 1,285 | 740 |  | OK |
