# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| atmosphere_coastline_sunset | off | 1.70 | 0.53 | 0.00 | 1.03 | 0.29 |  | 0.12 | 0.07 | 0.48 | 0.08 | 299.70 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | on | 1.92 | 0.65 | 0.00 | 1.12 | 0.27 |  | 0.13 | 0.08 | 0.54 | 0.10 | 356.60 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | delta | +0.22 | +0.12 | +0.00 | +0.09 | -0.02 |  | +0.01 | +0.01 | +0.06 | +0.02 |  | +0.0% |  |  |  |
| atmosphere_firefight_dusk | off | 1.18 | 0.54 |  | 0.44 | 0.20 |  | 0.13 | 0.07 |  | 0.04 | 416.10 | 127 | 70 | OK:30 | OK |
| atmosphere_firefight_dusk | on | 1.91 | 1.36 |  | 0.37 | 0.15 |  | 0.12 | 0.06 |  | 0.04 | 364.70 | 127 | 70 | OK:30 | OK |
| atmosphere_firefight_dusk | delta | +0.73 | +0.82 |  | -0.07 | -0.05 |  | -0.01 | -0.01 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| atmosphere_noon_fields | off | 16.79 | 16.20 | 0.06 | 0.45 | 0.23 |  | 0.11 | 0.06 |  | 0.04 | 46.30 | 6,347 | 4,506 | OK:30 | OK |
| atmosphere_noon_fields | on | 15.68 | 15.06 | 0.07 | 0.48 | 0.27 |  | 0.11 | 0.06 |  | 0.04 | 47.70 | 6,347 | 4,506 | OK:30 | OK |
| atmosphere_noon_fields | delta | -1.11 | -1.14 | +0.01 | +0.03 | +0.04 |  | +0.00 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| atmosphere_sunrise_ridge | off | 24.59 | 23.25 | 0.08 | 1.18 | 0.50 |  | 0.13 | 0.06 | 0.27 | 0.22 | 26.80 | 12,910 | 8,897 | OK:30 | OK |
| atmosphere_sunrise_ridge | on | 25.97 | 24.56 | 0.10 | 1.25 | 0.51 |  | 0.13 | 0.06 | 0.27 | 0.28 | 26.50 | 12,910 | 8,897 | OK:30 | OK |
| atmosphere_sunrise_ridge | delta | +1.38 | +1.31 | +0.02 | +0.07 | +0.01 |  | +0.00 | +0.00 | +0.00 | +0.06 |  | +0.0% | +0.0% |  |  |
| atmosphere_coast_oc_up | off | 2.44 | 1.09 | 0.00 | 1.16 | 0.31 |  | 0.06 | 0.06 | 0.34 | 0.39 | 298.50 | 186 | 0 | OK:30 | OK |
| atmosphere_coast_oc_up | on | 3.77 | 1.46 | 0.00 | 2.12 | 1.26 |  | 0.06 | 0.06 | 0.34 | 0.40 | 172.00 | 186 | 0 | OK:30 | OK |
| atmosphere_coast_oc_up | delta | +1.33 | +0.37 | +0.00 | +0.96 | +0.95 |  | +0.00 | +0.00 | +0.00 | +0.01 |  | +0.0% |  |  |  |
| atmosphere_town_night | off | 3.73 | 3.07 |  | 0.46 | 0.20 |  | 0.16 | 0.06 |  | 0.04 | 162.80 | 777 | 546 | OK:30 | OK |
| atmosphere_town_night | on | 3.92 | 3.31 |  | 0.42 | 0.16 |  | 0.15 | 0.06 |  | 0.04 | 175.40 | 777 | 546 | OK:30 | OK |
| atmosphere_town_night | delta | +0.19 | +0.24 |  | -0.04 | -0.04 |  | -0.01 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
