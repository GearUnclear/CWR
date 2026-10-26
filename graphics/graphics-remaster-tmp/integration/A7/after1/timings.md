# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| atmosphere_coastline_sunset | off | 2.20 | 0.91 | 0.00 |  |  | 1.16 | 0.25 |  | 0.11 | 0.07 | 0.45 | 0.29 | 364.90 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | on | 2.63 | 0.86 | 0.00 | 0.11 |  | 1.55 | 0.37 |  | 0.15 | 0.09 | 0.62 | 0.32 | 326.60 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | delta | +0.43 | -0.05 | +0.00 |  |  | +0.39 | +0.12 |  | +0.04 | +0.02 | +0.17 | +0.03 |  | +0.0% |  |  |  |
| atmosphere_firefight_dusk | off | 1.32 | 0.72 |  |  |  | 0.41 | 0.19 |  | 0.12 | 0.06 |  | 0.04 | 430.90 | 127 | 70 | OK:30 | OK |
| atmosphere_firefight_dusk | on | 1.21 | 0.52 |  | 0.09 |  | 0.45 | 0.21 |  | 0.13 | 0.07 |  | 0.05 | 404.50 | 127 | 70 | OK:30 | OK |
| atmosphere_firefight_dusk | delta | -0.11 | -0.20 |  |  |  | +0.04 | +0.02 |  | +0.01 | +0.01 |  | +0.01 |  | +0.0% | +0.0% |  |  |
| atmosphere_noon_fields | off | 10.64 | 9.87 | 0.06 |  |  | 0.63 | 0.42 |  | 0.11 | 0.06 |  | 0.04 | 61.80 | 6,347 | 4,506 | OK:30 | OK |
| atmosphere_noon_fields | on | 10.34 | 9.68 | 0.06 | 0.08 |  | 0.49 | 0.27 |  | 0.11 | 0.06 |  | 0.04 | 63.00 | 6,347 | 4,506 | OK:30 | OK |
| atmosphere_noon_fields | delta | -0.30 | -0.19 | +0.00 |  |  | -0.14 | -0.15 |  | +0.00 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| atmosphere_sunrise_ridge | off | 28.51 | 27.27 | 0.08 |  |  | 1.08 | 0.50 |  | 0.13 | 0.06 | 0.27 | 0.12 | 32.30 | 12,910 | 8,897 | OK:30 | OK |
| atmosphere_sunrise_ridge | on | 27.20 | 25.94 | 0.08 | 0.09 |  | 1.07 | 0.45 |  | 0.13 | 0.06 | 0.27 | 0.16 | 32.20 | 12,910 | 8,897 | OK:30 | OK |
| atmosphere_sunrise_ridge | delta | -1.31 | -1.33 | +0.00 |  |  | -0.01 | -0.05 |  | +0.00 | +0.00 | +0.00 | +0.04 |  | +0.0% | +0.0% |  |  |
| atmosphere_coast_oc_up | off | 1.52 | 0.47 | 0.00 |  |  | 0.86 | 0.32 |  | 0.06 | 0.06 | 0.34 | 0.08 | 358.50 | 186 | 0 | OK:30 | OK |
| atmosphere_coast_oc_up | on | 1.65 | 0.59 | 0.00 | 0.13 |  | 0.82 | 0.29 |  | 0.06 | 0.06 | 0.34 | 0.07 | 355.80 | 186 | 0 | OK:30 | OK |
| atmosphere_coast_oc_up | delta | +0.13 | +0.12 | +0.00 |  |  | -0.04 | -0.03 |  | +0.00 | +0.00 | +0.00 | -0.01 |  | +0.0% |  |  |  |
| atmosphere_town_night | off | 2.23 | 1.53 |  |  |  | 0.50 | 0.24 |  | 0.15 | 0.06 |  | 0.04 | 191.90 | 777 | 546 | OK:30 | OK |
| atmosphere_town_night | on | 2.26 | 1.55 |  |  |  | 0.53 | 0.27 |  | 0.15 | 0.06 |  | 0.04 | 221.00 | 777 | 546 | OK:30 | OK |
| atmosphere_town_night | delta | +0.03 | +0.02 |  |  |  | +0.03 | +0.03 |  | +0.00 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  | 229.20 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 3.27 | 2.50 | 0.05 | 0.05 |  | 0.69 | 0.23 |  | 0.09 | 0.05 | 0.22 | 0.09 | 196.00 | 1,285 | 740 |  | OK |
| smoke_3 | classic | 2.08 | 1.94 | 0.05 | 0.05 |  | 0.66 | 0.25 |  | 0.09 | 0.05 | 0.22 | 0.06 | 172.70 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 3.16 | 2.40 | 0.05 | 0.05 |  | 0.66 | 0.26 |  | 0.09 | 0.05 | 0.22 | 0.04 | 164.20 | 1,285 | 740 |  | OK |
