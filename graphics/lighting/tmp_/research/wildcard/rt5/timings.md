# timings (rt5)

| view | label | GPU frame ms | world | shadow (incl. sky maps) | post | raw |
|---|---|---|---|---|---|---|
| lowsun_fp | t1_B | 6.64 | 5.9 | 0.98 | 0.64 | frame 6.64 ms | world 5.90 (shadow 0.98) | post 0.64: ao 0.34 bloom 0.05 shafts 0.22 composite 0.03 |
| lowsun_fp | t1_G | 8.15 | 7.42 | 1.5 | 0.64 | frame 8.15 ms | world 7.42 (shadow 1.50) | post 0.64: ao 0.34 bloom 0.05 shafts 0.22 composite 0.03 |
| lowsun_fp | t1_off | 5.3 | 3.63 | 0.13 | 1.4 | frame 5.30 ms | world 3.63 (shadow 0.13) | post 1.40: ao 0.79 bloom 0.12 shafts 0.37 composite 0.12 |
| lowsun_fp | t2_B | 6.53 | 5.81 | 0.92 | 0.62 | frame 6.53 ms | world 5.81 (shadow 0.92) | post 0.62: ao 0.33 bloom 0.05 shafts 0.22 composite 0.03 |
| lowsun_fp | t2_G | 8.19 | 7.43 | 1.42 | 0.66 | frame 8.19 ms | world 7.43 (shadow 1.42) | post 0.66: ao 0.37 bloom 0.05 shafts 0.22 composite 0.03 |
| lowsun_fp | t2_off | 4.64 | 3.75 | 0.08 | 0.78 | frame 4.64 ms | world 3.75 (shadow 0.08) | post 0.78: ao 0.34 bloom 0.06 shafts 0.28 composite 0.09 |
| lowsun_fp | t3_B | 6.62 | 5.86 | 0.93 | 0.66 | frame 6.62 ms | world 5.86 (shadow 0.93) | post 0.66: ao 0.36 bloom 0.05 shafts 0.22 composite 0.03 |
| lowsun_fp | t3_G | 8.04 | 7.31 | 1.32 | 0.63 | frame 8.04 ms | world 7.31 (shadow 1.32) | post 0.63: ao 0.33 bloom 0.05 shafts 0.22 composite 0.03 |
| lowsun_fp | t3_off | 4.77 | 3.89 | 0.08 | 0.75 | frame 4.77 ms | world 3.89 (shadow 0.08) | post 0.75: ao 0.36 bloom 0.06 shafts 0.29 composite 0.04 |
| overcast_close | t1_B | 5.5 | 4.96 | 1.1 | 0.44 | frame 5.50 ms | world 4.96 (shadow 1.10) | post 0.44: ao 0.37 bloom 0.05 composite 0.03 |
| overcast_close | t1_G | 6.95 | 6.45 | 1.51 | 0.4 | frame 6.95 ms | world 6.45 (shadow 1.51) | post 0.40: ao 0.33 bloom 0.05 composite 0.03 |
| overcast_close | t1_off | 4.54 | 3.82 | 0.2 | 0.54 | frame 4.54 ms | world 3.82 (shadow 0.20) | post 0.54: ao 0.38 bloom 0.10 composite 0.06 |
| overcast_close | t2_B | 6.56 | 5.96 | 1.13 | 0.49 | frame 6.56 ms | world 5.96 (shadow 1.13) | post 0.49: ao 0.39 bloom 0.06 composite 0.04 |
| overcast_close | t2_G | 6.69 | 6.19 | 1.42 | 0.4 | frame 6.69 ms | world 6.19 (shadow 1.42) | post 0.40: ao 0.32 bloom 0.05 composite 0.03 |
| overcast_close | t2_off | 3.94 | 3.3 | 0.18 | 0.5 | frame 3.94 ms | world 3.30 (shadow 0.18) | post 0.50: ao 0.38 bloom 0.08 composite 0.05 |
| overcast_close | t3_B | 6.55 | 5.96 | 1.09 | 0.47 | frame 6.55 ms | world 5.96 (shadow 1.09) | post 0.47: ao 0.38 bloom 0.06 composite 0.04 |
| overcast_close | t3_G | 6.93 | 6.4 | 1.42 | 0.43 | frame 6.93 ms | world 6.40 (shadow 1.42) | post 0.43: ao 0.36 bloom 0.05 composite 0.03 |
| overcast_close | t3_off | 3.73 | 3.12 | 0.16 | 0.48 | frame 3.73 ms | world 3.12 (shadow 0.16) | post 0.48: ao 0.37 bloom 0.07 composite 0.04 |
| town_street | t1_B | 7.07 | 6.17 | 0.98 | 0.79 | frame 7.07 ms | world 6.17 (shadow 0.98) | post 0.79: ao 0.43 bloom 0.05 shafts 0.26 composite 0.04 |
| town_street | t1_G | 10.1 | 9.18 | 1.45 | 0.82 | frame 10.10 ms | world 9.18 (shadow 1.45) | post 0.82: ao 0.46 bloom 0.05 shafts 0.26 composite 0.05 |
| town_street | t1_off | 3.35 | 2.21 | 0.21 | 0.99 | frame 3.35 ms | world 2.21 (shadow 0.21) | post 0.99: ao 0.47 bloom 0.07 shafts 0.39 composite 0.05 |
| town_street | t2_B | 6.86 | 6.0 | 0.95 | 0.76 | frame 6.86 ms | world 6.00 (shadow 0.95) | post 0.76: ao 0.42 bloom 0.05 shafts 0.26 composite 0.03 |
| town_street | t2_G | 9.97 | 9.1 | 1.36 | 0.77 | frame 9.97 ms | world 9.10 (shadow 1.36) | post 0.77: ao 0.41 bloom 0.05 shafts 0.27 composite 0.04 |
| town_street | t2_off | 3.47 | 2.25 | 0.22 | 1.08 | frame 3.47 ms | world 2.25 (shadow 0.22) | post 1.08: ao 0.48 bloom 0.08 shafts 0.41 composite 0.11 |
| town_street | t3_B | 6.91 | 5.99 | 0.92 | 0.82 | frame 6.91 ms | world 5.99 (shadow 0.92) | post 0.82: ao 0.43 bloom 0.05 shafts 0.26 composite 0.07 |
| town_street | t3_G | 9.74 | 8.91 | 1.44 | 0.74 | frame 9.74 ms | world 8.91 (shadow 1.44) | post 0.74: ao 0.40 bloom 0.05 shafts 0.27 composite 0.03 |
| town_street | t3_off | 3.63 | 2.44 | 0.22 | 1.04 | frame 3.63 ms | world 2.44 (shadow 0.22) | post 1.04: ao 0.47 bloom 0.08 shafts 0.41 composite 0.08 |
| woods_mixed_noon | t1_B | 11.03 | 10.44 | 1.07 | 0.5 | frame 11.03 ms | world 10.44 (shadow 1.07) | post 0.50: ao 0.43 bloom 0.05 composite 0.03 |
| woods_mixed_noon | t1_G | 15.92 | 15.31 | 1.54 | 0.52 | frame 15.92 ms | world 15.31 (shadow 1.54) | post 0.52: ao 0.45 bloom 0.05 composite 0.03 |
| woods_mixed_noon | t1_off | 3.87 | 3.19 | 0.23 | 0.57 | frame 3.87 ms | world 3.19 (shadow 0.23) | post 0.57: ao 0.48 bloom 0.06 composite 0.03 |
| woods_mixed_noon | t2_B | 10.75 | 10.19 | 1.03 | 0.48 | frame 10.75 ms | world 10.19 (shadow 1.03) | post 0.48: ao 0.40 bloom 0.05 composite 0.03 |
| woods_mixed_noon | t2_G | 15.97 | 15.33 | 1.54 | 0.55 | frame 15.97 ms | world 15.33 (shadow 1.54) | post 0.55: ao 0.47 bloom 0.05 composite 0.03 |
| woods_mixed_noon | t2_off | 4.36 | 3.69 | 0.26 | 0.56 | frame 4.36 ms | world 3.69 (shadow 0.26) | post 0.56: ao 0.45 bloom 0.07 composite 0.04 |
| woods_mixed_noon | t3_B | 11.05 | 10.42 | 1.05 | 0.54 | frame 11.05 ms | world 10.42 (shadow 1.05) | post 0.54: ao 0.46 bloom 0.05 composite 0.03 |
| woods_mixed_noon | t3_G | 15.52 | 14.93 | 1.47 | 0.5 | frame 15.52 ms | world 14.93 (shadow 1.47) | post 0.50: ao 0.42 bloom 0.05 composite 0.03 |
| woods_mixed_noon | t3_off | 4.44 | 3.74 | 0.25 | 0.58 | frame 4.44 ms | world 3.74 (shadow 0.25) | post 0.58: ao 0.47 bloom 0.07 composite 0.04 |
