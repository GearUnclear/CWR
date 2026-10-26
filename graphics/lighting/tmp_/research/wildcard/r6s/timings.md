# timings (r6s)

| view | label | GPU frame ms | world | shadow (incl. sky maps) | post | raw |
|---|---|---|---|---|---|---|
| forest_edge_golden | classic | 4.61 | 4.49 | None | None | frame 4.61 ms | world 4.49 |
| forest_edge_golden | remastered | 3.71 | 3.2 | 0.03 | 0.38 | frame 3.71 ms | world 3.20 (shadow 0.03) | post 0.38: ao 0.30 bloom 0.05 composite 0.03 |
| lowsun_fp | classic | 2.12 | 2.0 | None | None | frame 2.12 ms | world 2.00 |
| lowsun_fp | remastered | 3.81 | 3.06 | 0.04 | 0.64 | frame 3.81 ms | world 3.06 (shadow 0.04) | post 0.64: ao 0.34 bloom 0.05 shafts 0.22 composite 0.03 |
| overcast_broken | classic | 8.84 | 8.74 | None | None | frame 8.84 ms | world 8.74 |
| overcast_broken | remastered | 11.06 | 10.39 | 0.03 | 0.57 | frame 11.06 ms | world 10.39 (shadow 0.03) | post 0.57: ao 0.50 bloom 0.05 composite 0.03 |
| overcast_close | classic | 3.82 | 3.69 | None | None | frame 3.82 ms | world 3.69 |
| overcast_close | remastered | 3.01 | 2.48 | 0.04 | 0.41 | frame 3.01 ms | world 2.48 (shadow 0.04) | post 0.41: ao 0.33 bloom 0.05 composite 0.03 |
| town_day | classic | 2.53 | 2.4 | None | None | frame 2.53 ms | world 2.40 |
| town_day | remastered | 2.54 | 1.95 | 0.04 | 0.45 | frame 2.54 ms | world 1.95 (shadow 0.04) | post 0.45: ao 0.37 bloom 0.05 composite 0.03 |
| town_street | classic | 2.4 | 2.29 | None | None | frame 2.40 ms | world 2.29 |
| town_street | remastered | 4.27 | 3.02 | 0.03 | 1.13 | frame 4.27 ms | world 3.02 (shadow 0.03) | post 1.13: ao 0.31 bloom 0.05 shafts 0.27 composite 0.51 |
| woods_mixed_noon | classic | 3.12 | 3.02 | None | None | frame 3.12 ms | world 3.02 |
| woods_mixed_noon | remastered | 3.35 | 2.77 | 0.03 | 0.48 | frame 3.35 ms | world 2.77 (shadow 0.03) | post 0.48: ao 0.41 bloom 0.05 composite 0.03 |
| woods_overcast | classic | 3.54 | 3.43 | None | None | frame 3.54 ms | world 3.43 |
| woods_overcast | remastered | 3.08 | 2.46 | 0.03 | 0.51 | frame 3.08 ms | world 2.46 (shadow 0.03) | post 0.51: ao 0.43 bloom 0.05 composite 0.03 |
| woods_pine2_evening | classic | 3.29 | 3.18 | None | None | frame 3.29 ms | world 3.18 |
| woods_pine2_evening | remastered | 7.15 | 5.97 | 0.03 | 1.08 | frame 7.15 ms | world 5.97 (shadow 0.03) | post 1.08: ao 0.41 bloom 0.05 shafts 0.29 composite 0.33 |
