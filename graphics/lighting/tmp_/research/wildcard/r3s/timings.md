# timings (r3s)

| view | label | GPU frame ms | world | shadow (incl. sky maps) | post | raw |
|---|---|---|---|---|---|---|
| forest_edge_golden | classic | 3.22 | 3.09 | None | None | frame 3.22 ms | world 3.09 |
| forest_edge_golden | remastered | 4.33 | 3.83 | 0.03 | 0.37 | frame 4.33 ms | world 3.83 (shadow 0.03) | post 0.37: ao 0.30 bloom 0.05 composite 0.03 |
| lowsun_fp | classic | 4.12 | 4.02 | None | None | frame 4.12 ms | world 4.02 |
| lowsun_fp | remastered | 3.77 | 3.03 | 0.04 | 0.64 | frame 3.77 ms | world 3.03 (shadow 0.04) | post 0.64: ao 0.34 bloom 0.05 shafts 0.22 composite 0.03 |
| noon_fields | classic | 7.34 | 7.24 | None | None | frame 7.34 ms | world 7.24 |
| noon_fields | remastered | 13.54 | 13.1 | 0.04 | 0.33 | frame 13.54 ms | world 13.10 (shadow 0.04) | post 0.33: ao 0.26 bloom 0.05 composite 0.03 |
| overcast_broken | classic | 17.35 | 17.24 | None | None | frame 17.35 ms | world 17.24 |
| overcast_broken | remastered | 15.82 | 14.99 | 0.03 | 0.72 | frame 15.82 ms | world 14.99 (shadow 0.03) | post 0.72: ao 0.65 bloom 0.05 composite 0.03 |
| overcast_close | classic | 2.11 | 2.0 | None | None | frame 2.11 ms | world 2.00 |
| overcast_close | remastered | 2.61 | 2.12 | 0.04 | 0.39 | frame 2.61 ms | world 2.12 (shadow 0.04) | post 0.39: ao 0.31 bloom 0.05 composite 0.03 |
| town_day | classic | 1.51 | 1.37 | None | None | frame 1.51 ms | world 1.37 |
| town_day | remastered | 2.05 | 1.49 | 0.04 | 0.43 | frame 2.05 ms | world 1.49 (shadow 0.04) | post 0.43: ao 0.35 bloom 0.05 composite 0.03 |
