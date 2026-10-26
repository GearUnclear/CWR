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

| view | mode | n | frame | world | shadow | ground | pass1 | rest | post | resolve | lindepth | ao | bloom | shafts | composite | frame_min | CPU frame | draws | objects | GPU MHz | P-states | settle |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| sunrise_ridge | off | 1 | 19.34 / 19.34 | 18.15 / 18.15 | 0.08 / 0.08 | 0.27 / 0.27 | 15.73 / 15.73 | 2.14 / 2.14 | 1.03 / 1.03 | 0.52 / 0.52 |  | 0.13 / 0.13 | 0.06 / 0.06 | 0.27 / 0.27 | 0.06 / 0.06 | 18.68 / 18.68 | 29.06 / 29.06 | 12,910 | 8,897 | 2,790 | P0 | OK:30 |
| sunrise_ridge | on | 1 | 4.00 / 4.00 | 2.63 / 2.63 | 0.14 / 0.14 | 0.40 / 0.40 | 1.79 / 1.79 | 0.44 / 0.44 | 1.17 / 1.17 | 0.38 / 0.38 |  | 0.21 / 0.21 | 0.10 / 0.10 | 0.40 / 0.40 | 0.09 / 0.09 | 2.34 / 2.34 | 14.25 / 14.25 | 1,895 | 8,897 | 2,790 | P0 | OK:30 |
| sunrise_walk | off | 1 | 20.95 / 20.95 | 19.78 / 19.78 | 0.10 / 0.10 | 3.49 / 3.49 | 15.19 / 15.19 | 1.10 / 1.10 | 1.01 / 1.01 | 0.47 / 0.47 |  | 0.12 / 0.12 | 0.06 / 0.06 | 0.27 / 0.27 | 0.08 / 0.08 | 16.96 / 16.96 | 26.82 / 26.82 | 12,939 | 8,936 | 2,790 | P0 | OK:30 |
| sunrise_walk | on | 1 | 5.32 / 5.32 | 3.59 / 3.59 | 0.28 / 0.28 | 0.56 / 0.56 | 2.45 / 2.45 | 0.59 / 0.59 | 1.47 / 1.47 | 0.41 / 0.41 |  | 0.23 / 0.23 | 0.13 / 0.13 | 0.56 / 0.56 | 0.15 / 0.15 | 2.31 / 2.31 | 15.18 / 15.18 | 1,920 | 8,936 | 1,200 | P3 | OK:30 |

## Repeat noise between rounds (Remastered shots)

Per pair of rounds of the same view on the same build: max channel difference / pixels over 2 /
pixels over 8, per row band (sky 0-422, horizon 422-591, ground 591-1080, whole frame). capBegin
pins the sim clock to the time each round starts, and the rounds are minutes apart, so sky and
cloud drift, sea animation, fire and smoke differ between rounds. This shows how far one view
moves inside one game; it is NOT the gate item 5 noise floor (that is a pinned same-view repeat).

| view | pair | sky | horizon | ground | whole |
|---|---|---|---|---|---|
