# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 5.22 | 5.08 |  | 0.11 | 2.41 | 2.57 |  |  |  |  |  |  |  |  |  |  | 3.49 | 105.60 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | 14.34 | 12.46 | 0.33 | 2.33 | 5.47 | 4.65 | 0.09 |  |  | 1.66 | 0.25 |  | 0.14 | 0.07 | 0.37 | 0.61 | 7.12 | 61.70 | 1,212 | 1,316 | OK:30 | OK |
| lowsun_fp | delta | +9.12 | +7.38 |  | +2.22 | +3.06 | +2.08 |  |  |  |  |  |  |  |  |  |  | +3.63 |  | -42.0% | +0.0% |  |  |
| noon_fields | classic | 27.76 | 27.64 |  | 0.11 | 20.44 | 7.09 |  |  |  |  |  |  |  |  |  |  | 14.62 | 30.70 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | 10.66 | 9.33 | 0.18 | 1.45 | 6.59 | 1.29 | 0.10 |  |  | 1.12 | 0.21 |  | 0.12 | 0.07 |  | 0.03 | 7.37 | 58.70 | 1,566 | 4,508 | OK:30 | OK |
| noon_fields | delta | -17.10 | -18.31 |  | +1.34 | -13.85 | -5.80 |  |  |  |  |  |  |  |  |  |  | -7.25 |  | -76.9% | +0.0% |  |  |
| forest_edge_golden | classic | 9.59 | 9.43 |  | 0.09 | 5.27 | 4.06 |  |  |  |  |  |  |  |  |  |  | 4.58 | 101.70 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | 6.59 | 5.22 | 0.42 | 1.12 | 2.63 | 1.47 | 0.11 |  |  | 1.09 | 0.21 |  | 0.13 | 0.07 |  | 0.03 | 5.00 | 107.30 | 866 | 1,459 | OK:30 | OK |
| forest_edge_golden | delta | -3.00 | -4.21 |  | +1.03 | -2.64 | -2.59 |  |  |  |  |  |  |  |  |  |  | +0.42 |  | -65.4% | +0.0% |  |  |
| sunrise_ridge | classic | 22.82 | 22.67 |  | 0.13 | 18.80 | 3.74 |  |  |  |  |  |  |  |  |  |  | 20.74 | 30.00 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | 9.07 | 7.62 | 0.08 | 5.47 | 1.80 | 0.35 | 0.11 |  |  | 1.19 | 0.40 |  | 0.13 | 0.06 | 0.27 | 0.09 | 4.48 | 63.80 | 1,912 | 8,897 | OK:30 | OK |
| sunrise_ridge | delta | -13.75 | -15.05 |  | +5.34 | -17.00 | -3.39 |  |  |  |  |  |  |  |  |  |  | -16.26 |  | -85.4% | +0.0% |  |  |
| coastline_sunset | classic | 0.99 | 0.86 |  | 0.23 | 0.00 | 0.63 |  |  |  |  |  |  |  |  |  |  | 0.56 | 398.90 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 3.14 | 1.82 | 0.00 | 1.44 | 0.00 | 0.37 | 0.09 |  |  | 1.13 | 0.30 |  | 0.10 | 0.07 | 0.29 | 0.12 | 2.50 | 268.70 | 177 | 0 | OK:30 | OK |
| coastline_sunset | delta | +2.15 | +0.96 |  | +1.21 | +0.00 | -0.26 |  |  |  |  |  |  |  |  |  |  | +1.94 |  | +0.0% |  |  |  |
| town_night | classic | 6.06 | 5.87 |  | 0.08 | 1.62 | 4.17 |  |  |  |  |  |  |  |  |  |  | 4.44 | 117.20 | 1,188 | 1,011 | OK:195 | OK |
| town_night | remastered | 9.94 | 8.53 |  | 1.44 | 3.08 | 4.01 | 0.07 |  |  | 1.21 | 0.58 |  | 0.16 | 0.07 |  | 0.03 | 7.46 | 67.90 | 1,120 | 1,019 | OK:112 | OK |
| town_night | delta | +3.88 | +2.66 |  | +1.36 | +1.46 | -0.16 |  |  |  |  |  |  |  |  |  |  | +3.02 |  | -5.7% | +0.8% |  |  |
| overcast_broken | classic | 14.62 | 14.49 |  | 0.15 | 10.93 | 3.41 |  |  |  |  |  |  |  |  |  |  | 11.40 | 40.30 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | 8.22 | 7.12 | 0.23 | 1.94 | 4.63 | 0.55 | 0.09 |  |  | 0.87 | 0.42 |  | 0.12 | 0.07 |  | 0.03 | 7.69 | 61.10 | 2,214 | 5,692 | OK:30 | OK |
| overcast_broken | delta | -6.40 | -7.37 |  | +1.79 | -6.30 | -2.86 |  |  |  |  |  |  |  |  |  |  | -3.71 |  | -73.1% | +0.0% |  |  |
| sinai_desert_noon | classic | 7.98 | 7.80 |  | 0.20 | 3.74 | 3.86 |  |  |  |  |  |  |  |  |  |  | 6.96 | 69.80 | 2,689 | 1,363 | OK:30 | OK |
| sinai_desert_noon | remastered | 6.90 | 5.57 | 0.10 | 0.96 | 2.93 | 1.68 | 0.11 |  |  | 1.08 | 0.61 |  | 0.13 | 0.07 |  | 0.03 | 5.95 | 85.00 | 1,266 | 1,363 | OK:30 | OK |
| sinai_desert_noon | delta | -1.08 | -2.23 |  | +0.76 | -0.81 | -2.18 |  |  |  |  |  |  |  |  |  |  | -1.01 |  | -52.9% | +0.0% |  |  |
