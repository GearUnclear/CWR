# timings (r4d)

| view | label | GPU frame ms | world | shadow (incl. sky maps) | post | raw |
|---|---|---|---|---|---|---|
| forest_edge_golden | dbg_cov | 6.59 | 5.95 | 0.55 | 0.48 | frame 6.59 ms | world 5.95 (shadow 0.55) | post 0.48: ao 0.36 bloom 0.07 composite 0.05 |
| forest_edge_golden | dbg_dir | 6.66 | 6.01 | 0.53 | 0.49 | frame 6.66 ms | world 6.01 (shadow 0.53) | post 0.49: ao 0.37 bloom 0.07 composite 0.05 |
| forest_edge_golden | dbg_leaf | 4.86 | 4.32 | 0.47 | 0.41 | frame 4.86 ms | world 4.32 (shadow 0.47) | post 0.41: ao 0.33 bloom 0.05 composite 0.03 |
| forest_edge_golden | dbg_recv | 4.77 | 4.23 | 0.46 | 0.41 | frame 4.77 ms | world 4.23 (shadow 0.46) | post 0.41: ao 0.33 bloom 0.05 composite 0.03 |
| forest_edge_golden | dbg_vis | 6.36 | 5.73 | 0.54 | 0.47 | frame 6.36 ms | world 5.73 (shadow 0.54) | post 0.47: ao 0.35 bloom 0.07 composite 0.04 |
| town_day | dbg_cov | 6.23 | 5.13 | 0.78 | 0.78 | frame 6.23 ms | world 5.13 (shadow 0.78) | post 0.78: ao 0.53 bloom 0.13 composite 0.11 |
| town_day | dbg_dir | 6.45 | 5.35 | 0.78 | 0.78 | frame 6.45 ms | world 5.35 (shadow 0.78) | post 0.78: ao 0.52 bloom 0.14 composite 0.12 |
| town_day | dbg_leaf | 6.87 | 5.79 | 0.82 | 0.75 | frame 6.87 ms | world 5.79 (shadow 0.82) | post 0.75: ao 0.55 bloom 0.13 composite 0.07 |
| town_day | dbg_recv | 6.37 | 5.16 | 0.78 | 0.91 | frame 6.37 ms | world 5.16 (shadow 0.78) | post 0.91: ao 0.66 bloom 0.13 composite 0.11 |
| town_day | dbg_vis | 8.35 | 6.99 | 0.94 | 0.87 | frame 8.35 ms | world 6.99 (shadow 0.94) | post 0.87: ao 0.58 bloom 0.14 composite 0.15 |
