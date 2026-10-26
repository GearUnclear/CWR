# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| night_road_flat_drive_moonless | off | 13.84 | 12.63 |  | 1.32 | 7.18 | 4.13 |  |  | 1.05 | 0.23 |  | 0.13 | 0.06 |  | 0.03 | 7.30 | 63.00 | 1,410 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | on | 15.03 | 13.72 |  | 1.46 | 7.68 | 4.58 |  |  | 1.14 | 0.21 |  | 0.13 | 0.06 |  | 0.03 | 9.19 | 55.20 | 1,411 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | delta | +1.19 | +1.09 |  | +0.14 | +0.50 | +0.45 |  |  | +0.09 | -0.02 |  | +0.00 | +0.00 |  | +0.00 | +1.89 |  | +0.1% | +0.0% |  |  |
| b3_road_flat_head | off | 17.35 | 15.85 |  | 1.59 | 8.98 | 5.28 |  |  | 1.35 | 0.24 |  | 0.14 | 0.07 |  | 0.03 | 7.63 | 55.90 | 1,368 | 916 | OK:30 | OK |
| b3_road_flat_head | on | 18.80 | 17.30 | 0.66 | 1.68 | 10.06 | 5.56 |  |  | 1.35 | 0.26 |  | 0.14 | 0.07 |  | 0.03 | 10.24 | 48.60 | 1,369 | 916 | OK:30 | OK |
| b3_road_flat_head | delta | +1.45 | +1.45 |  | +0.09 | +1.08 | +0.28 |  |  | +0.00 | +0.02 |  | +0.00 | +0.00 |  | +0.00 | +2.61 |  | +0.1% | +0.0% |  |  |
| b3_town_night | off | 20.04 | 18.40 |  | 2.18 | 5.72 | 10.50 |  |  | 1.46 | 0.25 |  | 0.16 | 0.07 |  | 0.03 | 15.09 | 47.00 | 1,230 | 1,136 | OK:200 | OK |
| b3_town_night | on | 24.82 | 22.98 | 0.79 | 2.65 | 8.68 | 11.65 |  |  | 1.65 | 0.27 |  | 0.16 | 0.07 |  | 0.03 | 18.03 | 38.50 | 1,219 | 1,113 | OK:202 | OK |
| b3_town_night | delta | +4.78 | +4.58 |  | +0.47 | +2.96 | +1.15 |  |  | +0.19 | +0.02 |  | +0.00 | +0.00 |  | +0.00 | +2.94 |  | -0.9% | -2.0% |  |  |
| b3_town_lamps | off | 11.95 | 10.64 |  | 2.22 | 5.47 | 2.95 |  |  | 1.12 | 0.18 |  | 0.16 | 0.07 |  | 0.03 | 5.35 | 93.30 | 732 | 526 | OK:206 | OK |
| b3_town_lamps | on | 12.23 | 10.83 |  | 2.05 | 5.39 | 3.39 |  |  | 1.21 | 0.27 |  | 0.16 | 0.07 |  | 0.03 | 5.95 | 75.60 | 733 | 526 | OK:30 | OK |
| b3_town_lamps | delta | +0.28 | +0.19 |  | -0.17 | -0.08 | +0.44 |  |  | +0.09 | +0.09 |  | +0.00 | +0.00 |  | +0.00 | +0.60 |  | +0.1% | +0.0% |  |  |
| b3_jeep_side | off | 8.60 | 7.61 |  | 1.02 | 4.13 | 2.46 |  |  | 0.83 | 0.29 |  | 0.14 | 0.06 |  | 0.03 | 8.09 | 76.40 | 1,444 | 917 | OK:30 | OK |
| b3_jeep_side | on | 9.06 | 7.89 |  | 1.06 | 4.34 | 2.49 |  |  | 1.01 | 0.35 |  | 0.14 | 0.06 |  | 0.03 | 8.39 | 77.40 | 1,445 | 917 | OK:30 | OK |
| b3_jeep_side | delta | +0.46 | +0.28 |  | +0.04 | +0.21 | +0.03 |  |  | +0.18 | +0.06 |  | +0.00 | +0.00 |  | +0.00 | +0.30 |  | +0.1% | +0.0% |  |  |
