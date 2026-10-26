# Water-region metrics

Luminance (0-255) over the water region of each 960x640 capture: mean, sd of a lightly blurred
image (wave-scale contrast), and sd of a 9x9 high-pass (fine detail). Higher sd = more defined.

| view | image | mean | sd (wave scale) | sd (fine) | GPU frame ms | GPU world ms |
|---|---|---|---|---|---|---|
| coastline_sunset | Classic | 69.9 | 29.37 | 3.66 | 3.00 | 2.79 |
| coastline_sunset | Remastered now | 101.1 | 29.83 | 3.04 | 2.56 | 1.19 |
| coastline_sunset | Variant A | 91.2 | 21.6 | 10.87 | 2.25 | 0.96 |
| coastline_sunset | Variant B | 113.5 | 26.59 | 16.95 | 1.81 | 0.83 |
| coastline_sunset | Variant C | 112.6 | 25.43 | 18.41 | 2.14 | 1.12 |
| sea_noon | Classic | 145.1 | 13.96 | 6.08 | 2.17 | 1.91 |
| sea_noon | Remastered now | 129.3 | 6.91 | 4.37 | 1.16 | 0.57 |
| sea_noon | Variant A | 129.7 | 7.43 | 7.93 | 3.55 | 2.42 |
| sea_noon | Variant B | 133.4 | 7.6 | 7.72 | 3.11 | 2.07 |
| sea_noon | Variant C | 133.6 | 7.74 | 7.79 | 3.34 | 2.31 |
| sea_away_from_sun | Classic | 112.0 | 6.49 | 7.4 | 1.45 | 1.22 |
| sea_away_from_sun | Remastered now | 113.6 | 7.36 | 5.72 | 2.14 | 1.19 |
| sea_away_from_sun | Variant A | 114.9 | 6.29 | 5.07 | 2.25 | 1.15 |
| sea_away_from_sun | Variant B | 125.3 | 6.72 | 6.52 | 3.97 | 2.96 |
| sea_away_from_sun | Variant C | 125.2 | 6.72 | 6.88 | 4.17 | 3.03 |
| coast_shore_midsun | Classic | 176.4 | 24.55 | 6.99 | 2.14 | 2.04 |
| coast_shore_midsun | Remastered now | 154.3 | 15.36 | 3.52 | 2.96 | 1.65 |
| coast_shore_midsun | Variant A | 151.5 | 14.68 | 7.47 | 3.19 | 1.74 |
| coast_shore_midsun | Variant B | 154.8 | 17.59 | 12.21 | 4.81 | 2.94 |
| coast_shore_midsun | Variant C | 154.6 | 18.45 | 13.21 | 5.34 | 3.92 |

Classic shot of each variant run vs the stock run (mean abs difference, 0-255):

- coastline_sunset/A: 0.601
- coastline_sunset/B: 0.592
- coastline_sunset/C: 0.59
- sea_noon/A: 2.811
- sea_noon/B: 2.795
- sea_noon/C: 2.99
- sea_away_from_sun/A: 1.404
- sea_away_from_sun/B: 2.031
- sea_away_from_sun/C: 1.391
- coast_shore_midsun/A: 2.412
- coast_shore_midsun/B: 7.149
- coast_shore_midsun/C: 7.001
