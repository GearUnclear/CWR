# timings (r5d)

| view | label | GPU frame ms | world | shadow (incl. sky maps) | post | raw |
|---|---|---|---|---|---|---|
| forest_edge_golden | dbg_leaf | 6.61 | 6.11 | 0.72 | 0.38 | frame 6.61 ms | world 6.11 (shadow 0.72) | post 0.38: ao 0.30 bloom 0.05 composite 0.03 |
| forest_edge_golden | dbg_nan | 5.17 | 4.65 | 0.7 | 0.39 | frame 5.17 ms | world 4.65 (shadow 0.70) | post 0.39: ao 0.31 bloom 0.05 composite 0.03 |
| forest_edge_golden | dbg_vis | 5.45 | 4.89 | 0.84 | 0.44 | frame 5.45 ms | world 4.89 (shadow 0.84) | post 0.44: ao 0.35 bloom 0.05 composite 0.03 |
| forest_edge_golden | remastered | 4.37 | 3.81 | 0.2 | 0.42 | frame 4.37 ms | world 3.81 (shadow 0.20) | post 0.42: ao 0.34 bloom 0.05 composite 0.03 |
| forest_edge_golden | svm_occ | 4.88 | 4.33 | 0.63 | 0.41 | frame 4.88 ms | world 4.33 (shadow 0.63) | post 0.41: ao 0.34 bloom 0.05 composite 0.03 |
| town_day | dbg_leaf | 4.7 | 3.99 | 0.74 | 0.54 | frame 4.70 ms | world 3.99 (shadow 0.74) | post 0.54: ao 0.43 bloom 0.07 composite 0.04 |
| town_day | dbg_nan | 6.78 | 5.5 | 1.04 | 1.02 | frame 6.78 ms | world 5.50 (shadow 1.04) | post 1.02: ao 0.84 bloom 0.12 composite 0.07 |
| town_day | dbg_vis | 8.63 | 6.95 | 1.25 | 1.35 | frame 8.63 ms | world 6.95 (shadow 1.25) | post 1.35: ao 1.05 bloom 0.17 composite 0.13 |
| town_day | remastered | 3.93 | 2.94 | 0.29 | 0.73 | frame 3.93 ms | world 2.94 (shadow 0.29) | post 0.73: ao 0.50 bloom 0.13 composite 0.09 |
| town_day | svm_occ | 5.63 | 4.85 | 0.84 | 0.57 | frame 5.63 ms | world 4.85 (shadow 0.84) | post 0.57: ao 0.42 bloom 0.09 composite 0.06 |
