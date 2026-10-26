# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 4.49 | 4.36 |  | 0.11 | 2.05 | 2.19 |  |  |  |  |  |  |  |  |  | 3.31 | 141.90 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| lowsun_fp_alb | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| lowsun_fp_alb | remastered | 3.96 | 3.78 | 0.07 | 0.66 | 1.68 | 1.43 | 0.08 |  |  |  |  |  |  |  |  | 3.64 | 133.20 | 1,212 | 1,316 | OK:30 | OK |
| lowsun_box | classic | 5.01 | 4.88 |  | 0.12 | 2.97 | 1.79 |  |  |  |  |  |  |  |  |  | 4.67 | 109.00 | 2,812 | 1,640 | OK:30 | OK |
| lowsun_box | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| lowsun_box_alb | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| lowsun_box_alb | remastered | 6.12 | 5.91 | 0.10 | 0.78 | 4.63 | 0.50 | 0.10 |  |  |  |  |  |  |  |  | 5.74 | 104.20 | 1,173 | 1,640 | UNSETTLED:300 | OK |
| dawn_box | classic | 6.16 | 6.01 |  | 0.15 | 3.57 | 2.28 |  |  |  |  |  |  |  |  |  | 5.25 | 110.50 | 2,921 | 1,650 | OK:30 | OK |
| dawn_box | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_box_alb | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_box_alb | remastered | 6.25 | 6.05 | 0.12 | 0.77 | 4.72 | 0.56 | 0.10 |  |  |  |  |  |  |  |  | 5.63 | 103.70 | 1,221 | 1,650 | UNSETTLED:300 | OK |
| dawn_box5 | classic | 7.73 | 7.58 |  | 0.14 | 4.90 | 2.53 |  |  |  |  |  |  |  |  |  | 5.14 | 106.00 | 2,991 | 1,671 | OK:30 | OK |
| dawn_box5 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_box5_alb | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_box5_alb | remastered | 6.24 | 6.04 | 0.10 | 0.81 | 4.57 | 0.66 | 0.09 |  |  |  |  |  |  |  |  | 5.61 | 101.40 | 1,231 | 1,671 | UNSETTLED:300 | OK |
| forest_edge_golden | classic | 6.39 | 6.18 |  | 0.12 | 3.55 | 2.50 |  |  |  |  |  |  |  |  |  | 5.75 | 101.20 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden_alb | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden_alb | remastered | 5.77 | 5.42 | 0.32 | 1.30 | 2.77 | 1.36 | 0.15 |  |  |  |  |  |  |  |  | 4.21 | 102.70 | 866 | 1,459 | OK:30 | OK |
| noon_fields | classic | 8.82 | 8.69 |  | 0.11 | 6.34 | 2.25 |  |  |  |  |  |  |  |  |  | 8.64 | 68.90 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields_alb | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields_alb | remastered | 5.81 | 5.60 | 0.08 | 0.72 | 4.48 | 0.41 | 0.10 |  |  |  |  |  |  |  |  | 4.97 | 85.80 | 1,566 | 4,508 | OK:30 | OK |
| sinai_lowsun_fp | classic | 3.49 | 3.23 |  | 0.27 | 1.08 | 1.88 |  |  |  |  |  |  |  |  |  | 2.82 | 135.70 | 789 | 371 | OK:30 | OK |
| sinai_lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp_alb | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp_alb | remastered | 4.30 | 3.82 | 0.26 | 1.81 | 0.75 | 1.27 | 0.18 |  |  |  |  |  |  |  |  | 2.59 | 122.80 | 497 | 371 | OK:30 | OK |
| sinai_box | classic | 9.31 | 9.07 |  | 0.19 | 5.44 | 3.44 |  |  |  |  |  |  |  |  |  | 8.35 | 63.40 | 4,293 | 710 | OK:30 | OK |
| sinai_box | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_box_alb | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_box_alb | remastered | 3.58 | 3.16 | 0.15 | 1.51 | 0.66 | 0.98 | 0.20 |  |  |  |  |  |  |  |  | 3.17 | 105.70 | 854 | 710 | OK:30 | OK |
| sinai_dusk_box | classic | 9.77 | 9.41 |  | 0.36 | 5.34 | 3.72 |  |  |  |  |  |  |  |  |  | 8.24 | 61.70 | 4,367 | 715 | OK:30 | OK |
| sinai_dusk_box | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_dusk_box_alb | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_dusk_box_alb | remastered | 3.61 | 3.19 | 0.14 | 1.55 | 0.64 | 1.00 | 0.19 |  |  |  |  |  |  |  |  | 3.23 | 103.90 | 859 | 715 | OK:30 | OK |
