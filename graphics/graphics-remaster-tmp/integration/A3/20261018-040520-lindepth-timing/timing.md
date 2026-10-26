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

| view | mode | n | frame | world | shadow | post | resolve | lindepth | ao | bloom | shafts | composite | CPU frame | draws | objects | GPU MHz | P-states | settle |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge | off | 3 | 17.64 / 17.57 | 16.44 / 16.44 | 0.09 / 0.08 | 1.00 / 0.97 | 0.48 / 0.45 |  | 0.13 / 0.13 | 0.06 / 0.06 | 0.27 / 0.27 | 0.07 / 0.06 | 26.69 / 26.62 | 12,910 | 8,897 | 2,790 | P0 | OK:30,OK:30,OK:30 |
| sunrise_ridge | on | 3 | 17.60 / 17.60 | 16.41 / 16.35 | 0.08 / 0.08 | 1.04 / 1.02 | 0.48 / 0.44 | 0.01 / 0.01 | 0.13 / 0.13 | 0.06 / 0.06 | 0.27 / 0.27 | 0.08 / 0.06 | 26.59 / 26.33 | 12,910 | 8,897 | 2,790 | P0 | OK:30,OK:30,OK:30 |
| forest_edge_golden | off | 3 | 5.67 / 5.40 | 4.92 / 4.67 | 0.26 / 0.26 | 0.54 / 0.54 | 0.24 / 0.24 |  | 0.17 / 0.17 | 0.08 / 0.08 |  | 0.05 / 0.05 | 8.07 / 8.02 | 2,142 | 1,459 | 2,100 | P0 | OK:30,OK:30,OK:30 |
| forest_edge_golden | on | 3 | 5.97 / 5.93 | 5.13 / 5.10 | 0.28 / 0.28 | 0.63 / 0.62 | 0.25 / 0.24 | 0.01 / 0.01 | 0.18 / 0.18 | 0.09 / 0.09 |  | 0.05 / 0.05 | 8.56 / 8.51 | 2,142 | 1,459 | 1,875 | P0 | OK:30,OK:30,OK:30 |
| coastline_sunset | off | 3 | 3.79 / 3.64 | 1.54 / 1.46 | 0.00 / 0.00 | 1.92 / 1.80 | 0.33 / 0.32 |  | 0.26 / 0.26 | 0.17 / 0.17 | 0.84 / 0.84 | 0.31 / 0.20 | 3.74 / 3.74 | 177 | 0 | 1,050 | P3 | OK:30,OK:30,OK:30 |
| coastline_sunset | on | 3 | 4.53 / 4.36 | 1.95 / 1.76 | 0.00 / 0.00 | 2.30 / 2.26 | 0.35 / 0.34 | 0.03 / 0.03 | 0.32 / 0.32 | 0.22 / 0.20 | 1.03 / 1.03 | 0.19 / 0.19 | 4.28 / 4.24 | 177 | 0 | 645 | P3 | OK:30,OK:30,OK:30 |
| woods_pine2_evening | off | 3 | 6.51 / 6.46 | 5.22 / 5.18 | 0.35 / 0.35 | 1.13 / 1.13 | 0.29 / 0.27 |  | 0.20 / 0.20 | 0.07 / 0.07 | 0.50 / 0.49 | 0.09 / 0.07 | 9.04 / 9.04 | 2,842 | 1,811 | 2,775 | P0 | OK:30,OK:30,OK:30 |
| woods_pine2_evening | on | 3 | 6.56 / 6.52 | 5.19 / 5.18 | 0.35 / 0.35 | 1.18 / 1.18 | 0.30 / 0.30 | 0.01 / 0.01 | 0.19 / 0.19 | 0.07 / 0.07 | 0.50 / 0.49 | 0.08 / 0.07 | 9.05 / 9.03 | 2,842 | 1,811 | 2,775 | P0 | OK:30,OK:30,OK:30 |
| noon_fields | off | 3 | 9.23 / 9.21 | 8.63 / 8.63 | 0.07 / 0.07 | 0.46 / 0.44 | 0.24 / 0.22 |  | 0.11 / 0.11 | 0.06 / 0.06 |  | 0.04 / 0.04 | 14.18 / 14.15 | 6,398 | 4,508 | 2,775 | P0 | OK:30,OK:30,OK:30 |
| noon_fields | on | 3 | 9.19 / 9.18 | 8.54 / 8.54 | 0.07 / 0.07 | 0.50 / 0.49 | 0.24 / 0.23 | 0.01 / 0.01 | 0.11 / 0.11 | 0.06 / 0.06 |  | 0.04 / 0.04 | 14.11 / 14.03 | 6,398 | 4,508 | 2,775 | P0 | OK:30,OK:30,OK:30 |

## Repeat noise between rounds (Remastered shots)

Per pair of rounds of the same view on the same build: max channel difference / pixels over 2 /
pixels over 8, per row band (sky 0-422, horizon 422-591, ground 591-1080, whole frame). capBegin
pins the sim clock to the time each round starts, and the rounds are minutes apart, so sky and
cloud drift, sea animation, fire and smoke differ between rounds. This shows how far one view
moves inside one game; it is NOT the gate item 5 noise floor (that is a pinned same-view repeat).

| view | pair | sky | horizon | ground | whole |
|---|---|---|---|---|---|
