# Review fix (seaBody takes the physical lamps and low beams): pixels

max channel difference / pixels over 2 levels / pixels over 8 levels per row third; mean luma of each side. Runs: 20261018-160437-before (base 7b7e3e6), 20261018-161927-new (the reviewed change), 20261018-164358-seaprefix (the reviewed build, a copy of its game exe, on the sea views), 20261018-164621-seafix (this fix).

| pair | top third | middle third | bottom third | whole | mean luma a / b |
|---|---|---|---|---|---|
| hero: fix off vs base on (gate 5) | 8 / 175 / 0 | 101 / 25,913 / 4,750 | 6 / 494 / 0 | 101 / 26,582 / 4,750 | 29.3 / 29.3 |
| hero: base off vs base on (noise) | 7 / 123 / 0 | 80 / 21,930 / 3,904 | 5 / 110 / 0 | 80 / 22,163 / 3,904 | 29.3 / 29.3 |
| hero: fix off vs pre-fix off | 2 / 0 / 0 | 93 / 30 / 17 | 0 / 0 / 0 | 93 / 30 / 17 | 29.3 / 29.3 |
| hero: fix on vs pre-fix on | 16 / 623 / 7 | 97 / 13,617 / 812 | 9 / 572 / 2 | 97 / 14,812 / 821 | 52.0 / 52.0 |
| sea_beam: fix off vs pre-fix off | 5 / 27 / 0 | 73 / 8,257 / 1,498 | 82 / 1,576 / 673 | 82 / 9,860 / 2,171 | 11.8 / 11.8 |
| sea_beam: fix on vs pre-fix on | 40 / 777 / 12 | 163 / 335,529 / 219,298 | 146 / 4,031 / 1,390 | 163 / 340,337 / 220,700 | 15.0 / 11.3 |
| sea_beam: fix off vs fix on | 224 / 3,108 / 2,314 | 181 / 345,099 / 217,115 | 243 / 13,057 / 4,296 | 243 / 361,264 / 223,725 | 11.8 / 15.0 |
| sea_front: fix off vs pre-fix off | 7 / 85 / 0 | 14 / 695 / 11 | 8 / 8,209 / 0 | 14 / 8,989 / 11 | 16.6 / 16.6 |
| sea_front: fix on vs pre-fix on | 9 / 196 / 1 | 13 / 1,266 / 15 | 5 / 785 / 0 | 13 / 2,247 / 16 | 15.9 / 15.9 |
| sea_front: fix off vs fix on | 210 / 1,457 / 1,042 | 231 / 31,247 / 771 | 46 / 119,608 / 85,932 | 231 / 152,312 / 87,745 | 16.6 / 15.9 |

Water in b3_sea_beam (1920x1080 frame), luma mean / p95:

| region | off | on before the fix | on with the fix |
|---|---|---|---|
| ahead of the Jeep, x 880-1040 y 450-500 | 49 / 57 (48, 49, 53) | 14 / 16 (14, 14, 15) | 39 / 66 (38, 39, 42) |
| ahead of the Jeep, x 880-1040 y 500-525 | 56 / 70 (55, 56, 61) | 18 / 21 (18, 18, 19) | 87 / 120 (84, 87, 94) |
| left of the Jeep, x 700-820 y 500-580 | 14 / 21 (13, 14, 15) | 12 / 14 (12, 12, 14) | 72 / 101 (70, 72, 78) |
| right of the Jeep, x 1100-1220 y 500-580 | 14 / 24 (14, 14, 15) | 12 / 14 (12, 12, 13) | 72 / 100 (70, 72, 79) |
| unlit sea, x 100-300 y 500-580 | 12 / 14 (12, 12, 13) | 12 / 14 (12, 12, 13) | 20 / 25 (19, 20, 21) |

Notes (read from the masks in images/):
- hero off, fix against the reviewed build: 30 pixels over 2 levels. The fix touches only the sea
  program's lit branch, and only through terms that are 0 while nightLights is off.
- hero on, fix against the reviewed build: 14,812 pixels over 2 levels, mean signed difference about
  -0.3 levels, all in the horizon band (the far town's lamp-lit facades and the foliage dither by the
  headlight flare; review_mask_hero_on_fix_vs_prefix_band.png). The base build's own off/on repeat of
  identical code is 22,163 in the same band, so this is shot noise; the hero has no sea.
- sea_beam off, fix against the reviewed build: the Jeep's outline and the shimmer of the classic pool
  (review_mask_sea_beam_off_fix_vs_prefix.png); the Jeep stands on a 14 deg beach (shore probe profile
  in the run's udcap.log) and settles a little differently each run.
- sea_beam_front: on equals the reviewed build's on (2,247 pixels over 2 levels); from the sea the
  near water stays dark on, where the classic cone lit a streak (off).
