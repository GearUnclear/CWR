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
| valley_0515 | on | 3 | 33.29 / 25.96 | 30.13 / 23.32 | 0.04 / 0.04 | 2.77 / 2.23 | 24.59 / 18.78 | 2.77 / 2.31 | 0.09 / 0.09 |  |  | 2.89 / 2.38 | 0.17 / 0.17 | 0.01 / 0.01 | 0.06 / 0.06 | 0.09 / 0.06 |  | 0.18 / 0.04 | 24.83 / 24.68 | 49.80 / 36.42 | 5,242 | 7,966 | 2,775 | P0 | OK:30,OK:30,OK:30 |
| valley_0515 | base | 3 | 55.34 / 33.18 | 53.50 / 31.86 | 0.04 / 0.04 | 5.44 / 5.37 | 44.92 / 24.51 | 3.21 / 1.85 |  |  |  | 1.70 / 1.18 | 0.81 / 0.68 |  | 0.06 / 0.06 | 0.07 / 0.07 | 0.31 / 0.31 | 0.38 / 0.06 | 33.55 / 32.14 | 56.94 / 35.43 | 11,406 | 7,966 | 2,775 | P0 | OK:30,OK:30,OK:30 |
| sunrise_ridge | on | 3 | 10.25 / 10.13 | 8.41 / 8.24 | 0.08 / 0.08 | 5.89 / 5.87 | 2.15 / 1.88 | 0.38 / 0.35 | 0.12 / 0.12 |  |  | 1.54 / 1.48 | 0.38 / 0.38 | 0.01 / 0.01 | 0.13 / 0.13 | 0.06 / 0.06 |  | 0.04 / 0.03 | 5.02 / 5.02 | 15.31 / 15.22 | 1,912 | 8,897 | 2,775 | P0 | OK:30,OK:30,OK:30 |
| sunrise_ridge | base | 3 | 25.53 / 24.92 | 24.26 / 23.64 | 0.08 / 0.08 | 4.86 / 0.36 | 20.57 / 17.92 | 2.24 / 1.47 |  |  |  | 1.11 / 1.08 | 0.49 / 0.44 |  | 0.13 / 0.13 | 0.06 / 0.06 | 0.27 / 0.27 | 0.13 / 0.08 | 22.54 / 20.20 | 34.01 / 30.20 | 12,910 | 8,897 | 2,775 | P0 | OK:30,OK:30,OK:30 |

## Repeat noise between rounds (Remastered shots)

Per pair of rounds of the same view on the same build: max channel difference / pixels over 2 /
pixels over 8, per row band (sky 0-422, horizon 422-591, ground 591-1080, whole frame). capBegin
pins the sim clock to the time each round starts, and the rounds are minutes apart, so sky and
cloud drift, sea animation, fire and smoke differ between rounds. This shows how far one view
moves inside one game; it is NOT the gate item 5 noise floor (that is a pinned same-view repeat).

| view | pair | sky | horizon | ground | whole |
|---|---|---|---|---|---|
