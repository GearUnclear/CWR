| view | variant | GPU frame samples (ms) | min | min - vb min | median | median - vb | world median |
|---|---|---|---|---|---|---|---|
| lowsun_fp | Remastered as shipped (late shapes on the CPU path) | 10.00, 5.09, 5.21, 6.45, 4.57, 10.74 | 4.57 | -0.08 | 5.83 | +0.82 | 4.88 |
| lowsun_fp | + late-VB fix, no bump | 4.65, 10.03, 4.82, 4.72, 5.20, 5.71 | 4.65 | +0.00 | 5.01 | +0.00 | 4.16 |
| lowsun_fp | + load-time normal map | 4.22, 10.55, 5.48, 7.03, 6.55, 9.69 | 4.22 | -0.43 | 6.79 | +1.78 | 5.82 |
| lowsun_fp | + map, micro shadows, cavities | 4.04, 14.46, 4.69, 7.17, 4.86, 8.83 | 4.04 | -0.61 | 6.02 | +1.01 | 4.97 |
| lowsun_fp | + all x2 | 7.97, 13.21, 5.67, 5.91, 5.89, 7.38 | 5.67 | +1.02 | 6.64 | +1.63 | 5.62 |
| noon_fields | Remastered as shipped (late shapes on the CPU path) | 8.95, 11.53, 15.85, 26.97, 12.99, 18.17 | 8.95 | -0.38 | 14.42 | +0.87 | 13.97 |
| noon_fields | + late-VB fix, no bump | 17.88, 10.27, 12.87, 14.24, 25.57, 9.33 | 9.33 | +0.00 | 13.55 | +0.00 | 13.07 |
| noon_fields | + load-time normal map | 11.48, 11.89, 10.40, 15.42, 22.11, 12.34 | 10.40 | +1.07 | 12.12 | -1.44 | 11.57 |
| noon_fields | + map, micro shadows, cavities | 10.91, 12.54, 12.62, 20.33, 26.54, 11.33 | 10.91 | +1.58 | 12.58 | -0.98 | 12.10 |
| noon_fields | + all x2 | 10.71, 12.03, 10.95, 18.04, 26.01, 19.98 | 10.71 | +1.38 | 15.04 | +1.48 | 14.59 |

(Six interleaved rounds per variant, override_v5 (terrain detail-only relief), shared GPU with up to 4 games of other agents; 'v9' here is V9 = all x2 without mean preservation; V6 adds 6 texture taps to V9. Load-time generation: see NOTES.md.)
