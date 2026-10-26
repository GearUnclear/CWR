| view | shot | GPU frame | world | shadow | post | ao/gi | bloom | shafts | composite |
|---|---|---|---|---|---|---|---|---|---|
| lowsun_fp | classic | 2.84 | 2.73 |  |  |  |  |  |  |
| lowsun_fp | remastered | 6.01 | 5.01 | 0.05 | 0.89 | 0.58 | 0.05 | 0.22 | 0.04 |
| lowsun_fp | gbufonly | 5.82 | 4.71 | 0.04 | 1.00 | 0.62 | 0.05 | 0.22 | 0.11 |
| lowsun_fp | gi | 8.85 | 6.62 | 0.04 | 2.12 | 1.40 | 0.05 | 0.22 | 0.44 |
| lowsun_fp | dbg_normals | 5.51 | 4.27 | 0.04 | 1.12 | 1.11 |  |  | 0.02 |
| lowsun_fp | dbg_rough | 6.83 | 5.69 | 0.04 | 1.03 | 1.01 |  |  | 0.02 |
| lowsun_fp | dbg_direct | 5.18 | 4.00 | 0.04 | 1.07 | 1.04 |  |  | 0.02 |
| lowsun_fp | dbg_albedo | 7.68 | 6.51 | 0.04 | 1.06 | 1.04 |  |  | 0.02 |
| lowsun_fp | dbg_bounce | 5.06 | 3.83 | 0.04 | 1.12 | 1.10 |  |  | 0.02 |
| lowsun_fp | dbg_vis | 8.04 | 6.62 | 0.04 | 1.31 | 1.28 |  |  | 0.02 |
| lowsun_fp | dbg_sky | 6.04 | 4.84 | 0.04 | 1.09 | 1.05 |  |  | 0.04 |

engine log lines:

[2026-10-17 14:39:26.570] [app-69dc] [INFO] [GRAPHICS]  GL33 post: gbuffer prototype programs gi=185 temporal=187 blur=189 debug=191
