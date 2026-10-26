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
| b9t_hill_1730 | off | 3 | 2.29 / 2.25 | 1.61 / 1.50 | 0.04 / 0.04 | 0.96 / 0.86 | 0.57 / 0.57 | 0.07 / 0.07 | 0.05 / 0.05 |  |  | 0.55 / 0.55 | 0.13 / 0.12 |  | 0.06 / 0.06 | 0.07 / 0.07 |  | 0.03 / 0.03 | 2.10 / 2.10 | 3.04 / 3.02 | 692 | 934 | 2,775 | P0 | OK:30,OK:30,OK:30 |
| b9t_hill_1730 | on | 3 | 4.36 / 3.77 | 3.13 / 2.73 | 0.04 / 0.04 | 1.96 / 1.65 | 1.08 / 0.97 | 0.08 / 0.08 | 0.05 / 0.05 |  |  | 1.08 / 0.92 | 0.28 / 0.22 |  | 0.34 / 0.25 | 0.07 / 0.07 |  | 0.03 / 0.03 | 2.14 / 2.13 | 3.04 / 2.99 | 692 | 934 | 2,775 | P0 | OK:30,OK:30,OK:30 |
| b9t_slope_tele_1730 | off | 3 | 2.53 / 1.90 | 1.65 / 1.25 | 0.03 / 0.03 | 1.05 / 0.87 | 0.31 / 0.24 | 0.07 / 0.07 | 0.01 / 0.01 |  |  | 0.84 / 0.62 | 0.11 / 0.11 |  | 0.08 / 0.07 | 0.13 / 0.08 |  | 0.03 / 0.03 | 1.52 / 1.51 | 1.89 / 1.87 | 310 | 426 | 2,775 | P0 | OK:30,OK:30,OK:30 |
| b9t_slope_tele_1730 | on | 3 | 1.99 / 1.63 | 1.34 / 1.07 | 0.03 / 0.03 | 0.94 / 0.78 | 0.31 / 0.23 | 0.07 / 0.06 | 0.01 / 0.01 |  |  | 0.62 / 0.52 | 0.13 / 0.11 |  | 0.16 / 0.08 | 0.07 / 0.07 |  | 0.03 / 0.03 | 1.56 / 1.55 | 1.91 / 1.84 | 310 | 426 | 2,775 | P0 | OK:30,OK:30,OK:30 |
| b9t_forest_edge_golden | off | 3 | 4.64 / 4.57 | 3.52 / 3.52 | 0.24 / 0.21 | 1.19 / 1.05 | 1.79 / 1.73 | 0.68 / 0.61 | 0.10 / 0.10 |  |  | 0.88 / 0.82 | 0.26 / 0.25 |  | 0.13 / 0.13 | 0.07 / 0.07 |  | 0.03 / 0.03 | 3.99 / 3.98 | 9.16 / 7.62 | 816 | 1,457 | 2,775 | P0 | OK:30,OK:30,OK:30 |
| b9t_forest_edge_golden | on | 3 | 8.20 / 5.71 | 6.85 / 4.39 | 0.31 / 0.30 | 1.81 / 1.23 | 3.61 / 2.21 | 1.43 / 0.95 | 0.10 / 0.10 |  |  | 1.11 / 1.09 | 0.24 / 0.21 |  | 0.13 / 0.13 | 0.07 / 0.07 |  | 0.03 / 0.03 | 4.02 / 4.01 | 8.77 / 8.50 | 816 | 1,457 | 2,775 | P0 | OK:30,OK:30,OK:30 |
| b9t_hill_noon | off | 3 | 3.07 / 2.79 | 2.26 / 1.90 | 0.03 / 0.03 | 1.44 / 1.09 | 0.75 / 0.74 | 0.07 / 0.07 | 0.05 / 0.05 |  |  | 0.76 / 0.68 | 0.21 / 0.14 |  | 0.17 / 0.12 | 0.08 / 0.07 |  | 0.03 / 0.03 | 2.09 / 2.09 | 3.00 / 2.96 | 694 | 934 | 2,775 | P0 | OK:30,OK:30,OK:30 |
| b9t_hill_noon | on | 3 | 2.44 / 2.38 | 1.68 / 1.59 | 0.03 / 0.03 | 1.04 / 0.90 | 0.62 / 0.57 | 0.07 / 0.07 | 0.06 / 0.05 |  |  | 0.72 / 0.57 | 0.19 / 0.13 |  | 0.09 / 0.06 | 0.08 / 0.07 |  | 0.03 / 0.03 | 2.10 / 2.10 | 3.00 / 2.95 | 694 | 934 | 2,775 | P0 | OK:30,OK:30,OK:30 |
| b9t_lapse | on | 3 | 2.70 / 2.49 | 1.81 / 1.68 | 0.04 / 0.04 | 1.04 / 0.95 | 0.65 / 0.62 | 0.09 / 0.08 | 0.07 / 0.07 | 0.06 / 0.06 | 0.66 / 0.62 | 0.61 / 0.55 | 0.14 / 0.13 |  | 0.07 / 0.06 | 0.07 / 0.07 |  | 0.03 / 0.03 | 2.14 / 2.13 | 2.98 / 2.96 | 692 | 934 | 2,775 | P0 | OK:30,OK:30,OK:30 |

## Repeat noise between rounds (Remastered shots)

Per pair of rounds of the same view on the same build: max channel difference / pixels over 2 /
pixels over 8, per row band (sky 0-422, horizon 422-591, ground 591-1080, whole frame). capBegin
pins the sim clock to the time each round starts, and the rounds are minutes apart, so sky and
cloud drift, sea animation, fire and smoke differ between rounds. This shows how far one view
moves inside one game; it is NOT the gate item 5 noise floor (that is a pinned same-view repeat).

| view | pair | sky | horizon | ground | whole |
|---|---|---|---|---|---|
