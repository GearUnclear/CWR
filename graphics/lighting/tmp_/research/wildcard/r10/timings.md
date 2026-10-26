# timings (r10)

| view | label | GPU frame ms | world | shadow (incl. sky maps) | post | raw |
|---|---|---|---|---|---|---|
| lowsun_fp | B_ground | 6.74 | 5.85 | 1.12 | 0.77 | frame 6.74 ms | world 5.85 (shadow 1.12) | post 0.77: ao 0.36 bloom 0.07 shafts 0.30 composite 0.04 |
| lowsun_fp | C_gain | 6.78 | 5.88 | 1.09 | 0.78 | frame 6.78 ms | world 5.88 (shadow 1.09) | post 0.78: ao 0.37 bloom 0.07 shafts 0.30 composite 0.04 |
| lowsun_fp | classic | 3.43 | 3.18 | None | None | frame 3.43 ms | world 3.18 |
| lowsun_fp | dbg_visB | 6.85 | 5.96 | 1.11 | 0.77 | frame 6.85 ms | world 5.96 (shadow 1.11) | post 0.77: ao 0.34 bloom 0.07 shafts 0.30 composite 0.06 |
| lowsun_fp | remastered | 5.74 | 4.63 | 0.11 | 0.95 | frame 5.74 ms | world 4.63 (shadow 0.11) | post 0.95: ao 0.47 bloom 0.08 shafts 0.33 composite 0.07 |
| overcast_close | B_ground | 5.05 | 4.47 | 1.18 | 0.47 | frame 5.05 ms | world 4.47 (shadow 1.18) | post 0.47: ao 0.37 bloom 0.06 composite 0.04 |
| overcast_close | C_gain | 6.06 | 5.39 | 1.24 | 0.52 | frame 6.06 ms | world 5.39 (shadow 1.24) | post 0.52: ao 0.39 bloom 0.08 composite 0.05 |
| overcast_close | classic | 4.94 | 4.67 | None | None | frame 4.94 ms | world 4.67 |
| overcast_close | dbg_visB | 5.96 | 5.24 | 1.33 | 0.58 | frame 5.96 ms | world 5.24 (shadow 1.33) | post 0.58: ao 0.45 bloom 0.08 composite 0.05 |
| overcast_close | remastered | 6.19 | 4.93 | 0.44 | 1.01 | frame 6.19 ms | world 4.93 (shadow 0.44) | post 1.01: ao 0.76 bloom 0.12 composite 0.12 |
| town_day | B_ground | 5.45 | 4.81 | 1.21 | 0.49 | frame 5.45 ms | world 4.81 (shadow 1.21) | post 0.49: ao 0.38 bloom 0.07 composite 0.04 |
| town_day | C_gain | 6.47 | 5.73 | 1.26 | 0.56 | frame 6.47 ms | world 5.73 (shadow 1.26) | post 0.56: ao 0.42 bloom 0.08 composite 0.05 |
| town_day | classic | 4.0 | 3.62 | None | None | frame 4.00 ms | world 3.62 |
| town_day | dbg_visB | 5.65 | 5.0 | 1.21 | 0.5 | frame 5.65 ms | world 5.00 (shadow 1.21) | post 0.50: ao 0.38 bloom 0.07 composite 0.04 |
| town_day | remastered | 5.57 | 3.91 | 0.38 | 1.32 | frame 5.57 ms | world 3.91 (shadow 0.38) | post 1.32: ao 1.08 bloom 0.13 composite 0.11 |
| town_street | B_ground | 5.6 | 4.63 | 1.11 | 0.84 | frame 5.60 ms | world 4.63 (shadow 1.11) | post 0.84: ao 0.42 bloom 0.06 shafts 0.33 composite 0.04 |
| town_street | C_gain | 6.42 | 5.36 | 1.23 | 0.92 | frame 6.42 ms | world 5.36 (shadow 1.23) | post 0.92: ao 0.43 bloom 0.07 shafts 0.38 composite 0.05 |
| town_street | classic | 2.46 | 2.22 | None | None | frame 2.46 ms | world 2.22 |
| town_street | dbg_visB | 6.67 | 5.58 | 1.23 | 0.96 | frame 6.67 ms | world 5.58 (shadow 1.23) | post 0.96: ao 0.43 bloom 0.07 shafts 0.38 composite 0.07 |
| town_street | remastered | 6.08 | 3.83 | 0.43 | 1.96 | frame 6.08 ms | world 3.83 (shadow 0.43) | post 1.96: ao 1.04 bloom 0.13 shafts 0.67 composite 0.11 |
| woods_mixed_noon | B_ground | 7.11 | 6.5 | 1.12 | 0.52 | frame 7.11 ms | world 6.50 (shadow 1.12) | post 0.52: ao 0.45 bloom 0.05 composite 0.03 |
| woods_mixed_noon | C_gain | 7.02 | 6.44 | 1.08 | 0.49 | frame 7.02 ms | world 6.44 (shadow 1.08) | post 0.49: ao 0.42 bloom 0.05 composite 0.03 |
| woods_mixed_noon | classic | 2.46 | 2.37 | None | None | frame 2.46 ms | world 2.37 |
| woods_mixed_noon | dbg_visB | 7.04 | 6.44 | 1.11 | 0.51 | frame 7.04 ms | world 6.44 (shadow 1.11) | post 0.51: ao 0.44 bloom 0.05 composite 0.03 |
| woods_mixed_noon | remastered | 6.66 | 5.09 | 0.46 | 1.36 | frame 6.66 ms | world 5.09 (shadow 0.46) | post 1.36: ao 1.14 bloom 0.12 composite 0.10 |
| woods_pine2_evening | B_ground | 7.79 | 6.96 | 1.2 | 0.74 | frame 7.79 ms | world 6.96 (shadow 1.20) | post 0.74: ao 0.38 bloom 0.05 shafts 0.29 composite 0.03 |
| woods_pine2_evening | C_gain | 7.76 | 6.95 | 1.2 | 0.72 | frame 7.76 ms | world 6.95 (shadow 1.20) | post 0.72: ao 0.35 bloom 0.05 shafts 0.29 composite 0.03 |
| woods_pine2_evening | classic | 3.65 | 3.57 | None | None | frame 3.65 ms | world 3.57 |
| woods_pine2_evening | dbg_visB | 7.78 | 6.91 | 1.19 | 0.78 | frame 7.78 ms | world 6.91 (shadow 1.19) | post 0.78: ao 0.42 bloom 0.05 shafts 0.28 composite 0.03 |
| woods_pine2_evening | remastered | 6.84 | 5.81 | 0.39 | 0.92 | frame 6.84 ms | world 5.81 (shadow 0.39) | post 0.92: ao 0.42 bloom 0.07 shafts 0.39 composite 0.04 |
