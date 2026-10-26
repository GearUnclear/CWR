# Water-region metrics

Luminance (0-255) over the water region of each 960x640 capture: mean, sd of a lightly blurred
image (wave-scale contrast), and sd of a 9x9 high-pass (fine detail). Higher sd = more defined.

| view | image | mean | sd (wave scale) | sd (fine) | GPU frame ms | GPU world ms |
|---|---|---|---|---|---|---|
| coastline_sunset | Classic | 69.9 | 29.37 | 3.66 | 3.00 | 2.79 |
| coastline_sunset | Remastered now | 101.1 | 29.83 | 3.04 | 2.56 | 1.19 |
| coastline_sunset | Variant A | 91.2 | 21.6 | 10.87 | 2.25 | 0.96 |
| coastline_sunset | Variant B | 112.8 | 29.15 | 23.33 | 3.39 | 2.06 |
| coastline_sunset | Variant C | 112.0 | 28.81 | 25.4 | 5.18 | 3.88 |
| sea_noon | Classic | 145.1 | 13.96 | 6.08 | 2.17 | 1.91 |
| sea_noon | Remastered now | 129.3 | 6.91 | 4.37 | 1.16 | 0.57 |
| sea_noon | Variant A | 129.7 | 7.43 | 7.93 | 3.55 | 2.42 |
| sea_noon | Variant B | 125.3 | 8.09 | 10.59 | 3.32 | 2.10 |
| sea_noon | Variant C | 126.4 | 8.6 | 11.59 | 6.21 | 4.30 |
| sea_away_from_sun | Classic | 112.0 | 6.49 | 7.4 | 1.45 | 1.22 |
| sea_away_from_sun | Remastered now | 113.6 | 7.36 | 5.72 | 2.14 | 1.19 |
| sea_away_from_sun | Variant A | 114.9 | 6.29 | 5.07 | 2.25 | 1.15 |
| sea_away_from_sun | Variant B | 123.3 | 6.0 | 13.77 | 6.41 | 5.13 |
| sea_away_from_sun | Variant C | 124.3 | 6.82 | 15.5 | 5.78 | 4.75 |
| coast_shore_midsun | Classic | 176.4 | 24.55 | 6.99 | 2.14 | 2.04 |
| coast_shore_midsun | Remastered now | 154.3 | 15.36 | 3.52 | 2.96 | 1.65 |
| coast_shore_midsun | Variant A | 151.5 | 14.68 | 7.47 | 3.19 | 1.74 |
| coast_shore_midsun | Variant B | 138.6 | 17.51 | 13.03 | 3.77 | 2.41 |
| coast_shore_midsun | Variant C | 139.4 | 18.47 | 14.28 | 8.19 | 6.23 |

Classic shot of each variant run vs the stock run (mean abs difference, 0-255):

- coastline_sunset/A: 0.601
- coastline_sunset/B: 0.952
- coastline_sunset/C: 1.183
- sea_noon/A: 2.811
- sea_noon/B: 5.839
- sea_noon/C: 6.061
- sea_away_from_sun/A: 1.404
- sea_away_from_sun/B: 3.227
- sea_away_from_sun/C: 3.832
- coast_shore_midsun/A: 2.412
- coast_shore_midsun/B: 8.737
- coast_shore_midsun/C: 8.783
