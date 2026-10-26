# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| atmosphere_coastline_sunset | off | 2.12 | 0.73 | 0.00 |  |  | 1.24 | 0.28 |  | 0.14 | 0.08 | 0.58 | 0.15 | 366.80 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | on | 2.24 | 0.80 | 0.00 | 0.10 |  | 1.21 | 0.29 |  | 0.14 | 0.08 | 0.57 | 0.13 | 319.10 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | delta | +0.12 | +0.07 | +0.00 |  |  | -0.03 | +0.01 |  | +0.00 | +0.00 | -0.01 | -0.02 |  | +0.0% |  |  |  |
| atmosphere_firefight_dusk | off | 2.61 | 1.19 |  |  |  | 1.01 | 0.26 |  | 0.36 | 0.23 |  | 0.16 | 302.10 | 127 | 70 | OK:30 | OK |
| atmosphere_firefight_dusk | on | 1.70 | 0.75 |  | 0.15 |  | 0.59 | 0.26 |  | 0.17 | 0.09 |  | 0.07 | 274.00 | 127 | 70 | OK:30 | OK |
| atmosphere_firefight_dusk | delta | -0.91 | -0.44 |  |  |  | -0.42 | +0.00 |  | -0.19 | -0.14 |  | -0.09 |  | +0.0% | +0.0% |  |  |
| atmosphere_noon_fields | off | 9.40 | 8.83 | 0.07 |  |  | 0.43 | 0.21 |  | 0.11 | 0.06 |  | 0.04 | 68.10 | 6,347 | 4,506 | OK:30 | OK |
| atmosphere_noon_fields | on | 10.02 | 9.35 | 0.06 | 0.08 |  | 0.48 | 0.26 |  | 0.11 | 0.06 |  | 0.04 | 65.00 | 6,347 | 4,506 | OK:30 | OK |
| atmosphere_noon_fields | delta | +0.62 | +0.52 | -0.01 |  |  | +0.05 | +0.05 |  | +0.00 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| atmosphere_sunrise_ridge | off | 24.60 | 23.43 | 0.08 |  |  | 1.01 | 0.48 |  | 0.13 | 0.06 | 0.27 | 0.08 | 37.10 | 12,910 | 8,897 | OK:30 | OK |
| atmosphere_sunrise_ridge | on | 24.81 | 23.63 | 0.08 | 0.10 |  | 0.95 | 0.44 |  | 0.13 | 0.06 | 0.27 | 0.05 | 37.30 | 12,910 | 8,897 | OK:30 | OK |
| atmosphere_sunrise_ridge | delta | +0.21 | +0.20 | +0.00 |  |  | -0.06 | -0.04 |  | +0.00 | +0.00 | +0.00 | -0.03 |  | +0.0% | +0.0% |  |  |
| atmosphere_coast_oc_up | off | 3.31 | 1.28 | 0.00 |  |  | 1.68 | 0.34 |  | 0.15 | 0.15 | 0.87 | 0.17 | 274.80 | 186 | 0 | OK:30 | OK |
| atmosphere_coast_oc_up | on | 4.77 | 1.70 | 0.00 | 0.50 |  | 2.09 | 0.31 |  | 0.18 | 0.18 | 1.08 | 0.34 | 203.90 | 186 | 0 | OK:30 | OK |
| atmosphere_coast_oc_up | delta | +1.46 | +0.42 | +0.00 |  |  | +0.41 | -0.03 |  | +0.03 | +0.03 | +0.21 | +0.17 |  | +0.0% |  |  |  |
| atmosphere_town_night | off | 5.31 | 4.05 |  |  |  | 0.90 | 0.28 |  | 0.34 | 0.17 |  | 0.11 | 139.70 | 777 | 546 | OK:30 | OK |
| atmosphere_town_night | on | 5.15 | 3.89 |  |  |  | 0.91 | 0.30 |  | 0.34 | 0.16 |  | 0.11 | 136.90 | 777 | 546 | OK:30 | OK |
| atmosphere_town_night | delta | -0.16 | -0.16 |  |  |  | +0.01 | +0.02 |  | +0.00 | -0.01 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  | 188.00 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 3.17 | 2.38 | 0.05 | 0.06 |  | 0.69 | 0.26 |  | 0.09 | 0.05 | 0.23 | 0.05 | 168.00 | 1,285 | 740 |  | OK |
| smoke_3 | classic | 1.75 | 1.61 | 0.05 | 0.06 |  | 0.66 | 0.23 |  | 0.09 | 0.05 | 0.23 | 0.05 | 157.70 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 3.19 | 2.42 | 0.05 | 0.06 |  | 0.64 | 0.23 |  | 0.09 | 0.05 | 0.23 | 0.04 | 159.90 | 1,285 | 740 |  | OK |
