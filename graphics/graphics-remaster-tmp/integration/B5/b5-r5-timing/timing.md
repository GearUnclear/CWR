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

| view | mode | n | frame | world | shadow | ground | pass1 | rest | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU frame | draws | objects | GPU MHz | P-states | settle |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset_sea | off | 3 | 3.01 / 2.83 | 1.22 / 1.14 | 0.00 / 0.00 | 0.76 / 0.74 | 0.00 / 0.00 | 0.48 / 0.37 | 0.12 / 0.08 |  | 1.64 / 1.44 | 0.39 / 0.34 |  | 0.18 / 0.13 | 0.08 / 0.06 | 0.41 / 0.29 | 0.40 / 0.26 | 2.29 / 1.72 | 3.67 / 3.48 | 176 | 0 | 1,868 | P0 | OK:30,OK:30,OK:30 |
| coastline_sunset_sea | on | 3 | 3.19 / 2.83 | 1.55 / 1.35 | 0.00 / 0.00 | 1.32 / 1.08 | 0.00 / 0.00 | 0.27 / 0.21 | 0.11 / 0.09 |  | 1.38 / 1.31 | 0.42 / 0.29 |  | 0.16 / 0.13 | 0.08 / 0.07 | 0.36 / 0.29 | 0.10 / 0.08 | 2.55 / 2.11 | 3.92 / 3.73 | 176 | 0 | 1,860 | P0 | OK:30,OK:30,OK:30 |
| sea_noon | off | 3 | 2.48 / 2.42 | 1.40 / 1.23 | 0.00 / 0.00 | 0.98 / 0.82 | 0.00 / 0.00 | 0.58 / 0.25 | 0.09 / 0.06 |  | 1.01 / 0.95 | 0.38 / 0.25 |  | 0.14 / 0.09 | 0.09 / 0.07 |  | 0.05 / 0.03 | 1.87 / 1.53 | 3.82 / 3.70 | 179 | 0 | 2,070 | P0 | OK:30,OK:30,OK:30 |
| sea_noon | on | 3 | 3.48 / 3.27 | 2.36 / 2.19 | 0.00 / 0.00 | 1.98 / 1.94 | 0.00 / 0.00 | 0.26 / 0.24 | 0.08 / 0.06 |  | 0.93 / 0.90 | 0.34 / 0.34 |  | 0.12 / 0.09 | 0.08 / 0.06 |  | 0.04 / 0.03 | 2.85 / 2.31 | 4.67 / 4.51 | 179 | 0 | 1,710 | P0/P3 | OK:30,OK:30,OK:30 |
| shore_midsun | off | 3 | 3.32 / 2.69 | 1.52 / 1.19 | 0.00 / 0.00 | 0.99 / 0.81 | 0.00 / 0.00 | 0.35 / 0.20 | 0.04 / 0.03 |  | 1.74 / 1.43 | 0.45 / 0.37 |  | 0.13 / 0.10 | 0.08 / 0.07 | 0.42 / 0.33 | 0.25 / 0.11 | 2.13 / 1.89 | 4.08 / 3.32 | 182 | 0 | 2,355 | P0 | OK:30,OK:30,OK:30 |
| shore_midsun | on | 3 | 3.86 / 3.69 | 2.31 / 2.12 | 0.00 / 0.00 | 2.09 / 1.80 | 0.00 / 0.00 | 0.32 / 0.21 | 0.03 / 0.03 |  | 1.54 / 1.30 | 0.52 / 0.38 |  | 0.12 / 0.10 | 0.08 / 0.07 | 0.37 / 0.33 | 0.14 / 0.08 | 3.21 / 2.91 | 5.14 / 4.48 | 182 | 0 | 2,280 | P0 | OK:30,OK:30,OK:30 |

## Repeat noise between rounds (Remastered shots)

Per pair of rounds of the same view on the same build: max channel difference / pixels over 2 /
pixels over 8, per row band (sky 0-422, horizon 422-591, ground 591-1080, whole frame). capBegin
pins the sim clock to the time each round starts, and the rounds are minutes apart, so sky and
cloud drift, sea animation, fire and smoke differ between rounds. This shows how far one view
moves inside one game; it is NOT the gate item 5 noise floor (that is a pinned same-view repeat).

| view | pair | sky | horizon | ground | whole |
|---|---|---|---|---|---|
