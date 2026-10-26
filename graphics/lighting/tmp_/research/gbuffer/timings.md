# gbuffer track: GPU timings

Hardware and setup: NVIDIA GeForce RTX 4060 Ti (from the game log), 960x640 window, render scale 1.5
(1440x960), 4x MSAA, `triCaptureAtPresent`. GPU numbers are the engine's smoothed timestamp-query
averages (`triGpuTimes`). The GPU was shared with up to three other research games the whole time
(tri slot log), so the world-pass numbers swing by up to 2x between runs and between rounds of the
same run. The post-pass segments are short and much more stable; trust them, not the world deltas.

Modes: `rm_vbo` = Remastered + late vertex buffers (the baseline every G-buffer mode builds on),
`gbufonly` = + MRT G-buffer writes and resolve, GI off (stock AO still runs), `v2gi` = + GI v2
(r 6 m, 8 directions x 14 steps, half resolution, temporal accumulation, 2-pass bilateral blur).

## Interleaved rounds (each mode once per round, 120 settle frames, no screenshots)

run06_v2 (4 rounds):

| view | mode | frame median | world median | post median | ao/gi median |
|---|---|---|---|---|---|
| forest_edge_golden | rm_vbo | 15.70 | 15.24 | 0.32 | 0.24 |
| forest_edge_golden | gbufonly | 16.89 | 15.14 | 1.75 | 0.39 |
| forest_edge_golden | v2gi | 23.86 | 20.33 | 2.86 | 2.75 |
| lowsun_fp | rm_vbo | 17.62 | 15.52 | 1.98 | 0.30 |
| lowsun_fp | gbufonly | 12.59 | 10.95 | 1.57 | 0.41 |
| lowsun_fp | v2gi | 12.40 | 9.00 | 3.35 | 2.15 |
| town_day | rm_vbo | 8.06 | 7.60 | 0.33 | 0.24 |
| town_day | gbufonly | 13.29 | 10.88 | 2.29 | 0.37 |
| town_day | v2gi | 13.18 | 9.99 | 2.99 | 2.63 |

run07_v3 (8 rounds):

| view | mode | frame median | frame min | world median | world min | post median | ao/gi median |
|---|---|---|---|---|---|---|---|
| lowsun_fp | rm_vbo | 15.34 | 8.38 | 13.41 | 6.56 | 1.79 | 0.18 |
| lowsun_fp | gbufonly | 16.38 | 16.21 | 14.26 | 14.04 | 2.01 | 0.31 |
| lowsun_fp | v2gi | 18.17 | 16.17 | 14.04 | 12.54 | 3.81 | 2.23 |
| noon_fields | rm_vbo | 25.43 | 13.73 | 25.07 | 13.30 | 0.25 | 0.17 |
| noon_fields | gbufonly | 28.49 | 12.36 | 27.93 | 11.80 | 0.42 | 0.34 |
| noon_fields | v2gi | 26.86 | 16.65 | 24.82 | 14.98 | 1.98 | 1.88 |
| town_day | rm_vbo | 6.43 | 2.35 | 5.94 | 1.63 | 0.38 | 0.29 |
| town_day | gbufonly | 7.54 | 5.31 | 6.16 | 4.19 | 1.35 | 0.42 |
| town_day | v2gi | 9.00 | 4.59 | 6.53 | 2.75 | 2.08 | 1.98 |

Full per-round tables: `run06_v2/timing_rounds.md`, `run07_v3/timing_rounds.md` (+ `.json`).

## What the numbers support

| item | cost | confidence |
|---|---|---|
| GI pass (the `ao/gi` segment: ray march + temporal + 2 blur passes, half res) | 1.6 to 3.1 ms, vs 0.17 to 0.6 ms for the stock AO it replaces, so about +1.5 to +2.5 ms | good (seen on every shot of every run) |
| GI at r 15 m, 20 steps | 1.8 to 2.6 ms, the same band as r 6 m / 14 steps | good |
| G-buffer resolve (3 multisample blits) | post segment +0.2 to +2.0 ms (gbufonly minus rm_vbo, 6 view/run pairs: +1.43, -0.41, +1.96, +0.22, +0.17, +0.97) | fair: noisy, the theory below says about 0.3 to 0.5 ms |
| MRT writes in the world pass | not resolvable here: median world deltas -4.6 to +3.3 ms across the 6 pairs | poor: drowned by GPU sharing |
| Whole prototype on top of Remastered | roughly +3 to +5 ms GPU at 1440x960 4x MSAA | estimate |

Bandwidth estimate for the MRT (to sanity-check the unresolvable world delta): 1440 x 960 x 4 samples
= 5.5 M samples; the three extra attachments are 8 + 8 + 4 = 20 bytes per sample, so about 110 MB
per full-screen layer of writes, another 110 MB for the per-frame clear, and 110 MB read + 28 MB
written by the resolve. At the 4060 Ti's 288 GB/s that is about 0.4 ms per pass over the data, so
with typical overdraw the G-buffer should cost about 1 to 2 ms in total before colour compression.
A packed layout (octahedral normal RG16 + roughness, R11G11B10F radiance, no third target) would
roughly halve it.

## Per-shot tables

`run05_main/timings_gb.md`, `run06_v2/timings_gb.md`, `run07_v3/timings_gb.md`, and every BEST and
comparison sheet prints the GPU frame / world / post / ao-gi numbers of its own shot on each panel.
