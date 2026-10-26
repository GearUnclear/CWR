# Interleaved GPU timing (tiling)

Each rep shoots v0 and the variants back to back on the same view (120 frames each);
the delta is variant minus v0 within the same rep. Other agents' games share the GPU,
so absolute numbers are inflated; the paired median delta is the cost estimate.

## noon_fields

| rep | mode | GPU frame ms | world ms |
|---|---|---|---|
| - | classic | 0.99 | 0.89 |
| r0 | v0 | 3.44 | 2.96 |
| r0 | v1 | 3.60 | 3.07 |
| r0 | v4 | 4.45 | 3.95 |
| r1 | v0 | 2.69 | 2.22 |
| r1 | v1 | 4.58 | 4.16 |
| r1 | v4 | 4.47 | 3.99 |
| r2 | v0 | 3.84 | 3.27 |
| r2 | v1 | 4.62 | 4.25 |
| r2 | v4 | 2.83 | 2.36 |
| r3 | v0 | 3.31 | 2.86 |
| r3 | v1 | 4.40 | 3.91 |
| r3 | v4 | 3.87 | 3.31 |

- v1 minus v0: frame median +0.94 ms (n=4, range +0.16..+1.89); world median +1.02 ms
- v4 minus v0: frame median +0.79 ms (n=4, range -1.01..+1.78); world median +0.72 ms
- v0 frame median 3.38 ms
- classic frame 0.99 ms (single read)

## high_fields_noon

| rep | mode | GPU frame ms | world ms |
|---|---|---|---|
| - | classic | 2.45 | 2.37 |
| r0 | v0 | 3.92 | 3.57 |
| r0 | v1 | 5.56 | 5.22 |
| r0 | v4 | 6.01 | 5.63 |
| r1 | v0 | 3.43 | 2.99 |
| r1 | v1 | 6.62 | 6.25 |
| r1 | v4 | 3.89 | 3.47 |
| r2 | v0 | 3.88 | 3.45 |
| r2 | v1 | 6.94 | 6.61 |
| r2 | v4 | 9.53 | 9.22 |
| r3 | v0 | 6.45 | 6.13 |
| r3 | v1 | 6.38 | 5.99 |
| r3 | v4 | 5.81 | 5.45 |

- v1 minus v0: frame median +2.35 ms (n=4, range -0.07..+3.19); world median +2.41 ms
- v4 minus v0: frame median +1.27 ms (n=4, range -0.64..+5.65); world median +1.27 ms
- v0 frame median 3.90 ms
- classic frame 2.45 ms (single read)

## forest_edge_golden

| rep | mode | GPU frame ms | world ms |
|---|---|---|---|
| - | classic | 1.36 | 1.23 |
| r0 | v0 | 4.99 | 4.58 |
| r0 | v1 | 7.01 | 6.44 |
| r0 | v4 | 4.75 | 4.25 |
| r1 | v0 | 4.48 | 3.94 |
| r1 | v1 | 4.66 | 4.06 |
| r1 | v4 | 6.59 | 6.09 |
| r2 | v0 | 4.21 | 3.73 |
| r2 | v1 | 3.62 | 3.11 |
| r2 | v4 | 3.64 | 3.13 |
| r3 | v0 | 5.49 | 5.06 |
| r3 | v1 | 6.73 | 6.29 |
| r3 | v4 | 6.61 | 6.21 |

- v1 minus v0: frame median +0.71 ms (n=4, range -0.59..+2.02); world median +0.68 ms
- v4 minus v0: frame median +0.44 ms (n=4, range -0.57..+2.11); world median +0.41 ms
- v0 frame median 4.74 ms
- classic frame 1.36 ms (single read)

