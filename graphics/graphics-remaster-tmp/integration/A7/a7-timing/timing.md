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
| coastline_sunset | off | 3 | 3.53 / 3.49 | 1.39 / 1.27 | 0.00 / 0.00 |  |  | 1.98 / 1.90 | 0.33 / 0.31 |  | 0.23 / 0.22 | 0.15 / 0.15 | 1.06 / 1.05 | 0.22 / 0.16 | 3.55 / 3.53 | 175 | 0 | 930 | P3 | OK:30,OK:30,OK:30 |
| coastline_sunset | on | 3 | 5.07 / 4.81 | 2.05 / 1.90 | 0.00 / 0.00 | 0.28 / 0.28 |  | 2.47 / 2.38 | 0.33 / 0.33 |  | 0.28 / 0.28 | 0.18 / 0.18 | 1.32 / 1.31 | 0.28 / 0.27 | 4.71 / 4.65 | 175 | 0 | 802 | P3 | OK:30,OK:30,OK:30 |
| sunrise_ridge | off | 3 | 24.37 / 24.33 | 23.23 / 23.19 | 0.08 / 0.08 |  |  | 0.98 / 0.98 | 0.44 / 0.44 |  | 0.13 / 0.13 | 0.06 / 0.06 | 0.27 / 0.27 | 0.09 / 0.08 | 26.86 / 26.78 | 12,910 | 8,897 | 2,775 | P0 | OK:30,OK:30,OK:30 |
| sunrise_ridge | on | 3 | 24.86 / 24.81 | 23.66 / 23.62 | 0.08 / 0.08 | 0.10 / 0.10 |  | 1.00 / 0.99 | 0.47 / 0.44 |  | 0.13 / 0.13 | 0.06 / 0.06 | 0.27 / 0.27 | 0.08 / 0.06 | 27.08 / 26.89 | 12,910 | 8,897 | 2,775 | P0 | OK:30,OK:30,OK:30 |
| noon_fields | off | 3 | 9.54 / 9.43 | 8.96 / 8.86 | 0.06 / 0.06 |  |  | 0.44 / 0.43 | 0.23 / 0.22 |  | 0.11 / 0.11 | 0.06 / 0.06 |  | 0.04 / 0.04 | 14.73 / 14.67 | 6,347 | 4,506 | 2,775 | P0 | OK:30,OK:30,OK:30 |
| noon_fields | on | 3 | 10.07 / 9.98 | 9.43 / 9.39 | 0.06 / 0.06 | 0.08 / 0.08 |  | 0.46 / 0.42 | 0.24 / 0.20 |  | 0.11 / 0.11 | 0.06 / 0.06 |  | 0.04 / 0.04 | 15.20 / 15.16 | 6,347 | 4,506 | 2,775 | P0 | OK:30,OK:30,OK:30 |

## Repeat noise between rounds (Remastered shots)

Per pair of rounds of the same view on the same build: max channel difference / pixels over 2 /
pixels over 8, per row band (sky 0-422, horizon 422-591, ground 591-1080, whole frame). capBegin
pins the sim clock to the time each round starts, and the rounds are minutes apart, so sky and
cloud drift, sea animation, fire and smoke differ between rounds. This shows how far one view
moves inside one game; it is NOT the gate item 5 noise floor (that is a pinned same-view repeat).

| view | pair | sky | horizon | ground | whole |
|---|---|---|---|---|---|
