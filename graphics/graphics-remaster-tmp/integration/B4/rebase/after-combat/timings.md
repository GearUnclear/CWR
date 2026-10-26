# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Draws = GL draw calls and objects = objects with a visible LOD, both of the shot frame
(`triSceneStats`); load = the `triSettleScene` answer before the mode's timers (frames run). Delta =
Remastered minus Classic, or on minus off (draws and objects: share of the larger).

| view | mode | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU fps | draws | objects | load | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| combat_fires | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 811 | 699 |  | OK |
| combat_fires | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_shell_k05 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 839 | 717 |  | OK |
| combat_shell_k05 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_shell_k06 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 833 | 749 |  | OK |
| combat_shell_k06 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_shell_k08 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 840 | 752 |  | OK |
| combat_shell_k08 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_shell_k11 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 856 | 772 |  | OK |
| combat_shell_k11 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_shell_k16 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 807 | 723 |  | OK |
| combat_shell_k16 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k00 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 812 | 695 |  | OK |
| combat_eye_k00 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k05 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 835 | 717 |  | OK |
| combat_eye_k05 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k06 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 864 | 762 |  | OK |
| combat_eye_k06 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k08 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 861 | 767 |  | OK |
| combat_eye_k08 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_eye_k11 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 875 | 765 |  | OK |
| combat_eye_k11 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_flare_k90 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 798 | 694 |  | OK |
| combat_flare_k90 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_flare_k240 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 783 | 674 |  | OK |
| combat_flare_k240 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_muzzle_k04 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 156 | 77 |  | OK |
| combat_muzzle_k04 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_muzzle_k05 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 156 | 77 |  | OK |
| combat_muzzle_k05 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_muzzle_k06 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 162 | 83 |  | OK |
| combat_muzzle_k06 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_muzzle_k08 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 165 | 89 |  | OK |
| combat_muzzle_k08 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_day_k00 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 817 | 687 |  | OK |
| combat_day_k00 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_day_k05 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 848 | 711 |  | OK |
| combat_day_k05 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| combat_day_k06 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 857 | 758 |  | OK |
| combat_day_k06 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| nc_sunrise_ridge | off | 11.03 | 8.90 | 0.09 | 6.43 | 2.08 | 0.39 | 0.13 |  |  | 1.78 | 0.48 | 0.01 | 0.14 | 0.07 |  | 0.04 | 5.65 | 61.10 | 1,912 | 8,897 | OK:30 | OK |
| nc_sunrise_ridge | on | 10.10 | 7.98 | 0.10 | 5.55 | 2.06 | 0.38 | 0.14 |  |  | 1.76 | 0.41 | 0.01 | 0.15 | 0.09 |  | 0.04 | 5.80 | 62.30 | 1,912 | 8,897 | OK:30 | OK |
| nc_sunrise_ridge | delta | -0.93 | -0.92 | +0.01 | -0.88 | -0.02 | -0.01 | +0.01 |  |  | -0.02 | -0.07 | +0.00 | +0.01 | +0.02 |  | +0.00 | +0.15 |  | +0.0% | +0.0% |  |  |
| nc_road_night | off | 8.64 | 6.98 |  | 0.98 | 3.83 | 2.17 | 0.06 |  |  | 1.48 | 0.42 | 0.01 | 0.13 | 0.06 |  | 0.03 | 8.03 | 88.00 | 1,411 | 914 | OK:30 | OK |
| nc_road_night | on | 8.47 | 6.94 |  | 1.06 | 3.79 | 2.09 | 0.06 |  |  | 1.36 | 0.38 | 0.01 | 0.13 | 0.06 |  | 0.03 | 8.20 | 88.20 | 1,411 | 914 | OK:30 | OK |
| nc_road_night | delta | -0.17 | -0.04 |  | +0.08 | -0.04 | -0.08 | +0.00 |  |  | -0.12 | -0.04 | +0.00 | +0.00 | +0.00 |  | +0.00 | +0.17 |  | +0.0% | +0.0% |  |  |
| combat_timing_r1 | off | 6.35 | 5.14 |  | 0.70 | 1.10 | 3.34 | 0.03 |  |  | 1.05 | 0.47 | 0.01 | 0.15 | 0.07 |  | 0.04 | 6.10 | 128.50 | 797 | 683 | UNSETTLED:300 | OK |
| combat_timing_r1 | on | 6.70 | 5.42 |  | 0.76 | 1.15 | 3.51 | 0.04 |  |  | 1.00 | 0.38 | 0.01 | 0.16 | 0.07 |  | 0.05 | 6.47 | 126.40 | 784 | 678 | UNSETTLED:300 | OK |
| combat_timing_r1 | delta | +0.35 | +0.28 |  | +0.06 | +0.05 | +0.17 | +0.01 |  |  | -0.05 | -0.09 | +0.00 | +0.01 | +0.00 |  | +0.01 | +0.37 |  | -1.6% | -0.7% |  |  |
| combat_timing_r2 | off | 6.40 | 5.23 |  | 0.77 | 1.15 | 3.32 | 0.03 |  |  | 1.01 | 0.42 | 0.01 | 0.16 | 0.07 |  | 0.04 | 6.16 | 128.60 | 777 | 673 | UNSETTLED:300 | OK |
| combat_timing_r2 | on | 6.60 | 5.32 |  | 0.76 | 1.15 | 3.40 | 0.04 |  |  | 1.00 | 0.38 | 0.01 | 0.16 | 0.07 |  | 0.05 | 6.33 | 124.40 | 771 | 666 | UNSETTLED:300 | OK |
| combat_timing_r2 | delta | +0.20 | +0.09 |  | -0.01 | +0.00 | +0.08 | +0.01 |  |  | -0.01 | -0.04 | +0.00 | +0.00 | +0.00 |  | +0.01 | +0.17 |  | -0.8% | -1.0% |  |  |
| combat_timing_r3 | off | 6.31 | 5.17 |  | 0.74 | 1.13 | 3.30 | 0.03 |  |  | 0.98 | 0.39 | 0.01 | 0.16 | 0.07 |  | 0.04 | 6.00 | 130.50 | 772 | 666 | UNSETTLED:300 | OK |
| combat_timing_r3 | on | 6.49 | 5.21 |  | 0.76 | 1.14 | 3.31 | 0.04 |  |  | 1.00 | 0.39 | 0.01 | 0.16 | 0.07 |  | 0.05 | 6.24 | 129.10 | 768 | 666 | UNSETTLED:300 | OK |
| combat_timing_r3 | delta | +0.18 | +0.04 |  | +0.02 | +0.01 | +0.01 | +0.01 |  |  | +0.02 | +0.00 | +0.00 | +0.00 | +0.00 |  | +0.01 | +0.24 |  | -0.5% | +0.0% |  |  |
| combat_flashtiming_r1 | off | 6.11 | 5.16 |  | 0.58 | 1.09 | 3.48 | 0.06 |  |  | 0.74 | 0.18 | 0.01 | 0.15 | 0.07 |  | 0.04 | 5.93 | 134.50 | 822 | 711 |  | OK |
| combat_flashtiming_r1 | on | 6.53 | 5.32 |  | 0.64 | 1.12 | 3.56 | 0.07 |  |  | 0.79 | 0.18 | 0.01 | 0.15 | 0.07 |  | 0.06 | 6.36 | 125.10 | 822 | 711 | UNSETTLED:300 | OK |
| combat_flashtiming_r1 | delta | +0.42 | +0.16 |  | +0.06 | +0.03 | +0.08 | +0.01 |  |  | +0.05 | +0.00 | +0.00 | +0.00 | +0.00 |  | +0.02 | +0.43 |  | +0.0% | +0.0% |  |  |
| combat_flashtiming_r2 | off | 6.39 | 5.37 |  | 0.59 | 1.16 | 3.62 | 0.06 |  |  | 0.79 | 0.21 | 0.01 | 0.15 | 0.07 |  | 0.04 | 5.90 | 129.70 | 822 | 711 |  | OK |
| combat_flashtiming_r2 | on | 6.28 | 5.14 |  | 0.61 | 1.13 | 3.39 | 0.06 |  |  | 0.76 | 0.19 | 0.01 | 0.14 | 0.07 |  | 0.05 | 5.96 | 128.80 | 822 | 711 | UNSETTLED:300 | OK |
| combat_flashtiming_r2 | delta | -0.11 | -0.23 |  | +0.02 | -0.03 | -0.23 | +0.00 |  |  | -0.03 | -0.02 | +0.00 | -0.01 | +0.00 |  | +0.01 | +0.06 |  | +0.0% | +0.0% |  |  |
| combat_flashtiming_r3 | off | 6.05 | 5.10 |  | 0.57 | 1.07 | 3.45 | 0.06 |  |  | 0.76 | 0.21 | 0.01 | 0.14 | 0.07 |  | 0.04 | 5.71 | 134.00 | 822 | 711 |  | OK |
| combat_flashtiming_r3 | on | 6.39 | 5.25 |  | 0.77 | 1.08 | 3.40 | 0.06 |  |  | 0.76 | 0.21 | 0.01 | 0.14 | 0.07 |  | 0.05 | 6.01 | 126.70 | 822 | 711 | UNSETTLED:300 | OK |
| combat_flashtiming_r3 | delta | +0.34 | +0.15 |  | +0.20 | +0.01 | -0.05 | +0.00 |  |  | +0.00 | +0.00 | +0.00 | +0.00 | +0.00 |  | +0.01 | +0.30 |  | +0.0% | +0.0% |  |  |
| smoke_1 | classic |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | 152.40 | 1,291 | 740 |  | OK |
| smoke_1 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_2 | remastered | 6.84 | 5.32 | 0.21 | 1.13 | 2.22 | 1.98 | 0.10 |  |  | 1.35 | 0.30 | 0.01 | 0.12 | 0.07 |  | 0.03 | 1.84 | 134.20 | 763 | 740 |  | OK |
| smoke_3 | classic | 2.08 | 1.89 | 0.18 | 0.15 | 1.11 | 0.63 | 0.09 |  |  | 1.27 | 0.28 | 0.01 | 0.11 | 0.06 |  | 0.03 | 1.73 | 127.70 | 1,291 | 740 |  | OK |
| smoke_3 | remastered | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | classic | missing |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| smoke_4 | remastered | 6.32 | 4.91 | 0.17 | 1.10 | 2.22 | 1.59 | 0.09 |  |  | 1.24 | 0.29 | 0.01 | 0.11 | 0.06 |  | 0.03 | 1.73 | 123.20 | 763 | 740 |  | OK |
