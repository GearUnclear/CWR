# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| combat_fires | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 784 | 687 |  | OK |
| combat_fires | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_shell_k05 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 800 | 708 |  | OK |
| combat_shell_k05 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_shell_k06 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 829 | 740 |  | OK |
| combat_shell_k06 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_shell_k08 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 825 | 740 |  | OK |
| combat_shell_k08 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_shell_k11 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 820 | 752 |  | OK |
| combat_shell_k11 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_shell_k16 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 793 | 711 |  | OK |
| combat_shell_k16 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k00 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 809 | 693 |  | OK |
| combat_eye_k00 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k05 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 829 | 718 |  | OK |
| combat_eye_k05 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k06 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 859 | 767 |  | OK |
| combat_eye_k06 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k08 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 858 | 765 |  | OK |
| combat_eye_k08 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k11 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 860 | 765 |  | OK |
| combat_eye_k11 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_flare_k90 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 799 | 689 |  | OK |
| combat_flare_k90 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_flare_k240 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 794 | 680 |  | OK |
| combat_flare_k240 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| nc_sunrise_ridge | off | 9.21 | 7.32 | 0.15 | 2.21 | 3.82 | 1.30 | 0.11 |  | 1.62 | 0.46 |  | 0.13 | 0.07 | 0.27 | 0.46 | 4.45 | 49.20 | 1,912 | 8,897 | OK:30 | OK |
| nc_sunrise_ridge | on | 7.74 | 5.97 | 0.08 | 1.89 | 3.07 | 1.01 | 0.11 |  | 1.50 | 0.43 |  | 0.13 | 0.07 | 0.27 | 0.37 | 4.51 | 47.80 | 1,912 | 8,897 | OK:30 | OK |
| nc_sunrise_ridge | delta | -1.47 | -1.35 | -0.07 | -0.32 | -0.75 | -0.29 | +0.00 |  | -0.12 | -0.03 |  | +0.00 | +0.00 | +0.00 | -0.09 | +0.06 |  | +0.0% | +0.0% |  |  |
| nc_road_night | off | 9.56 | 8.29 |  | 1.09 | 4.29 | 2.91 |  |  | 1.10 | 0.43 |  | 0.13 | 0.06 |  | 0.03 | 6.36 | 79.60 | 1,410 | 914 | OK:30 | OK |
| nc_road_night | on | 8.18 | 6.98 |  | 1.01 | 3.64 | 2.33 |  |  | 1.02 | 0.42 |  | 0.14 | 0.06 |  | 0.03 | 6.32 | 77.80 | 1,410 | 914 | OK:30 | OK |
| nc_road_night | delta | -1.38 | -1.31 |  | -0.08 | -0.65 | -0.58 |  |  | -0.08 | -0.01 |  | +0.01 | +0.00 |  | +0.00 | -0.04 |  | +0.0% | +0.0% |  |  |
| combat_timing_r1 | off | 6.92 | 5.58 |  | 0.66 | 1.11 | 3.81 |  |  | 1.19 | 0.60 |  | 0.14 | 0.06 |  | 0.03 | 5.13 | 112.30 | 766 | 678 | UNSETTLED:300 | OK |
| combat_timing_r1 | on | 7.21 | 5.92 |  | 0.81 | 1.28 | 3.82 |  |  | 1.15 | 0.56 |  | 0.14 | 0.06 |  | 0.03 | 4.93 | 112.80 | 764 | 663 | UNSETTLED:300 | OK |
| combat_timing_r1 | delta | +0.29 | +0.34 |  | +0.15 | +0.17 | +0.01 |  |  | -0.04 | -0.04 |  | +0.00 | +0.00 |  | +0.00 | -0.20 |  | -0.3% | -2.2% |  |  |
| combat_timing_r2 | off | 6.05 | 4.72 |  | 0.63 | 1.04 | 3.05 |  |  | 1.18 | 0.67 |  | 0.15 | 0.07 |  | 0.04 | 5.07 | 122.20 | 739 | 646 | UNSETTLED:300 | OK |
| combat_timing_r2 | on | 5.70 | 4.36 |  | 0.58 | 0.96 | 2.81 |  |  | 1.17 | 0.58 |  | 0.14 | 0.06 |  | 0.03 | 4.92 | 136.30 | 769 | 665 | UNSETTLED:300 | OK |
| combat_timing_r2 | delta | -0.35 | -0.36 |  | -0.05 | -0.08 | -0.24 |  |  | -0.01 | -0.09 |  | -0.01 | -0.01 |  | -0.01 | -0.15 |  | +3.9% | +2.9% |  |  |
| combat_timing_r3 | off | 5.97 | 4.67 |  | 0.55 | 0.92 | 3.20 |  |  | 1.15 | 0.57 |  | 0.14 | 0.06 |  | 0.03 | 4.96 | 118.10 | 749 | 651 | UNSETTLED:300 | OK |
| combat_timing_r3 | on | 6.41 | 4.97 |  | 0.60 | 0.96 | 3.42 |  |  | 1.29 | 0.81 |  | 0.14 | 0.06 |  | 0.03 | 4.86 | 77.30 | 756 | 660 | UNSETTLED:300 | OK |
| combat_timing_r3 | delta | +0.44 | +0.30 |  | +0.05 | +0.04 | +0.22 |  |  | +0.14 | +0.24 |  | +0.00 | +0.00 |  | +0.00 | -0.10 |  | +0.9% | +1.4% |  |  |
