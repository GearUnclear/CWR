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
| valley_0515 | on | 2 | 15.68 / 15.45 | 14.49 / 14.29 | 0.04 / 0.04 | 0.45 / 0.45 | 12.64 / 12.59 | 1.40 / 1.24 |  |  |  | 1.08 / 1.06 | 0.58 / 0.56 |  | 0.06 / 0.06 | 0.07 / 0.07 | 0.31 / 0.31 | 0.06 / 0.06 | 15.12 / 14.91 | 24.14 / 23.49 | 5,242 | 7,966 | 2,790 | P0 | OK:30,OK:30 |
| valley_0515 | base | 2 | 26.16 / 22.41 | 24.79 / 21.11 | 0.04 / 0.04 | 3.83 / 0.46 | 19.40 / 19.24 | 1.57 / 1.42 |  |  |  | 1.24 / 1.16 | 0.75 / 0.67 |  | 0.06 / 0.06 | 0.07 / 0.07 | 0.31 / 0.31 | 0.06 / 0.06 | 25.27 / 21.79 | 34.03 / 30.17 | 11,406 | 7,966 | 2,790 | P0 | OK:30,OK:30 |

## Repeat noise between rounds (Remastered shots)

Per pair of rounds of the same view on the same build: max channel difference / pixels over 2 /
pixels over 8, per row band (sky 0-422, horizon 422-591, ground 591-1080, whole frame). capBegin
pins the sim clock to the time each round starts, and the rounds are minutes apart, so sky and
cloud drift, sea animation, fire and smoke differ between rounds. This shows how far one view
moves inside one game; it is NOT the gate item 5 noise floor (that is a pinned same-view repeat).

| view | pair | sky | horizon | ground | whole |
|---|---|---|---|---|---|
