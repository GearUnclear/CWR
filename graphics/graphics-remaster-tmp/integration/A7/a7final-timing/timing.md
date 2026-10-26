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

| view | mode | n | frame | world | shadow | sky | skytable | post | resolve | lindepth | ao | bloom | shafts | composite | CPU frame | draws | objects | GPU MHz | P-states | settle |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| coastline_sunset | off | 3 | 3.98 / 3.86 | 1.55 / 1.30 | 0.00 / 0.00 |  |  | 2.32 / 2.18 | 0.44 / 0.42 |  | 0.22 / 0.22 | 0.15 / 0.14 | 1.03 / 0.98 | 0.34 / 0.23 | 4.12 / 4.03 | 175 | 0 | 1,080 | P3 | OK:30,OK:30,OK:30 |
| coastline_sunset | on | 3 | 5.25 / 5.19 | 2.04 / 1.95 | 0.00 / 0.00 | 0.27 / 0.27 |  | 2.62 / 2.57 | 0.46 / 0.44 |  | 0.28 / 0.28 | 0.18 / 0.17 | 1.34 / 1.33 | 0.34 / 0.26 | 5.24 / 5.12 | 175 | 0 | 780 | P3 | OK:30,OK:30,OK:30 |
| sunrise_ridge | off | 3 | 28.58 / 28.18 | 27.20 / 26.85 | 0.08 / 0.08 |  |  | 1.17 / 1.08 | 0.63 / 0.53 |  | 0.13 / 0.13 | 0.06 / 0.06 | 0.27 / 0.27 | 0.09 / 0.09 | 32.37 / 31.64 | 12,910 | 8,897 | 2,790 | P0 | OK:30,OK:30,OK:30 |
| sunrise_ridge | on | 3 | 30.97 / 28.77 | 29.45 / 27.46 | 0.08 / 0.08 | 0.10 / 0.10 |  | 1.20 / 1.09 | 0.66 / 0.56 |  | 0.13 / 0.13 | 0.06 / 0.06 | 0.27 / 0.27 | 0.08 / 0.07 | 34.05 / 31.63 | 12,910 | 8,897 | 2,790 | P0 | OK:30,OK:30,OK:30 |
| noon_fields | off | 3 | 10.13 / 10.12 | 9.46 / 9.44 | 0.06 / 0.06 |  |  | 0.53 / 0.52 | 0.31 / 0.30 |  | 0.11 / 0.11 | 0.06 / 0.06 |  | 0.04 / 0.04 | 17.03 / 17.01 | 6,347 | 4,506 | 2,790 | P0 | OK:30,OK:30,OK:30 |
| noon_fields | on | 3 | 10.78 / 10.45 | 10.03 / 9.72 | 0.06 / 0.06 | 0.08 / 0.08 |  | 0.55 / 0.53 | 0.33 / 0.31 |  | 0.11 / 0.11 | 0.06 / 0.06 |  | 0.04 / 0.04 | 17.48 / 17.45 | 6,347 | 4,506 | 2,790 | P0 | OK:30,OK:30,OK:30 |

## Repeat noise between rounds (Remastered shots)

Per pair of rounds of the same view on the same build: max channel difference / pixels over 2 /
pixels over 8, per row band (sky 0-422, horizon 422-591, ground 591-1080, whole frame). capBegin
pins the sim clock to the time each round starts, and the rounds are minutes apart, so sky and
cloud drift, sea animation, fire and smoke differ between rounds. This shows how far one view
moves inside one game; it is NOT the gate item 5 noise floor (that is a pinned same-view repeat).

| view | pair | sky | horizon | ground | whole |
|---|---|---|---|---|---|
