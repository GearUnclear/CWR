# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 7.28 | 7.14 |  | 0.11 | 2.21 | 4.83 |  |  |  |  |  |  |  |  |  | 3.33 | 106.30 | 2,091 | 1,316 | OK:30 | OK |
| lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| lowsun_fp_alb | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| lowsun_fp_alb | remastered | 4.35 | 4.17 | 0.10 | 0.66 | 1.90 | 1.61 | 0.08 |  |  |  |  |  |  |  |  | 3.62 | 108.70 | 1,212 | 1,316 | OK:30 | OK |
| lowsun_box | classic | 7.39 | 7.25 |  | 0.12 | 4.67 | 2.46 |  |  |  |  |  |  |  |  |  | 4.66 | 97.40 | 2,812 | 1,640 | OK:30 | OK |
| lowsun_box | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| lowsun_box_alb | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| lowsun_box_alb | remastered | 7.05 | 6.87 | 0.09 | 0.85 | 5.39 | 0.63 | 0.08 |  |  |  |  |  |  |  |  | 5.01 | 106.90 | 1,173 | 1,640 | UNSETTLED:300 | OK |
| dawn_box | classic | 7.76 | 7.64 |  | 0.12 | 3.98 | 3.53 |  |  |  |  |  |  |  |  |  | 4.73 | 104.90 | 2,921 | 1,650 | OK:30 | OK |
| dawn_box | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_box_alb | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_box_alb | remastered | 7.88 | 7.70 | 0.09 | 0.87 | 5.88 | 0.96 | 0.08 |  |  |  |  |  |  |  |  | 5.00 | 96.80 | 1,221 | 1,650 | UNSETTLED:300 | OK |
| dawn_box5 | classic | 7.24 | 7.11 |  | 0.12 | 3.12 | 3.87 |  |  |  |  |  |  |  |  |  | 4.93 | 102.40 | 2,991 | 1,671 | OK:30 | OK |
| dawn_box5 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_box5_alb | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| dawn_box5_alb | remastered | 6.07 | 5.89 | 0.10 | 0.74 | 4.35 | 0.80 | 0.08 |  |  |  |  |  |  |  |  | 4.93 | 97.30 | 1,231 | 1,671 | UNSETTLED:300 | OK |
| forest_edge_golden | classic | 6.19 | 6.03 |  | 0.09 | 2.79 | 3.15 |  |  |  |  |  |  |  |  |  | 4.35 | 102.90 | 2,502 | 1,459 | OK:30 | OK |
| forest_edge_golden | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden_alb | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| forest_edge_golden_alb | remastered | 8.38 | 8.16 | 0.33 | 1.26 | 3.78 | 3.12 | 0.10 |  |  |  |  |  |  |  |  | 3.44 | 83.50 | 866 | 1,459 | OK:30 | OK |
| noon_fields | classic | 15.42 | 15.29 |  | 0.11 | 10.94 | 4.23 |  |  |  |  |  |  |  |  |  | 8.86 | 35.90 | 6,767 | 4,508 | OK:30 | OK |
| noon_fields | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields_alb | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| noon_fields_alb | remastered | 5.61 | 5.43 | 0.08 | 0.77 | 4.20 | 0.45 | 0.09 |  |  |  |  |  |  |  |  | 5.01 | 79.70 | 1,566 | 4,508 | OK:30 | OK |
| sinai_lowsun_fp | classic | 3.78 | 3.63 |  | 0.10 | 1.56 | 1.97 |  |  |  |  |  |  |  |  |  | 1.91 | 123.50 | 788 | 371 | OK:30 | OK |
| sinai_lowsun_fp | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp_alb | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_lowsun_fp_alb | remastered | 3.61 | 3.42 | 0.23 | 1.07 | 0.44 | 1.91 | 0.08 |  |  |  |  |  |  |  |  | 2.06 | 129.40 | 496 | 371 | OK:30 | OK |
| sinai_box | classic | 10.48 | 10.33 |  | 0.10 | 5.65 | 4.59 |  |  |  |  |  |  |  |  |  | 7.59 | 59.50 | 4,293 | 710 | OK:30 | OK |
| sinai_box | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_box_alb | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_box_alb | remastered | 4.19 | 4.00 | 0.09 | 0.98 | 0.40 | 2.62 | 0.08 |  |  |  |  |  |  |  |  | 3.07 | 100.20 | 854 | 710 | OK:30 | OK |
| sinai_dusk_box | classic | 10.60 | 10.46 |  | 0.09 | 5.88 | 4.49 |  |  |  |  |  |  |  |  |  | 7.64 | 57.70 | 4,366 | 715 | OK:30 | OK |
| sinai_dusk_box | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_dusk_box_alb | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| sinai_dusk_box_alb | remastered | 4.35 | 4.16 | 0.09 | 1.20 | 0.41 | 2.56 | 0.08 |  |  |  |  |  |  |  |  | 3.87 | 98.50 | 858 | 715 | OK:30 | OK |
