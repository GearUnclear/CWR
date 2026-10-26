# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| combat_fires | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 803 | 690 |  | OK |
| combat_fires | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_shell_k05 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 826 | 714 |  | OK |
| combat_shell_k05 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_shell_k06 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 860 | 761 |  | OK |
| combat_shell_k06 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_shell_k08 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 869 | 765 |  | OK |
| combat_shell_k08 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_shell_k11 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 844 | 763 |  | OK |
| combat_shell_k11 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_shell_k16 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 814 | 732 |  | OK |
| combat_shell_k16 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k00 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 800 | 691 |  | OK |
| combat_eye_k00 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k05 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 818 | 714 |  | OK |
| combat_eye_k05 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k06 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 841 | 747 |  | OK |
| combat_eye_k06 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k08 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 845 | 749 |  | OK |
| combat_eye_k08 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k11 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 866 | 770 |  | OK |
| combat_eye_k11 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_flare_k90 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 790 | 699 |  | OK |
| combat_flare_k90 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_flare_k240 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 793 | 684 |  | OK |
| combat_flare_k240 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_muzzle_k04 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 156 | 77 |  | OK |
| combat_muzzle_k04 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_muzzle_k05 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 156 | 77 |  | OK |
| combat_muzzle_k05 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_muzzle_k06 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 162 | 83 |  | OK |
| combat_muzzle_k06 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_muzzle_k08 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 165 | 89 |  | OK |
| combat_muzzle_k08 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_day_k00 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 826 | 688 |  | OK |
| combat_day_k00 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_day_k05 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 834 | 711 |  | OK |
| combat_day_k05 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_day_k06 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 857 | 758 |  | OK |
| combat_day_k06 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| nc_sunrise_ridge | off | 11.60 | 9.06 | 0.11 | 5.67 | 2.81 | 0.58 | 0.12 |  |  | 2.23 | 0.60 | 0.02 | 0.13 | 0.08 |  | 0.03 | 5.52 | 55.10 | 1,912 | 8,897 | OK:30 | OK |
| nc_sunrise_ridge | on | 9.86 | 8.00 | 0.09 | 5.33 | 2.31 | 0.37 | 0.12 |  |  | 1.55 | 0.44 | 0.01 | 0.13 | 0.06 |  | 0.03 | 5.00 | 59.90 | 1,912 | 8,897 | OK:30 | OK |
| nc_sunrise_ridge | delta | -1.74 | -1.06 | -0.02 | -0.34 | -0.50 | -0.21 | +0.00 |  |  | -0.68 | -0.16 | -0.01 | +0.00 | -0.02 |  | +0.00 | -0.52 |  | +0.0% | +0.0% |  |  |
| nc_road_night | off | 9.52 | 7.41 |  | 1.06 | 3.69 | 2.66 | 0.06 |  |  | 1.93 | 0.40 | 0.01 | 0.13 | 0.06 |  | 0.03 | 7.87 | 68.40 | 1,411 | 914 | OK:30 | OK |
| nc_road_night | on | 8.42 | 6.83 |  | 1.07 | 3.54 | 2.21 | 0.06 |  |  | 1.43 | 0.35 | 0.01 | 0.13 | 0.06 |  | 0.03 | 7.68 | 77.50 | 1,411 | 914 | OK:30 | OK |
| nc_road_night | delta | -1.10 | -0.58 |  | +0.01 | -0.15 | -0.45 | +0.00 |  |  | -0.50 | -0.05 | +0.00 | +0.00 | +0.00 |  | +0.00 | -0.19 |  | +0.0% | +0.0% |  |  |
| combat_timing_r1 | off | 6.61 | 5.29 |  | 0.74 | 1.11 | 3.44 | 0.04 |  |  | 1.15 | 0.56 | 0.01 | 0.15 | 0.07 |  | 0.04 | 5.87 | 126.50 | 795 | 694 | UNSETTLED:300 | OK |
| combat_timing_r1 | on | 6.42 | 5.22 |  | 0.69 | 1.04 | 3.49 | 0.04 |  |  | 1.03 | 0.44 | 0.01 | 0.15 | 0.07 |  | 0.04 | 5.88 | 126.60 | 794 | 692 | UNSETTLED:300 | OK |
| combat_timing_r1 | delta | -0.19 | -0.07 |  | -0.05 | -0.07 | +0.05 | +0.00 |  |  | -0.12 | -0.12 | +0.00 | +0.00 | +0.00 |  | +0.00 | +0.01 |  | -0.1% | -0.3% |  |  |
| combat_timing_r2 | off | 6.41 | 5.17 |  | 0.75 | 1.08 | 3.34 | 0.04 |  |  | 1.04 | 0.44 | 0.01 | 0.15 | 0.07 |  | 0.04 | 5.88 | 130.10 | 785 | 672 | UNSETTLED:300 | OK |
| combat_timing_r2 | on | 6.26 | 5.08 |  | 0.71 | 1.06 | 3.31 | 0.04 |  |  | 1.02 | 0.39 | 0.01 | 0.15 | 0.07 |  | 0.04 | 5.83 | 130.80 | 783 | 672 | UNSETTLED:300 | OK |
| combat_timing_r2 | delta | -0.15 | -0.09 |  | -0.04 | -0.02 | -0.03 | +0.00 |  |  | -0.02 | -0.05 | +0.00 | +0.00 | +0.00 |  | +0.00 | -0.05 |  | -0.3% | +0.0% |  |  |
| combat_timing_r3 | off | 6.68 | 5.32 |  | 0.76 | 1.11 | 3.45 | 0.04 |  |  | 1.19 | 0.56 | 0.01 | 0.15 | 0.07 |  | 0.04 | 5.94 | 114.20 | 753 | 660 | UNSETTLED:300 | OK |
| combat_timing_r3 | on | 6.58 | 5.21 |  | 0.68 | 1.06 | 3.47 | 0.04 |  |  | 1.21 | 0.62 | 0.01 | 0.15 | 0.07 |  | 0.04 | 5.85 | 115.60 | 766 | 658 | UNSETTLED:300 | OK |
| combat_timing_r3 | delta | -0.10 | -0.11 |  | -0.08 | -0.05 | +0.02 | +0.00 |  |  | +0.02 | +0.06 | +0.00 | +0.00 | +0.00 |  | +0.00 | -0.09 |  | +1.7% | -0.3% |  |  |
| combat_flashtiming_r1 | off | 6.60 | 5.49 |  | 0.55 | 1.04 | 3.90 | 0.06 |  |  | 0.86 | 0.31 | 0.01 | 0.14 | 0.07 |  | 0.03 | 5.74 | 116.10 | 827 | 715 |  | OK |
| combat_flashtiming_r1 | on | 6.49 | 5.43 |  | 0.56 | 1.11 | 3.76 | 0.06 |  |  | 0.86 | 0.29 | 0.01 | 0.14 | 0.07 |  | 0.03 | 5.90 | 115.70 | 827 | 715 | UNSETTLED:300 | OK |
| combat_flashtiming_r1 | delta | -0.11 | -0.06 |  | +0.01 | +0.07 | -0.14 | +0.00 |  |  | +0.00 | -0.02 | +0.00 | +0.00 | +0.00 |  | +0.00 | +0.16 |  | +0.0% | +0.0% |  |  |
| combat_flashtiming_r2 | off | 6.43 | 5.38 |  | 0.60 | 1.14 | 3.64 | 0.06 |  |  | 0.85 | 0.26 | 0.01 | 0.14 | 0.07 |  | 0.03 | 5.73 | 121.40 | 827 | 715 |  | OK |
| combat_flashtiming_r2 | on | 6.38 | 5.36 |  | 0.57 | 1.10 | 3.69 | 0.06 |  |  | 0.81 | 0.25 | 0.01 | 0.14 | 0.07 |  | 0.03 | 5.82 | 122.50 | 827 | 715 | UNSETTLED:300 | OK |
| combat_flashtiming_r2 | delta | -0.05 | -0.02 |  | -0.03 | -0.04 | +0.05 | +0.00 |  |  | -0.04 | -0.01 | +0.00 | +0.00 | +0.00 |  | +0.00 | +0.09 |  | +0.0% | +0.0% |  |  |
| combat_flashtiming_r3 | off | 6.42 | 5.33 |  | 0.57 | 1.08 | 3.68 | 0.06 |  |  | 0.89 | 0.28 | 0.01 | 0.15 | 0.07 |  | 0.03 | 5.90 | 120.60 | 827 | 715 |  | OK |
| combat_flashtiming_r3 | on | 6.52 | 5.43 |  | 0.56 | 1.02 | 3.85 | 0.06 |  |  | 0.88 | 0.29 | 0.01 | 0.14 | 0.07 |  | 0.03 | 5.85 | 120.80 | 827 | 715 | UNSETTLED:300 | OK |
| combat_flashtiming_r3 | delta | +0.10 | +0.10 |  | -0.01 | -0.06 | +0.17 | +0.00 |  |  | -0.01 | +0.01 | +0.00 | -0.01 | +0.00 |  | +0.00 | -0.05 |  | +0.0% | +0.0% |  |  |
