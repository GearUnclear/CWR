# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.18 | 4.02 |  | 0.13 | 1.69 | 2.19 |  |  |  |  |  |  |  |  |  |  | 3.37 | 141.80 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | 8.84 | 7.01 | 0.14 | 1.28 | 3.56 | 2.16 | 0.12 |  |  | 1.55 | 0.32 | 0.01 | 0.18 | 0.07 |  | 0.12 | 8.47 | 90.00 | 1,090 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | +4.66 | +2.99 |  | +1.15 | +1.87 | -0.03 |  |  |  |  |  |  |  |  |  |  | +5.10 |  | -47.9% | -10.4% |  |  |
| sunrise_ridge | classic | 26.10 | 25.95 |  | 7.82 | 15.18 | 2.94 |  |  |  |  |  |  |  |  |  |  | 25.51 | 34.40 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | 9.15 | 7.55 | 0.11 | 5.08 | 2.11 | 0.37 | 0.16 |  |  | 1.22 | 0.27 | 0.01 | 0.19 | 0.06 |  | 0.13 | 4.90 | 68.10 | 1,659 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | -16.95 | -18.40 |  | -2.74 | -13.07 | -2.57 |  |  |  |  |  |  |  |  |  |  | -20.61 |  | -87.3% | -13.7% |  |  |
| noon_fields | classic | 8.88 | 8.75 |  | 0.11 | 6.46 | 2.18 |  |  |  |  |  |  |  |  |  |  | 8.71 | 67.90 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | 9.58 | 8.19 | 0.10 | 1.58 | 6.13 | 0.47 | 0.10 |  |  | 1.12 | 0.28 | 0.01 | 0.16 | 0.05 |  | 0.11 | 9.38 | 68.60 | 1,447 | 3,920 | OK:30 | OK |
| noon_fields | delta | +0.70 | -0.56 |  | +1.47 | -0.33 | -1.71 |  |  |  |  |  |  |  |  |  |  | +0.67 |  | -78.6% | -13.0% |  |  |
| forest_edge_golden | classic | 5.42 | 5.24 |  | 0.10 | 2.92 | 2.21 |  |  |  |  |  |  |  |  |  |  | 4.54 | 122.00 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | 6.58 | 4.97 | 0.19 | 1.36 | 2.77 | 0.83 | 0.14 |  |  | 1.24 | 0.28 | 0.01 | 0.22 | 0.05 |  | 0.12 | 6.16 | 103.70 | 757 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | +1.16 | -0.27 |  | +1.26 | -0.15 | -1.38 |  |  |  |  |  |  |  |  |  |  | +1.62 |  | -69.7% | -9.8% |  |  |
| coastline_sunset | classic | 1.19 | 0.97 |  | 0.53 | 0.00 | 0.44 |  |  |  |  |  |  |  |  |  |  | 0.79 | 365.10 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 6.14 | 3.07 | 0.00 | 2.76 | 0.00 | 0.31 | 0.23 |  |  | 2.57 | 0.19 | 0.01 | 0.30 | 0.20 |  | 0.20 | 5.40 | 165.70 | 154 | 0 | OK:30 | OK |
| coastline_sunset | delta | +4.95 | +2.10 |  | +2.23 | +0.00 | -0.13 |  |  |  |  |  |  |  |  |  |  | +4.61 |  | -13.0% |  |  |  |
| town_night | classic | 6.64 | 6.41 |  | 0.10 | 1.38 | 4.93 |  |  |  |  |  |  |  |  |  |  | 6.18 | 114.60 | 1,298 | 1,120 | OK:245 | OK |
| town_night | remastered | 11.04 | 9.34 | 0.15 | 1.14 | 3.61 | 4.58 | 0.05 |  |  | 1.37 | 0.33 | 0.01 | 0.15 | 0.05 |  | 0.10 | 10.46 | 72.60 | 1,170 | 1,064 | OK:127 | OK |
| town_night | delta | +4.40 | +2.93 |  | +1.04 | +2.23 | -0.35 |  |  |  |  |  |  |  |  |  |  | +4.28 |  | -9.9% | -5.0% |  |  |
| firefight_1 | classic | 4.91 | 4.30 |  | 0.35 | 0.62 | 3.34 |  |  |  |  |  |  |  |  |  |  | 0.83 | 159.20 | 406 | 332 | OK:47 | OK |
| firefight_1 | remastered | 6.53 | 4.54 |  | 1.00 | 1.81 | 1.72 | 0.19 |  |  | 1.41 | 0.30 | 0.01 | 0.20 | 0.07 |  | 0.15 | 5.83 | 128.60 | 531 | 531 | UNSETTLED:300 | OK |
| firefight_1 | delta | +1.62 | +0.24 |  | +0.65 | +1.19 | -1.62 |  |  |  |  |  |  |  |  |  |  | +5.00 |  | +23.5% | +37.5% |  |  |
| firefight_2 | classic | 4.91 | 4.30 |  | 0.35 | 0.62 | 3.34 |  |  |  |  |  |  |  |  |  |  | 0.83 | 159.20 | 482 | 458 | OK:47 | OK |
| firefight_2 | remastered | 6.53 | 4.54 |  | 1.00 | 1.81 | 1.72 | 0.19 |  |  | 1.41 | 0.30 | 0.01 | 0.20 | 0.07 |  | 0.15 | 5.83 | 128.60 | 599 | 611 | UNSETTLED:300 | OK |
| firefight_2 | delta | +1.62 | +0.24 |  | +0.65 | +1.19 | -1.62 |  |  |  |  |  |  |  |  |  |  | +5.00 |  | +19.5% | +25.0% |  |  |
| firefight_3 | classic | 4.91 | 4.30 |  | 0.35 | 0.62 | 3.34 |  |  |  |  |  |  |  |  |  |  | 0.83 | 159.20 | 506 | 441 | OK:47 | OK |
| firefight_3 | remastered | 6.53 | 4.54 |  | 1.00 | 1.81 | 1.72 | 0.19 |  |  | 1.41 | 0.30 | 0.01 | 0.20 | 0.07 |  | 0.15 | 5.83 | 128.60 | 588 | 602 | UNSETTLED:300 | OK |
| firefight_3 | delta | +1.62 | +0.24 |  | +0.65 | +1.19 | -1.62 |  |  |  |  |  |  |  |  |  |  | +5.00 |  | +13.9% | +26.7% |  |  |
| tank_gunner_interior | classic | 5.32 | 5.04 |  | 0.09 | 2.12 | 2.83 |  |  |  |  |  |  |  |  |  |  | 4.46 | 124.20 | 2,532 | 1,219 | OK:30 | OK |
| tank_gunner_interior | remastered | 8.26 | 7.32 | 0.06 | 0.88 | 2.98 | 3.46 | 0.09 |  |  | 0.62 | 0.15 |  | 0.28 | 0.05 |  | 0.10 | 7.86 | 78.60 | 1,536 | 1,233 | OK:30 | OK |
| tank_gunner_interior | delta | +2.94 | +2.28 |  | +0.79 | +0.86 | +0.63 |  |  |  |  |  |  |  |  |  |  | +3.40 |  | -39.3% | +1.1% |  |  |
| tank_gunner_optics | classic | 3.81 | 3.52 |  | 0.12 | 1.68 | 1.72 |  |  |  |  |  |  |  |  |  |  | 3.31 | 159.20 | 1,721 | 743 | OK:30 | OK |
| tank_gunner_optics | remastered | 7.58 | 6.74 | 0.06 | 0.94 | 3.50 | 2.30 | 0.11 |  |  | 0.58 | 0.12 |  | 0.16 |  |  | 0.27 | 7.22 | 107.30 | 1,147 | 743 | OK:30 | OK |
| tank_gunner_optics | delta | +3.77 | +3.22 |  | +0.82 | +1.82 | +0.58 |  |  |  |  |  |  |  |  |  |  | +3.91 |  | -33.4% | +0.0% |  |  |
| overcast_broken | classic | 11.64 | 11.51 |  | 0.15 | 8.65 | 2.71 |  |  |  |  |  |  |  |  |  |  | 11.15 | 51.70 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | 8.88 | 7.38 | 0.16 | 2.23 | 4.64 | 0.51 | 0.10 |  |  | 1.23 | 0.28 | 0.01 | 0.15 | 0.06 |  | 0.17 | 8.40 | 64.90 | 1,863 | 5,083 | OK:30 | OK |
| overcast_broken | delta | -2.76 | -4.13 |  | +2.08 | -4.01 | -2.20 |  |  |  |  |  |  |  |  |  |  | -2.75 |  | -77.4% | -10.7% |  |  |
| overcast_heavy | classic | 11.66 | 11.52 |  | 0.12 | 8.68 | 2.72 |  |  |  |  |  |  |  |  |  |  | 11.13 | 52.30 | 7,957 | 5,692 | OK:30 | OK |
| overcast_heavy | remastered | 8.74 | 6.82 | 0.14 | 1.88 | 4.45 | 0.49 | 0.10 |  |  | 1.63 | 0.30 | 0.01 | 0.15 | 0.10 |  | 0.11 | 8.41 | 67.40 | 1,619 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | -2.92 | -4.70 |  | +1.76 | -4.23 | -2.23 |  |  |  |  |  |  |  |  |  |  | -2.72 |  | -79.7% | -10.7% |  |  |
| woods_pine2_evening | classic | 5.77 | 5.61 |  | 0.12 | 3.05 | 2.43 |  |  |  |  |  |  |  |  |  |  | 5.10 | 108.40 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.27 | 0.99 |  | 0.54 | 0.00 | 0.45 |  |  |  |  |  |  |  |  |  |  | 0.82 | 334.00 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_valley | classic | 21.74 | 21.60 |  | 3.92 | 14.98 | 2.70 |  |  |  |  |  |  |  |  |  |  | 17.50 | 37.40 | 11,553 | 7,966 | OK:30 | OK |
| dawn_valley | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| pine_rays | classic | 5.64 | 5.45 |  | 0.08 | 2.33 | 3.04 |  |  |  |  |  |  |  |  |  |  | 4.41 | 118.70 | 3,163 | 1,678 | OK:30 | OK |
| pine_rays | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_fog_night | classic | 6.26 | 6.07 |  | 0.09 | 1.64 | 4.34 |  |  |  |  |  |  |  |  |  |  | 5.11 | 129.80 | 1,264 | 995 | OK:54 | OK |
| town_fog_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night_rain | classic | 5.78 | 5.49 |  | 0.21 | 0.87 | 4.41 |  |  |  |  |  |  |  |  |  |  | 4.24 | 137.60 | 987 | 736 | UNSETTLED:300 | OK |
| town_night_rain | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 4.97 | 4.86 |  | 0.13 | 3.47 | 1.26 |  |  |  |  |  |  |  |  |  |  | 4.13 | 126.00 | 2,723 | 1,774 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 2.50 | 2.27 |  | 0.25 | 0.84 | 1.18 |  |  |  |  |  |  |  |  |  |  | 1.69 | 216.50 | 985 | 541 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 4.63 | 4.47 |  | 0.10 | 2.64 | 1.72 |  |  |  |  |  |  |  |  |  |  | 4.20 | 158.10 | 2,435 | 1,448 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| rain_day | classic | 4.29 | 4.09 |  | 0.12 | 2.23 | 1.74 |  |  |  |  |  |  |  |  |  |  | 2.83 | 149.00 | 1,769 | 800 | OK:30 | OK |
| rain_day | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | classic | 20.59 | 20.46 |  | 6.87 | 11.46 | 2.13 |  |  |  |  |  |  |  |  |  |  | 19.47 | 42.40 | 10,841 | 7,519 | OK:30 | OK |
| noon_vista | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 2.84 | 2.66 |  | 0.14 | 1.16 | 1.35 |  |  |  |  |  |  |  |  |  |  | 1.50 | 196.90 | 603 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 6.48 | 4.93 | 0.22 | 1.16 | 2.76 | 1.02 | 0.12 |  |  | 1.22 | 0.26 | 0.01 | 0.22 | 0.06 |  | 0.12 | 6.06 | 108.90 | 582 | 314 | UNSETTLED:300 | OK |
| sinai_lowsun_fp | delta | +3.64 | +2.27 |  | +1.02 | +1.60 | -0.33 |  |  |  |  |  |  |  |  |  |  | +4.56 |  | -3.5% | +0.0% |  |  |
| sinai_desert_noon | classic | 5.84 | 5.71 |  | 0.14 | 3.06 | 2.50 |  |  |  |  |  |  |  |  |  |  | 4.88 | 93.40 | 2,243 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 7.93 | 6.34 | 0.10 | 1.22 | 3.20 | 1.92 | 0.11 |  |  | 1.30 | 0.26 | 0.01 | 0.17 | 0.06 |  | 0.29 | 7.41 | 79.40 | 1,181 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +2.09 | +0.63 |  | +1.08 | +0.14 | -0.58 |  |  |  |  |  |  |  |  |  |  | +2.53 |  | -47.3% | +0.0% |  |  |
