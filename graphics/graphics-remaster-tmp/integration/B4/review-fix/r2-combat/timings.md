# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| combat_fires | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 803 | 686 |  | OK |
| combat_fires | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_shell_k05 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 833 | 708 |  | OK |
| combat_shell_k05 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_shell_k06 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 855 | 760 |  | OK |
| combat_shell_k06 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_shell_k08 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 852 | 766 |  | OK |
| combat_shell_k08 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_shell_k11 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 848 | 758 |  | OK |
| combat_shell_k11 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_shell_k16 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 825 | 730 |  | OK |
| combat_shell_k16 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k00 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 785 | 689 |  | OK |
| combat_eye_k00 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k05 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 836 | 711 |  | OK |
| combat_eye_k05 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k06 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 872 | 763 |  | OK |
| combat_eye_k06 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k08 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 875 | 767 |  | OK |
| combat_eye_k08 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k11 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 845 | 759 |  | OK |
| combat_eye_k11 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_flare_k90 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 788 | 690 |  | OK |
| combat_flare_k90 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_flare_k240 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 784 | 679 |  | OK |
| combat_flare_k240 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_muzzle_k04 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 156 | 77 |  | OK |
| combat_muzzle_k04 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_muzzle_k05 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 156 | 77 |  | OK |
| combat_muzzle_k05 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_muzzle_k06 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 162 | 83 |  | OK |
| combat_muzzle_k06 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_muzzle_k08 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 165 | 89 |  | OK |
| combat_muzzle_k08 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_day_k00 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 819 | 700 |  | OK |
| combat_day_k00 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_day_k05 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 843 | 722 |  | OK |
| combat_day_k05 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_day_k06 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 826 | 748 |  | OK |
| combat_day_k06 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| nc_sunrise_ridge | off | 6.06 | 4.21 | 0.14 | 1.69 | 2.02 | 0.49 | 0.14 |  | 1.47 | 0.52 |  | 0.16 | 0.08 | 0.34 | 0.09 | 4.59 | 54.60 | 1,912 | 8,897 | OK:30 | OK |
| nc_sunrise_ridge | on | 7.60 | 5.74 | 0.15 | 1.61 | 3.28 | 0.85 | 0.11 |  | 1.57 | 0.57 |  | 0.13 | 0.07 | 0.27 | 0.30 | 4.65 | 47.00 | 1,912 | 8,897 | OK:30 | OK |
| nc_sunrise_ridge | delta | +1.54 | +1.53 | +0.01 | -0.08 | +1.26 | +0.36 | -0.03 |  | +0.10 | +0.05 |  | -0.03 | -0.01 | -0.07 | +0.21 | +0.06 |  | +0.0% | +0.0% |  |  |
| nc_road_night | off | 12.28 | 11.04 |  | 1.71 | 6.98 | 2.35 |  |  | 1.06 | 0.53 |  | 0.13 | 0.06 |  | 0.03 | 8.78 | 57.80 | 1,410 | 914 | OK:30 | OK |
| nc_road_night | on | 16.56 | 14.88 |  | 1.60 | 8.23 | 5.05 |  |  | 1.51 | 0.27 |  | 0.14 | 0.06 |  | 0.03 | 7.29 | 52.90 | 1,410 | 914 | OK:30 | OK |
| nc_road_night | delta | +4.28 | +3.84 |  | -0.11 | +1.25 | +2.70 |  |  | +0.45 | -0.26 |  | +0.01 | +0.00 |  | +0.00 | -1.49 |  | +0.0% | +0.0% |  |  |
| combat_timing_r1 | off | 9.83 | 8.15 |  | 0.88 | 1.65 | 5.62 |  |  | 1.52 | 0.60 |  | 0.14 | 0.07 |  | 0.03 | 5.48 | 64.50 | 828 | 706 | UNSETTLED:300 | OK |
| combat_timing_r1 | on | 14.24 | 12.10 |  | 1.32 | 3.03 | 7.75 |  |  | 1.78 | 0.52 | 0.02 | 0.13 | 0.07 |  | 0.04 | 8.45 | 62.90 | 771 | 667 | UNSETTLED:300 | OK |
| combat_timing_r1 | delta | +4.41 | +3.95 |  | +0.44 | +1.38 | +2.13 |  |  | +0.26 | -0.08 |  | -0.01 | +0.00 |  | +0.01 | +2.97 |  | -6.9% | -5.5% |  |  |
| combat_timing_r2 | off | 16.66 | 14.83 |  | 1.49 | 3.94 | 9.40 |  |  | 1.69 | 0.32 |  | 0.14 | 0.06 |  | 0.03 | 5.43 | 63.20 | 761 | 665 | UNSETTLED:300 | OK |
| combat_timing_r2 | on | 12.17 | 10.39 |  | 1.41 | 3.00 | 5.98 |  |  | 1.54 | 0.38 | 0.01 | 0.13 | 0.07 |  | 0.04 | 8.19 | 78.10 | 766 | 666 | UNSETTLED:300 | OK |
| combat_timing_r2 | delta | -4.49 | -4.44 |  | -0.08 | -0.94 | -3.42 |  |  | -0.15 | +0.06 |  | -0.01 | +0.01 |  | +0.01 | +2.76 |  | +0.7% | +0.2% |  |  |
| combat_timing_r3 | off | 9.74 | 8.04 |  | 1.10 | 1.86 | 5.08 |  |  | 1.55 | 0.34 |  | 0.14 | 0.06 |  | 0.03 | 7.14 | 83.60 | 760 | 659 | UNSETTLED:300 | OK |
| combat_timing_r3 | on | 20.71 | 17.15 |  | 1.74 | 3.47 | 11.94 |  |  | 2.05 | 0.25 | 0.01 | 0.13 | 0.07 |  | 0.04 | 17.91 | 46.60 | 761 | 665 | UNSETTLED:300 | OK |
| combat_timing_r3 | delta | +10.97 | +9.11 |  | +0.64 | +1.61 | +6.86 |  |  | +0.50 | -0.09 |  | -0.01 | +0.01 |  | +0.01 | +10.77 |  | +0.1% | +0.9% |  |  |
| combat_flashtiming_r1 | off | 23.30 | 21.08 |  | 2.80 | 3.98 | 14.30 |  |  | 2.05 | 0.13 |  | 0.13 | 0.06 |  | 0.03 | 22.41 | 42.70 | 820 | 721 |  | OK |
| combat_flashtiming_r1 | on | 22.39 | 18.52 |  | 1.94 | 3.75 | 12.84 |  |  | 2.08 | 0.13 | 0.01 | 0.13 | 0.06 |  | 0.05 | 16.40 | 44.20 | 820 | 721 | UNSETTLED:300 | OK |
| combat_flashtiming_r1 | delta | -0.91 | -2.56 |  | -0.86 | -0.23 | -1.46 |  |  | +0.03 | +0.00 |  | +0.00 | +0.00 |  | +0.02 | -6.01 |  | +0.0% | +0.0% |  |  |
| combat_flashtiming_r2 | off | 17.82 | 15.97 |  | 1.68 | 3.06 | 11.22 |  |  | 1.68 | 0.19 |  | 0.13 | 0.06 |  | 0.03 | 13.49 | 47.50 | 820 | 721 |  | OK |
| combat_flashtiming_r2 | on | 22.30 | 18.59 |  | 1.99 | 3.79 | 12.81 |  |  | 2.06 | 0.13 | 0.01 | 0.13 | 0.06 |  | 0.05 | 16.44 | 45.90 | 820 | 721 | UNSETTLED:300 | OK |
| combat_flashtiming_r2 | delta | +4.48 | +2.62 |  | +0.31 | +0.73 | +1.59 |  |  | +0.38 | -0.06 |  | +0.00 | +0.00 |  | +0.02 | +2.95 |  | +0.0% | +0.0% |  |  |
| combat_flashtiming_r3 | off | 22.61 | 20.47 |  | 2.38 | 3.81 | 14.27 |  |  | 1.97 | 0.13 |  | 0.13 | 0.06 |  | 0.03 | 20.22 | 44.10 | 820 | 721 |  | OK |
| combat_flashtiming_r3 | on | 22.75 | 18.75 |  | 1.98 | 3.87 | 12.90 |  |  | 2.19 | 0.13 | 0.01 | 0.13 | 0.06 |  | 0.05 | 21.32 | 43.50 | 820 | 721 | UNSETTLED:300 | OK |
| combat_flashtiming_r3 | delta | +0.14 | -1.72 |  | -0.40 | +0.06 | -1.37 |  |  | +0.22 | +0.00 |  | +0.00 | +0.00 |  | +0.02 | +1.10 |  | +0.0% | +0.0% |  |  |
