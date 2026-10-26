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
| valley_0515 | on | 3 | 31.14 / 29.12 | 29.51 / 27.83 | 0.04 / 0.04 |  |  |  |  |  |  | 1.48 / 1.15 | 0.69 / 0.66 |  | 0.06 / 0.06 | 0.07 / 0.07 | 0.31 / 0.31 | 0.35 / 0.06 |  | 52.10 / 33.19 | 11,406 | 7,966 | 2,775 | P0 | OK:30,OK:30,OK:30 |
| valley_0515 | base | 3 | 33.86 / 30.31 | 32.18 / 28.64 | 0.04 / 0.04 |  |  |  |  |  |  | 1.54 / 1.52 | 0.76 / 0.69 |  | 0.06 / 0.06 | 0.07 / 0.07 | 0.31 / 0.31 | 0.40 / 0.31 |  | 37.79 / 33.91 | 11,406 | 7,966 | 2,775 | P0 | OK:30,OK:30,OK:30 |
| sunrise_ridge | on | 3 | 25.48 / 24.99 | 24.20 / 23.77 | 0.13 / 0.09 |  |  |  |  |  |  | 1.12 / 1.06 | 0.53 / 0.45 |  | 0.13 / 0.13 | 0.06 / 0.06 | 0.27 / 0.27 | 0.20 / 0.05 |  | 36.99 / 33.79 | 12,910 | 8,897 | 2,775 | P0 | OK:30,OK:30,OK:30 |
| sunrise_ridge | base | 3 | 37.02 / 22.54 | 35.26 / 21.23 | 0.27 / 0.13 |  |  |  |  |  |  | 1.60 / 1.14 | 0.57 / 0.47 |  | 0.13 / 0.13 | 0.06 / 0.06 | 0.27 / 0.27 | 0.56 / 0.21 |  | 41.31 / 37.64 | 12,910 | 8,897 | 2,775 | P0 | OK:30,OK:30,OK:30 |

## Repeat noise between rounds (Remastered shots)

Per pair of rounds of the same view on the same build: max channel difference / pixels over 2 /
pixels over 8, per row band (sky 0-422, horizon 422-591, ground 591-1080, whole frame). capBegin
pins the sim clock to the time each round starts, and the rounds are minutes apart, so sky and
cloud drift, sea animation, fire and smoke differ between rounds. This shows how far one view
moves inside one game; it is NOT the gate item 5 noise floor (that is a pinned same-view repeat).

| view | pair | sky | horizon | ground | whole |
|---|---|---|---|---|---|
