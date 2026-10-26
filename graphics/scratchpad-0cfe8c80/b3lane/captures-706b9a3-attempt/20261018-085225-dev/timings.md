# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| night_road_flat_drive_moonless | off | 7.80 | 6.87 |  | 1.00 | 3.74 | 2.13 |  |  | 0.78 | 0.33 |  | 0.13 | 0.06 |  | 0.03 | 7.19 | 92.10 | 1,410 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | on | 8.10 | 7.17 |  | 1.06 | 3.84 | 2.27 |  |  | 0.77 | 0.33 |  | 0.13 | 0.06 |  | 0.03 | 7.65 | 89.60 | 1,411 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | delta | +0.30 | +0.30 |  | +0.06 | +0.10 | +0.14 |  |  | -0.01 | +0.00 |  | +0.00 | +0.00 |  | +0.00 | +0.46 |  | +0.1% | +0.0% |  |  |
| b3_road_flat_head | off | 8.12 | 7.05 |  | 1.03 | 3.93 | 2.09 |  |  | 0.93 | 0.46 |  | 0.14 | 0.06 |  | 0.03 | 7.73 | 78.60 | 1,368 | 916 | OK:30 | OK |
| b3_road_flat_head | on | 8.95 | 7.79 | 0.14 | 1.14 | 4.27 | 2.37 |  |  | 1.01 | 0.54 |  | 0.14 | 0.07 |  | 0.03 | 8.43 | 69.90 | 1,369 | 916 | OK:30 | OK |
| b3_road_flat_head | delta | +0.83 | +0.74 |  | +0.11 | +0.34 | +0.28 |  |  | +0.08 | +0.08 |  | +0.00 | +0.01 |  | +0.00 | +0.70 |  | +0.1% | +0.0% |  |  |
| b3_town_night | off | 9.08 | 7.47 |  | 0.92 | 2.64 | 3.91 |  |  | 1.16 | 0.61 |  | 0.16 | 0.07 |  | 0.03 | 8.39 | 76.10 | 1,205 | 1,122 | OK:227 | OK |
| b3_town_night | on | 9.94 | 8.55 | 0.17 | 1.15 | 3.28 | 4.12 |  |  | 1.19 | 0.54 |  | 0.16 | 0.07 |  | 0.03 | 9.41 | 67.90 | 1,206 | 1,119 | OK:142 | OK |
| b3_town_night | delta | +0.86 | +1.08 |  | +0.23 | +0.64 | +0.21 |  |  | +0.03 | -0.07 |  | +0.00 | +0.00 |  | +0.00 | +1.02 |  | +0.1% | -0.3% |  |  |
| b3_town_lamps | off | 7.03 | 5.49 |  | 1.03 | 2.52 | 1.93 |  |  | 1.18 | 0.62 |  | 0.17 | 0.07 |  | 0.03 | 5.82 | 94.30 | 732 | 526 | OK:212 | OK |
| b3_town_lamps | on | 6.65 | 5.51 |  | 1.12 | 2.81 | 1.58 |  |  | 0.95 | 0.43 |  | 0.16 | 0.07 |  | 0.03 | 6.05 | 94.60 | 733 | 526 | OK:30 | OK |
| b3_town_lamps | delta | -0.38 | +0.02 |  | +0.09 | +0.29 | -0.35 |  |  | -0.23 | -0.19 |  | -0.01 | +0.00 |  | +0.00 | +0.23 |  | +0.1% | +0.0% |  |  |
| b3_moon_sky | off | 5.07 | 3.64 |  | 0.87 | 2.49 | 0.28 |  |  | 1.06 | 0.31 |  | 0.20 | 0.10 |  | 0.05 | 4.57 | 161.80 | 322 | 188 | OK:30 | OK |
| b3_moon_sky | on | 6.19 | 4.51 | 0.25 | 0.84 | 2.94 | 0.74 |  |  | 1.19 | 0.40 |  | 0.20 | 0.10 |  | 0.05 | 5.42 | 115.40 | 323 | 188 | OK:30 | OK |
| b3_moon_sky | delta | +1.12 | +0.87 |  | -0.03 | +0.45 | +0.46 |  |  | +0.13 | +0.09 |  | +0.00 | +0.00 |  | +0.00 | +0.85 |  | +0.3% | +0.0% |  |  |
| b3_jeep_side | off | 8.16 | 7.19 |  | 0.97 | 4.03 | 2.19 |  |  | 0.81 | 0.34 |  | 0.14 | 0.06 |  | 0.03 | 7.69 | 87.30 | 1,444 | 917 | OK:30 | OK |
| b3_jeep_side | on | 8.45 | 7.47 |  | 1.01 | 4.00 | 2.47 |  |  | 0.83 | 0.34 |  | 0.14 | 0.06 |  | 0.03 | 8.26 | 86.30 | 1,445 | 917 | OK:30 | OK |
| b3_jeep_side | delta | +0.29 | +0.28 |  | +0.04 | -0.03 | +0.28 |  |  | +0.02 | +0.00 |  | +0.00 | +0.00 |  | +0.00 | +0.57 |  | +0.1% | +0.0% |  |  |
