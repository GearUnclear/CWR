# Timing: median / minimum over interleaved rounds

GPU sections in ms (`triGpuTimes`, smoothed over the 90 frames before each shot), as
`median / min` over the rounds; CPU frame ms from `triPerfStats` (median / min); draws and
objects are the shot frame's medians; n = rounds found. 1920x1080 windowed, render scale 1.0,
4x MSAA, presented frame.

GPU MHz: median graphics clock of the nvidia-smi samples stamped from 3 s before each shot
up to the shot, median over the rounds, with the P-states seen (the driver picks the clock
from the load, so light views run at lower clocks than heavy ones). gpu_sampler.ps1 stamps
its rows in whole seconds and writes one every 2 to 3 s, so the window edges are good to
about a second: the row stamped in the shot's own second may have been read just after it.

| view | mode | n | frame | world | shadow | ground | pass1 | rest | sky | skytable | canopy | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU frame | draws | objects | GPU MHz | P-states | settle |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge | classic | 1 | 40.68 / 40.68 | 40.53 / 40.53 |  | 7.74 / 7.74 | 26.82 / 26.82 | 5.97 / 5.97 |  |  |  |  |  |  |  |  |  |  | 30.47 / 30.47 | 60.53 / 60.53 | 13,076 | 8,897 | 2,775 | P0 | OK:30 |
| sunrise_ridge | remastered | 1 | 17.73 / 17.73 | 13.87 / 13.87 | 0.08 / 0.08 | 6.32 / 6.32 | 5.60 / 5.60 | 1.94 / 1.94 | 0.27 / 0.27 |  |  | 3.23 / 3.23 | 0.35 / 0.35 | 0.01 / 0.01 | 0.13 / 0.13 | 0.06 / 0.06 |  | 0.04 / 0.04 | 5.07 / 5.07 | 21.01 / 21.01 | 1,913 | 8,897 | 2,775 | P0 | OK:30 |
| forest_edge_golden | classic | 1 | 14.78 / 14.78 | 14.62 / 14.62 |  | 0.09 / 0.09 | 7.78 / 7.78 | 6.75 / 6.75 |  |  |  |  |  |  |  |  |  |  | 4.54 / 4.54 | 14.85 / 14.85 | 2,502 | 1,459 | 2,775 | P0 | OK:30 |
| forest_edge_golden | remastered | 1 | 18.25 / 18.25 | 15.24 / 15.24 | 1.14 / 1.14 | 3.11 / 3.11 | 8.30 / 8.30 | 3.82 / 3.82 | 0.12 / 0.12 |  |  | 2.29 / 2.29 | 0.16 / 0.16 | 0.01 / 0.01 | 0.13 / 0.13 | 0.88 / 0.88 |  | 0.04 / 0.04 | 6.27 / 6.27 | 17.84 / 17.84 | 866 | 1,459 | 2,775 | P0 | OK:30 |
| town_night | classic | 1 | 7.93 / 7.93 | 7.74 / 7.74 |  | 0.07 / 0.07 | 1.41 / 1.41 | 6.25 / 6.25 |  |  |  |  |  |  |  |  |  |  | 5.00 / 5.00 | 8.11 / 8.11 | 1,289 | 1,113 | 2,775 | P0 | OK:246 |
| town_night | remastered | 1 | 16.18 / 16.18 | 14.18 / 14.18 | 0.22 / 0.22 | 2.62 / 2.62 | 4.96 / 4.96 | 6.61 / 6.61 | 0.08 / 0.08 |  |  | 1.57 / 1.57 | 0.36 / 0.36 | 0.02 / 0.02 | 0.16 / 0.16 | 0.20 / 0.20 |  | 0.18 / 0.18 | 13.18 / 13.18 | 18.96 / 18.96 | 1,224 | 1,111 | 2,775 | P0 | OK:37 |
| coastline_sunset | classic | 1 | 1.41 / 1.41 | 1.23 / 1.23 |  | 0.67 / 0.67 | 0.00 / 0.00 | 0.56 / 0.56 |  |  |  |  |  |  |  |  |  |  | 0.57 / 0.57 | 2.83 / 2.83 | 177 | 0 | 2,775 | P0 | OK:30 |
| coastline_sunset | remastered | 1 | 4.55 / 4.55 | 2.49 / 2.49 | 0.00 / 0.00 | 1.94 / 1.94 | 0.00 / 0.00 | 0.54 / 0.54 | 0.10 / 0.10 |  |  | 1.84 / 1.84 | 0.15 / 0.15 | 0.01 / 0.01 | 0.10 / 0.10 | 0.08 / 0.08 |  | 0.03 / 0.03 | 3.20 / 3.20 | 4.67 / 4.67 | 177 | 0 | 1,455 | P3 | OK:30 |
| woods_pine2_evening | classic | 1 | 7.11 / 7.11 | 6.97 / 6.97 |  | 0.12 / 0.12 | 2.83 / 2.83 | 4.02 / 4.02 |  |  |  |  |  |  |  |  |  |  | 5.13 / 5.13 | 11.61 / 11.61 | 3,250 | 1,811 | 2,775 | P0 | OK:30 |
| woods_pine2_evening | remastered | 1 | 13.00 / 13.00 | 10.92 / 10.92 | 0.50 / 0.50 | 2.58 / 2.58 | 7.31 / 7.31 | 1.02 / 1.02 | 0.10 / 0.10 |  |  | 1.81 / 1.81 | 0.34 / 0.34 | 0.01 / 0.01 | 0.21 / 0.21 | 0.06 / 0.06 |  | 0.04 / 0.04 | 9.45 / 9.45 | 14.34 / 14.34 | 1,421 | 1,811 | 2,775 | P0 | OK:30 |
| lowsun_fp | classic | 1 | 5.86 / 5.86 | 5.73 / 5.73 |  | 0.11 / 0.11 | 2.85 / 2.85 | 2.76 / 2.76 |  |  |  |  |  |  |  |  |  |  | 3.38 / 3.38 | 8.25 / 8.25 | 2,091 | 1,316 | 2,775 | P0 | OK:30 |
| lowsun_fp | remastered | 1 | 16.67 / 16.67 | 14.05 / 14.05 | 0.32 / 0.32 | 2.19 / 2.19 | 6.69 / 6.69 | 5.17 / 5.17 | 0.10 / 0.10 |  |  | 2.36 / 2.36 | 0.15 / 0.15 | 0.01 / 0.01 | 0.14 / 0.14 | 0.06 / 0.06 |  | 0.04 / 0.04 | 7.34 / 7.34 | 16.68 / 16.68 | 1,212 | 1,316 | 2,775 | P0 | OK:30 |
| road_flat_drive_moonless | classic | 1 | 6.88 / 6.88 | 6.72 / 6.72 |  | 0.08 / 0.08 | 3.75 / 3.75 | 2.89 / 2.89 |  |  |  |  |  |  |  |  |  |  | 3.06 / 3.06 | 9.66 / 9.66 | 1,806 | 914 | 2,775 | P0 | OK:30 |
| road_flat_drive_moonless | remastered | 1 | 16.81 / 16.81 | 14.50 / 14.50 |  | 1.85 / 1.85 | 8.28 / 8.28 | 4.37 / 4.37 | 0.06 / 0.06 |  |  | 2.11 / 2.11 | 0.23 / 0.23 | 0.01 / 0.01 | 0.13 / 0.13 | 0.06 / 0.06 |  | 0.03 / 0.03 | 9.23 / 9.23 | 18.48 / 18.48 | 1,411 | 914 | 2,775 | P0 | OK:30 |
| ffh | classic | 1 | 7.53 / 7.53 | 7.38 / 7.38 |  | 0.10 / 0.10 | 0.64 / 0.64 | 6.64 / 6.64 |  |  |  |  |  |  |  |  |  |  | 3.87 / 3.87 | 10.08 / 10.08 | 803 | 698 | 2,775 | P0 | UNSETTLED:300 |
| ffh | remastered | 1 | 12.02 / 12.02 | 10.52 / 10.52 |  | 1.16 / 1.16 | 2.24 / 2.24 | 7.12 / 7.12 | 0.04 / 0.04 |  |  | 1.25 / 1.25 | 0.19 / 0.19 | 0.01 / 0.01 | 0.13 / 0.13 | 0.14 / 0.14 |  | 0.04 / 0.04 | 6.21 / 6.21 | 12.82 / 12.82 | 789 | 680 | 2,775 | P0 | UNSETTLED:300 |
| bay_offshore | classic | 1 | 11.29 / 11.29 | 11.16 / 11.16 |  | 0.75 / 0.75 | 9.58 / 9.58 | 0.83 / 0.83 |  |  |  |  |  |  |  |  |  |  | 5.45 / 5.45 | 14.77 / 14.77 | 4,562 | 2,961 | 2,775 | P0 | OK:30 |

## Repeat noise between rounds (Remastered shots)

Per pair of rounds of the same view on the same build: max channel difference / pixels over 2 /
pixels over 8, per row band (sky 0-422, horizon 422-591, ground 591-1080, whole frame). capBegin
pins the sim clock to the time each round starts, and the rounds are minutes apart, so sky and
cloud drift, sea animation, fire and smoke differ between rounds. This shows how far one view
moves inside one game; it is NOT the gate item 5 noise floor (that is a pinned same-view repeat).

| view | pair | sky | horizon | ground | whole |
|---|---|---|---|---|---|
