# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| atmosphere_coastline_sunset | off | 4.80 | 1.78 | 0.00 |  |  | 2.75 | 0.47 |  | 0.30 | 0.24 | 1.37 | 0.37 | 204.10 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | on | 5.28 | 2.10 | 0.00 | 0.27 |  | 2.69 | 0.53 |  | 0.27 | 0.17 | 1.31 | 0.40 | 177.00 | 175 | 0 | OK:30 | OK |
| atmosphere_coastline_sunset | delta | +0.48 | +0.32 | +0.00 |  |  | -0.06 | +0.06 |  | -0.03 | -0.07 | -0.06 | +0.03 |  | +0.0% |  |  |  |
| atmosphere_firefight_dusk | off | 2.66 | 1.14 |  |  |  | 1.17 | 0.46 |  | 0.30 | 0.28 |  | 0.13 | 277.90 | 127 | 70 | OK:30 | OK |
| atmosphere_firefight_dusk | on | 4.05 | 1.72 |  | 0.43 |  | 1.44 | 0.42 |  | 0.50 | 0.34 |  | 0.18 | 198.30 | 127 | 70 | OK:30 | OK |
| atmosphere_firefight_dusk | delta | +1.39 | +0.58 |  |  |  | +0.27 | -0.04 |  | +0.20 | +0.06 |  | +0.05 |  | +0.0% | +0.0% |  |  |
| atmosphere_noon_fields | off | 11.26 | 10.52 | 0.07 |  |  | 0.61 | 0.39 |  | 0.11 | 0.06 |  | 0.04 | 52.60 | 6,347 | 4,506 | OK:30 | OK |
| atmosphere_noon_fields | on | 11.26 | 10.54 | 0.06 | 0.08 |  | 0.54 | 0.32 |  | 0.11 | 0.06 |  | 0.04 | 53.00 | 6,347 | 4,506 | OK:30 | OK |
| atmosphere_noon_fields | delta | +0.00 | +0.02 | -0.01 |  |  | -0.07 | -0.07 |  | +0.00 | +0.00 |  | +0.00 |  | +0.0% | +0.0% |  |  |
| atmosphere_sunrise_ridge | off | 34.19 | 32.71 | 0.08 |  |  | 1.31 | 0.74 |  | 0.13 | 0.06 | 0.27 | 0.11 | 25.50 | 12,910 | 8,897 | OK:30 | OK |
| atmosphere_sunrise_ridge | on | 35.37 | 33.79 | 0.08 | 0.10 |  | 1.38 | 0.83 |  | 0.13 | 0.06 | 0.27 | 0.09 | 25.90 | 12,910 | 8,897 | OK:30 | OK |
| atmosphere_sunrise_ridge | delta | +1.18 | +1.08 | +0.00 |  |  | +0.07 | +0.09 |  | +0.00 | +0.00 | +0.00 | -0.02 |  | +0.0% | +0.0% |  |  |
| atmosphere_coast_oc_up | off | 3.98 | 1.86 | 0.00 |  |  | 1.77 | 0.50 |  | 0.14 | 0.14 | 0.81 | 0.17 | 200.80 | 186 | 0 | OK:30 | OK |
| atmosphere_coast_oc_up | on | 9.21 | 4.65 | 0.00 | 0.51 |  | 3.60 | 1.82 |  | 0.19 | 0.19 | 1.12 | 0.28 | 113.60 | 186 | 0 | OK:30 | OK |
| atmosphere_coast_oc_up | delta | +5.23 | +2.79 | +0.00 |  |  | +1.83 | +1.32 |  | +0.05 | +0.05 | +0.31 | +0.11 |  | +0.0% |  |  |  |
| atmosphere_town_night | off | 4.24 | 3.13 |  |  |  | 0.79 | 0.30 |  | 0.27 | 0.13 |  | 0.09 | 158.70 | 777 | 546 | OK:30 | OK |
| atmosphere_town_night | on | 4.99 | 3.75 |  |  |  | 0.89 | 0.27 |  | 0.33 | 0.18 |  | 0.11 | 144.50 | 777 | 546 | OK:30 | OK |
| atmosphere_town_night | delta | +0.75 | +0.62 |  |  |  | +0.10 | -0.03 |  | +0.06 | +0.05 |  | +0.02 |  | +0.0% | +0.0% |  |  |
