# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| night_road_flat_drive_moonless | off | 7.16 | 6.21 |  | 0.88 | 3.32 | 2.01 |  |  | 0.79 | 0.35 |  | 0.13 | 0.06 |  | 0.03 | 6.58 | 98.10 | 1,410 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | on | 7.04 | 6.11 |  | 0.93 | 3.25 | 1.92 |  |  | 0.78 | 0.33 |  | 0.13 | 0.06 |  | 0.03 | 6.62 | 98.10 | 1,410 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | delta | -0.12 | -0.10 |  | +0.05 | -0.07 | -0.09 |  |  | -0.01 | -0.02 |  | +0.00 | +0.00 |  | +0.00 | +0.04 |  | +0.0% | +0.0% |  |  |
| b3_road_flat_head | off | 7.39 | 6.43 |  | 0.97 | 3.57 | 1.89 |  |  | 0.81 | 0.33 |  | 0.14 | 0.06 |  | 0.03 | 6.85 | 96.90 | 1,368 | 916 | OK:30 | OK |
| b3_road_flat_head | on | 7.62 | 6.64 |  | 0.98 | 3.70 | 1.96 |  |  | 0.84 | 0.36 |  | 0.14 | 0.06 |  | 0.03 | 7.08 | 93.30 | 1,368 | 916 | OK:30 | OK |
| b3_road_flat_head | delta | +0.23 | +0.21 |  | +0.01 | +0.13 | +0.07 |  |  | +0.03 | +0.03 |  | +0.00 | +0.00 |  | +0.00 | +0.23 |  | +0.0% | +0.0% |  |  |
| b3_town_night | off | 8.65 | 7.47 |  | 0.88 | 2.37 | 4.21 |  |  | 1.00 | 0.48 |  | 0.16 | 0.07 |  | 0.03 | 7.76 | 79.90 | 1,215 | 1,122 | OK:207 | OK |
| b3_town_night | on | 8.24 | 7.03 |  | 0.84 | 2.45 | 3.73 |  |  | 1.03 | 0.51 |  | 0.16 | 0.07 |  | 0.03 | 7.72 | 85.50 | 1,233 | 1,135 | OK:159 | OK |
| b3_town_night | delta | -0.41 | -0.44 |  | -0.04 | +0.08 | -0.48 |  |  | +0.03 | +0.03 |  | +0.00 | +0.00 |  | +0.00 | -0.04 |  | +1.5% | +1.1% |  |  |
| b3_town_lamps | off | 6.12 | 4.98 |  | 1.00 | 2.58 | 1.41 |  |  | 0.94 | 0.36 |  | 0.19 | 0.07 |  | 0.04 | 5.52 | 116.50 | 732 | 526 | OK:199 | OK |
| b3_town_lamps | on | 6.23 | 5.16 |  | 1.04 | 2.54 | 1.58 |  |  | 0.87 | 0.29 |  | 0.19 | 0.08 |  | 0.04 | 5.68 | 114.60 | 732 | 526 | OK:30 | OK |
| b3_town_lamps | delta | +0.11 | +0.18 |  | +0.04 | -0.04 | +0.17 |  |  | -0.07 | -0.07 |  | +0.00 | +0.01 |  | +0.00 | +0.16 |  | +0.0% | +0.0% |  |  |
| b3_moon_sky | off | 5.07 | 3.43 |  | 0.91 | 2.23 | 0.29 |  |  | 1.17 | 0.31 |  | 0.21 | 0.11 |  | 0.06 | 4.36 | 168.60 | 322 | 188 | OK:30 | OK |
| b3_moon_sky | on | 5.35 | 3.73 |  | 0.91 | 2.45 | 0.36 |  |  | 1.18 | 0.31 |  | 0.21 | 0.13 |  | 0.07 | 4.92 | 159.10 | 322 | 188 | OK:30 | OK |
| b3_moon_sky | delta | +0.28 | +0.30 |  | +0.00 | +0.22 | +0.07 |  |  | +0.01 | +0.00 |  | +0.00 | +0.02 |  | +0.01 | +0.56 |  | +0.0% | +0.0% |  |  |
| b3_jeep_side | off | 7.68 | 6.71 |  | 0.91 | 3.78 | 2.02 |  |  | 0.82 | 0.32 |  | 0.14 | 0.06 |  | 0.03 | 7.15 | 93.50 | 1,444 | 917 | OK:30 | OK |
| b3_jeep_side | on | 7.64 | 6.65 |  | 0.99 | 3.66 | 2.00 |  |  | 0.84 | 0.32 |  | 0.14 | 0.06 |  | 0.03 | 7.13 | 93.60 | 1,444 | 917 | OK:30 | OK |
| b3_jeep_side | delta | -0.04 | -0.06 |  | +0.08 | -0.12 | -0.02 |  |  | +0.02 | +0.00 |  | +0.00 | +0.00 |  | +0.00 | -0.02 |  | +0.0% | +0.0% |  |  |
| b3_day_town | off | 6.18 | 4.95 | 0.26 | 1.03 | 2.54 | 1.38 | 0.13 |  | 0.89 | 0.29 |  | 0.19 | 0.08 |  | 0.04 | 5.62 | 114.80 | 710 | 540 | OK:30 | OK |
| b3_day_town | on | 6.26 | 4.96 | 0.27 | 1.06 | 2.59 | 1.30 | 0.14 |  | 0.96 | 0.30 |  | 0.19 | 0.09 |  | 0.05 | 5.79 | 112.60 | 696 | 540 | OK:30 | OK |
| b3_day_town | delta | +0.08 | +0.01 | +0.01 | +0.03 | +0.05 | -0.08 | +0.01 |  | +0.07 | +0.01 |  | +0.00 | +0.01 |  | +0.01 | +0.17 |  | -2.0% | +0.0% |  |  |
| b3_nv_town | off | 5.52 |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 3.35 | 121.40 | 707 | 546 | OK:30 | OK |
| b3_nv_town | on | 2.76 |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 2.26 | 162.50 | 707 | 546 | OK:30 | OK |
| b3_nv_town | delta | -2.76 |  |  |  |  |  |  |  |  |  |  |  |  |  |  | -1.09 |  | +0.0% | +0.0% |  |  |
