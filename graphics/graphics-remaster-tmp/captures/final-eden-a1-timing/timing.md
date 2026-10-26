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
| overcast_broken | classic | 3 | 10.71 / 10.56 | 10.57 / 10.43 |  | 0.15 / 0.15 | 8.23 / 8.19 | 2.23 / 2.05 |  |  |  |  |  |  |  |  |  |  | 9.92 / 9.74 | 17.35 / 17.06 | 8,243 | 5,692 | 2,790 | P0 | OK:30,OK:30,OK:30 |
| overcast_broken | remastered | 3 | 8.92 / 8.91 | 7.52 / 7.43 | 0.23 / 0.23 | 1.99 / 1.95 | 4.95 / 4.82 | 0.58 / 0.55 | 0.10 / 0.10 |  |  | 1.13 / 1.12 | 0.43 / 0.37 | 0.01 / 0.01 | 0.11 / 0.11 | 0.06 / 0.06 |  | 0.04 / 0.04 | 8.43 / 8.43 | 15.70 / 15.70 | 2,214 | 5,692 | 2,790 | P0 | OK:30,OK:30,OK:30 |
| broken_high | classic | 3 | 20.01 / 19.95 | 19.90 / 19.85 |  | 6.44 / 6.34 | 11.85 / 11.73 | 1.68 / 1.59 |  |  |  |  |  |  |  |  |  |  | 18.80 / 18.70 | 22.95 / 22.87 | 10,789 | 7,549 | 2,790 | P0 | OK:30,OK:30,OK:30 |
| broken_high | remastered | 3 | 29.20 / 29.09 | 27.18 / 27.10 | 0.04 / 0.04 | 7.59 / 7.57 | 18.07 / 17.96 | 1.54 / 1.52 | 0.07 / 0.07 |  |  | 1.78 / 1.77 | 0.18 / 0.18 | 0.01 / 0.01 | 0.06 / 0.06 | 0.06 / 0.06 |  | 0.03 / 0.03 | 28.21 / 28.14 | 31.95 / 31.90 | 5,479 | 7,549 | 2,790 | P0 | OK:30,OK:30,OK:30 |

## Repeat noise between rounds (Remastered shots)

Per pair of rounds of the same view on the same build: max channel difference / pixels over 2 /
pixels over 8, per row band (sky 0-422, horizon 422-591, ground 591-1080, whole frame). capBegin
pins the sim clock to the time each round starts, and the rounds are minutes apart, so sky and
cloud drift, sea animation, fire and smoke differ between rounds. This shows how far one view
moves inside one game; it is NOT the gate item 5 noise floor (that is a pinned same-view repeat).

| view | pair | sky | horizon | ground | whole |
|---|---|---|---|---|---|
| overcast_broken | r1-r2 | 60 / 410 / 68 | 131 / 14,255 / 1,833 | 35 / 8,351 / 343 | 131 / 23,016 / 2,244 |
| overcast_broken | r1-r3 | 167 / 477 / 100 | 132 / 17,052 / 2,588 | 32 / 9,164 / 369 | 167 / 26,693 / 3,057 |
| overcast_broken | r2-r3 | 151 / 431 / 62 | 132 / 11,796 / 1,716 | 36 / 6,767 / 308 | 151 / 18,994 / 2,086 |
| broken_high | r1-r2 | 3 / 1 / 0 | 100 / 4,214 / 637 | 47 / 13,088 / 1,000 | 100 / 17,303 / 1,637 |
| broken_high | r1-r3 | 3 / 1 / 0 | 135 / 4,284 / 650 | 42 / 14,919 / 848 | 135 / 19,204 / 1,498 |
| broken_high | r2-r3 | 2 / 0 / 0 | 134 / 2,844 / 300 | 50 / 7,318 / 446 | 134 / 10,162 / 746 |
