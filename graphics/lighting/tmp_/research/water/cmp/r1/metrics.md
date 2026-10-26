# Water-region metrics

Luminance (0-255) over the water region of each 960x640 capture: mean, sd of a lightly blurred
image (wave-scale contrast), and sd of a 9x9 high-pass (fine detail). Higher sd = more defined.

| view | image | mean | sd (wave scale) | sd (fine) | GPU frame ms | GPU world ms |
|---|---|---|---|---|---|---|
| coastline_sunset | Classic | 69.9 | 29.37 | 3.66 | 3.00 | 2.79 |
| coastline_sunset | Remastered now | 101.1 | 29.83 | 3.04 | 2.56 | 1.19 |
| coastline_sunset | Variant A | 91.2 | 21.6 | 10.87 | 2.25 | 0.96 |
| coastline_sunset | Variant B | 113.9 | 20.73 | 14.58 | 2.44 | 1.06 |
| coastline_sunset | Variant C | 114.0 | 20.66 | 16.99 | 2.55 | 1.27 |
| sea_noon | Classic | 145.1 | 13.96 | 6.08 | 2.17 | 1.91 |
| sea_noon | Remastered now | 129.3 | 6.91 | 4.37 | 1.16 | 0.57 |
| sea_noon | Variant A | 129.7 | 7.43 | 7.93 | 3.55 | 2.42 |
| sea_noon | Variant B | 121.1 | 14.67 | 7.67 | 3.11 | 1.84 |
| sea_noon | Variant C | 122.4 | 14.66 | 10.09 | 3.34 | 2.15 |
| sea_away_from_sun | Classic | 112.0 | 6.49 | 7.4 | 1.45 | 1.22 |
| sea_away_from_sun | Remastered now | 113.6 | 7.36 | 5.72 | 2.14 | 1.19 |
| sea_away_from_sun | Variant A | 114.9 | 6.29 | 5.07 | 2.25 | 1.15 |
| sea_away_from_sun | Variant B | 133.1 | 9.5 | 8.59 | 2.39 | 1.25 |
| sea_away_from_sun | Variant C | 133.6 | 9.94 | 10.41 | 3.45 | 2.69 |
| coast_shore_midsun | Classic | 176.4 | 24.55 | 6.99 | 2.14 | 2.04 |
| coast_shore_midsun | Remastered now | 154.3 | 15.36 | 3.52 | 2.96 | 1.65 |
| coast_shore_midsun | Variant A | 151.5 | 14.68 | 7.47 | 3.19 | 1.74 |
| coast_shore_midsun | Variant B | 120.1 | 16.74 | 10.72 | 2.69 | 1.36 |
| coast_shore_midsun | Variant C | 122.1 | 17.84 | 12.58 | 3.71 | 2.34 |

Classic shot of each variant run vs the stock run (mean abs difference, 0-255):

- coastline_sunset/A: 0.601
- coastline_sunset/B: 0.646
- coastline_sunset/C: 0.505
- sea_noon/A: 2.811
- sea_noon/B: 2.794
- sea_noon/C: 4.355
- sea_away_from_sun/A: 1.404
- sea_away_from_sun/B: 0.831
- sea_away_from_sun/C: 1.739
- coast_shore_midsun/A: 2.412
- coast_shore_midsun/B: 4.089
- coast_shore_midsun/C: 6.43
