# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.28 | 4.11 |  | 0.14 | 1.76 | 2.22 |  |  |  |  |  |  |  |  |  |  | 3.35 | 143.50 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | 8.57 | 6.74 | 0.14 | 1.28 | 3.39 | 2.07 | 0.12 |  |  | 1.55 | 0.29 | 0.01 | 0.18 | 0.09 |  | 0.12 | 8.29 | 93.80 | 1,090 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | +4.29 | +2.63 |  | +1.14 | +1.63 | -0.15 |  |  |  |  |  |  |  |  |  |  | +4.94 |  | -47.9% | -10.4% |  |  |
| sunrise_ridge | classic | 26.26 | 26.11 |  | 7.84 | 15.32 | 2.96 |  |  |  |  |  |  |  |  |  |  | 25.37 | 34.40 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | 9.00 | 7.35 | 0.10 | 5.00 | 2.05 | 0.30 | 0.16 |  |  | 1.23 | 0.26 | 0.01 | 0.20 | 0.06 |  | 0.12 | 4.91 | 69.90 | 1,659 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | -17.26 | -18.76 |  | -2.84 | -13.27 | -2.66 |  |  |  |  |  |  |  |  |  |  | -20.46 |  | -87.3% | -13.7% |  |  |
| noon_fields | classic | 8.88 | 8.75 |  | 0.11 | 6.34 | 2.30 |  |  |  |  |  |  |  |  |  |  | 8.70 | 68.20 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | 9.57 | 8.17 | 0.10 | 1.59 | 6.08 | 0.50 | 0.11 |  |  | 1.14 | 0.28 | 0.01 | 0.16 | 0.05 |  | 0.11 | 9.37 | 70.70 | 1,447 | 3,920 | OK:30 | OK |
| noon_fields | delta | +0.69 | -0.58 |  | +1.48 | -0.26 | -1.80 |  |  |  |  |  |  |  |  |  |  | +0.67 |  | -78.6% | -13.0% |  |  |
| forest_edge_golden | classic | 5.47 | 5.29 |  | 0.10 | 2.97 | 2.21 |  |  |  |  |  |  |  |  |  |  | 4.47 | 123.00 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | 6.47 | 4.85 | 0.19 | 1.35 | 2.68 | 0.82 | 0.14 |  |  | 1.24 | 0.26 | 0.01 | 0.22 | 0.05 |  | 0.12 | 6.18 | 106.50 | 757 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | +1.00 | -0.44 |  | +1.25 | -0.29 | -1.39 |  |  |  |  |  |  |  |  |  |  | +1.71 |  | -69.7% | -9.8% |  |  |
| coastline_sunset | classic | 1.21 | 0.99 |  | 0.54 | 0.00 | 0.45 |  |  |  |  |  |  |  |  |  |  | 0.82 | 369.60 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 3.35 | 1.65 | 0.00 | 1.47 | 0.00 | 0.18 | 0.12 |  |  | 1.44 | 0.12 | 0.01 | 0.17 | 0.06 |  | 0.11 | 3.10 | 250.30 | 154 | 0 | OK:30 | OK |
| coastline_sunset | delta | +2.14 | +0.66 |  | +0.93 | +0.00 | -0.27 |  |  |  |  |  |  |  |  |  |  | +2.28 |  | -13.0% |  |  |  |
| town_night | classic | 6.55 | 6.31 |  | 0.11 | 1.44 | 4.77 |  |  |  |  |  |  |  |  |  |  | 5.82 | 117.20 | 1,259 | 1,085 | OK:224 | OK |
| town_night | remastered | 10.83 | 9.27 | 0.15 | 1.09 | 3.51 | 4.67 | 0.05 |  |  | 1.23 | 0.30 | 0.01 | 0.15 | 0.06 |  | 0.10 | 10.64 | 72.20 | 1,191 | 1,074 | OK:32 | OK |
| town_night | delta | +4.28 | +2.96 |  | +0.98 | +2.07 | -0.10 |  |  |  |  |  |  |  |  |  |  | +4.82 |  | -5.4% | -1.0% |  |  |
| firefight_1 | classic | 4.35 | 3.84 |  | 0.25 | 0.59 | 3.00 |  |  |  |  |  |  |  |  |  |  | 0.83 | 201.60 | 464 | 437 | OK:47 | OK |
| firefight_1 | remastered | 6.24 | 4.32 |  | 0.97 | 1.71 | 1.64 | 0.20 |  |  | 1.30 | 0.27 | 0.01 | 0.21 | 0.07 |  | 0.17 | 5.50 | 129.30 | 454 | 506 | UNSETTLED:300 | OK |
| firefight_1 | delta | +1.89 | +0.48 |  | +0.72 | +1.12 | -1.36 |  |  |  |  |  |  |  |  |  |  | +4.67 |  | -2.2% | +13.6% |  |  |
| firefight_2 | classic | 4.35 | 3.84 |  | 0.25 | 0.59 | 3.00 |  |  |  |  |  |  |  |  |  |  | 0.83 | 201.60 | 591 | 538 | OK:47 | OK |
| firefight_2 | remastered | 6.24 | 4.32 |  | 0.97 | 1.71 | 1.64 | 0.20 |  |  | 1.30 | 0.27 | 0.01 | 0.21 | 0.07 |  | 0.17 | 5.50 | 129.30 | 481 | 510 | UNSETTLED:300 | OK |
| firefight_2 | delta | +1.89 | +0.48 |  | +0.72 | +1.12 | -1.36 |  |  |  |  |  |  |  |  |  |  | +4.67 |  | -18.6% | -5.2% |  |  |
| firefight_3 | classic | 4.35 | 3.84 |  | 0.25 | 0.59 | 3.00 |  |  |  |  |  |  |  |  |  |  | 0.83 | 201.60 | 508 | 468 | OK:47 | OK |
| firefight_3 | remastered | 6.24 | 4.32 |  | 0.97 | 1.71 | 1.64 | 0.20 |  |  | 1.30 | 0.27 | 0.01 | 0.21 | 0.07 |  | 0.17 | 5.50 | 129.30 | 563 | 529 | UNSETTLED:300 | OK |
| firefight_3 | delta | +1.89 | +0.48 |  | +0.72 | +1.12 | -1.36 |  |  |  |  |  |  |  |  |  |  | +4.67 |  | +9.8% | +11.5% |  |  |
| tank_gunner_interior | classic | 5.30 | 5.01 |  | 0.09 | 2.08 | 2.84 |  |  |  |  |  |  |  |  |  |  | 4.24 | 125.40 | 2,529 | 1,216 | OK:30 | OK |
| tank_gunner_interior | remastered | 8.24 | 7.30 | 0.13 | 0.89 | 2.84 | 3.56 | 0.09 |  |  | 0.63 | 0.14 |  | 0.28 | 0.06 |  | 0.10 | 8.02 | 79.80 | 1,532 | 1,229 | OK:30 | OK |
| tank_gunner_interior | delta | +2.94 | +2.29 |  | +0.80 | +0.76 | +0.72 |  |  |  |  |  |  |  |  |  |  | +3.78 |  | -39.4% | +1.1% |  |  |
| tank_gunner_optics | classic | 3.65 | 3.38 |  | 0.12 | 1.66 | 1.60 |  |  |  |  |  |  |  |  |  |  | 2.84 | 163.90 | 1,728 | 742 | OK:30 | OK |
| tank_gunner_optics | remastered | 7.67 | 6.54 | 0.06 | 0.90 | 3.49 | 2.15 | 0.10 |  |  | 0.87 | 0.22 | 0.01 | 0.15 |  |  | 0.10 | 7.29 | 109.10 | 1,136 | 741 | OK:30 | OK |
| tank_gunner_optics | delta | +4.02 | +3.16 |  | +0.78 | +1.83 | +0.55 |  |  |  |  |  |  |  |  |  |  | +4.45 |  | -34.3% | -0.1% |  |  |
| overcast_broken | classic | 11.64 | 11.51 |  | 0.15 | 8.71 | 2.65 |  |  |  |  |  |  |  |  |  |  | 11.13 | 51.40 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | 8.90 | 7.43 | 0.16 | 2.22 | 4.74 | 0.46 | 0.11 |  |  | 1.19 | 0.25 | 0.01 | 0.15 | 0.06 |  | 0.18 | 8.70 | 65.80 | 1,863 | 5,083 | OK:30 | OK |
| overcast_broken | delta | -2.74 | -4.08 |  | +2.07 | -3.97 | -2.19 |  |  |  |  |  |  |  |  |  |  | -2.43 |  | -77.4% | -10.7% |  |  |
| overcast_heavy | classic | 11.53 | 11.39 |  | 0.13 | 8.58 | 2.68 |  |  |  |  |  |  |  |  |  |  | 10.75 | 53.00 | 7,957 | 5,692 | OK:30 | OK |
| overcast_heavy | remastered | 8.74 | 6.91 | 0.13 | 1.87 | 4.61 | 0.44 | 0.11 |  |  | 1.54 | 0.28 | 0.01 | 0.15 | 0.07 |  | 0.11 | 8.43 | 68.50 | 1,619 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | -2.79 | -4.48 |  | +1.74 | -3.97 | -2.24 |  |  |  |  |  |  |  |  |  |  | -2.32 |  | -79.7% | -10.7% |  |  |
| optics | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| optics | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 6.17 | 6.02 |  | 0.13 | 3.09 | 2.80 |  |  |  |  |  |  |  |  |  |  | 5.03 | 108.70 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.35 | 1.07 |  | 0.69 | 0.00 | 0.38 |  |  |  |  |  |  |  |  |  |  | 0.80 | 322.70 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_valley | classic | 21.55 | 21.42 |  | 3.90 | 14.98 | 2.54 |  |  |  |  |  |  |  |  |  |  | 17.47 | 37.60 | 11,553 | 7,966 | OK:30 | OK |
| dawn_valley | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| pine_rays | classic | 5.52 | 5.33 |  | 0.08 | 2.26 | 2.99 |  |  |  |  |  |  |  |  |  |  | 4.40 | 120.50 | 3,163 | 1,678 | OK:30 | OK |
| pine_rays | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_fog_night | classic | 6.25 | 6.06 |  | 0.09 | 1.61 | 4.36 |  |  |  |  |  |  |  |  |  |  | 5.23 | 126.30 | 1,295 | 1,037 | OK:75 | OK |
| town_fog_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night_rain | classic | 4.38 | 4.07 |  | 0.18 | 0.70 | 3.19 |  |  |  |  |  |  |  |  |  |  | 3.84 | 153.70 | 811 | 537 | OK:260 | OK |
| town_night_rain | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 4.91 | 4.80 |  | 0.12 | 3.41 | 1.27 |  |  |  |  |  |  |  |  |  |  | 4.12 | 130.30 | 2,723 | 1,774 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 2.34 | 2.10 |  | 0.14 | 0.80 | 1.16 |  |  |  |  |  |  |  |  |  |  | 1.68 | 222.60 | 985 | 541 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 5.23 | 5.05 |  | 0.11 | 2.92 | 2.03 |  |  |  |  |  |  |  |  |  |  | 4.17 | 162.90 | 2,435 | 1,448 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| rain_day | classic | 4.01 | 3.82 |  | 0.12 | 1.97 | 1.73 |  |  |  |  |  |  |  |  |  |  | 2.70 | 157.30 | 1,751 | 783 | OK:30 | OK |
| rain_day | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | classic | 21.19 | 21.06 |  | 6.96 | 11.91 | 2.18 |  |  |  |  |  |  |  |  |  |  | 19.60 | 42.50 | 10,841 | 7,519 | OK:30 | OK |
| noon_vista | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 2.82 | 2.62 |  | 0.14 | 1.08 | 1.40 |  |  |  |  |  |  |  |  |  |  | 1.54 | 195.40 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 6.16 | 4.67 | 0.21 | 1.16 | 2.59 | 0.92 | 0.12 |  |  | 1.17 | 0.26 | 0.01 | 0.21 | 0.06 |  | 0.11 | 5.62 | 110.60 | 417 | 314 | UNSETTLED:300 | OK |
| sinai_lowsun_fp | delta | +3.34 | +2.05 |  | +1.02 | +1.51 | -0.48 |  |  |  |  |  |  |  |  |  |  | +4.08 |  | -30.8% | +0.0% |  |  |
| sinai_desert_noon | classic | 5.71 | 5.59 |  | 0.14 | 2.96 | 2.48 |  |  |  |  |  |  |  |  |  |  | 4.71 | 94.40 | 2,242 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 7.90 | 6.22 | 0.10 | 1.25 | 3.00 | 1.97 | 0.12 |  |  | 1.39 | 0.32 | 0.01 | 0.17 | 0.06 |  | 0.31 | 7.36 | 77.50 | 1,180 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +2.19 | +0.63 |  | +1.11 | +0.04 | -0.51 |  |  |  |  |  |  |  |  |  |  | +2.65 |  | -47.4% | +0.0% |  |  |
