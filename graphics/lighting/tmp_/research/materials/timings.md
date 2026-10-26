# GPU timings, materials track

Shared GPU: up to four games from other research agents ran at the same time, so single readings swing by 2x or more (Classic noon_fields read 11.0 ms in r6, 19.6 ms in r7, 6.5 ms in r8). The honest number is the paired delta: each materials shot minus the Remastered shot taken seconds before it on the same view.

## Paired deltas (materials incl. vbfix minus Remastered, adjacent shots)

```
## runs/r6
abel_rain              n=3 frame delta median +2.38 ms  all [2.38, 5.49, -0.51]  world +1.71  ao(SSR) +0.41
lowsun_fp              n=3 frame delta median +0.15 ms  all [0.15, 0.15, 0.02]  world -0.01  ao(SSR) +0.08
noon_fields            n=1 frame delta median +5.97 ms  all [5.97]  world +6.01  ao(SSR) -0.03
overcast_heavy_rain    n=1 frame delta median +1.43 ms  all [1.43]  world +1.78  ao(SSR) -0.35
sinai_lowsun_fp        n=1 frame delta median -0.31 ms  all [-0.31]  world +0.04  ao(SSR) +0.68
tank_gunner_optics     n=1 frame delta median -0.62 ms  all [-0.62]  world -0.87  ao(SSR) -0.11
## runs/r7
abel_rain              n=3 frame delta median -0.20 ms  all [-0.2, 1.51, -0.33]  world -0.50  ao(SSR) +0.25
lowsun_fp              n=3 frame delta median +0.91 ms  all [-3.21, 0.91, 1.98]  world +1.02  ao(SSR) +0.15
noon_fields            n=3 frame delta median -1.55 ms  all [-1.55, -3.17, 3.3]  world -1.56  ao(SSR) +0.02
overcast_heavy_rain    n=1 frame delta median +0.77 ms  all [0.77]  world +0.82  ao(SSR) -0.06
sinai_lowsun_fp        n=1 frame delta median -2.37 ms  all [-2.37]  world -2.28  ao(SSR) -0.15
tank_gunner_optics     n=1 frame delta median +0.86 ms  all [0.86]  world +0.50  ao(SSR) +0.01
town_night             n=3 frame delta median +0.10 ms  all [-0.69, 7.66, 0.1]  world +0.13  ao(SSR) -0.05
town_night_rain        n=1 frame delta median -0.37 ms  all [-0.37]  world -1.31  ao(SSR) +0.16
## runs/r8
abel_rain              n=3 frame delta median +0.54 ms  all [0.34, 0.54, 1.48]  world +0.36  ao(SSR) +0.16
lowsun_fp              n=3 frame delta median +0.87 ms  all [0.87, 4.04, 0.61]  world +0.81  ao(SSR) -0.04
noon_fields            n=3 frame delta median +0.01 ms  all [0.01, -2.04, 2.5]  world -0.01  ao(SSR) -0.01
sinai_lowsun_fp        n=1 frame delta median -4.45 ms  all [-4.45]  world -4.41  ao(SSR) +0.00
town_night             n=3 frame delta median +1.14 ms  all [0.37, 1.14, 1.37]  world +1.16  ao(SSR) +0.01
town_night_rain        n=1 frame delta median -1.55 ms  all [-1.55]  world -2.11  ao(SSR) +0.41
## runs/r9
town_night             n=3 frame delta median +0.24 ms  all [2.45, -4.61, 0.24]  world +0.23  ao(SSR) +0.03
town_night_rain        n=1 frame delta median +2.50 ms  all [2.5]  world +2.19  ao(SSR) +0.30
```

Pooled over the 33 pairs from the views shot with three repeats (r6 to r9, shaders v6 to v9; the single-pair views are left out): median +0.37 ms, interquartile range -0.27 to +1.75 ms, at 960x640 x 1.5 render scale, 4x MSAA.

## Medians, run r6

| view | variant | n | GPU frame ms (median) | world ms | post ms (ao bucket incl. SSR) | frame minus Remastered |
|---|---|---|---|---|---|---|
| abel_rain | classic | 1 | 2.01 | 1.89 | None |  |
| abel_rain | remastered | 3 | 2.60 | 1.57 | 0.92 | +0.00 |
| abel_rain | vbfix | 2 | 3.71 | 2.45 | 1.13 | +1.11 |
| abel_rain | materials | 3 | 5.49 | 3.76 | 1.5 | +2.89 |
| abel_rain | wet | 3 | 3.31 | 1.81 | 1.37 | +0.71 |
| abel_rain | wetnossr | 1 | 3.63 | 2.55 | 0.95 | +1.03 |
| lowsun_fp | classic | 1 | 4.72 | 4.61 | None |  |
| lowsun_fp | remastered | 3 | 9.60 | 8.43 | 0.96 | +0.00 |
| lowsun_fp | vbfix | 2 | 9.69 | 8.17 | 1.4 | +0.09 |
| lowsun_fp | materials | 3 | 9.66 | 8.48 | 1.12 | +0.06 |
| lowsun_fp | nobump | 1 | 9.41 | 7.91 | 1.39 | -0.19 |
| lowsun_fp | wet | 3 | 10.62 | 8.96 | 1.53 | +1.02 |
| noon_fields | classic | 1 | 11.03 | 10.93 | None |  |
| noon_fields | remastered | 1 | 16.41 | 15.69 | 0.6 | +0.00 |
| noon_fields | vbfix | 1 | 25.60 | 24.75 | 0.74 | +9.19 |
| noon_fields | materials | 1 | 22.38 | 21.70 | 0.57 | +5.97 |
| noon_fields | wet | 1 | 15.28 | 14.63 | 0.54 | -1.13 |
| overcast_heavy_rain | classic | 1 | 3.16 | 3.05 | None |  |
| overcast_heavy_rain | remastered | 1 | 3.02 | 2.16 | 0.74 | +0.00 |
| overcast_heavy_rain | vbfix | 1 | 3.66 | 3.15 | 0.39 | +0.64 |
| overcast_heavy_rain | materials | 1 | 4.45 | 3.94 | 0.39 | +1.43 |
| overcast_heavy_rain | wet | 1 | 4.80 | 4.33 | 0.35 | +1.78 |
| sinai_lowsun_fp | classic | 1 | 5.21 | 5.10 | None |  |
| sinai_lowsun_fp | remastered | 1 | 13.90 | 11.83 | 1.95 | +0.00 |
| sinai_lowsun_fp | vbfix | 1 | 3.87 | 2.62 | 1.13 | -10.03 |
| sinai_lowsun_fp | materials | 1 | 13.59 | 11.87 | 1.6 | -0.31 |
| tank_gunner_optics | classic | 1 | 6.58 | 5.79 | None |  |
| tank_gunner_optics | remastered | 1 | 11.19 | 10.24 | 0.36 | +0.00 |
| tank_gunner_optics | vbfix | 1 | 13.30 | 11.72 | 0.26 | +2.11 |
| tank_gunner_optics | materials | 1 | 10.57 | 9.37 | 0.24 | -0.62 |

## Medians, run r7

| view | variant | n | GPU frame ms (median) | world ms | post ms (ao bucket incl. SSR) | frame minus Remastered |
|---|---|---|---|---|---|---|
| abel_rain | classic | 1 | 1.86 | 1.75 | None |  |
| abel_rain | remastered | 3 | 2.68 | 1.84 | 0.72 | +0.00 |
| abel_rain | vbfix | 1 | 2.12 | 1.16 | 0.84 | -0.56 |
| abel_rain | materials | 3 | 3.52 | 2.03 | 1.19 | +0.84 |
| abel_rain | wetnossr | 1 | 4.70 | 3.47 | 1.11 | +2.02 |
| lowsun_fp | classic | 1 | 1.98 | 1.88 | None |  |
| lowsun_fp | remastered | 3 | 9.05 | 7.91 | 1.03 | +0.00 |
| lowsun_fp | vbfix | 1 | 5.06 | 4.09 | 0.86 | -3.99 |
| lowsun_fp | materials | 3 | 7.37 | 6.47 | 0.86 | -1.68 |
| lowsun_fp | norim | 1 | 6.43 | 5.50 | 0.82 | -2.62 |
| lowsun_fp | wet | 1 | 6.21 | 4.92 | 1.18 | -2.84 |
| noon_fields | classic | 1 | 19.59 | 19.49 | None |  |
| noon_fields | remastered | 3 | 18.79 | 18.41 | 0.27 | +0.00 |
| noon_fields | vbfix | 1 | 23.72 | 23.37 | 0.25 | +4.93 |
| noon_fields | materials | 3 | 15.62 | 15.24 | 0.27 | -3.17 |
| noon_fields | norim | 1 | 22.41 | 22.02 | 0.28 | +3.62 |
| noon_fields | wet | 1 | 11.03 | 10.34 | 0.55 | -7.76 |
| overcast_heavy_rain | classic | 1 | 1.49 | 1.36 | None |  |
| overcast_heavy_rain | remastered | 1 | 1.56 | 0.80 | 0.65 | +0.00 |
| overcast_heavy_rain | vbfix | 1 | 2.65 | 2.00 | 0.53 | +1.09 |
| overcast_heavy_rain | materials | 1 | 2.33 | 1.62 | 0.59 | +0.77 |
| overcast_heavy_rain | wet | 1 | 2.07 | 1.30 | 0.65 | +0.51 |
| sinai_lowsun_fp | classic | 1 | 2.31 | 2.20 | None |  |
| sinai_lowsun_fp | remastered | 1 | 6.42 | 5.17 | 1.12 | +0.00 |
| sinai_lowsun_fp | vbfix | 1 | 2.61 | 1.66 | 0.83 | -3.81 |
| sinai_lowsun_fp | materials | 1 | 4.05 | 2.89 | 1.04 | -2.37 |
| sinai_lowsun_fp | norim | 1 | 4.98 | 3.69 | 1.17 | -1.44 |
| tank_gunner_optics | classic | 1 | 10.49 | 9.37 | None |  |
| tank_gunner_optics | remastered | 1 | 9.24 | 8.39 | 0.23 | +0.00 |
| tank_gunner_optics | vbfix | 1 | 10.66 | 9.56 | 0.21 | +1.42 |
| tank_gunner_optics | materials | 1 | 10.10 | 8.89 | 0.24 | +0.86 |
| town_night | classic | 1 | 4.19 | 4.05 | None |  |
| town_night | remastered | 3 | 9.88 | 9.37 | 0.36 | +0.00 |
| town_night | materials | 3 | 16.33 | 15.86 | 0.31 | +6.45 |
| town_night | wet | 1 | 6.44 | 5.55 | 0.74 | -3.44 |
| town_night | wetnossr | 1 | 7.18 | 6.54 | 0.44 | -2.70 |
| town_night_rain | classic | 1 | 11.53 | 11.41 | None |  |
| town_night_rain | remastered | 1 | 11.92 | 11.44 | 0.35 | +0.00 |
| town_night_rain | materials | 1 | 11.55 | 10.13 | 1.3 | -0.37 |
| town_night_rain | wetnossr | 1 | 11.01 | 10.52 | 0.36 | -0.91 |

## Medians, run r8

| view | variant | n | GPU frame ms (median) | world ms | post ms (ao bucket incl. SSR) | frame minus Remastered |
|---|---|---|---|---|---|---|
| abel_rain | classic | 1 | 4.57 | 4.45 | None |  |
| abel_rain | remastered | 3 | 5.37 | 3.62 | 1.38 | +0.00 |
| abel_rain | vbfix | 1 | 5.29 | 3.85 | 1.32 | -0.08 |
| abel_rain | materials | 3 | 5.71 | 3.98 | 1.6 | +0.34 |
| abel_rain | wetnossr | 1 | 6.37 | 4.33 | 1.92 | +1.00 |
| lowsun_fp | classic | 1 | 8.65 | 8.54 | None |  |
| lowsun_fp | remastered | 3 | 8.00 | 7.11 | 0.77 | +0.00 |
| lowsun_fp | vbfix | 1 | 11.86 | 10.18 | 1.56 | +3.86 |
| lowsun_fp | materials | 3 | 8.61 | 7.35 | 1.14 | +0.61 |
| lowsun_fp | norim | 1 | 14.23 | 12.76 | 1.36 | +6.23 |
| lowsun_fp | wet | 1 | 8.42 | 7.16 | 1.15 | +0.42 |
| noon_fields | classic | 1 | 6.53 | 6.42 | None |  |
| noon_fields | remastered | 3 | 22.24 | 21.81 | 0.27 | +0.00 |
| noon_fields | vbfix | 1 | 19.04 | 18.64 | 0.28 | -3.20 |
| noon_fields | materials | 3 | 21.44 | 21.06 | 0.27 | -0.80 |
| noon_fields | norim | 1 | 21.69 | 21.30 | 0.28 | -0.55 |
| noon_fields | wet | 1 | 29.72 | 29.24 | 0.37 | +7.48 |
| sinai_lowsun_fp | classic | 1 | 5.71 | 5.60 | None |  |
| sinai_lowsun_fp | remastered | 1 | 10.45 | 8.97 | 1.36 | +0.00 |
| sinai_lowsun_fp | vbfix | 1 | 6.49 | 5.07 | 1.3 | -3.96 |
| sinai_lowsun_fp | materials | 1 | 6.00 | 4.56 | 1.31 | -4.45 |
| sinai_lowsun_fp | norim | 1 | 6.72 | 4.97 | 1.63 | -3.73 |
| town_night | classic | 1 | 4.88 | 4.73 | None |  |
| town_night | remastered | 3 | 14.60 | 14.13 | 0.3 | +0.00 |
| town_night | materials | 3 | 15.74 | 15.29 | 0.29 | +1.14 |
| town_night | wet | 2 | 12.07 | 10.78 | 1.12 | -2.53 |
| town_night | wetnossr | 1 | 11.36 | 10.81 | 0.39 | -3.24 |
| town_night_rain | classic | 1 | 4.54 | 4.42 | None |  |
| town_night_rain | remastered | 1 | 6.79 | 6.36 | 0.3 | +0.00 |
| town_night_rain | materials | 1 | 5.24 | 4.25 | 0.86 | -1.55 |

## Medians, run r9

| view | variant | n | GPU frame ms (median) | world ms | post ms (ao bucket incl. SSR) | frame minus Remastered |
|---|---|---|---|---|---|---|
| town_night | classic | 1 | 6.93 | 6.77 | None |  |
| town_night | remastered | 3 | 8.85 | 8.21 | 0.48 | +0.00 |
| town_night | materials | 3 | 9.09 | 8.44 | 0.52 | +0.24 |
| town_night | wet | 3 | 7.03 | 6.05 | 0.81 | -1.82 |
| town_night | wetnossr | 1 | 10.86 | 10.35 | 0.34 | +2.01 |
| town_night_rain | classic | 1 | 3.16 | 3.04 | None |  |
| town_night_rain | remastered | 1 | 4.34 | 3.71 | 0.49 | +0.00 |
| town_night_rain | materials | 1 | 6.84 | 5.90 | 0.81 | +2.50 |
| town_night_rain | wetnossr | 1 | 5.22 | 4.58 | 0.5 | +0.88 |

