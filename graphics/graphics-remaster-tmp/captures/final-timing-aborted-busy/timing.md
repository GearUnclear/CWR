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
| sunrise_ridge | classic | 1 | 26.24 / 26.24 | 26.09 / 26.09 |  | 7.96 / 7.96 | 15.17 / 15.17 | 2.96 / 2.96 |  |  |  |  |  |  |  |  |  |  | 25.03 / 25.03 | 34.47 / 34.47 | 13,076 | 8,897 | 2,775 | P0 | OK:30 |
| sunrise_ridge | remastered | 1 | 9.42 / 9.42 | 7.55 / 7.55 | 0.09 / 0.09 | 5.32 / 5.32 | 1.87 / 1.87 | 0.36 / 0.36 | 0.12 / 0.12 |  |  | 1.57 / 1.57 | 0.43 / 0.43 | 0.01 / 0.01 | 0.13 / 0.13 | 0.06 / 0.06 |  | 0.04 / 0.04 | 5.06 / 5.06 | 16.26 / 16.26 | 1,912 | 8,897 | 2,775 | P0 | OK:30 |
| forest_edge_golden | classic | 1 | 6.87 / 6.87 | 6.72 / 6.72 |  | 0.09 / 0.09 | 4.49 / 4.49 | 2.14 / 2.14 |  |  |  |  |  |  |  |  |  |  | 4.54 / 4.54 | 9.95 / 9.95 | 2,502 | 1,459 | 2,775 | P0 | OK:30 |
| forest_edge_golden | remastered | 1 | 8.89 / 8.89 | 6.85 / 6.85 | 0.30 / 0.30 | 2.07 / 2.07 | 3.50 / 3.50 | 1.29 / 1.29 | 0.12 / 0.12 |  |  | 1.71 / 1.71 | 0.41 / 0.41 | 0.01 / 0.01 | 0.13 / 0.13 | 0.23 / 0.23 |  | 0.03 / 0.03 | 5.78 / 5.78 | 11.30 / 11.30 | 866 | 1,459 | 2,775 | P0 | OK:30 |

## Repeat noise between rounds (Remastered shots)

Per pair of rounds of the same view on the same build: max channel difference / pixels over 2 /
pixels over 8, per row band (sky 0-422, horizon 422-591, ground 591-1080, whole frame). capBegin
pins the sim clock to the time each round starts, and the rounds are minutes apart, so sky and
cloud drift, sea animation, fire and smoke differ between rounds. This shows how far one view
moves inside one game; it is NOT the gate item 5 noise floor (that is a pinned same-view repeat).

| view | pair | sky | horizon | ground | whole |
|---|---|---|---|---|---|
