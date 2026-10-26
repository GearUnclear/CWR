# timings (r3)

| view | label | GPU frame ms | world | shadow (incl. sky maps) | post | raw |
|---|---|---|---|---|---|---|
| forest_edge_golden | classic | 2.36 | 2.23 | None | None | frame 2.36 ms | world 2.23 |
| forest_edge_golden | dbg_vis | 5.4 | 4.92 | 0.68 | 0.34 | frame 5.40 ms | world 4.92 (shadow 0.68) | post 0.34: ao 0.27 bloom 0.05 composite 0.03 |
| forest_edge_golden | remastered | 4.3 | 3.74 | 0.04 | 0.4 | frame 4.30 ms | world 3.74 (shadow 0.04) | post 0.40: ao 0.31 bloom 0.06 composite 0.03 |
| forest_edge_golden | svm_full | 4.88 | 4.37 | 0.87 | 0.38 | frame 4.88 ms | world 4.37 (shadow 0.87) | post 0.38: ao 0.30 bloom 0.05 composite 0.03 |
| forest_edge_golden | svm_occ | 4.74 | 4.22 | 0.45 | 0.38 | frame 4.74 ms | world 4.22 (shadow 0.45) | post 0.38: ao 0.31 bloom 0.05 composite 0.03 |
| lowsun_fp | classic | 2.43 | 2.26 | None | None | frame 2.43 ms | world 2.26 |
| lowsun_fp | dbg_vis | 8.31 | 7.25 | 1.54 | 0.95 | frame 8.31 ms | world 7.25 (shadow 1.54) | post 0.95: ao 0.20 bloom 0.05 shafts 0.21 composite 0.50 |
| lowsun_fp | remastered | 5.89 | 4.58 | 0.05 | 1.18 | frame 5.89 ms | world 4.58 (shadow 0.05) | post 1.18: ao 0.33 bloom 0.06 shafts 0.28 composite 0.51 |
| lowsun_fp | svm_full | 8.66 | 7.63 | 2.38 | 0.93 | frame 8.66 ms | world 7.63 (shadow 2.38) | post 0.93: ao 0.21 bloom 0.05 shafts 0.22 composite 0.45 |
| lowsun_fp | svm_occ | 5.32 | 4.46 | 0.55 | 0.74 | frame 5.32 ms | world 4.46 (shadow 0.55) | post 0.74: ao 0.37 bloom 0.06 shafts 0.27 composite 0.04 |
| noon_fields | classic | 7.34 | 7.23 | None | None | frame 7.34 ms | world 7.23 |
| noon_fields | dbg_vis | 10.27 | 9.8 | 0.7 | 0.37 | frame 10.27 ms | world 9.80 (shadow 0.70) | post 0.37: ao 0.29 bloom 0.05 composite 0.03 |
| noon_fields | remastered | 15.81 | 15.35 | 0.04 | 0.35 | frame 15.81 ms | world 15.35 (shadow 0.04) | post 0.35: ao 0.28 bloom 0.05 composite 0.03 |
| noon_fields | svm_full | 17.66 | 17.21 | 1.91 | 0.34 | frame 17.66 ms | world 17.21 (shadow 1.91) | post 0.34: ao 0.26 bloom 0.05 composite 0.03 |
| noon_fields | svm_occ | 10.34 | 9.86 | 0.52 | 0.37 | frame 10.34 ms | world 9.86 (shadow 0.52) | post 0.37: ao 0.30 bloom 0.05 composite 0.03 |
| overcast_broken | classic | 15.15 | 15.05 | None | None | frame 15.15 ms | world 15.05 |
| overcast_broken | dbg_vis | 13.08 | 12.47 | 0.53 | 0.51 | frame 13.08 ms | world 12.47 (shadow 0.53) | post 0.51: ao 0.44 bloom 0.05 composite 0.03 |
| overcast_broken | remastered | 10.44 | 9.76 | 0.04 | 0.58 | frame 10.44 ms | world 9.76 (shadow 0.04) | post 0.58: ao 0.50 bloom 0.05 composite 0.03 |
| overcast_broken | svm_full | 11.08 | 10.38 | 0.97 | 0.6 | frame 11.08 ms | world 10.38 (shadow 0.97) | post 0.60: ao 0.52 bloom 0.05 composite 0.03 |
| overcast_broken | svm_occ | 13.05 | 12.44 | 0.49 | 0.52 | frame 13.05 ms | world 12.44 (shadow 0.49) | post 0.52: ao 0.44 bloom 0.05 composite 0.03 |
| overcast_close | classic | 4.1 | 3.88 | None | None | frame 4.10 ms | world 3.88 |
| overcast_close | dbg_vis | 5.52 | 4.8 | 0.76 | 0.56 | frame 5.52 ms | world 4.80 (shadow 0.76) | post 0.56: ao 0.40 bloom 0.09 composite 0.06 |
| overcast_close | remastered | 5.56 | 4.41 | 0.11 | 0.88 | frame 5.56 ms | world 4.41 (shadow 0.11) | post 0.88: ao 0.62 bloom 0.14 composite 0.12 |
| overcast_close | svm_full | 5.34 | 4.67 | 1.03 | 0.53 | frame 5.34 ms | world 4.67 (shadow 1.03) | post 0.53: ao 0.41 bloom 0.07 composite 0.04 |
| overcast_close | svm_occ | 6.53 | 5.42 | 0.8 | 0.89 | frame 6.53 ms | world 5.42 (shadow 0.80) | post 0.89: ao 0.65 bloom 0.14 composite 0.10 |
| town_day | classic | 2.51 | 2.38 | None | None | frame 2.51 ms | world 2.38 |
| town_day | dbg_vis | 5.44 | 4.97 | 1.21 | 0.34 | frame 5.44 ms | world 4.97 (shadow 1.21) | post 0.34: ao 0.26 bloom 0.05 composite 0.03 |
| town_day | remastered | 2.89 | 2.34 | 0.04 | 0.41 | frame 2.89 ms | world 2.34 (shadow 0.04) | post 0.41: ao 0.33 bloom 0.05 composite 0.03 |
| town_day | svm_full | 6.34 | 5.86 | 2.14 | 0.34 | frame 6.34 ms | world 5.86 (shadow 2.14) | post 0.34: ao 0.26 bloom 0.05 composite 0.03 |
| town_day | svm_occ | 4.85 | 4.4 | 1.1 | 0.31 | frame 4.85 ms | world 4.40 (shadow 1.10) | post 0.31: ao 0.24 bloom 0.05 composite 0.03 |
