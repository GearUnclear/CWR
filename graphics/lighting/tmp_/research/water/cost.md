# GPU cost of the water variants

Median (and spread) over the final passes of `triGpuTimes` (smoothed GPU timestamps, ~90 frames per
shot). Other research agents were running games on the same GPU at the same time, so single values
move by a millisecond or more; read the medians, and only large differences.

| view | shader | n | frame ms median | frame min-max | world ms median | world min-max | world delta vs now |
|---|---|---|---|---|---|---|---|
| coastline_sunset | Classic | 12 | 1.35 | 0.55-4.69 | 1.25 | 0.46-3.83 |  |
| coastline_sunset | Remastered now | 3 | 2.57 | 1.78-2.77 | 1.11 | 1.05-2.22 |  |
| coastline_sunset | Variant A | 3 | 3.78 | 2.69-5.37 | 2.70 | 2.05-3.25 | +1.59 |
| coastline_sunset | Variant B | 3 | 3.20 | 1.24-5.72 | 2.70 | 0.52-3.05 | +1.59 |
| coastline_sunset | Variant C | 3 | 4.59 | 3.47-5.31 | 2.92 | 2.50-2.95 | +1.81 |
| sea_noon | Classic | 12 | 2.08 | 0.85-4.35 | 1.89 | 0.77-3.88 |  |
| sea_noon | Remastered now | 3 | 2.03 | 1.72-5.11 | 1.55 | 1.30-3.27 |  |
| sea_noon | Variant A | 3 | 3.50 | 2.12-5.49 | 3.16 | 1.73-3.70 | +1.61 |
| sea_noon | Variant B | 3 | 4.70 | 3.41-5.59 | 3.34 | 3.09-3.97 | +1.79 |
| sea_noon | Variant C | 3 | 4.59 | 3.52-6.13 | 3.28 | 3.15-4.46 | +1.73 |
| sea_away_from_sun | Classic | 12 | 2.02 | 0.77-3.72 | 1.71 | 0.68-2.81 |  |
| sea_away_from_sun | Remastered now | 3 | 2.56 | 1.00-5.12 | 2.19 | 0.51-2.87 |  |
| sea_away_from_sun | Variant A | 3 | 3.23 | 2.30-5.15 | 2.91 | 1.86-3.03 | +0.72 |
| sea_away_from_sun | Variant B | 3 | 5.44 | 4.74-8.21 | 3.49 | 2.90-6.30 | +1.30 |
| sea_away_from_sun | Variant C | 3 | 2.89 | 1.11-3.63 | 2.37 | 0.57-2.48 | +0.18 |
| coast_shore_midsun | Classic | 12 | 1.90 | 0.79-2.79 | 1.80 | 0.74-2.62 |  |
| coast_shore_midsun | Remastered now | 3 | 5.03 | 3.50-5.49 | 2.93 | 2.28-3.19 |  |
| coast_shore_midsun | Variant A | 3 | 4.02 | 3.41-5.90 | 2.79 | 2.37-3.13 | -0.14 |
| coast_shore_midsun | Variant B | 3 | 5.64 | 2.35-5.88 | 3.31 | 1.54-3.64 | +0.38 |
| coast_shore_midsun | Variant C | 3 | 4.92 | 1.55-5.81 | 3.41 | 0.85-3.65 | +0.48 |
