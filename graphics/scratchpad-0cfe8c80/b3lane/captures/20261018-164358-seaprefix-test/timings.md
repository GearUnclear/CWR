# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| night_road_flat_drive_moonless | off | 11.08 | 9.54 |  | 1.00 | 4.21 | 4.34 | 0.07 |  |  | 1.37 | 0.85 |  | 0.13 | 0.06 |  | 0.03 | 8.00 | 48.90 | 1,410 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | on | 10.49 | 8.92 |  | 1.05 | 4.37 | 3.50 | 0.07 |  |  | 1.13 | 0.59 |  | 0.13 | 0.06 |  | 0.03 | 8.24 | 46.70 | 1,411 | 914 | OK:30 | OK |
| night_road_flat_drive_moonless | delta | -0.59 | -0.62 |  | +0.05 | +0.16 | -0.84 | +0.00 |  |  | -0.24 | -0.26 |  | +0.00 | +0.00 |  | +0.00 | +0.24 |  | +0.1% | +0.0% |  |  |
| b3_sea_beam | off | 5.90 | 3.99 |  | 2.19 | 0.48 | 1.32 | 0.07 |  |  | 1.68 | 0.95 |  | 0.18 | 0.08 |  | 0.04 | 3.03 | 157.80 | 163 | 3 | OK:30 | OK |
| b3_sea_beam | on | 3.61 | 2.45 |  | 1.55 | 0.37 | 0.53 | 0.06 |  |  | 0.98 | 0.51 |  | 0.13 | 0.06 |  | 0.03 | 2.82 | 195.80 | 164 | 3 | OK:30 | OK |
| b3_sea_beam | delta | -2.29 | -1.54 |  | -0.64 | -0.11 | -0.79 | -0.01 |  |  | -0.70 | -0.44 |  | -0.05 | -0.02 |  | -0.01 | -0.21 |  | +0.6% | +0.0% |  |  |
| b3_sea_beam_front | off | 3.55 | 2.52 |  | 1.02 | 0.23 | 1.27 | 0.07 |  |  | 0.87 | 0.18 |  | 0.11 | 0.06 |  | 0.03 | 1.98 | 215.10 | 98 | 3 | OK:30 | OK |
| b3_sea_beam_front | on | 3.36 | 2.25 |  | 1.02 | 0.24 | 0.99 | 0.07 |  |  | 0.94 | 0.29 |  | 0.11 | 0.06 |  | 0.03 | 1.95 | 269.80 | 99 | 3 | OK:30 | OK |
| b3_sea_beam_front | delta | -0.19 | -0.27 |  | +0.00 | +0.01 | -0.28 | +0.00 |  |  | +0.07 | +0.11 |  | +0.00 | +0.00 |  | +0.00 | -0.03 |  | +1.0% | +0.0% |  |  |
