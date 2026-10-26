| test (window) | variant | present ms (shots) | median | GPU frame ms (shots) |
|---|---|---|---|---|
| aa_cost1080 | Classic: stock MSAA resolve + GL_LINEAR blit | 0.602, 0.158 | 0.380 | 4.45, 8.61 |
| aa_cost1080 | Remastered p0: resolve to texture + bilinear shader | 0.403, 0.581 | 0.492 | 9.37, 19.73 |
| aa_cost1080 | Remastered p1: Lanczos2 2D | 0.625, 2.335 | 1.480 | 12.92, 23.90 |
| aa_cost1080 | Remastered p2: Lanczos2 2D + RCAS 0.5 | 1.057, 2.391 | 1.724 | 13.32, 23.92 |
| aa_cost1080 | Remastered p7: separable Lanczos2 (2 passes) | 0.995, 1.199 | 1.097 | 19.39, 23.76 |
| aa_cost1080 | Remastered p6: separable Lanczos2 + RCAS 0.5 | 0.745, 1.385 | 1.065 | 14.29, 25.94 |
| aa_cost960 | Classic: stock MSAA resolve + GL_LINEAR blit | 0.456, 0.505 | 0.481 | 4.68, 7.90 |
| aa_cost960 | Remastered p0: resolve to texture + bilinear shader | 0.034, 0.135 | 0.085 | 8.16, 16.12 |
| aa_cost960 | Remastered p1: Lanczos2 2D | 0.775, 0.873 | 0.824 | 18.72, 18.52 |
| aa_cost960 | Remastered p2: Lanczos2 2D + RCAS 0.5 | 1.559, 1.058 | 1.308 | 17.34, 17.87 |
| aa_cost960 | Remastered p7: separable Lanczos2 (2 passes) | 0.976, 0.661 | 0.819 | 19.95, 13.08 |
| aa_cost960 | Remastered p6: separable Lanczos2 + RCAS 0.5 | 0.391, 0.985 | 0.688 | 13.11, 20.91 |
