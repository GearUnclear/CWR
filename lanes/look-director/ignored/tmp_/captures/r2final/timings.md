# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.48 | 4.32 |  | 0.13 | 1.72 | 2.47 |  |  |  |  |  |  |  |  |  |  | 3.37 | 141.70 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | 8.46 | 6.72 | 0.14 | 1.30 | 3.38 | 2.04 | 0.12 |  |  | 1.46 | 0.26 | 0.01 | 0.18 | 0.05 |  | 0.11 | 8.12 | 92.70 | 1,090 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | +3.98 | +2.40 |  | +1.17 | +1.66 | -0.43 |  |  |  |  |  |  |  |  |  |  | +4.75 |  | -47.9% | -10.4% |  |  |
| sunrise_ridge | classic | 26.21 | 26.06 |  | 7.85 | 15.28 | 2.93 |  |  |  |  |  |  |  |  |  |  | 25.06 | 34.60 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | 9.27 | 7.66 | 0.11 | 5.14 | 2.15 | 0.37 | 0.16 |  |  | 1.23 | 0.26 | 0.01 | 0.19 | 0.06 |  | 0.12 | 4.92 | 69.90 | 1,659 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | -16.94 | -18.40 |  | -2.71 | -13.13 | -2.56 |  |  |  |  |  |  |  |  |  |  | -20.14 |  | -87.3% | -13.7% |  |  |
| noon_fields | classic | 8.99 | 8.86 |  | 0.11 | 6.46 | 2.30 |  |  |  |  |  |  |  |  |  |  | 8.55 | 68.40 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | 9.44 | 8.06 | 0.10 | 1.54 | 6.05 | 0.47 | 0.11 |  |  | 1.12 | 0.28 | 0.01 | 0.16 | 0.05 |  | 0.11 | 9.23 | 70.20 | 1,447 | 3,920 | OK:30 | OK |
| noon_fields | delta | +0.45 | -0.80 |  | +1.43 | -0.41 | -1.83 |  |  |  |  |  |  |  |  |  |  | +0.68 |  | -78.6% | -13.0% |  |  |
| forest_edge_golden | classic | 5.62 | 5.44 |  | 0.10 | 3.08 | 2.25 |  |  |  |  |  |  |  |  |  |  | 4.52 | 121.30 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | 6.54 | 4.90 | 0.19 | 1.41 | 2.69 | 0.80 | 0.15 |  |  | 1.26 | 0.26 | 0.01 | 0.23 | 0.05 |  | 0.12 | 6.32 | 104.50 | 757 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | +0.92 | -0.54 |  | +1.31 | -0.39 | -1.45 |  |  |  |  |  |  |  |  |  |  | +1.80 |  | -69.7% | -9.8% |  |  |
| coastline_sunset | classic | 1.44 | 1.22 |  | 0.72 | 0.00 | 0.49 |  |  |  |  |  |  |  |  |  |  | 0.79 | 344.40 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 3.25 | 1.55 | 0.00 | 1.37 | 0.00 | 0.18 | 0.12 |  |  | 1.43 | 0.11 | 0.01 | 0.17 | 0.06 |  | 0.11 | 3.10 | 249.30 | 154 | 0 | OK:30 | OK |
| coastline_sunset | delta | +1.81 | +0.33 |  | +0.65 | +0.00 | -0.31 |  |  |  |  |  |  |  |  |  |  | +2.31 |  | -13.0% |  |  |  |
| town_night | classic | 6.22 | 5.95 |  | 0.16 | 1.24 | 4.55 |  |  |  |  |  |  |  |  |  |  | 5.44 | 124.10 | 1,212 | 1,031 | OK:140 | OK |
| town_night | remastered | 10.65 | 9.09 | 0.16 | 1.10 | 3.39 | 4.60 | 0.05 |  |  | 1.22 | 0.30 | 0.01 | 0.15 | 0.05 |  | 0.10 | 10.38 | 74.70 | 1,188 | 1,073 | OK:36 | OK |
| town_night | delta | +4.43 | +3.14 |  | +0.94 | +2.15 | +0.05 |  |  |  |  |  |  |  |  |  |  | +4.94 |  | -2.0% | +3.9% |  |  |
| firefight_1 | classic | 4.86 | 4.41 |  | 0.29 | 0.63 | 3.49 |  |  |  |  |  |  |  |  |  |  | 0.86 | 195.80 | 447 | 351 | OK:45 | OK |
| firefight_1 | remastered | 6.75 | 4.78 |  | 1.02 | 1.88 | 1.88 | 0.21 |  |  | 1.32 | 0.25 | 0.01 | 0.22 | 0.07 |  | 0.19 | 5.71 | 127.60 | 466 | 497 | UNSETTLED:300 | OK |
| firefight_1 | delta | +1.89 | +0.37 |  | +0.73 | +1.25 | -1.61 |  |  |  |  |  |  |  |  |  |  | +4.85 |  | +4.1% | +29.4% |  |  |
| firefight_2 | classic | 4.86 | 4.41 |  | 0.29 | 0.63 | 3.49 |  |  |  |  |  |  |  |  |  |  | 0.86 | 195.80 | 514 | 444 | OK:45 | OK |
| firefight_2 | remastered | 6.75 | 4.78 |  | 1.02 | 1.88 | 1.88 | 0.21 |  |  | 1.32 | 0.25 | 0.01 | 0.22 | 0.07 |  | 0.19 | 5.71 | 127.60 | 463 | 447 | UNSETTLED:300 | OK |
| firefight_2 | delta | +1.89 | +0.37 |  | +0.73 | +1.25 | -1.61 |  |  |  |  |  |  |  |  |  |  | +4.85 |  | -9.9% | +0.7% |  |  |
| firefight_3 | classic | 4.86 | 4.41 |  | 0.29 | 0.63 | 3.49 |  |  |  |  |  |  |  |  |  |  | 0.86 | 195.80 | 490 | 387 | OK:45 | OK |
| firefight_3 | remastered | 6.75 | 4.78 |  | 1.02 | 1.88 | 1.88 | 0.21 |  |  | 1.32 | 0.25 | 0.01 | 0.22 | 0.07 |  | 0.19 | 5.71 | 127.60 | 522 | 547 | UNSETTLED:300 | OK |
| firefight_3 | delta | +1.89 | +0.37 |  | +0.73 | +1.25 | -1.61 |  |  |  |  |  |  |  |  |  |  | +4.85 |  | +6.1% | +29.3% |  |  |
| tank_gunner_interior | classic | 5.34 | 5.06 |  | 0.10 | 2.21 | 2.75 |  |  |  |  |  |  |  |  |  |  | 4.30 | 126.60 | 2,531 | 1,218 | OK:30 | OK |
| tank_gunner_interior | remastered | 8.17 | 7.26 | 0.07 | 0.88 | 2.98 | 3.40 | 0.09 |  |  | 0.60 | 0.14 |  | 0.28 | 0.05 |  | 0.09 | 7.96 | 79.50 | 1,533 | 1,230 | OK:30 | OK |
| tank_gunner_interior | delta | +2.83 | +2.20 |  | +0.78 | +0.77 | +0.65 |  |  |  |  |  |  |  |  |  |  | +3.66 |  | -39.4% | +1.0% |  |  |
| tank_gunner_optics | classic | 3.64 | 3.37 |  | 0.12 | 1.64 | 1.60 |  |  |  |  |  |  |  |  |  |  | 2.84 | 165.30 | 1,730 | 744 | OK:30 | OK |
| tank_gunner_optics | remastered | 7.69 | 6.52 | 0.06 | 0.97 | 3.36 | 2.19 | 0.11 |  |  | 0.90 | 0.22 | 0.01 | 0.15 |  |  | 0.10 | 7.37 | 110.00 | 1,139 | 744 | OK:30 | OK |
| tank_gunner_optics | delta | +4.05 | +3.15 |  | +0.85 | +1.72 | +0.59 |  |  |  |  |  |  |  |  |  |  | +4.53 |  | -34.2% | +0.0% |  |  |
| overcast_broken | classic | 11.36 | 11.23 |  | 0.15 | 8.47 | 2.61 |  |  |  |  |  |  |  |  |  |  | 11.01 | 52.40 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | 8.83 | 7.29 | 0.15 | 2.25 | 4.57 | 0.47 | 0.11 |  |  | 1.26 | 0.29 | 0.01 | 0.15 | 0.06 |  | 0.18 | 8.44 | 66.20 | 1,863 | 5,083 | OK:30 | OK |
| overcast_broken | delta | -2.53 | -3.94 |  | +2.10 | -3.90 | -2.14 |  |  |  |  |  |  |  |  |  |  | -2.57 |  | -77.4% | -10.7% |  |  |
| overcast_heavy | classic | 11.36 | 11.22 |  | 0.13 | 8.50 | 2.59 |  |  |  |  |  |  |  |  |  |  | 10.96 | 53.30 | 7,957 | 5,692 | OK:30 | OK |
| overcast_heavy | remastered | 8.70 | 6.85 | 0.14 | 1.87 | 4.48 | 0.50 | 0.11 |  |  | 1.56 | 0.27 | 0.01 | 0.15 | 0.07 |  | 0.11 | 8.46 | 66.60 | 1,619 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | -2.66 | -4.37 |  | +1.74 | -4.02 | -2.09 |  |  |  |  |  |  |  |  |  |  | -2.50 |  | -79.7% | -10.7% |  |  |
| optics | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| optics | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| woods_pine2_evening | classic | 5.85 | 5.70 |  | 0.13 | 3.18 | 2.39 |  |  |  |  |  |  |  |  |  |  | 4.98 | 111.70 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.44 | 1.17 |  | 0.75 | 0.00 | 0.42 |  |  |  |  |  |  |  |  |  |  | 0.77 | 324.20 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_valley | classic | 20.67 | 20.53 |  | 3.23 | 14.72 | 2.58 |  |  |  |  |  |  |  |  |  |  | 17.24 | 37.30 | 11,553 | 7,966 | OK:30 | OK |
| dawn_valley | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| pine_rays | classic | 5.74 | 5.54 |  | 0.08 | 2.45 | 3.01 |  |  |  |  |  |  |  |  |  |  | 4.36 | 121.80 | 3,163 | 1,678 | OK:30 | OK |
| pine_rays | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_fog_night | classic | 5.83 | 5.64 |  | 0.09 | 1.52 | 4.03 |  |  |  |  |  |  |  |  |  |  | 4.69 | 141.00 | 1,233 | 964 | OK:44 | OK |
| town_fog_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night_rain | classic | 5.23 | 5.02 |  | 0.10 | 0.90 | 4.02 |  |  |  |  |  |  |  |  |  |  | 3.75 | 155.20 | 995 | 759 | OK:52 | OK |
| town_night_rain | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 4.74 | 4.63 |  | 0.12 | 3.33 | 1.18 |  |  |  |  |  |  |  |  |  |  | 3.95 | 133.30 | 2,723 | 1,774 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 2.35 | 2.11 |  | 0.15 | 0.80 | 1.16 |  |  |  |  |  |  |  |  |  |  | 1.67 | 222.90 | 985 | 541 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 5.13 | 4.96 |  | 0.11 | 3.01 | 1.84 |  |  |  |  |  |  |  |  |  |  | 4.15 | 166.50 | 2,435 | 1,448 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| rain_day | classic | 3.85 | 3.64 |  | 0.12 | 1.76 | 1.76 |  |  |  |  |  |  |  |  |  |  | 2.55 | 170.60 | 1,753 | 785 | OK:30 | OK |
| rain_day | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | classic | 20.23 | 20.11 |  | 6.76 | 11.34 | 2.01 |  |  |  |  |  |  |  |  |  |  | 19.16 | 43.20 | 10,841 | 7,519 | OK:30 | OK |
| noon_vista | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 2.84 | 2.64 |  | 0.15 | 1.13 | 1.36 |  |  |  |  |  |  |  |  |  |  | 1.55 | 204.60 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 6.59 | 4.99 | 0.24 | 1.17 | 2.84 | 0.98 | 0.13 |  |  | 1.26 | 0.25 | 0.01 | 0.23 | 0.06 |  | 0.12 | 5.92 | 111.70 | 582 | 314 | UNSETTLED:300 | OK |
| sinai_lowsun_fp | delta | +3.75 | +2.35 |  | +1.02 | +1.71 | -0.38 |  |  |  |  |  |  |  |  |  |  | +4.37 |  | -3.5% | +0.0% |  |  |
| sinai_desert_noon | classic | 5.73 | 5.60 |  | 0.15 | 2.98 | 2.47 |  |  |  |  |  |  |  |  |  |  | 4.59 | 96.30 | 2,242 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 7.76 | 6.14 | 0.10 | 1.19 | 3.04 | 1.91 | 0.12 |  |  | 1.33 | 0.26 | 0.01 | 0.17 | 0.05 |  | 0.31 | 7.29 | 81.70 | 1,180 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +2.03 | +0.54 |  | +1.04 | +0.06 | -0.56 |  |  |  |  |  |  |  |  |  |  | +2.70 |  | -47.4% | +0.0% |  |  |
