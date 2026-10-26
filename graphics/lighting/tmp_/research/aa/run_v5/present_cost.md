| test (window) | variant | present ms EMA (shots) | median | present min ms (shots) | median min | GPU frame ms (shots) |
|---|---|---|---|---|---|---|
| aa_cost5_1080 | Classic: stock MSAA resolve + GL_LINEAR blit | 1.382, 0.703, 0.152 | 0.703 | 0.141, 0.142, 0.104 | 0.141 | 7.55, 13.60, 4.03 |
| aa_cost5_1080 | Remastered p0: resolve to texture + bilinear shader | 0.724, 1.222, 0.692 | 0.724 | 0.104, 0.104, 0.098 | 0.104 | 23.55, 27.89, 18.42 |
| aa_cost5_1080 | Remastered p1: Lanczos2 2D | 2.545, 1.650, 1.816 | 1.816 | 0.424, 0.421, 0.465 | 0.424 | 28.49, 21.03, 22.36 |
| aa_cost5_1080 | Remastered p2: Lanczos2 2D + RCAS 0.5 | 2.013, 2.491, 1.902 | 2.013 | 0.458, 0.455, 1.011 | 0.458 | 24.97, 27.05, 22.99 |
| aa_cost5_1080 | Remastered p7: separable Lanczos2 (2 passes) | 1.341, 0.851, 1.011 | 1.011 | 0.255, 0.262, 0.257 | 0.257 | 23.91, 18.27, 21.95 |
| aa_cost5_1080 | Remastered p6: separable Lanczos2 + RCAS 0.5 | 1.661, 1.677, 1.151 | 1.661 | 0.289, 0.296, 0.302 | 0.296 | 25.04, 28.95, 22.57 |
| aa_cost5_960 | Classic: stock MSAA resolve + GL_LINEAR blit | 0.057, 0.168, 0.040 | 0.057 | 0.030, 0.032, 0.032 | 0.032 | 4.63, 4.29, 3.01 |
| aa_cost5_960 | Remastered p0: resolve to texture + bilinear shader | 0.032, 0.032, 0.032 | 0.032 | 0.026, 0.026, 0.026 | 0.026 | 8.85, 15.19, 13.39 |
| aa_cost5_960 | Remastered p1: Lanczos2 2D | 0.764, 0.147, 0.890 | 0.764 | 0.123, 0.123, 0.123 | 0.123 | 15.73, 4.67, 16.19 |
| aa_cost5_960 | Remastered p2: Lanczos2 2D + RCAS 0.5 | 1.439, 0.142, 1.090 | 1.090 | 0.136, 0.136, 0.136 | 0.136 | 16.79, 4.72, 15.65 |
| aa_cost5_960 | Remastered p7: separable Lanczos2 (2 passes) | 0.254, 0.081, 0.427 | 0.254 | 0.075, 0.074, 0.075 | 0.075 | 15.10, 8.30, 16.40 |
| aa_cost5_960 | Remastered p6: separable Lanczos2 + RCAS 0.5 | 0.381, 0.093, 0.795 | 0.381 | 0.087, 0.087, 0.087 | 0.087 | 11.65, 4.66, 16.08 |
