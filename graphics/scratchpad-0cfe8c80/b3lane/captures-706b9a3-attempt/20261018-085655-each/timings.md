# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| night_road_flat_drive_moonless | off | 7.69 | 6.74 |  | 0.96 | 3.61 | 2.17 |  |  | 0.78 | 0.34 |  | 0.13 | 0.06 |  | 0.03 | 7.16 | 81.90 | 1,410 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | on | 7.84 | 6.91 |  | 1.03 | 3.69 | 2.19 |  |  | 0.77 | 0.32 |  | 0.13 | 0.06 |  | 0.03 | 7.41 | 91.40 | 1,411 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | delta | +0.15 | +0.17 |  | +0.07 | +0.08 | +0.02 |  |  | -0.01 | -0.02 |  | +0.00 | +0.00 |  | +0.00 | +0.25 |  | +0.1% | +0.0% |  |  |
| b3_road_flat_head | off | 8.04 | 7.10 |  | 1.02 | 4.09 | 1.98 |  |  | 0.80 | 0.33 |  | 0.14 | 0.06 |  | 0.03 | 7.52 | 89.20 | 1,368 | 916 | OK:30 | OK |
| b3_road_flat_head | on | 7.92 | 7.01 | 0.14 | 1.02 | 3.89 | 2.09 |  |  | 0.76 | 0.29 |  | 0.14 | 0.06 |  | 0.03 | 7.53 | 91.50 | 1,369 | 916 | OK:30 | OK |
| b3_road_flat_head | delta | -0.12 | -0.09 |  | +0.00 | -0.20 | +0.11 |  |  | -0.04 | -0.04 |  | +0.00 | +0.00 |  | +0.00 | +0.01 |  | +0.1% | +0.0% |  |  |
| b3_town_night | off | 7.45 | 6.38 |  | 0.97 | 2.32 | 3.08 |  |  | 0.85 | 0.35 |  | 0.16 | 0.07 |  | 0.03 | 7.00 | 98.60 | 1,125 | 1,020 | UNSETTLED:300 | OK |
| b3_town_night | on | 8.90 | 7.80 | 0.17 | 1.06 | 3.08 | 3.66 |  |  | 0.89 | 0.37 |  | 0.15 | 0.07 |  | 0.03 | 8.57 | 85.00 | 1,163 | 1,067 | OK:110 | OK |
| b3_town_night | delta | +1.45 | +1.42 |  | +0.09 | +0.76 | +0.58 |  |  | +0.04 | +0.02 |  | -0.01 | +0.00 |  | +0.00 | +1.57 |  | +3.3% | +4.4% |  |  |
| b3_town_lamps | off | 6.17 | 5.11 |  | 1.04 | 2.59 | 1.48 |  |  | 0.87 | 0.27 |  | 0.18 | 0.07 |  | 0.03 | 5.71 | 114.80 | 732 | 526 | OK:218 | OK |
| b3_town_lamps | on | 6.59 | 5.40 |  | 1.08 | 2.78 | 1.54 |  |  | 0.84 | 0.33 |  | 0.16 | 0.07 |  | 0.03 | 5.92 | 110.70 | 733 | 526 | OK:30 | OK |
| b3_town_lamps | delta | +0.42 | +0.29 |  | +0.04 | +0.19 | +0.06 |  |  | -0.03 | +0.06 |  | -0.02 | +0.00 |  | +0.00 | +0.21 |  | +0.1% | +0.0% |  |  |
| b3_moon_sky | off | 5.13 | 3.59 |  | 0.89 | 2.39 | 0.30 |  |  | 1.13 | 0.27 |  | 0.20 | 0.14 |  | 0.05 | 4.45 | 167.00 | 322 | 188 | OK:30 | OK |
| b3_moon_sky | on | 5.67 | 4.16 | 0.29 | 0.87 | 2.71 | 0.58 |  |  | 1.07 | 0.26 |  | 0.20 | 0.12 |  | 0.06 | 5.28 | 148.80 | 323 | 188 | OK:30 | OK |
| b3_moon_sky | delta | +0.54 | +0.57 |  | -0.02 | +0.32 | +0.28 |  |  | -0.06 | -0.01 |  | +0.00 | -0.02 |  | +0.01 | +0.83 |  | +0.3% | +0.0% |  |  |
| b3_jeep_side | off | 8.06 | 7.08 |  | 0.92 | 4.01 | 2.15 |  |  | 0.82 | 0.34 |  | 0.14 | 0.06 |  | 0.03 | 7.57 | 89.70 | 1,444 | 917 | OK:30 | OK |
| b3_jeep_side | on | 8.04 | 7.13 |  | 0.90 | 3.91 | 2.32 |  |  | 0.75 | 0.29 |  | 0.14 | 0.06 |  | 0.03 | 7.68 | 90.20 | 1,445 | 917 | OK:30 | OK |
| b3_jeep_side | delta | -0.02 | +0.05 |  | -0.02 | -0.10 | +0.17 |  |  | -0.07 | -0.05 |  | +0.00 | +0.00 |  | +0.00 | +0.11 |  | +0.1% | +0.0% |  |  |
| b3_day_town | off | 6.23 | 4.98 | 0.28 | 1.06 | 2.62 | 1.29 | 0.13 |  | 0.87 | 0.28 |  | 0.18 | 0.08 |  | 0.04 | 5.72 | 116.10 | 710 | 540 | OK:30 | OK |
| b3_day_town | on | 6.27 | 4.98 | 0.25 | 1.08 | 2.64 | 1.26 | 0.13 |  | 0.91 | 0.32 |  | 0.18 | 0.08 |  | 0.04 | 5.73 | 112.60 | 710 | 540 | OK:30 | OK |
| b3_day_town | delta | +0.04 | +0.00 | -0.03 | +0.02 | +0.02 | -0.03 | +0.00 |  | +0.04 | +0.04 |  | +0.00 | +0.00 |  | +0.00 | +0.01 |  | +0.0% | +0.0% |  |  |
| b3_nv_town | off | 5.46 |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 3.29 | 127.00 | 707 | 546 | OK:30 | OK |
| b3_nv_town | on | 2.74 |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 2.44 | 135.40 | 707 | 546 | OK:30 | OK |
| b3_nv_town | delta | -2.72 |  |  |  |  |  |  |  |  |  |  |  |  |  |  | -0.85 |  | +0.0% | +0.0% |  |  |
