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
| valley_0515 | on | 3 | 26.29 / 25.75 | 23.42 / 23.13 | 0.04 / 0.04 | 2.26 / 2.25 | 18.94 / 18.88 | 2.29 / 1.93 | 0.09 / 0.09 |  |  | 2.60 / 2.35 | 0.17 / 0.16 | 0.01 / 0.01 | 0.06 / 0.06 | 0.06 / 0.06 |  | 0.05 / 0.04 | 25.47 / 24.54 | 35.70 / 34.02 | 5,242 | 7,966 | 2,775 | P0 | OK:30,OK:30,OK:30 |
| valley_0515 | base | 3 | 34.34 / 32.81 | 32.85 / 31.44 | 0.04 / 0.04 | 5.55 / 5.48 | 24.79 / 24.36 | 1.91 / 1.60 |  |  |  | 1.35 / 1.23 | 0.85 / 0.74 |  | 0.06 / 0.06 | 0.07 / 0.07 | 0.31 / 0.31 | 0.06 / 0.06 | 33.26 / 32.02 | 38.21 / 35.41 | 11,406 | 7,966 | 2,775 | P0 | OK:30,OK:30,OK:30 |
| sunrise_ridge | on | 3 | 14.19 / 9.95 | 11.50 / 8.14 | 0.08 / 0.08 | 6.18 / 5.59 | 2.56 / 2.14 | 0.66 / 0.41 | 0.12 / 0.12 |  |  | 2.37 / 1.51 | 0.40 / 0.26 | 0.01 / 0.01 | 0.13 / 0.13 | 0.06 / 0.06 |  | 0.04 / 0.03 | 5.09 / 5.05 | 18.56 / 16.28 | 1,912 | 8,897 | 2,775 | P0 | OK:30,OK:30,OK:30 |
| sunrise_ridge | base | 3 | 40.85 / 22.55 | 38.94 / 21.44 | 0.14 / 0.08 | 5.25 / 0.78 | 32.48 / 14.90 | 2.44 / 1.29 |  |  |  | 1.65 / 0.95 | 0.59 / 0.44 |  | 0.13 / 0.13 | 0.06 / 0.06 | 0.27 / 0.27 | 0.58 / 0.05 | 23.24 / 17.07 | 42.84 / 27.65 | 12,910 | 8,897 | 2,775 | P0 | OK:30,OK:30,OK:30 |

## Repeat noise between rounds (Remastered shots)

Per pair of rounds of the same view on the same build: max channel difference / pixels over 2 /
pixels over 8, per row band (sky 0-422, horizon 422-591, ground 591-1080, whole frame). capBegin
pins the sim clock to the time each round starts, and the rounds are minutes apart, so sky and
cloud drift, sea animation, fire and smoke differ between rounds. This shows how far one view
moves inside one game; it is NOT the gate item 5 noise floor (that is a pinned same-view repeat).

| view | pair | sky | horizon | ground | whole |
|---|---|---|---|---|---|
