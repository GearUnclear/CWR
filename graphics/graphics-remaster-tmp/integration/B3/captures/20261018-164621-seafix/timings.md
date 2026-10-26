# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| night_road_flat_drive_moonless | off | 22.64 | 20.88 |  | 1.84 | 11.75 | 7.28 | 0.07 |  |  | 1.58 | 0.17 |  | 0.13 | 0.06 |  | 0.03 | 11.89 | 41.60 | 1,410 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | on | 19.05 | 17.49 |  | 1.66 | 9.50 | 6.33 | 0.07 |  |  | 1.30 | 0.24 |  | 0.13 | 0.06 |  | 0.03 | 11.98 | 39.50 | 1,411 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | delta | -3.59 | -3.39 |  | -0.18 | -2.25 | -0.95 | +0.00 |  |  | -0.28 | +0.07 |  | +0.00 | +0.00 |  | +0.00 | +0.09 |  | +0.1% | +0.0% |  |  |
| b3_sea_beam | off | 6.13 | 4.87 |  | 3.24 | 0.28 | 1.35 | 0.06 |  |  | 1.09 | 0.20 |  | 0.13 | 0.06 |  | 0.03 | 2.62 | 175.10 | 163 | 3 | OK:30 | OK |
| b3_sea_beam | on | 6.35 | 4.86 |  | 2.99 | 0.32 | 1.54 | 0.06 |  |  | 1.31 | 0.21 |  | 0.13 | 0.06 |  | 0.03 | 2.86 | 146.90 | 164 | 3 | OK:30 | OK |
| b3_sea_beam | delta | +0.22 | -0.01 |  | -0.25 | +0.04 | +0.19 | +0.00 |  |  | +0.22 | +0.01 |  | +0.00 | +0.00 |  | +0.00 | +0.24 |  | +0.6% | +0.0% |  |  |
| b3_sea_beam_front | off | 3.52 | 2.27 |  | 0.91 | 0.23 | 1.13 | 0.07 |  |  | 1.09 | 0.27 |  | 0.11 | 0.06 |  | 0.03 | 1.90 | 267.40 | 98 | 3 | OK:30 | OK |
| b3_sea_beam_front | on | 3.14 | 2.07 |  | 0.88 | 0.24 | 0.95 | 0.07 |  |  | 0.92 | 0.22 |  | 0.11 | 0.06 |  | 0.03 | 1.95 | 252.60 | 99 | 3 | OK:30 | OK |
| b3_sea_beam_front | delta | -0.38 | -0.20 |  | -0.03 | +0.01 | -0.18 | +0.00 |  |  | -0.17 | -0.05 |  | +0.00 | +0.00 |  | +0.00 | +0.05 |  | +1.0% | +0.0% |  |  |
