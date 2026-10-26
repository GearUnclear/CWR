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

| view | mode | n | frame | world | shadow | post | ao | bloom | shafts | composite | CPU frame | draws | objects | GPU MHz | P-states | settle |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge | classic | 1 | 19.65 / 19.65 | 19.50 / 19.50 |  |  |  |  |  |  | 27.22 / 27.22 | 13,076 | 8,897 | 2,790 | P0 | OK:30 |
| sunrise_ridge | remastered | 1 | 17.85 / 17.85 | 16.67 / 16.67 | 0.09 / 0.09 | 1.02 / 1.02 | 0.62 / 0.62 | 0.06 / 0.06 | 0.27 / 0.27 | 0.07 / 0.07 | 26.83 / 26.83 | 12,910 | 8,897 | 2,790 | P0 | OK:30 |
| forest_edge_golden | classic | 1 | 5.72 / 5.72 | 5.53 / 5.53 |  |  |  |  |  |  | 8.69 / 8.69 | 2,502 | 1,459 | 1,965 | P0 | OK:30 |
| forest_edge_golden | remastered | 1 | 6.32 / 6.32 | 5.50 / 5.50 | 0.31 / 0.31 | 0.61 / 0.61 | 0.46 / 0.46 | 0.09 / 0.09 |  | 0.06 / 0.06 | 8.71 / 8.71 | 2,142 | 1,459 | 1,875 | P0 | OK:30 |
| town_night | classic | 1 | 6.40 / 6.40 | 6.12 / 6.12 |  |  |  |  |  |  | 8.23 / 8.23 | 1,174 | 999 | 1,590 | P3 | OK:187 |
| town_night | remastered | 1 | 6.17 / 6.17 | 5.29 / 5.29 |  | 0.66 / 0.66 | 0.53 / 0.53 | 0.08 / 0.08 |  | 0.05 / 0.05 | 8.19 / 8.19 | 1,177 | 987 | 1,980 | P0 | OK:60 |
| coastline_sunset | classic | 1 | 3.11 / 3.11 | 2.39 / 2.39 |  |  |  |  |  |  | 4.17 / 4.17 | 177 | 0 | 840 | P3 | OK:30 |
| coastline_sunset | remastered | 1 | 1.80 / 1.80 | 0.68 / 0.68 | 0.00 / 0.00 | 0.98 / 0.98 | 0.43 / 0.43 | 0.08 / 0.08 | 0.39 / 0.39 | 0.08 / 0.08 | 3.60 / 3.60 | 177 | 0 | 1,905 | P0 | OK:30 |
| woods_pine2_evening | classic | 1 | 6.23 / 6.23 | 6.08 / 6.08 |  |  |  |  |  |  | 9.19 / 9.19 | 3,250 | 1,811 | 2,775 | P0 | OK:30 |
| woods_pine2_evening | remastered | 1 | 6.58 / 6.58 | 5.33 / 5.33 | 0.36 / 0.36 | 1.10 / 1.10 | 0.46 / 0.46 | 0.07 / 0.07 | 0.50 / 0.50 | 0.07 / 0.07 | 9.16 / 9.16 | 2,842 | 1,811 | 2,775 | P0 | OK:30 |
| lowsun_fp | classic | 1 | 4.96 / 4.96 | 4.76 / 4.76 |  |  |  |  |  |  | 7.70 / 7.70 | 2,091 | 1,316 | 1,530 | P3 | OK:30 |
| lowsun_fp | remastered | 1 | 4.51 / 4.51 | 3.45 / 3.45 | 0.08 / 0.08 | 0.92 / 0.92 | 0.38 / 0.38 | 0.07 / 0.07 | 0.40 / 0.40 | 0.07 / 0.07 | 6.84 / 6.84 | 1,890 | 1,316 | 2,520 | P0 | OK:30 |
| road_flat_drive_moonless | classic | 1 | 4.48 / 4.48 | 4.24 / 4.24 |  |  |  |  |  |  | 6.67 / 6.67 | 1,806 | 914 | 2,055 | P0 | OK:30 |
| road_flat_drive_moonless | remastered | 1 | 6.45 / 6.45 | 5.53 / 5.53 |  | 0.66 / 0.66 | 0.49 / 0.49 | 0.11 / 0.11 |  | 0.07 / 0.07 | 8.70 / 8.70 | 1,806 | 914 | 1,320 | P3 | OK:30 |
| ffh | classic | 1 | 5.54 / 5.54 | 5.33 / 5.33 |  |  |  |  |  |  | 7.73 / 7.73 | 801 | 700 | 1,470 | P3 | UNSETTLED:300 |
| ffh | remastered | 1 | 6.31 / 6.31 | 5.36 / 5.36 |  | 0.77 / 0.77 | 0.64 / 0.64 | 0.08 / 0.08 |  | 0.06 / 0.06 | 8.13 / 8.13 | 775 | 672 | 1,920 | P0 | UNSETTLED:300 |
| bay_offshore | classic | 1 | 5.66 / 5.66 | 5.53 / 5.53 |  |  |  |  |  |  | 10.11 / 10.11 | 4,562 | 2,961 | 2,775 | P0 | OK:30 |
| bay_offshore | remastered | 1 | 7.22 / 7.22 | 6.61 / 6.61 | 0.03 / 0.03 | 0.47 / 0.47 | 0.37 / 0.37 | 0.06 / 0.06 |  | 0.03 / 0.03 | 11.56 / 11.56 | 4,559 | 2,961 | 2,775 | P0 | OK:30 |
| town_night_rain | classic | 1 | 5.14 / 5.14 | 4.84 / 4.84 |  |  |  |  |  |  | 6.83 / 6.83 | 880 | 631 | 1,215 | P3 | OK:78 |
| town_night_rain | remastered | 1 | 5.56 / 5.56 | 4.63 / 4.63 |  | 0.70 / 0.70 | 0.55 / 0.55 | 0.09 / 0.09 |  | 0.06 / 0.06 | 6.69 / 6.69 | 895 | 634 | 2,122 | P0 | OK:165 |
| pinewood_morn | classic | 1 | 6.26 / 6.26 | 5.99 / 5.99 |  |  |  |  |  |  | 9.75 / 9.75 | 3,163 | 1,678 | 1,650 | P0 | OK:30 |
| pinewood_morn | remastered | 1 | 5.34 / 5.34 | 4.27 / 4.27 | 0.46 / 0.46 | 0.88 / 0.88 | 0.44 / 0.44 | 0.07 / 0.07 | 0.30 / 0.30 | 0.07 / 0.07 | 8.29 / 8.29 | 2,845 | 1,678 | 1,778 | P0/P3 | OK:30 |
| valley_0515 | classic | 1 | 19.94 / 19.94 | 19.81 / 19.81 |  |  |  |  |  |  | 25.43 / 25.43 | 11,553 | 7,966 | 2,775 | P0 | OK:30 |
| valley_0515 | remastered | 1 | 22.72 / 22.72 | 21.38 / 21.38 | 0.04 / 0.04 | 1.21 / 1.21 | 0.76 / 0.76 | 0.07 / 0.07 | 0.31 / 0.31 | 0.07 / 0.07 | 30.54 / 30.54 | 11,406 | 7,966 | 2,775 | P0 | OK:30 |

## Repeat noise between rounds (Remastered shots)

Per pair of rounds of the same view on the same build: max channel difference / pixels over 2 /
pixels over 8, per row band (sky 0-422, horizon 422-591, ground 591-1080, whole frame). capBegin
pins the sim clock to the time each round starts, and the rounds are minutes apart, so sky and
cloud drift, sea animation, fire and smoke differ between rounds. This shows how far one view
moves inside one game; it is NOT the gate item 5 noise floor (that is a pinned same-view repeat).

| view | pair | sky | horizon | ground | whole |
|---|---|---|---|---|---|
