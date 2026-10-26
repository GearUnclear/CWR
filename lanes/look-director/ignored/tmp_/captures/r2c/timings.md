# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.53 | 4.36 |  | 0.14 | 1.67 | 2.56 |  |  |  |  |  |  |  |  |  |  | 3.40 | 140.70 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | 8.64 | 6.80 | 0.14 | 1.35 | 3.36 | 2.10 | 0.12 |  |  | 1.56 | 0.30 | 0.01 | 0.18 | 0.08 |  | 0.12 | 8.30 | 92.60 | 1,090 | 1,179 | OK:30 | OK |
| lowsun_fp | delta | +4.11 | +2.44 |  | +1.21 | +1.69 | -0.46 |  |  |  |  |  |  |  |  |  |  | +4.90 |  | -47.9% | -10.4% |  |  |
| sunrise_ridge | classic | 26.06 | 25.91 |  | 7.93 | 15.11 | 2.87 |  |  |  |  |  |  |  |  |  |  | 25.22 | 34.70 | 13,076 | 8,897 | OK:30 | OK |
| sunrise_ridge | remastered | 9.17 | 7.43 | 0.10 | 5.03 | 2.07 | 0.33 | 0.16 |  |  | 1.36 | 0.26 | 0.01 | 0.19 | 0.06 |  | 0.11 | 4.94 | 68.90 | 1,659 | 7,680 | OK:30 | OK |
| sunrise_ridge | delta | -16.89 | -18.48 |  | -2.90 | -13.04 | -2.54 |  |  |  |  |  |  |  |  |  |  | -20.28 |  | -87.3% | -13.7% |  |  |
| noon_fields | classic | 9.29 | 9.16 |  | 0.11 | 6.71 | 2.34 |  |  |  |  |  |  |  |  |  |  | 8.89 | 66.20 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | 9.73 | 8.25 | 0.10 | 1.67 | 6.11 | 0.48 | 0.11 |  |  | 1.21 | 0.28 | 0.01 | 0.16 | 0.05 |  | 0.11 | 9.42 | 69.40 | 1,447 | 3,920 | OK:30 | OK |
| noon_fields | delta | +0.44 | -0.91 |  | +1.56 | -0.60 | -1.86 |  |  |  |  |  |  |  |  |  |  | +0.53 |  | -78.6% | -13.0% |  |  |
| forest_edge_golden | classic | 5.18 | 5.00 |  | 0.10 | 2.78 | 2.13 |  |  |  |  |  |  |  |  |  |  | 4.54 | 122.30 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | 6.56 | 4.93 | 0.19 | 1.40 | 2.72 | 0.81 | 0.14 |  |  | 1.26 | 0.27 | 0.01 | 0.22 | 0.05 |  | 0.12 | 6.18 | 104.10 | 757 | 1,316 | OK:30 | OK |
| forest_edge_golden | delta | +1.38 | -0.07 |  | +1.30 | -0.06 | -1.32 |  |  |  |  |  |  |  |  |  |  | +1.64 |  | -69.7% | -9.8% |  |  |
| coastline_sunset | classic | 1.29 | 1.07 |  | 0.57 | 0.00 | 0.49 |  |  |  |  |  |  |  |  |  |  | 0.86 | 352.80 | 177 | 0 | OK:30 | OK |
| coastline_sunset | remastered | 3.34 | 1.61 | 0.00 | 1.40 | 0.00 | 0.21 | 0.13 |  |  | 1.47 | 0.11 | 0.01 | 0.17 | 0.06 |  | 0.12 | 3.13 | 240.10 | 154 | 0 | OK:30 | OK |
| coastline_sunset | delta | +2.05 | +0.54 |  | +0.83 | +0.00 | -0.28 |  |  |  |  |  |  |  |  |  |  | +2.27 |  | -13.0% |  |  |  |
| town_night | classic | 6.54 | 6.32 |  | 0.10 | 1.49 | 4.73 |  |  |  |  |  |  |  |  |  |  | 5.96 | 116.80 | 1,296 | 1,117 | OK:221 | OK |
| town_night | remastered | 10.98 | 9.23 | 0.15 | 1.12 | 3.62 | 4.49 | 0.05 |  |  | 1.41 | 0.34 | 0.01 | 0.15 | 0.05 |  | 0.10 | 10.55 | 73.40 | 1,177 | 1,058 | OK:30 | OK |
| town_night | delta | +4.44 | +2.91 |  | +1.02 | +2.13 | -0.24 |  |  |  |  |  |  |  |  |  |  | +4.59 |  | -9.2% | -5.3% |  |  |
| firefight_1 | classic | 4.68 | 4.15 |  | 0.23 | 0.66 | 3.26 |  |  |  |  |  |  |  |  |  |  | 0.87 | 194.30 | 467 | 395 | OK:46 | OK |
| firefight_1 | remastered | 6.29 | 4.20 |  | 0.96 | 1.67 | 1.58 | 0.20 |  |  | 1.47 | 0.28 | 0.01 | 0.20 | 0.07 |  | 0.17 | 5.75 | 126.80 | 490 | 462 | UNSETTLED:300 | OK |
| firefight_1 | delta | +1.61 | +0.05 |  | +0.73 | +1.01 | -1.68 |  |  |  |  |  |  |  |  |  |  | +4.88 |  | +4.7% | +14.5% |  |  |
| firefight_2 | classic | 4.68 | 4.15 |  | 0.23 | 0.66 | 3.26 |  |  |  |  |  |  |  |  |  |  | 0.87 | 194.30 | 544 | 491 | OK:46 | OK |
| firefight_2 | remastered | 6.29 | 4.20 |  | 0.96 | 1.67 | 1.58 | 0.20 |  |  | 1.47 | 0.28 | 0.01 | 0.20 | 0.07 |  | 0.17 | 5.75 | 126.80 | 484 | 480 | UNSETTLED:300 | OK |
| firefight_2 | delta | +1.61 | +0.05 |  | +0.73 | +1.01 | -1.68 |  |  |  |  |  |  |  |  |  |  | +4.88 |  | -11.0% | -2.2% |  |  |
| firefight_3 | classic | 4.68 | 4.15 |  | 0.23 | 0.66 | 3.26 |  |  |  |  |  |  |  |  |  |  | 0.87 | 194.30 | 501 | 425 | OK:46 | OK |
| firefight_3 | remastered | 6.29 | 4.20 |  | 0.96 | 1.67 | 1.58 | 0.20 |  |  | 1.47 | 0.28 | 0.01 | 0.20 | 0.07 |  | 0.17 | 5.75 | 126.80 | 532 | 576 | UNSETTLED:300 | OK |
| firefight_3 | delta | +1.61 | +0.05 |  | +0.73 | +1.01 | -1.68 |  |  |  |  |  |  |  |  |  |  | +4.88 |  | +5.8% | +26.2% |  |  |
| tank_gunner_interior | classic | 5.27 | 4.99 |  | 0.10 | 2.12 | 2.77 |  |  |  |  |  |  |  |  |  |  | 4.21 | 126.60 | 2,530 | 1,216 | OK:30 | OK |
| tank_gunner_interior | remastered | 8.01 | 7.10 | 0.06 | 0.88 | 2.83 | 3.39 | 0.09 |  |  | 0.59 | 0.13 |  | 0.28 | 0.05 |  | 0.10 | 7.83 | 78.50 | 1,534 | 1,232 | OK:30 | OK |
| tank_gunner_interior | delta | +2.74 | +2.11 |  | +0.78 | +0.71 | +0.62 |  |  |  |  |  |  |  |  |  |  | +3.62 |  | -39.4% | +1.3% |  |  |
| tank_gunner_optics | classic | 4.18 | 3.87 |  | 0.14 | 1.89 | 1.84 |  |  |  |  |  |  |  |  |  |  | 2.98 | 152.40 | 1,724 | 743 | OK:30 | OK |
| tank_gunner_optics | remastered | 7.19 | 6.46 | 0.06 | 0.89 | 3.45 | 2.11 | 0.10 |  |  | 0.50 | 0.09 |  | 0.15 |  |  | 0.23 | 6.74 | 111.90 | 1,141 | 743 | OK:30 | OK |
| tank_gunner_optics | delta | +3.01 | +2.59 |  | +0.75 | +1.56 | +0.27 |  |  |  |  |  |  |  |  |  |  | +3.76 |  | -33.8% | +0.0% |  |  |
| overcast_broken | classic | 12.30 | 12.17 |  | 0.15 | 9.08 | 2.93 |  |  |  |  |  |  |  |  |  |  | 11.09 | 48.40 | 8,243 | 5,692 | OK:30 | OK |
| overcast_broken | remastered | 8.64 | 7.17 | 0.14 | 2.25 | 4.49 | 0.44 | 0.11 |  |  | 1.20 | 0.26 | 0.01 | 0.15 | 0.06 |  | 0.18 | 8.40 | 65.10 | 1,863 | 5,083 | OK:30 | OK |
| overcast_broken | delta | -3.66 | -5.00 |  | +2.10 | -4.59 | -2.49 |  |  |  |  |  |  |  |  |  |  | -2.69 |  | -77.4% | -10.7% |  |  |
| overcast_heavy | classic | 11.53 | 11.38 |  | 0.13 | 8.61 | 2.65 |  |  |  |  |  |  |  |  |  |  | 11.06 | 52.20 | 7,957 | 5,692 | OK:30 | OK |
| overcast_heavy | remastered | 8.53 | 6.70 | 0.15 | 1.86 | 4.37 | 0.48 | 0.11 |  |  | 1.55 | 0.27 | 0.01 | 0.15 | 0.07 |  | 0.11 | 8.29 | 69.90 | 1,619 | 5,083 | OK:30 | OK |
| overcast_heavy | delta | -3.00 | -4.68 |  | +1.73 | -4.24 | -2.17 |  |  |  |  |  |  |  |  |  |  | -2.77 |  | -79.7% | -10.7% |  |  |
| woods_pine2_evening | classic | 5.61 | 5.47 |  | 0.12 | 3.05 | 2.29 |  |  |  |  |  |  |  |  |  |  | 5.06 | 108.30 | 3,250 | 1,811 | OK:30 | OK |
| woods_pine2_evening | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| blue_hour_coast | classic | 1.32 | 1.05 |  | 0.62 | 0.00 | 0.42 |  |  |  |  |  |  |  |  |  |  | 0.79 | 326.20 | 182 | 0 | OK:30 | OK |
| blue_hour_coast | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_valley | classic | 21.23 | 21.09 |  | 3.33 | 15.18 | 2.58 |  |  |  |  |  |  |  |  |  |  | 17.21 | 37.20 | 11,553 | 7,966 | OK:30 | OK |
| dawn_valley | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| pine_rays | classic | 5.35 | 5.16 |  | 0.08 | 2.25 | 2.83 |  |  |  |  |  |  |  |  |  |  | 4.40 | 119.80 | 3,163 | 1,678 | OK:30 | OK |
| pine_rays | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_fog_night | classic | 6.18 | 5.99 |  | 0.09 | 1.54 | 4.36 |  |  |  |  |  |  |  |  |  |  | 5.03 | 130.90 | 1,282 | 1,033 | OK:64 | OK |
| town_fog_night | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_night_rain | classic | 5.71 | 5.51 |  | 0.09 | 1.32 | 4.10 |  |  |  |  |  |  |  |  |  |  | 4.70 | 147.20 | 1,072 | 837 | OK:44 | OK |
| town_night_rain | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| jeep_close_noon | classic | 4.85 | 4.74 |  | 0.11 | 3.24 | 1.39 |  |  |  |  |  |  |  |  |  |  | 4.13 | 123.00 | 2,723 | 1,774 | OK:30 | OK |
| jeep_close_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| town_noon | classic | 2.74 | 2.53 |  | 0.11 | 0.98 | 1.44 |  |  |  |  |  |  |  |  |  |  | 1.68 | 213.20 | 985 | 541 | OK:30 | OK |
| town_noon | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_morning | classic | 4.67 | 4.52 |  | 0.10 | 2.68 | 1.74 |  |  |  |  |  |  |  |  |  |  | 4.20 | 157.10 | 2,435 | 1,448 | OK:30 | OK |
| forest_morning | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| rain_day | classic | 4.16 | 3.98 |  | 0.11 | 2.12 | 1.75 |  |  |  |  |  |  |  |  |  |  | 2.87 | 145.90 | 1,750 | 782 | OK:30 | OK |
| rain_day | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_vista | classic | 20.62 | 20.49 |  | 6.85 | 11.47 | 2.18 |  |  |  |  |  |  |  |  |  |  | 19.35 | 42.30 | 10,841 | 7,519 | OK:30 | OK |
| noon_vista | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp | classic | 2.92 | 2.74 |  | 0.12 | 1.22 | 1.40 |  |  |  |  |  |  |  |  |  |  | 1.44 | 190.40 | 602 | 314 | OK:30 | OK |
| sinai_lowsun_fp | remastered | 6.29 | 4.64 | 0.22 | 1.09 | 2.56 | 0.99 | 0.12 |  |  | 1.33 | 0.32 | 0.01 | 0.21 | 0.06 |  | 0.11 | 5.85 | 110.60 | 416 | 314 | UNSETTLED:300 | OK |
| sinai_lowsun_fp | delta | +3.37 | +1.90 |  | +0.97 | +1.34 | -0.41 |  |  |  |  |  |  |  |  |  |  | +4.41 |  | -30.9% | +0.0% |  |  |
| sinai_desert_noon | classic | 5.77 | 5.63 |  | 0.15 | 3.00 | 2.48 |  |  |  |  |  |  |  |  |  |  | 4.69 | 94.30 | 2,242 | 1,100 | OK:30 | OK |
| sinai_desert_noon | remastered | 7.96 | 6.32 | 0.10 | 1.23 | 3.17 | 1.92 | 0.12 |  |  | 1.35 | 0.26 | 0.01 | 0.17 | 0.06 |  | 0.33 | 7.32 | 78.50 | 1,180 | 1,100 | OK:30 | OK |
| sinai_desert_noon | delta | +2.19 | +0.69 |  | +1.08 | +0.17 | -0.56 |  |  |  |  |  |  |  |  |  |  | +2.63 |  | -47.4% | +0.0% |  |  |
