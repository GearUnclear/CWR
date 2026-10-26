# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge_r1 | classic | 40.68 | 40.53 |  | 7.74 | 26.82 | 5.97 |  |  |  |  |  |  |  |  |  |  | 30.47 | 16.50 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge_r1 | remastered | 17.73 | 13.87 | 0.08 | 6.32 | 5.60 | 1.94 | 0.27 |  |  | 3.23 | 0.35 | 0.01 | 0.13 | 0.06 |  | 0.04 | 5.07 | 47.60 | 1,913 | 8,897 | OK:30 | OK |
| sunrise_ridge_r1 | delta | -22.95 | -26.66 |  | -1.42 | -21.22 | -4.03 |  |  |  |  |  |  |  |  |  |  | -25.40 |  | -85.4% | +0.0% |  |  |
| forest_edge_golden_r1 | classic | 14.78 | 14.62 |  | 0.09 | 7.78 | 6.75 |  |  |  |  |  |  |  |  |  |  | 4.54 | 67.40 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden_r1 | remastered | 18.25 | 15.24 | 1.14 | 3.11 | 8.30 | 3.82 | 0.12 |  |  | 2.29 | 0.16 | 0.01 | 0.13 | 0.88 |  | 0.04 | 6.27 | 56.00 | 866 | 1,459 | OK:30 | OK |
| forest_edge_golden_r1 | delta | +3.47 | +0.62 |  | +3.02 | +0.52 | -2.93 |  |  |  |  |  |  |  |  |  |  | +1.73 |  | -65.4% | +0.0% |  |  |
| town_night_r1 | classic | 7.93 | 7.74 |  | 0.07 | 1.41 | 6.25 |  |  |  |  |  |  |  |  |  |  | 5.00 | 123.30 | 1,289 | 1,113 | OK:246 | OK |
| town_night_r1 | remastered | 16.18 | 14.18 | 0.22 | 2.62 | 4.96 | 6.61 | 0.08 |  |  | 1.57 | 0.36 | 0.02 | 0.16 | 0.20 |  | 0.18 | 13.18 | 52.70 | 1,224 | 1,111 | OK:37 | OK |
| town_night_r1 | delta | +8.25 | +6.44 |  | +2.55 | +3.55 | +0.36 |  |  |  |  |  |  |  |  |  |  | +8.18 |  | -5.0% | -0.2% |  |  |
| coastline_sunset_r1 | classic | 1.41 | 1.23 |  | 0.67 | 0.00 | 0.56 |  |  |  |  |  |  |  |  |  |  | 0.57 | 352.90 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r1 | remastered | 4.55 | 2.49 | 0.00 | 1.94 | 0.00 | 0.54 | 0.10 |  |  | 1.84 | 0.15 | 0.01 | 0.10 | 0.08 |  | 0.03 | 3.20 | 214.30 | 177 | 0 | OK:30 | OK |
| coastline_sunset_r1 | delta | +3.14 | +1.26 |  | +1.27 | +0.00 | -0.02 |  |  |  |  |  |  |  |  |  |  | +2.63 |  | +0.0% |  |  |  |
| woods_pine2_evening_r1 | classic | 7.11 | 6.97 |  | 0.12 | 2.83 | 4.02 |  |  |  |  |  |  |  |  |  |  | 5.13 | 86.10 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r1 | remastered | 13.00 | 10.92 | 0.50 | 2.58 | 7.31 | 1.02 | 0.10 |  |  | 1.81 | 0.34 | 0.01 | 0.21 | 0.06 |  | 0.04 | 9.45 | 69.70 | 1,421 | 1,811 | OK:30 | OK |
| woods_pine2_evening_r1 | delta | +5.89 | +3.95 |  | +2.46 | +4.48 | -3.00 |  |  |  |  |  |  |  |  |  |  | +4.32 |  | -56.3% | +0.0% |  |  |
| lowsun_fp_r1 | classic | 5.86 | 5.73 |  | 0.11 | 2.85 | 2.76 |  |  |  |  |  |  |  |  |  |  | 3.38 | 121.20 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp_r1 | remastered | 16.67 | 14.05 | 0.32 | 2.19 | 6.69 | 5.17 | 0.10 |  |  | 2.36 | 0.15 | 0.01 | 0.14 | 0.06 |  | 0.04 | 7.34 | 59.90 | 1,212 | 1,316 | OK:30 | OK |
| lowsun_fp_r1 | delta | +10.81 | +8.32 |  | +2.08 | +3.84 | +2.41 |  |  |  |  |  |  |  |  |  |  | +3.96 |  | -42.0% | +0.0% |  |  |
| road_flat_drive_moonless_r1 | classic | 6.88 | 6.72 |  | 0.08 | 3.75 | 2.89 |  |  |  |  |  |  |  |  |  |  | 3.06 | 103.50 | 1,806 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r1 | remastered | 16.81 | 14.50 |  | 1.85 | 8.28 | 4.37 | 0.06 |  |  | 2.11 | 0.23 | 0.01 | 0.13 | 0.06 |  | 0.03 | 9.23 | 54.10 | 1,411 | 914 | OK:30 | OK |
| road_flat_drive_moonless_r1 | delta | +9.93 | +7.78 |  | +1.77 | +4.53 | +1.48 |  |  |  |  |  |  |  |  |  |  | +6.17 |  | -21.9% | +0.0% |  |  |
| ffh_r1 | classic | 7.53 | 7.38 |  | 0.10 | 0.64 | 6.64 |  |  |  |  |  |  |  |  |  |  | 3.87 | 99.20 | 803 | 698 | UNSETTLED:300 | OK |
| ffh_r1 | remastered | 12.02 | 10.52 |  | 1.16 | 2.24 | 7.12 | 0.04 |  |  | 1.25 | 0.19 | 0.01 | 0.13 | 0.14 |  | 0.04 | 6.21 | 78.00 | 789 | 680 | UNSETTLED:300 | OK |
| ffh_r1 | delta | +4.49 | +3.14 |  | +1.06 | +1.60 | +0.48 |  |  |  |  |  |  |  |  |  |  | +2.34 |  | -1.7% | -2.6% |  |  |
| bay_offshore_r1 | classic | 11.29 | 11.16 |  | 0.75 | 9.58 | 0.83 |  |  |  |  |  |  |  |  |  |  | 5.45 | 67.70 | 4,562 | 2,961 | OK:30 | OK |
| bay_offshore_r1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
