# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.27 | 4.11 |  | 0.13 | 1.79 | 2.18 |  |  |  |  |  |  |  |  |  |  | 3.32 | 145.20 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | 8.64 | 6.86 | 0.14 | 1.27 | 3.43 | 2.16 | 0.12 |  |  | 1.49 | 0.28 | 0.01 | 0.18 | 0.07 |  | 0.11 | 8.33 | 93.50 | 1,090 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | +4.37 | +2.75 |  | +1.14 | +1.64 | -0.02 |  |  |  |  |  |  |  |  |  |  | +5.01 |  | -47.9% | -10.4% |  |  |
| sunrise_ridge | classic | 26.82 | 26.67 |  | 7.94 | 15.70 | 3.02 |  |  |  |  |  |  |  |  |  |  | 25.64 | 34.00 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | 8.99 | 7.39 | 0.11 | 5.01 | 2.06 | 0.32 | 0.16 |  |  | 1.21 | 0.26 | 0.01 | 0.20 | 0.06 |  | 0.11 | 4.92 | 70.10 | 1,659 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | -17.83 | -19.28 |  | -2.93 | -13.64 | -2.70 |  |  |  |  |  |  |  |  |  |  | -20.72 |  | -87.3% | -13.7% |  |  |
| noon_fields | classic | 9.00 | 8.88 |  | 0.11 | 6.42 | 2.35 |  |  |  |  |  |  |  |  |  |  | 8.85 | 67.20 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | 9.66 | 8.28 | 0.10 | 1.61 | 6.18 | 0.49 | 0.11 |  |  | 1.12 | 0.27 | 0.01 | 0.16 | 0.05 |  | 0.11 | 9.48 | 69.90 | 1,447 | 3,920 | OK:30 | OK |
| noon_fields | delta | +0.66 | -0.60 |  | +1.50 | -0.24 | -1.86 |  |  |  |  |  |  |  |  |  |  | +0.63 |  | -78.6% | -13.0% |  |  |
| forest_edge_golden | classic | 5.17 | 4.99 |  | 0.10 | 2.81 | 2.08 |  |  |  |  |  |  |  |  |  |  | 4.52 | 123.20 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | 6.53 | 4.91 | 0.19 | 1.32 | 2.77 | 0.82 | 0.14 |  |  | 1.25 | 0.27 | 0.01 | 0.22 | 0.05 |  | 0.12 | 6.15 | 105.30 | 757 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | +1.36 | -0.08 |  | +1.22 | -0.04 | -1.26 |  |  |  |  |  |  |  |  |  |  | +1.63 |  | -69.7% | -9.8% |  |  |
| coastline_sunset | classic | 1.18 | 0.96 |  | 0.52 | 0.00 | 0.44 |  |  |  |  |  |  |  |  |  |  | 0.75 | 383.00 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 6.06 | 3.24 | 0.00 | 3.00 | 0.00 | 0.24 | 0.25 |  |  | 2.30 | 0.18 | 0.01 | 0.26 | 0.12 |  | 0.21 | 5.61 | 171.20 | 154 | 0 | OK:30 | OK |
| coastline_sunset | delta | +4.88 | +2.28 |  | +2.48 | +0.00 | -0.20 |  |  |  |  |  |  |  |  |  |  | +4.86 |  | -13.0% |  |  |  |
| town_night | classic | 6.77 | 6.54 |  | 0.11 | 1.33 | 5.10 |  |  |  |  |  |  |  |  |  |  | 5.97 | 116.30 | 1,284 | 1,107 | OK:247 | OK |
| town_night | remastered | 10.50 | 8.94 | 0.15 | 1.14 | 3.62 | 4.18 | 0.05 |  |  | 1.23 | 0.29 | 0.01 | 0.15 | 0.05 |  | 0.10 | 10.10 | 75.10 | 1,081 | 964 | OK:71 | OK |
| town_night | delta | +3.73 | +2.40 |  | +1.03 | +2.29 | -0.92 |  |  |  |  |  |  |  |  |  |  | +4.13 |  | -15.8% | -12.9% |  |  |
| firefight_1 | classic | 4.85 | 4.30 |  | 0.31 | 0.70 | 3.28 |  |  |  |  |  |  |  |  |  |  | 0.89 | 202.60 | 470 | 446 | OK:47 | OK |
| firefight_1 | remastered | 6.29 | 4.40 |  | 0.95 | 1.64 | 1.81 | 0.19 |  |  | 1.31 | 0.25 | 0.01 | 0.20 | 0.07 |  | 0.18 | 5.35 | 129.20 | 535 | 535 | UNSETTLED:300 | OK |
| firefight_1 | delta | +1.44 | +0.10 |  | +0.64 | +0.94 | -1.47 |  |  |  |  |  |  |  |  |  |  | +4.46 |  | +12.1% | +16.6% |  |  |
| firefight_2 | classic | 4.85 | 4.30 |  | 0.31 | 0.70 | 3.28 |  |  |  |  |  |  |  |  |  |  | 0.89 | 202.60 | 557 | 529 | OK:47 | OK |
| firefight_2 | remastered | 6.29 | 4.40 |  | 0.95 | 1.64 | 1.81 | 0.19 |  |  | 1.31 | 0.25 | 0.01 | 0.20 | 0.07 |  | 0.18 | 5.35 | 129.20 | 538 | 543 | UNSETTLED:300 | OK |
| firefight_2 | delta | +1.44 | +0.10 |  | +0.64 | +0.94 | -1.47 |  |  |  |  |  |  |  |  |  |  | +4.46 |  | -3.4% | +2.6% |  |  |
| firefight_3 | classic | 4.85 | 4.30 |  | 0.31 | 0.70 | 3.28 |  |  |  |  |  |  |  |  |  |  | 0.89 | 202.60 | 483 | 434 | OK:47 | OK |
| firefight_3 | remastered | 6.29 | 4.40 |  | 0.95 | 1.64 | 1.81 | 0.19 |  |  | 1.31 | 0.25 | 0.01 | 0.20 | 0.07 |  | 0.18 | 5.35 | 129.20 | 538 | 533 | UNSETTLED:300 | OK |
| firefight_3 | delta | +1.44 | +0.10 |  | +0.64 | +0.94 | -1.47 |  |  |  |  |  |  |  |  |  |  | +4.46 |  | +10.2% | +18.6% |  |  |
| tank_gunner_interior | classic | 4.99 | 4.71 |  | 0.08 | 1.91 | 2.71 |  |  |  |  |  |  |  |  |  |  | 4.34 | 125.60 | 2,527 | 1,217 | OK:30 | OK |
| tank_gunner_interior | remastered | 8.57 | 7.61 | 0.07 | 0.93 | 3.02 | 3.65 | 0.09 |  |  | 0.63 | 0.14 |  | 0.28 | 0.07 |  | 0.10 | 8.03 | 77.20 | 1,531 | 1,229 | OK:30 | OK |
| tank_gunner_interior | delta | +3.58 | +2.90 |  | +0.85 | +1.11 | +0.94 |  |  |  |  |  |  |  |  |  |  | +3.69 |  | -39.4% | +1.0% |  |  |
| tank_gunner_optics | classic | 3.65 | 3.39 |  | 0.12 | 1.70 | 1.57 |  |  |  |  |  |  |  |  |  |  | 2.87 | 162.90 | 1,714 | 745 | OK:30 | OK |
| tank_gunner_optics | remastered | 7.49 | 6.37 | 0.06 | 0.90 | 3.41 | 2.05 | 0.10 |  |  | 0.86 | 0.21 | 0.01 | 0.15 |  |  | 0.10 | 6.96 | 113.20 | 1,141 | 745 | OK:30 | OK |
| tank_gunner_optics | delta | +3.84 | +2.98 |  | +0.78 | +1.71 | +0.48 |  |  |  |  |  |  |  |  |  |  | +4.09 |  | -33.4% | +0.0% |  |  |
| overcast_broken | classic | 11.80 | 11.67 |  | 0.15 | 8.83 | 2.69 |  |  |  |  |  |  |  |  |  |  | 11.33 | 50.70 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | 9.11 | 7.60 | 0.16 | 2.24 | 4.84 | 0.52 | 0.11 |  |  | 1.24 | 0.27 | 0.01 | 0.15 | 0.06 |  | 0.18 | 8.69 | 64.50 | 1,863 | 5,083 | OK:30 | OK |
| overcast_broken | delta | -2.69 | -4.07 |  | +2.09 | -3.99 | -2.17 |  |  |  |  |  |  |  |  |  |  | -2.64 |  | -77.4% | -10.7% |  |  |
| overcast_heavy | classic | 11.74 | 11.60 |  | 0.13 | 8.79 | 2.68 |  |  |  |  |  |  |  |  |  |  | 11.28 | 52.20 | 7,957 | 5,692 | OK:30 | OK |
| overcast_heavy | remastered | 8.86 | 7.04 | 0.18 | 1.87 | 4.64 | 0.52 | 0.11 |  |  | 1.54 | 0.27 | 0.01 | 0.15 | 0.07 |  | 0.11 | 8.60 | 67.60 | 1,619 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | -2.88 | -4.56 |  | +1.74 | -4.15 | -2.16 |  |  |  |  |  |  |  |  |  |  | -2.68 |  | -79.7% | -10.7% |  |  |
| optics | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| optics | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 5.48 | 5.34 |  | 0.12 | 2.95 | 2.27 |  |  |  |  |  |  |  |  |  |  | 5.04 | 109.60 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.24 | 0.97 |  | 0.55 | 0.00 | 0.42 |  |  |  |  |  |  |  |  |  |  | 0.79 | 335.10 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_valley | classic | 21.15 | 21.01 |  | 3.27 | 15.12 | 2.62 |  |  |  |  |  |  |  |  |  |  | 17.38 | 37.00 | 11,553 | 7,966 | OK:30 | OK |
| dawn_valley | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| pine_rays | classic | 5.31 | 5.13 |  | 0.07 | 2.17 | 2.89 |  |  |  |  |  |  |  |  |  |  | 4.43 | 118.30 | 3,163 | 1,678 | OK:30 | OK |
| pine_rays | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_fog_night | classic | 6.00 | 5.81 |  | 0.09 | 1.56 | 4.16 |  |  |  |  |  |  |  |  |  |  | 4.86 | 138.50 | 1,238 | 984 | OK:44 | OK |
| town_fog_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night_rain | classic | 4.78 | 4.53 |  | 0.12 | 0.79 | 3.62 |  |  |  |  |  |  |  |  |  |  | 3.68 | 157.20 | 916 | 670 | OK:206 | OK |
| town_night_rain | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 4.88 | 4.77 |  | 0.12 | 3.38 | 1.27 |  |  |  |  |  |  |  |  |  |  | 4.10 | 127.60 | 2,723 | 1,774 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 2.31 | 2.07 |  | 0.14 | 0.79 | 1.14 |  |  |  |  |  |  |  |  |  |  | 1.72 | 223.90 | 985 | 541 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 5.13 | 4.96 |  | 0.11 | 2.88 | 1.97 |  |  |  |  |  |  |  |  |  |  | 4.19 | 160.80 | 2,435 | 1,448 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| rain_day | classic | 4.02 | 3.85 |  | 0.11 | 2.03 | 1.71 |  |  |  |  |  |  |  |  |  |  | 2.75 | 152.80 | 1,753 | 785 | OK:30 | OK |
| rain_day | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | classic | 21.62 | 21.50 |  | 7.00 | 12.33 | 2.16 |  |  |  |  |  |  |  |  |  |  | 19.47 | 41.90 | 10,841 | 7,519 | OK:30 | OK |
| noon_vista | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 2.74 | 2.57 |  | 0.13 | 1.22 | 1.22 |  |  |  |  |  |  |  |  |  |  | 1.50 | 197.80 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 6.36 | 4.83 | 0.21 | 1.14 | 2.72 | 0.97 | 0.12 |  |  | 1.21 | 0.26 | 0.01 | 0.21 | 0.06 |  | 0.11 | 5.55 | 111.70 | 417 | 314 | UNSETTLED:300 | OK |
| sinai_lowsun_fp | delta | +3.62 | +2.26 |  | +1.01 | +1.50 | -0.25 |  |  |  |  |  |  |  |  |  |  | +4.05 |  | -30.8% | +0.0% |  |  |
| sinai_desert_noon | classic | 5.72 | 5.59 |  | 0.15 | 2.94 | 2.49 |  |  |  |  |  |  |  |  |  |  | 4.71 | 94.70 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 8.04 | 6.34 | 0.10 | 1.23 | 3.06 | 2.05 | 0.12 |  |  | 1.41 | 0.28 | 0.01 | 0.17 | 0.06 |  | 0.33 | 7.33 | 79.50 | 1,181 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +2.32 | +0.75 |  | +1.08 | +0.12 | -0.44 |  |  |  |  |  |  |  |  |  |  | +2.62 |  | -47.3% | +0.0% |  |  |
