# timings (r14)

| view | label | GPU frame ms | world | shadow (incl. sky maps) | post | raw |
|---|---|---|---|---|---|---|
| forest_edge_golden | B_back | 7.17 | 6.64 | 1.02 | 0.41 | frame 7.17 ms | world 6.64 (shadow 1.02) | post 0.41: ao 0.34 bloom 0.05 composite 0.03 |
| forest_edge_golden | B_front | 7.07 | 6.57 | 1.01 | 0.38 | frame 7.07 ms | world 6.57 (shadow 1.01) | post 0.38: ao 0.30 bloom 0.05 composite 0.03 |
| forest_edge_golden | C_front | 7.06 | 6.58 | 1.01 | 0.36 | frame 7.06 ms | world 6.58 (shadow 1.01) | post 0.36: ao 0.28 bloom 0.05 composite 0.03 |
| forest_edge_golden | classic | 3.91 | 3.79 | None | None | frame 3.91 ms | world 3.79 |
| forest_edge_golden | dbg_back | 7.21 | 6.71 | 1.21 | 0.38 | frame 7.21 ms | world 6.71 (shadow 1.21) | post 0.38: ao 0.30 bloom 0.05 composite 0.03 |
| forest_edge_golden | dbg_front | 7.02 | 6.55 | 1.01 | 0.36 | frame 7.02 ms | world 6.55 (shadow 1.01) | post 0.36: ao 0.28 bloom 0.05 composite 0.03 |
| forest_edge_golden | remastered | 5.12 | 4.5 | 0.25 | 0.47 | frame 5.12 ms | world 4.50 (shadow 0.25) | post 0.47: ao 0.36 bloom 0.07 composite 0.04 |
| lowsun_fp | B_back | 6.88 | 6.12 | 1.0 | 0.67 | frame 6.88 ms | world 6.12 (shadow 1.00) | post 0.67: ao 0.34 bloom 0.05 shafts 0.24 composite 0.03 |
| lowsun_fp | B_front | 6.91 | 6.15 | 1.02 | 0.66 | frame 6.91 ms | world 6.15 (shadow 1.02) | post 0.66: ao 0.34 bloom 0.05 shafts 0.24 composite 0.03 |
| lowsun_fp | C_front | 6.89 | 6.14 | 0.97 | 0.65 | frame 6.89 ms | world 6.14 (shadow 0.97) | post 0.65: ao 0.32 bloom 0.05 shafts 0.24 composite 0.03 |
| lowsun_fp | classic | 3.93 | 3.63 | None | None | frame 3.93 ms | world 3.63 |
| lowsun_fp | dbg_back | 7.12 | 6.35 | 1.15 | 0.66 | frame 7.12 ms | world 6.35 (shadow 1.15) | post 0.66: ao 0.33 bloom 0.05 shafts 0.24 composite 0.03 |
| lowsun_fp | dbg_front | 6.9 | 6.15 | 0.99 | 0.65 | frame 6.90 ms | world 6.15 (shadow 0.99) | post 0.65: ao 0.32 bloom 0.05 shafts 0.24 composite 0.03 |
| lowsun_fp | remastered | 6.32 | 5.01 | 0.13 | 1.12 | frame 6.32 ms | world 5.01 (shadow 0.13) | post 1.12: ao 0.57 bloom 0.09 shafts 0.38 composite 0.09 |
| overcast_close | B_back | 6.4 | 5.82 | 1.13 | 0.47 | frame 6.40 ms | world 5.82 (shadow 1.13) | post 0.47: ao 0.37 bloom 0.06 composite 0.04 |
| overcast_close | B_front | 5.66 | 5.12 | 1.1 | 0.43 | frame 5.66 ms | world 5.12 (shadow 1.10) | post 0.43: ao 0.35 bloom 0.05 composite 0.03 |
| overcast_close | C_front | 6.71 | 6.12 | 1.17 | 0.47 | frame 6.71 ms | world 6.12 (shadow 1.17) | post 0.47: ao 0.37 bloom 0.06 composite 0.04 |
| overcast_close | classic | 4.95 | 4.7 | None | None | frame 4.95 ms | world 4.70 |
| overcast_close | dbg_back | 6.96 | 6.35 | 1.33 | 0.49 | frame 6.96 ms | world 6.35 (shadow 1.33) | post 0.49: ao 0.38 bloom 0.06 composite 0.04 |
| overcast_close | dbg_front | 6.88 | 6.27 | 1.15 | 0.49 | frame 6.88 ms | world 6.27 (shadow 1.15) | post 0.49: ao 0.39 bloom 0.07 composite 0.04 |
| overcast_close | remastered | 6.29 | 5.02 | 0.44 | 1.03 | frame 6.29 ms | world 5.02 (shadow 0.44) | post 1.03: ao 0.78 bloom 0.12 composite 0.13 |
| town_day | B_back | 6.72 | 6.11 | 1.05 | 0.48 | frame 6.72 ms | world 6.11 (shadow 1.05) | post 0.48: ao 0.39 bloom 0.06 composite 0.03 |
| town_day | B_front | 6.27 | 5.66 | 1.08 | 0.48 | frame 6.27 ms | world 5.66 (shadow 1.08) | post 0.48: ao 0.40 bloom 0.05 composite 0.03 |
| town_day | C_front | 6.86 | 6.26 | 1.09 | 0.46 | frame 6.86 ms | world 6.26 (shadow 1.09) | post 0.46: ao 0.37 bloom 0.06 composite 0.03 |
| town_day | classic | 2.84 | 2.62 | None | None | frame 2.84 ms | world 2.62 |
| town_day | dbg_back | 7.11 | 6.5 | 1.27 | 0.48 | frame 7.11 ms | world 6.50 (shadow 1.27) | post 0.48: ao 0.39 bloom 0.06 composite 0.04 |
| town_day | dbg_front | 6.88 | 6.26 | 1.1 | 0.49 | frame 6.88 ms | world 6.26 (shadow 1.10) | post 0.49: ao 0.40 bloom 0.06 composite 0.03 |
| town_day | remastered | 5.47 | 3.77 | 0.36 | 1.27 | frame 5.47 ms | world 3.77 (shadow 0.36) | post 1.27: ao 1.03 bloom 0.11 composite 0.12 |
| town_street | B_back | 6.88 | 6.07 | 0.99 | 0.72 | frame 6.88 ms | world 6.07 (shadow 0.99) | post 0.72: ao 0.37 bloom 0.05 shafts 0.27 composite 0.03 |
| town_street | B_front | 6.78 | 5.97 | 1.0 | 0.72 | frame 6.78 ms | world 5.97 (shadow 1.00) | post 0.72: ao 0.38 bloom 0.05 shafts 0.26 composite 0.03 |
| town_street | C_front | 6.94 | 6.05 | 0.98 | 0.78 | frame 6.94 ms | world 6.05 (shadow 0.98) | post 0.78: ao 0.44 bloom 0.05 shafts 0.27 composite 0.03 |
| town_street | classic | 1.43 | 1.3 | None | None | frame 1.43 ms | world 1.30 |
| town_street | dbg_back | 7.03 | 6.2 | 1.12 | 0.73 | frame 7.03 ms | world 6.20 (shadow 1.12) | post 0.73: ao 0.39 bloom 0.05 shafts 0.26 composite 0.03 |
| town_street | dbg_front | 6.81 | 5.99 | 0.96 | 0.73 | frame 6.81 ms | world 5.99 (shadow 0.96) | post 0.73: ao 0.39 bloom 0.05 shafts 0.25 composite 0.03 |
| town_street | remastered | 5.64 | 3.55 | 0.41 | 1.8 | frame 5.64 ms | world 3.55 (shadow 0.41) | post 1.80: ao 0.96 bloom 0.12 shafts 0.59 composite 0.13 |
| woods_mixed_noon | B_back | 10.74 | 10.12 | 1.06 | 0.52 | frame 10.74 ms | world 10.12 (shadow 1.06) | post 0.52: ao 0.45 bloom 0.05 composite 0.03 |
| woods_mixed_noon | B_front | 10.65 | 10.09 | 1.04 | 0.48 | frame 10.65 ms | world 10.09 (shadow 1.04) | post 0.48: ao 0.40 bloom 0.05 composite 0.03 |
| woods_mixed_noon | C_front | 10.75 | 10.14 | 1.05 | 0.53 | frame 10.75 ms | world 10.14 (shadow 1.05) | post 0.53: ao 0.45 bloom 0.05 composite 0.03 |
| woods_mixed_noon | classic | 2.44 | 2.35 | None | None | frame 2.44 ms | world 2.35 |
| woods_mixed_noon | dbg_back | 10.88 | 10.27 | 1.22 | 0.52 | frame 10.88 ms | world 10.27 (shadow 1.22) | post 0.52: ao 0.44 bloom 0.05 composite 0.03 |
| woods_mixed_noon | dbg_front | 10.73 | 10.13 | 1.04 | 0.52 | frame 10.73 ms | world 10.13 (shadow 1.04) | post 0.52: ao 0.44 bloom 0.05 composite 0.03 |
| woods_mixed_noon | remastered | 6.85 | 5.28 | 0.46 | 1.34 | frame 6.85 ms | world 5.28 (shadow 0.46) | post 1.34: ao 1.11 bloom 0.12 composite 0.10 |
| woods_pine2_evening | B_back | 10.47 | 9.63 | 1.14 | 0.75 | frame 10.47 ms | world 9.63 (shadow 1.14) | post 0.75: ao 0.39 bloom 0.05 shafts 0.29 composite 0.03 |
| woods_pine2_evening | B_front | 10.46 | 9.62 | 1.15 | 0.75 | frame 10.46 ms | world 9.62 (shadow 1.15) | post 0.75: ao 0.39 bloom 0.05 shafts 0.29 composite 0.03 |
| woods_pine2_evening | C_front | 10.49 | 9.65 | 1.15 | 0.75 | frame 10.49 ms | world 9.65 (shadow 1.15) | post 0.75: ao 0.38 bloom 0.05 shafts 0.29 composite 0.03 |
| woods_pine2_evening | classic | 3.59 | 3.5 | None | None | frame 3.59 ms | world 3.50 |
| woods_pine2_evening | dbg_back | 10.62 | 9.76 | 1.31 | 0.77 | frame 10.62 ms | world 9.76 (shadow 1.31) | post 0.77: ao 0.41 bloom 0.05 shafts 0.28 composite 0.03 |
| woods_pine2_evening | dbg_front | 10.42 | 9.59 | 1.14 | 0.75 | frame 10.42 ms | world 9.59 (shadow 1.14) | post 0.75: ao 0.39 bloom 0.05 shafts 0.28 composite 0.03 |
| woods_pine2_evening | remastered | 6.72 | 5.71 | 0.37 | 0.91 | frame 6.72 ms | world 5.71 (shadow 0.37) | post 0.91: ao 0.42 bloom 0.06 shafts 0.38 composite 0.04 |
