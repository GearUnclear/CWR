# timings (r13)

| view | label | GPU frame ms | world | shadow (incl. sky maps) | post | raw |
|---|---|---|---|---|---|---|
| lowsun_fp | B_ground | 6.92 | 6.15 | 1.0 | 0.67 | frame 6.92 ms | world 6.15 (shadow 1.00) | post 0.67: ao 0.34 bloom 0.05 shafts 0.24 composite 0.03 |
| lowsun_fp | C_gain | 6.95 | 6.15 | 0.99 | 0.7 | frame 6.95 ms | world 6.15 (shadow 0.99) | post 0.70: ao 0.37 bloom 0.05 shafts 0.24 composite 0.03 |
| lowsun_fp | classic | 4.04 | 3.73 | None | None | frame 4.04 ms | world 3.73 |
| lowsun_fp | dbg_visB | 6.81 | 6.05 | 0.97 | 0.66 | frame 6.81 ms | world 6.05 (shadow 0.97) | post 0.66: ao 0.34 bloom 0.05 shafts 0.24 composite 0.03 |
| lowsun_fp | remastered | 6.07 | 4.88 | 0.12 | 1.03 | frame 6.07 ms | world 4.88 (shadow 0.12) | post 1.03: ao 0.48 bloom 0.09 shafts 0.37 composite 0.09 |
| overcast_close | B_ground | 5.68 | 5.14 | 1.07 | 0.43 | frame 5.68 ms | world 5.14 (shadow 1.07) | post 0.43: ao 0.35 bloom 0.05 composite 0.03 |
| overcast_close | C_gain | 6.65 | 6.04 | 1.16 | 0.49 | frame 6.65 ms | world 6.04 (shadow 1.16) | post 0.49: ao 0.39 bloom 0.06 composite 0.04 |
| overcast_close | classic | 5.03 | 4.76 | None | None | frame 5.03 ms | world 4.76 |
| overcast_close | dbg_visB | 6.85 | 6.24 | 1.19 | 0.48 | frame 6.85 ms | world 6.24 (shadow 1.19) | post 0.48: ao 0.38 bloom 0.06 composite 0.04 |
| overcast_close | remastered | 6.32 | 5.05 | 0.45 | 1.03 | frame 6.32 ms | world 5.05 (shadow 0.45) | post 1.03: ao 0.77 bloom 0.12 composite 0.13 |
| town_street | B_ground | 6.84 | 6.03 | 0.98 | 0.71 | frame 6.84 ms | world 6.03 (shadow 0.98) | post 0.71: ao 0.36 bloom 0.05 shafts 0.27 composite 0.03 |
| town_street | C_gain | 6.91 | 6.07 | 1.0 | 0.73 | frame 6.91 ms | world 6.07 (shadow 1.00) | post 0.73: ao 0.39 bloom 0.05 shafts 0.27 composite 0.03 |
| town_street | classic | 1.09 | 0.99 | None | None | frame 1.09 ms | world 0.99 |
| town_street | dbg_visB | 6.87 | 6.04 | 1.0 | 0.74 | frame 6.87 ms | world 6.04 (shadow 1.00) | post 0.74: ao 0.39 bloom 0.05 shafts 0.26 composite 0.03 |
| town_street | remastered | 6.05 | 3.8 | 0.45 | 1.94 | frame 6.05 ms | world 3.80 (shadow 0.45) | post 1.94: ao 1.05 bloom 0.13 shafts 0.66 composite 0.10 |
| woods_mixed_noon | B_ground | 10.7 | 10.15 | 1.05 | 0.47 | frame 10.70 ms | world 10.15 (shadow 1.05) | post 0.47: ao 0.39 bloom 0.05 composite 0.03 |
| woods_mixed_noon | C_gain | 10.69 | 10.14 | 1.04 | 0.47 | frame 10.69 ms | world 10.14 (shadow 1.04) | post 0.47: ao 0.39 bloom 0.05 composite 0.03 |
| woods_mixed_noon | classic | 2.47 | 2.37 | None | None | frame 2.47 ms | world 2.37 |
| woods_mixed_noon | dbg_visB | 10.74 | 10.14 | 1.07 | 0.52 | frame 10.74 ms | world 10.14 (shadow 1.07) | post 0.52: ao 0.44 bloom 0.05 composite 0.03 |
| woods_mixed_noon | remastered | 6.7 | 5.08 | 0.45 | 1.41 | frame 6.70 ms | world 5.08 (shadow 0.45) | post 1.41: ao 1.18 bloom 0.12 composite 0.11 |
