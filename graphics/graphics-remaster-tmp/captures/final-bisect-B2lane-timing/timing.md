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
| valley_0515 | on | 2 | 20.93 / 20.63 | 19.36 / 19.09 | 0.04 / 0.04 | 1.52 / 1.52 | 15.93 / 15.85 | 1.90 / 1.71 | 0.08 / 0.08 |  |  | 1.39 / 1.35 | 0.65 / 0.61 |  | 0.06 / 0.06 | 0.07 / 0.07 | 0.32 / 0.32 | 0.04 / 0.04 | 20.43 / 20.14 | 28.98 / 28.72 | 5,242 | 7,966 | 2,790 | P0 | OK:30,OK:30 |
| valley_0515 | base | 2 | 24.66 / 24.60 | 23.24 / 23.06 | 0.04 / 0.04 | 0.50 / 0.50 | 21.11 / 21.03 | 1.63 / 1.52 |  |  |  | 1.28 / 1.16 | 0.79 / 0.67 |  | 0.06 / 0.06 | 0.07 / 0.07 | 0.31 / 0.31 | 0.06 / 0.06 | 24.04 / 23.93 | 32.80 / 32.46 | 11,406 | 7,966 | 2,790 | P0 | OK:30,OK:30 |

## Repeat noise between rounds (Remastered shots)

Per pair of rounds of the same view on the same build: max channel difference / pixels over 2 /
pixels over 8, per row band (sky 0-422, horizon 422-591, ground 591-1080, whole frame). capBegin
pins the sim clock to the time each round starts, and the rounds are minutes apart, so sky and
cloud drift, sea animation, fire and smoke differ between rounds. This shows how far one view
moves inside one game; it is NOT the gate item 5 noise floor (that is a pinned same-view repeat).

| view | pair | sky | horizon | ground | whole |
|---|---|---|---|---|---|
