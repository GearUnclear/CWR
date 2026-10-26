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
| sinai_lowsun_fp | classic | 3 | 3.24 / 3.10 | 3.01 / 2.86 |  | 0.21 / 0.20 | 1.51 / 1.47 | 1.18 / 1.11 |  |  |  |  |  |  |  |  |  |  | 2.22 / 2.15 | 6.94 / 6.92 | 789 | 371 | 2,280 | P0 | OK:30,OK:30,OK:30 |
| sinai_lowsun_fp | remastered | 3 | 6.35 / 5.82 | 4.25 / 3.83 | 0.27 / 0.24 | 1.16 / 1.05 | 2.10 / 1.92 | 0.97 / 0.86 | 0.12 / 0.11 |  |  | 1.77 / 1.70 | 0.44 / 0.39 | 0.01 / 0.01 | 0.18 / 0.16 | 0.08 / 0.07 |  | 0.04 / 0.04 | 6.00 / 5.49 | 9.59 / 8.65 | 497 | 371 | 1,822 | P0/P3 | OK:30,OK:30,OK:30 |
| sinai_desert_noon | classic | 3 | 7.25 / 7.05 | 7.09 / 6.89 |  | 0.18 / 0.18 | 3.25 / 3.23 | 3.54 / 3.46 |  |  |  |  |  |  |  |  |  |  | 6.20 / 6.08 | 12.08 / 11.72 | 2,689 | 1,363 | 2,790 | P0 | OK:30,OK:30,OK:30 |
| sinai_desert_noon | remastered | 3 | 6.96 / 6.94 | 5.35 / 5.34 | 0.10 / 0.10 | 1.08 / 1.03 | 2.66 / 2.58 | 1.64 / 1.60 | 0.11 / 0.11 |  |  | 1.31 / 1.30 | 0.63 / 0.55 | 0.01 / 0.01 | 0.12 / 0.12 | 0.06 / 0.06 |  | 0.03 / 0.03 | 6.29 / 6.27 | 11.90 / 11.86 | 1,266 | 1,363 | 2,790 | P0 | OK:30,OK:30,OK:30 |

## Repeat noise between rounds (Remastered shots)

Per pair of rounds of the same view on the same build: max channel difference / pixels over 2 /
pixels over 8, per row band (sky 0-422, horizon 422-591, ground 591-1080, whole frame). capBegin
pins the sim clock to the time each round starts, and the rounds are minutes apart, so sky and
cloud drift, sea animation, fire and smoke differ between rounds. This shows how far one view
moves inside one game; it is NOT the gate item 5 noise floor (that is a pinned same-view repeat).

| view | pair | sky | horizon | ground | whole |
|---|---|---|---|---|---|
| sinai_lowsun_fp | r1-r2 | 193 / 23,391 / 3,399 | 96 / 75,769 / 18,006 | 21 / 148,066 / 13,579 | 193 / 247,226 / 34,984 |
| sinai_lowsun_fp | r1-r3 | 201 / 28,484 / 4,504 | 67 / 79,885 / 16,578 | 24 / 154,119 / 13,799 | 201 / 262,488 / 34,881 |
| sinai_lowsun_fp | r2-r3 | 107 / 18,025 / 2,981 | 98 / 23,053 / 2,572 | 34 / 19,941 / 121 | 107 / 61,019 / 5,674 |
| sinai_desert_noon | r1-r2 | 62 / 1,149 / 268 | 78 / 42,404 / 2,691 | 56 / 102,014 / 3,594 | 78 / 145,567 / 6,553 |
| sinai_desert_noon | r1-r3 | 59 / 1,150 / 273 | 79 / 45,721 / 2,176 | 176 / 98,569 / 2,473 | 176 / 145,440 / 4,922 |
| sinai_desert_noon | r2-r3 | 46 / 1,037 / 265 | 78 / 36,022 / 2,162 | 176 / 70,059 / 2,211 | 176 / 107,118 / 4,638 |
