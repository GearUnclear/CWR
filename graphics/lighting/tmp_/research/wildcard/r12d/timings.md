# timings (r12d)

| view | label | GPU frame ms | world | shadow (incl. sky maps) | post | raw |
|---|---|---|---|---|---|---|
| lowsun_fp | dbg_L0 | 7.41 | 6.26 | 1.35 | 0.99 | frame 7.41 ms | world 6.26 (shadow 1.35) | post 0.99: ao 0.43 bloom 0.10 shafts 0.40 composite 0.06 |
| lowsun_fp | dbg_L2 | 6.62 | 5.51 | 1.16 | 0.95 | frame 6.62 ms | world 5.51 (shadow 1.16) | post 0.95: ao 0.38 bloom 0.10 shafts 0.41 composite 0.06 |
| lowsun_fp | dbg_L4 | 6.87 | 5.6 | 1.38 | 1.08 | frame 6.87 ms | world 5.60 (shadow 1.38) | post 1.08: ao 0.52 bloom 0.11 shafts 0.35 composite 0.10 |
| lowsun_fp | dbg_L6 | 7.06 | 6.18 | 1.09 | 0.76 | frame 7.06 ms | world 6.18 (shadow 1.09) | post 0.76: ao 0.36 bloom 0.07 shafts 0.29 composite 0.04 |
| lowsun_fp | dbg_d01 | 6.89 | 6.01 | 1.06 | 0.76 | frame 6.89 ms | world 6.01 (shadow 1.06) | post 0.76: ao 0.36 bloom 0.07 shafts 0.29 composite 0.04 |
