# timings (r9d)

| view | label | GPU frame ms | world | shadow (incl. sky maps) | post | raw |
|---|---|---|---|---|---|---|
| lowsun_fp | dbg_cnt | 6.63 | 5.43 | 1.17 | 1.04 | frame 6.63 ms | world 5.43 (shadow 1.17) | post 1.04: ao 0.43 bloom 0.10 shafts 0.41 composite 0.10 |
| lowsun_fp | dbg_d02 | 6.65 | 5.45 | 1.2 | 1.03 | frame 6.65 ms | world 5.45 (shadow 1.20) | post 1.03: ao 0.43 bloom 0.10 shafts 0.44 composite 0.07 |
| lowsun_fp | dbg_d06 | 8.06 | 6.92 | 1.25 | 0.98 | frame 8.06 ms | world 6.92 (shadow 1.25) | post 0.98: ao 0.49 bloom 0.09 shafts 0.33 composite 0.07 |
| lowsun_fp | dbg_d10 | 6.47 | 5.62 | 1.1 | 0.73 | frame 6.47 ms | world 5.62 (shadow 1.10) | post 0.73: ao 0.36 bloom 0.06 shafts 0.28 composite 0.04 |
| lowsun_fp | dbg_d14 | 6.48 | 5.62 | 1.05 | 0.74 | frame 6.48 ms | world 5.62 (shadow 1.05) | post 0.74: ao 0.34 bloom 0.06 shafts 0.28 composite 0.05 |
| lowsun_fp | dbg_visB | 7.09 | 5.98 | 1.38 | 0.95 | frame 7.09 ms | world 5.98 (shadow 1.38) | post 0.95: ao 0.38 bloom 0.10 shafts 0.41 composite 0.06 |
| overcast_close | dbg_cnt | 5.62 | 4.95 | 1.23 | 0.54 | frame 5.62 ms | world 4.95 (shadow 1.23) | post 0.54: ao 0.42 bloom 0.07 composite 0.04 |
| overcast_close | dbg_d02 | 5.74 | 5.08 | 1.21 | 0.51 | frame 5.74 ms | world 5.08 (shadow 1.21) | post 0.51: ao 0.39 bloom 0.08 composite 0.05 |
| overcast_close | dbg_d06 | 6.32 | 5.61 | 1.25 | 0.56 | frame 6.32 ms | world 5.61 (shadow 1.25) | post 0.56: ao 0.42 bloom 0.09 composite 0.05 |
| overcast_close | dbg_d10 | 7.48 | 6.51 | 1.57 | 0.79 | frame 7.48 ms | world 6.51 (shadow 1.57) | post 0.79: ao 0.60 bloom 0.10 composite 0.08 |
| overcast_close | dbg_d14 | 5.56 | 4.92 | 1.2 | 0.51 | frame 5.56 ms | world 4.92 (shadow 1.20) | post 0.51: ao 0.40 bloom 0.07 composite 0.04 |
| overcast_close | dbg_visB | 7.8 | 7.05 | 1.56 | 0.6 | frame 7.80 ms | world 7.05 (shadow 1.56) | post 0.60: ao 0.47 bloom 0.08 composite 0.05 |
