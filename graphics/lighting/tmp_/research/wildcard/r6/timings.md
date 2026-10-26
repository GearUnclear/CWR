# timings (r6)

| view | label | GPU frame ms | world | shadow (incl. sky maps) | post | raw |
|---|---|---|---|---|---|---|
| forest_edge_golden | classic | 7.82 | 7.69 | None | None | frame 7.82 ms | world 7.69 |
| forest_edge_golden | dbg_vis | 5.85 | 5.36 | 0.89 | 0.35 | frame 5.85 ms | world 5.36 (shadow 0.89) | post 0.35: ao 0.27 bloom 0.05 composite 0.03 |
| forest_edge_golden | remastered | 5.25 | 4.77 | 0.23 | 0.34 | frame 5.25 ms | world 4.77 (shadow 0.23) | post 0.34: ao 0.26 bloom 0.05 composite 0.03 |
| forest_edge_golden | svm_full | 6.96 | 6.44 | 1.13 | 0.38 | frame 6.96 ms | world 6.44 (shadow 1.13) | post 0.38: ao 0.30 bloom 0.05 composite 0.03 |
| forest_edge_golden | svm_occ | 8.62 | 8.09 | 0.71 | 0.4 | frame 8.62 ms | world 8.09 (shadow 0.71) | post 0.40: ao 0.32 bloom 0.05 composite 0.03 |
| lowsun_fp | classic | 3.55 | 3.45 | None | None | frame 3.55 ms | world 3.45 |
| lowsun_fp | dbg_vis | 7.79 | 6.91 | 1.16 | 0.77 | frame 7.79 ms | world 6.91 (shadow 1.16) | post 0.77: ao 0.31 bloom 0.05 shafts 0.21 composite 0.20 |
| lowsun_fp | remastered | 3.71 | 2.95 | 0.06 | 0.65 | frame 3.71 ms | world 2.95 (shadow 0.06) | post 0.65: ao 0.35 bloom 0.05 shafts 0.22 composite 0.03 |
| lowsun_fp | svm_full | 10.97 | 10.1 | 3.37 | 0.77 | frame 10.97 ms | world 10.10 (shadow 3.37) | post 0.77: ao 0.27 bloom 0.05 shafts 0.22 composite 0.23 |
| lowsun_fp | svm_occ | 4.99 | 4.23 | 0.57 | 0.65 | frame 4.99 ms | world 4.23 (shadow 0.57) | post 0.65: ao 0.36 bloom 0.05 shafts 0.22 composite 0.03 |
| overcast_broken | classic | 10.62 | 10.52 | None | None | frame 10.62 ms | world 10.52 |
| overcast_broken | dbg_vis | 13.78 | 13.11 | 0.77 | 0.57 | frame 13.78 ms | world 13.11 (shadow 0.77) | post 0.57: ao 0.49 bloom 0.05 composite 0.03 |
| overcast_broken | remastered | 10.69 | 10.06 | 0.09 | 0.54 | frame 10.69 ms | world 10.06 (shadow 0.09) | post 0.54: ao 0.46 bloom 0.05 composite 0.03 |
| overcast_broken | svm_full | 12.64 | 11.99 | 1.25 | 0.55 | frame 12.64 ms | world 11.99 (shadow 1.25) | post 0.55: ao 0.48 bloom 0.05 composite 0.03 |
| overcast_broken | svm_occ | 13.37 | 12.76 | 0.63 | 0.52 | frame 13.37 ms | world 12.76 (shadow 0.63) | post 0.52: ao 0.44 bloom 0.05 composite 0.03 |
| overcast_close | classic | 3.21 | 3.04 | None | None | frame 3.21 ms | world 3.04 |
| overcast_close | dbg_vis | 4.2 | 3.65 | 0.75 | 0.42 | frame 4.20 ms | world 3.65 (shadow 0.75) | post 0.42: ao 0.33 bloom 0.06 composite 0.04 |
| overcast_close | remastered | 4.75 | 3.78 | 0.29 | 0.72 | frame 4.75 ms | world 3.78 (shadow 0.29) | post 0.72: ao 0.57 bloom 0.09 composite 0.06 |
| overcast_close | svm_full | 4.91 | 4.32 | 1.16 | 0.47 | frame 4.91 ms | world 4.32 (shadow 1.16) | post 0.47: ao 0.37 bloom 0.06 composite 0.04 |
| overcast_close | svm_occ | 4.25 | 3.69 | 0.71 | 0.43 | frame 4.25 ms | world 3.69 (shadow 0.71) | post 0.43: ao 0.33 bloom 0.06 composite 0.04 |
| town_day | classic | 2.26 | 2.14 | None | None | frame 2.26 ms | world 2.14 |
| town_day | dbg_vis | 4.07 | 3.48 | 0.89 | 0.45 | frame 4.07 ms | world 3.48 (shadow 0.89) | post 0.45: ao 0.37 bloom 0.05 composite 0.03 |
| town_day | remastered | 2.48 | 1.83 | 0.14 | 0.5 | frame 2.48 ms | world 1.83 (shadow 0.14) | post 0.50: ao 0.41 bloom 0.06 composite 0.03 |
| town_day | svm_full | 4.41 | 3.85 | 0.98 | 0.42 | frame 4.41 ms | world 3.85 (shadow 0.98) | post 0.42: ao 0.34 bloom 0.05 composite 0.03 |
| town_day | svm_occ | 3.58 | 3.01 | 0.69 | 0.43 | frame 3.58 ms | world 3.01 (shadow 0.69) | post 0.43: ao 0.35 bloom 0.05 composite 0.03 |
| town_street | classic | 2.41 | 2.3 | None | None | frame 2.41 ms | world 2.30 |
| town_street | dbg_vis | 7.39 | 6.29 | 1.44 | 0.98 | frame 7.39 ms | world 6.29 (shadow 1.44) | post 0.98: ao 0.28 bloom 0.05 shafts 0.26 composite 0.39 |
| town_street | remastered | 3.19 | 2.15 | 0.13 | 0.93 | frame 3.19 ms | world 2.15 (shadow 0.13) | post 0.93: ao 0.31 bloom 0.05 shafts 0.26 composite 0.31 |
| town_street | svm_full | 8.6 | 7.47 | 2.74 | 1.03 | frame 8.60 ms | world 7.47 (shadow 2.74) | post 1.03: ao 0.33 bloom 0.05 shafts 0.26 composite 0.38 |
| town_street | svm_occ | 5.88 | 4.72 | 1.11 | 1.04 | frame 5.88 ms | world 4.72 (shadow 1.11) | post 1.04: ao 0.32 bloom 0.05 shafts 0.26 composite 0.40 |
| woods_mixed_noon | classic | 3.0 | 2.91 | None | None | frame 3.00 ms | world 2.91 |
| woods_mixed_noon | dbg_vis | 6.13 | 5.52 | 0.71 | 0.51 | frame 6.13 ms | world 5.52 (shadow 0.71) | post 0.51: ao 0.43 bloom 0.05 composite 0.03 |
| woods_mixed_noon | remastered | 3.09 | 2.5 | 0.18 | 0.49 | frame 3.09 ms | world 2.50 (shadow 0.18) | post 0.49: ao 0.42 bloom 0.05 composite 0.03 |
| woods_mixed_noon | svm_full | 10.33 | 9.75 | 2.55 | 0.49 | frame 10.33 ms | world 9.75 (shadow 2.55) | post 0.49: ao 0.41 bloom 0.05 composite 0.03 |
| woods_mixed_noon | svm_occ | 10.02 | 9.59 | 2.2 | 0.33 | frame 10.02 ms | world 9.59 (shadow 2.20) | post 0.33: ao 0.26 bloom 0.05 composite 0.03 |
| woods_overcast | classic | 2.36 | 2.25 | None | None | frame 2.36 ms | world 2.25 |
| woods_overcast | dbg_vis | 7.25 | 6.74 | 1.45 | 0.4 | frame 7.25 ms | world 6.74 (shadow 1.45) | post 0.40: ao 0.32 bloom 0.05 composite 0.03 |
| woods_overcast | remastered | 5.0 | 4.47 | 0.45 | 0.41 | frame 5.00 ms | world 4.47 (shadow 0.45) | post 0.41: ao 0.33 bloom 0.06 composite 0.03 |
| woods_overcast | svm_full | 9.23 | 8.71 | 1.81 | 0.41 | frame 9.23 ms | world 8.71 (shadow 1.81) | post 0.41: ao 0.34 bloom 0.05 composite 0.03 |
| woods_overcast | svm_occ | 7.54 | 6.9 | 1.16 | 0.53 | frame 7.54 ms | world 6.90 (shadow 1.16) | post 0.53: ao 0.45 bloom 0.05 composite 0.03 |
| woods_pine2_evening | classic | 4.28 | 4.18 | None | None | frame 4.28 ms | world 4.18 |
| woods_pine2_evening | dbg_vis | 6.66 | 5.81 | 0.85 | 0.75 | frame 6.66 ms | world 5.81 (shadow 0.85) | post 0.75: ao 0.40 bloom 0.05 shafts 0.27 composite 0.03 |
| woods_pine2_evening | remastered | 5.26 | 4.4 | 0.29 | 0.77 | frame 5.26 ms | world 4.40 (shadow 0.29) | post 0.77: ao 0.39 bloom 0.05 shafts 0.29 composite 0.04 |
| woods_pine2_evening | svm_full | 7.32 | 6.45 | 1.15 | 0.76 | frame 7.32 ms | world 6.45 (shadow 1.15) | post 0.76: ao 0.39 bloom 0.05 shafts 0.28 composite 0.04 |
| woods_pine2_evening | svm_occ | 7.95 | 6.92 | 1.19 | 0.93 | frame 7.95 ms | world 6.92 (shadow 1.19) | post 0.93: ao 0.43 bloom 0.05 shafts 0.28 composite 0.17 |
