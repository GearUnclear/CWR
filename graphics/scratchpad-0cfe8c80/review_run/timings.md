# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic (draws and objects: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 5.46 | 5.21 |  |  |  |  |  |  | 132.20 | 1,575 | 1,179 | OK:30 | OK |
| lowsun_fp | remastered | 5.22 | 4.28 | 0.06 | 0.81 | 0.38 | 0.07 | 0.31 | 0.04 | 132.00 | 1,568 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | -0.24 | -0.93 |  |  |  |  |  |  |  | -0.4% | +0.0% |  |  |
| sunrise_ridge | classic | 13.41 | 13.30 |  |  |  |  |  |  | 46.40 | 11,172 | 7,680 | OK:30 | OK |
| sunrise_ridge | remastered | 17.26 | 16.45 | 0.00 | 0.70 | 0.48 | 0.05 | 0.15 | 0.03 | 45.00 | 11,172 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | +3.85 | +3.15 |  |  |  |  |  |  |  | +0.0% | +0.0% |  |  |
| noon_fields | classic | 5.97 | 5.87 |  |  |  |  |  |  | 97.00 | 5,572 | 3,920 | OK:30 | OK |
| noon_fields | remastered | 7.68 | 7.25 | 0.03 | 0.33 | 0.26 | 0.05 |  | 0.03 | 81.70 | 5,567 | 3,920 | OK:30 | OK |
| noon_fields | delta | +1.71 | +1.38 |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
| forest_edge_golden | classic | 3.87 | 3.66 |  |  |  |  |  |  | 235.30 | 1,850 | 1,316 | OK:30 | OK |
| forest_edge_golden | remastered | 4.92 | 4.30 | 0.05 | 0.46 | 0.34 | 0.07 |  | 0.04 | 127.80 | 1,847 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | +1.05 | +0.64 |  |  |  |  |  |  |  | -0.2% | +0.0% |  |  |
| coastline_sunset | classic | 2.62 | 2.19 |  |  |  |  |  |  | 252.20 | 155 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 4.11 | 1.84 | 0.00 | 1.92 | 0.81 | 0.21 | 0.75 | 0.16 | 223.70 | 155 | 0 | OK:30 | OK |
| coastline_sunset | delta | +1.49 | -0.35 |  |  |  |  |  |  |  | +0.0% |  |  |  |
| town_night | classic | 5.97 | 5.73 |  |  |  |  |  |  | 118.80 | 1,180 | 1,021 | OK:187 | OK |
| town_night | remastered | 5.83 | 5.09 |  | 0.56 | 0.46 | 0.06 |  | 0.04 | 128.10 | 1,170 | 1,008 | OK:30 | OK |
| town_night | delta | -0.14 | -0.64 |  |  |  |  |  |  |  | -0.8% | -1.3% |  |  |
| firefight_1 | classic | 4.94 | 4.50 |  |  |  |  |  |  | 174.60 | 412 | 364 | OK:47 | OK |
| firefight_1 | remastered | 6.27 | 4.52 |  | 1.34 | 1.00 | 0.20 |  | 0.14 | 129.30 | 473 | 482 | UNSETTLED:300 | OK |
| firefight_1 | delta | +1.33 | +0.02 |  |  |  |  |  |  |  | +12.9% | +24.5% |  |  |
| firefight_2 | classic | 4.94 | 4.50 |  |  |  |  |  |  | 174.60 | 483 | 486 | OK:47 | OK |
| firefight_2 | remastered | 6.27 | 4.52 |  | 1.34 | 1.00 | 0.20 |  | 0.14 | 129.30 | 507 | 532 | UNSETTLED:300 | OK |
| firefight_2 | delta | +1.33 | +0.02 |  |  |  |  |  |  |  | +4.7% | +8.6% |  |  |
| firefight_3 | classic | 4.94 | 4.50 |  |  |  |  |  |  | 174.60 | 440 | 416 | OK:47 | OK |
| firefight_3 | remastered | 6.27 | 4.52 |  | 1.34 | 1.00 | 0.20 |  | 0.14 | 129.30 | 473 | 466 | UNSETTLED:300 | OK |
| firefight_3 | delta | +1.33 | +0.02 |  |  |  |  |  |  |  | +7.0% | +10.7% |  |  |
| tank_gunner_interior | classic | 6.87 | 6.42 |  |  |  |  |  |  | 113.10 | 2,264 | 1,218 | OK:30 | OK |
| tank_gunner_interior | remastered | 6.81 | 6.04 | 0.05 | 0.48 |  | 0.45 |  | 0.03 | 105.10 | 2,274 | 1,233 | OK:30 | OK |
| tank_gunner_interior | delta | -0.06 | -0.38 |  |  |  |  |  |  |  | +0.4% | +1.2% |  |  |
| tank_gunner_optics | classic | 5.32 | 4.89 |  |  |  |  |  |  | 126.10 | 1,397 | 741 | OK:30 | OK |
| tank_gunner_optics | remastered | 5.08 | 4.40 | 0.06 | 0.41 | 0.37 |  |  | 0.03 | 127.40 | 1,378 | 741 | OK:30 | OK |
| tank_gunner_optics | delta | -0.24 | -0.49 |  |  |  |  |  |  |  | -1.4% | +0.0% |  |  |
| overcast_broken | classic | 8.35 | 8.25 |  |  |  |  |  |  | 72.60 | 6,837 | 5,083 | OK:30 | OK |
| overcast_broken | remastered | 9.73 | 9.06 | 0.03 | 0.57 | 0.50 | 0.05 |  | 0.03 | 64.70 | 6,835 | 5,083 | OK:30 | OK |
| overcast_broken | delta | +1.38 | +0.81 |  |  |  |  |  |  |  | -0.0% | +0.0% |  |  |
| overcast_heavy | classic | 8.33 | 8.22 |  |  |  |  |  |  | 73.90 | 6,593 | 5,083 | OK:30 | OK |
| overcast_heavy | remastered | 9.91 | 9.23 | 0.04 | 0.58 | 0.50 | 0.05 |  | 0.03 | 66.30 | 6,591 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | +1.58 | +1.01 |  |  |  |  |  |  |  | -0.0% | +0.0% |  |  |
| sinai_lowsun_fp | classic | 5.88 | 5.52 |  |  |  |  |  |  | 99.30 | 573 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 6.31 | 4.36 | 0.17 | 1.65 | 0.80 | 0.14 | 0.57 | 0.14 | 108.60 | 570 | 314 | OK:30 | OK |
| sinai_lowsun_fp | delta | +0.43 | -1.16 |  |  |  |  |  |  |  | -0.5% | +0.0% |  |  |
| sinai_desert_noon | classic | 7.37 | 7.07 |  |  |  |  |  |  | 92.60 | 2,233 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 7.11 | 6.18 | 0.06 | 0.78 | 0.66 | 0.08 |  | 0.04 | 86.80 | 2,230 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | -0.26 | -0.89 |  |  |  |  |  |  |  | -0.1% | +0.0% |  |  |
