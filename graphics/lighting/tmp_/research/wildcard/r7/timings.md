# timings (r7)

| view | label | GPU frame ms | world | shadow (incl. sky maps) | post | raw |
|---|---|---|---|---|---|---|
| lowsun_fp | A_phys | 7.31 | 6.27 | 1.78 | 0.94 | frame 7.31 ms | world 6.27 (shadow 1.78) | post 0.94: ao 0.26 bloom 0.05 shafts 0.22 composite 0.41 |
| lowsun_fp | B_ground | 7.47 | 6.38 | 1.44 | 0.99 | frame 7.47 ms | world 6.38 (shadow 1.44) | post 0.99: ao 0.20 bloom 0.05 shafts 0.22 composite 0.53 |
| lowsun_fp | C_gain | 5.69 | 4.77 | 1.03 | 0.81 | frame 5.69 ms | world 4.77 (shadow 1.03) | post 0.81: ao 0.35 bloom 0.05 shafts 0.22 composite 0.19 |
| lowsun_fp | D_tint | 6.07 | 5.25 | 0.93 | 0.71 | frame 6.07 ms | world 5.25 (shadow 0.93) | post 0.71: ao 0.34 bloom 0.05 shafts 0.22 composite 0.10 |
| lowsun_fp | classic | 3.38 | 3.27 | None | None | frame 3.38 ms | world 3.27 |
| lowsun_fp | dbg_visA | 4.9 | 4.17 | 0.88 | 0.62 | frame 4.90 ms | world 4.17 (shadow 0.88) | post 0.62: ao 0.33 bloom 0.05 shafts 0.21 composite 0.03 |
| lowsun_fp | remastered | 6.96 | 5.84 | 0.06 | 1.01 | frame 6.96 ms | world 5.84 (shadow 0.06) | post 1.01: ao 0.16 bloom 0.05 shafts 0.22 composite 0.58 |
| overcast_close | A_phys | 4.95 | 4.37 | 1.22 | 0.46 | frame 4.95 ms | world 4.37 (shadow 1.22) | post 0.46: ao 0.38 bloom 0.05 composite 0.03 |
| overcast_close | B_ground | 4.95 | 4.44 | 1.13 | 0.41 | frame 4.95 ms | world 4.44 (shadow 1.13) | post 0.41: ao 0.33 bloom 0.05 composite 0.03 |
| overcast_close | C_gain | 4.7 | 4.1 | 1.16 | 0.48 | frame 4.70 ms | world 4.10 (shadow 1.16) | post 0.48: ao 0.39 bloom 0.06 composite 0.03 |
| overcast_close | D_tint | 5.64 | 4.96 | 1.17 | 0.54 | frame 5.64 ms | world 4.96 (shadow 1.17) | post 0.54: ao 0.42 bloom 0.07 composite 0.04 |
| overcast_close | classic | 2.88 | 2.74 | None | None | frame 2.88 ms | world 2.74 |
| overcast_close | dbg_visA | 5.75 | 5.08 | 1.21 | 0.52 | frame 5.75 ms | world 5.08 (shadow 1.21) | post 0.52: ao 0.39 bloom 0.08 composite 0.05 |
| overcast_close | remastered | 3.31 | 2.72 | 0.16 | 0.45 | frame 3.31 ms | world 2.72 (shadow 0.16) | post 0.45: ao 0.35 bloom 0.06 composite 0.04 |
| town_day | A_phys | 4.69 | 4.17 | 1.26 | 0.39 | frame 4.69 ms | world 4.17 (shadow 1.26) | post 0.39: ao 0.31 bloom 0.05 composite 0.03 |
| town_day | B_ground | 4.07 | 3.49 | 1.0 | 0.45 | frame 4.07 ms | world 3.49 (shadow 1.00) | post 0.45: ao 0.37 bloom 0.05 composite 0.03 |
| town_day | C_gain | 6.29 | 5.82 | 2.03 | 0.34 | frame 6.29 ms | world 5.82 (shadow 2.03) | post 0.34: ao 0.27 bloom 0.05 composite 0.03 |
| town_day | D_tint | 5.05 | 4.58 | 1.51 | 0.33 | frame 5.05 ms | world 4.58 (shadow 1.51) | post 0.33: ao 0.25 bloom 0.05 composite 0.03 |
| town_day | classic | 3.79 | 3.66 | None | None | frame 3.79 ms | world 3.66 |
| town_day | dbg_visA | 6.06 | 5.53 | 1.82 | 0.39 | frame 6.06 ms | world 5.53 (shadow 1.82) | post 0.39: ao 0.31 bloom 0.05 composite 0.03 |
| town_day | remastered | 2.32 | 1.78 | 0.12 | 0.41 | frame 2.32 ms | world 1.78 (shadow 0.12) | post 0.41: ao 0.33 bloom 0.05 composite 0.03 |
| town_street | A_phys | 6.24 | 5.33 | 1.63 | 0.81 | frame 6.24 ms | world 5.33 (shadow 1.63) | post 0.81: ao 0.32 bloom 0.05 shafts 0.26 composite 0.17 |
| town_street | B_ground | 6.77 | 5.49 | 2.03 | 1.17 | frame 6.77 ms | world 5.49 (shadow 2.03) | post 1.17: ao 0.32 bloom 0.05 shafts 0.26 composite 0.53 |
| town_street | C_gain | 8.43 | 7.34 | 2.57 | 0.98 | frame 8.43 ms | world 7.34 (shadow 2.57) | post 0.98: ao 0.24 bloom 0.05 shafts 0.26 composite 0.42 |
| town_street | D_tint | 7.75 | 6.56 | 2.43 | 1.08 | frame 7.75 ms | world 6.56 (shadow 2.43) | post 1.08: ao 0.23 bloom 0.05 shafts 0.26 composite 0.53 |
| town_street | classic | 2.05 | 1.95 | None | None | frame 2.05 ms | world 1.95 |
| town_street | dbg_visA | 4.8 | 3.93 | 1.09 | 0.78 | frame 4.80 ms | world 3.93 (shadow 1.09) | post 0.78: ao 0.43 bloom 0.05 shafts 0.26 composite 0.04 |
| town_street | remastered | 3.59 | 2.48 | 0.15 | 1.0 | frame 3.59 ms | world 2.48 (shadow 0.15) | post 1.00: ao 0.38 bloom 0.05 shafts 0.29 composite 0.28 |
| woods_mixed_noon | A_phys | 10.01 | 9.5 | 1.94 | 0.42 | frame 10.01 ms | world 9.50 (shadow 1.94) | post 0.42: ao 0.35 bloom 0.05 composite 0.03 |
| woods_mixed_noon | B_ground | 10.12 | 9.58 | 2.09 | 0.46 | frame 10.12 ms | world 9.58 (shadow 2.09) | post 0.46: ao 0.38 bloom 0.05 composite 0.03 |
| woods_mixed_noon | C_gain | 9.43 | 8.91 | 2.31 | 0.42 | frame 9.43 ms | world 8.91 (shadow 2.31) | post 0.42: ao 0.34 bloom 0.05 composite 0.03 |
| woods_mixed_noon | D_tint | 9.73 | 9.15 | 1.78 | 0.49 | frame 9.73 ms | world 9.15 (shadow 1.78) | post 0.49: ao 0.41 bloom 0.05 composite 0.03 |
| woods_mixed_noon | classic | 2.83 | 2.73 | None | None | frame 2.83 ms | world 2.73 |
| woods_mixed_noon | dbg_visA | 9.49 | 8.96 | 1.96 | 0.43 | frame 9.49 ms | world 8.96 (shadow 1.96) | post 0.43: ao 0.36 bloom 0.05 composite 0.03 |
| woods_mixed_noon | remastered | 5.32 | 4.6 | 0.27 | 0.59 | frame 5.32 ms | world 4.60 (shadow 0.27) | post 0.59: ao 0.48 bloom 0.07 composite 0.04 |
| woods_overcast | A_phys | 9.6 | 9.06 | 1.86 | 0.44 | frame 9.60 ms | world 9.06 (shadow 1.86) | post 0.44: ao 0.36 bloom 0.05 composite 0.03 |
| woods_overcast | B_ground | 7.05 | 6.43 | 1.18 | 0.52 | frame 7.05 ms | world 6.43 (shadow 1.18) | post 0.52: ao 0.44 bloom 0.05 composite 0.03 |
| woods_overcast | C_gain | 8.62 | 8.0 | 1.39 | 0.53 | frame 8.62 ms | world 8.00 (shadow 1.39) | post 0.53: ao 0.45 bloom 0.05 composite 0.03 |
| woods_overcast | D_tint | 8.46 | 7.9 | 1.92 | 0.45 | frame 8.46 ms | world 7.90 (shadow 1.92) | post 0.45: ao 0.37 bloom 0.05 composite 0.03 |
| woods_overcast | classic | 3.02 | 2.91 | None | None | frame 3.02 ms | world 2.91 |
| woods_overcast | dbg_visA | 6.56 | 5.97 | 1.02 | 0.48 | frame 6.56 ms | world 5.97 (shadow 1.02) | post 0.48: ao 0.40 bloom 0.05 composite 0.03 |
| woods_overcast | remastered | 5.15 | 4.61 | 0.21 | 0.43 | frame 5.15 ms | world 4.61 (shadow 0.21) | post 0.43: ao 0.35 bloom 0.05 composite 0.03 |
