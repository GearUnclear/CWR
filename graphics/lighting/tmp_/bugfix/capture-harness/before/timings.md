# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls of the shot frame (`triSceneStats`); load = the `triSettleScene` answer
before the mode's timers (frames run). Delta = Remastered minus Classic (draws: share of the larger).

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | draws | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 5.96 | 5.85 |  |  |  |  |  |  | 128.40 |  |  | OK |
| lowsun_fp | remastered | 12.51 | 10.89 | 0.58 | 1.50 | 0.25 | 0.05 | 0.22 | 0.98 | 80.40 |  |  | OK |
| lowsun_fp | delta | +6.55 | +5.04 |  |  |  |  |  |  |  |  |  |  |
| sunrise_ridge | classic | 45.34 | 45.22 |  |  |  |  |  |  | 30.40 |  |  | OK |
| sunrise_ridge | remastered | 20.95 | 19.98 | 0.00 | 0.84 | 0.61 | 0.05 | 0.15 | 0.03 | 29.60 |  |  | OK |
| sunrise_ridge | delta | -24.39 | -25.24 |  |  |  |  |  |  |  |  |  |  |
| noon_fields | classic | 13.46 | 13.35 |  |  |  |  |  |  | 51.10 |  |  | OK |
| noon_fields | remastered | 24.11 | 23.74 | 0.54 | 0.26 | 0.19 | 0.05 |  | 0.03 | 35.80 |  |  | OK |
| noon_fields | delta | +10.65 | +10.39 |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden | classic | 5.67 | 5.55 |  |  |  |  |  |  | 165.50 |  |  | OK |
| forest_edge_golden | remastered | 11.86 | 11.43 | 0.03 | 0.30 | 0.23 | 0.05 |  | 0.03 | 65.30 |  |  | OK |
| forest_edge_golden | delta | +6.19 | +5.88 |  |  |  |  |  |  |  |  |  |  |
| coastline_sunset | classic | 1.88 | 1.78 |  |  |  |  |  |  | 288.40 |  |  | OK |
| coastline_sunset | remastered | 2.47 | 1.95 | 0.00 | 0.41 | 0.17 | 0.05 | 0.16 | 0.03 | 239.50 |  |  | OK |
| coastline_sunset | delta | +0.59 | +0.17 |  |  |  |  |  |  |  |  |  |  |
| town_night | classic | 10.79 | 10.58 |  |  |  |  |  |  | 85.60 |  |  | OK |
| town_night | remastered | 17.40 | 16.97 |  | 0.27 | 0.19 | 0.05 |  | 0.03 | 56.60 |  |  | OK |
| town_night | delta | +6.61 | +6.39 |  |  |  |  |  |  |  |  |  |  |
| firefight_1 | classic | 3.24 | 3.10 |  |  |  |  |  |  | 171.00 |  |  | OK |
| firefight_1 | remastered | 7.84 | 7.32 |  | 0.38 | 0.30 | 0.05 |  | 0.03 | 120.80 |  |  | OK |
| firefight_1 | delta | +4.60 | +4.22 |  |  |  |  |  |  |  |  |  |  |
| firefight_2 | classic | 3.24 | 3.10 |  |  |  |  |  |  | 171.00 |  |  | OK |
| firefight_2 | remastered | 7.84 | 7.32 |  | 0.38 | 0.30 | 0.05 |  | 0.03 | 120.80 |  |  | OK |
| firefight_2 | delta | +4.60 | +4.22 |  |  |  |  |  |  |  |  |  |  |
| firefight_3 | classic | 3.24 | 3.10 |  |  |  |  |  |  | 171.00 |  |  | OK |
| firefight_3 | remastered | 7.84 | 7.32 |  | 0.38 | 0.30 | 0.05 |  | 0.03 | 120.80 |  |  | OK |
| firefight_3 | delta | +4.60 | +4.22 |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_interior | classic | 11.67 | 10.94 |  |  |  |  |  |  | 70.30 |  |  | OK |
| tank_gunner_interior | remastered | 14.10 | 12.97 | 0.11 | 0.18 |  | 0.16 |  | 0.02 | 61.70 |  |  | OK |
| tank_gunner_interior | delta | +2.43 | +2.03 |  |  |  |  |  |  |  |  |  |  |
| tank_gunner_optics | classic | 9.76 | 8.61 |  |  |  |  |  |  | 96.10 |  |  | OK |
| tank_gunner_optics | remastered | 14.51 | 12.99 | 0.08 | 0.17 | 0.14 |  |  | 0.02 | 69.60 |  |  | OK |
| tank_gunner_optics | delta | +4.75 | +4.38 |  |  |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 19.99 | 19.88 |  |  |  |  |  |  | 40.30 |  |  | OK |
| overcast_broken | remastered | 23.28 | 22.65 | 0.04 | 0.54 | 0.46 | 0.05 |  | 0.03 | 34.80 |  |  | OK |
| overcast_broken | delta | +3.29 | +2.77 |  |  |  |  |  |  |  |  |  |  |
| overcast_heavy | classic | 18.85 | 18.74 |  |  |  |  |  |  | 41.30 |  |  | OK |
| overcast_heavy | remastered | 22.55 | 21.74 | 0.23 | 0.69 | 0.62 | 0.05 |  | 0.03 | 35.60 |  |  | OK |
| overcast_heavy | delta | +3.70 | +3.00 |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 2.87 | 2.76 |  |  |  |  |  |  | 158.80 |  |  | OK |
| sinai_lowsun_fp | remastered | 6.06 | 4.96 | 0.05 | 0.98 | 0.35 | 0.05 | 0.22 | 0.35 | 115.90 |  |  | OK |
| sinai_lowsun_fp | delta | +3.19 | +2.20 |  |  |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 10.13 | 10.02 |  |  |  |  |  |  | 63.30 |  |  | OK |
| sinai_desert_noon | remastered | 12.67 | 11.61 | 0.05 | 0.94 | 0.86 | 0.05 |  | 0.03 | 56.70 |  |  | OK |
| sinai_desert_noon | delta | +2.54 | +1.59 |  |  |  |  |  |  |  |  |  |  |
