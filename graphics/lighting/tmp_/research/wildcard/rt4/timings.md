# timings (rt4)

| view | label | GPU frame ms | world | shadow (incl. sky maps) | post | raw |
|---|---|---|---|---|---|---|
| lowsun_fp | t1_d8 | 5.92 | 4.99 | 0.57 | 0.8 | frame 5.92 ms | world 4.99 (shadow 0.57) | post 0.80: ao 0.38 bloom 0.07 shafts 0.31 composite 0.04 |
| lowsun_fp | t1_off | 5.84 | 4.01 | 0.15 | 1.56 | frame 5.84 ms | world 4.01 (shadow 0.15) | post 1.56: ao 0.72 bloom 0.14 shafts 0.59 composite 0.11 |
| lowsun_fp | t1_on | 5.34 | 4.61 | 0.96 | 0.63 | frame 5.34 ms | world 4.61 (shadow 0.96) | post 0.63: ao 0.31 bloom 0.05 shafts 0.24 composite 0.03 |
| lowsun_fp | t2_d8 | 5.61 | 4.71 | 0.57 | 0.77 | frame 5.61 ms | world 4.71 (shadow 0.57) | post 0.77: ao 0.37 bloom 0.07 shafts 0.29 composite 0.04 |
| lowsun_fp | t2_off | 5.05 | 4.12 | 0.08 | 0.8 | frame 5.05 ms | world 4.12 (shadow 0.08) | post 0.80: ao 0.36 bloom 0.07 shafts 0.32 composite 0.05 |
| lowsun_fp | t2_on | 6.81 | 5.92 | 1.1 | 0.77 | frame 6.81 ms | world 5.92 (shadow 1.10) | post 0.77: ao 0.35 bloom 0.07 shafts 0.31 composite 0.04 |
| lowsun_fp | t3_d8 | 6.06 | 5.12 | 0.57 | 0.8 | frame 6.06 ms | world 5.12 (shadow 0.57) | post 0.80: ao 0.36 bloom 0.07 shafts 0.33 composite 0.05 |
| lowsun_fp | t3_off | 5.67 | 4.65 | 0.09 | 0.86 | frame 5.67 ms | world 4.65 (shadow 0.09) | post 0.86: ao 0.37 bloom 0.08 shafts 0.35 composite 0.05 |
| lowsun_fp | t3_on | 5.82 | 5.0 | 1.0 | 0.7 | frame 5.82 ms | world 5.00 (shadow 1.00) | post 0.70: ao 0.34 bloom 0.06 shafts 0.26 composite 0.04 |
| overcast_close | t1_d8 | 5.58 | 4.87 | 0.75 | 0.55 | frame 5.58 ms | world 4.87 (shadow 0.75) | post 0.55: ao 0.39 bloom 0.10 composite 0.06 |
| overcast_close | t1_off | 6.02 | 4.79 | 0.43 | 0.98 | frame 6.02 ms | world 4.79 (shadow 0.43) | post 0.98: ao 0.76 bloom 0.12 composite 0.10 |
| overcast_close | t1_on | 5.79 | 5.11 | 1.22 | 0.53 | frame 5.79 ms | world 5.11 (shadow 1.22) | post 0.53: ao 0.41 bloom 0.08 composite 0.05 |
| overcast_close | t2_d8 | 4.56 | 3.96 | 0.67 | 0.47 | frame 4.56 ms | world 3.96 (shadow 0.67) | post 0.47: ao 0.35 bloom 0.07 composite 0.04 |
| overcast_close | t2_off | 6.31 | 5.04 | 0.44 | 1.03 | frame 6.31 ms | world 5.04 (shadow 0.44) | post 1.03: ao 0.80 bloom 0.12 composite 0.10 |
| overcast_close | t2_on | 5.71 | 5.06 | 1.21 | 0.51 | frame 5.71 ms | world 5.06 (shadow 1.21) | post 0.51: ao 0.39 bloom 0.08 composite 0.05 |
| overcast_close | t3_d8 | 4.73 | 4.1 | 0.69 | 0.49 | frame 4.73 ms | world 4.10 (shadow 0.69) | post 0.49: ao 0.37 bloom 0.08 composite 0.05 |
| overcast_close | t3_off | 4.88 | 4.13 | 0.22 | 0.57 | frame 4.88 ms | world 4.13 (shadow 0.22) | post 0.57: ao 0.39 bloom 0.11 composite 0.07 |
| overcast_close | t3_on | 5.21 | 4.57 | 1.17 | 0.52 | frame 5.21 ms | world 4.57 (shadow 1.17) | post 0.52: ao 0.41 bloom 0.07 composite 0.04 |
| town_day | t1_d8 | 5.45 | 4.65 | 0.81 | 0.59 | frame 5.45 ms | world 4.65 (shadow 0.81) | post 0.59: ao 0.43 bloom 0.10 composite 0.06 |
| town_day | t1_off | 4.12 | 3.21 | 0.27 | 0.68 | frame 4.12 ms | world 3.21 (shadow 0.27) | post 0.68: ao 0.49 bloom 0.11 composite 0.08 |
| town_day | t1_on | 5.59 | 4.91 | 1.2 | 0.52 | frame 5.59 ms | world 4.91 (shadow 1.20) | post 0.52: ao 0.40 bloom 0.07 composite 0.04 |
| town_day | t2_d8 | 5.3 | 4.53 | 0.79 | 0.56 | frame 5.30 ms | world 4.53 (shadow 0.79) | post 0.56: ao 0.40 bloom 0.10 composite 0.06 |
| town_day | t2_off | 5.74 | 4.03 | 0.39 | 1.35 | frame 5.74 ms | world 4.03 (shadow 0.39) | post 1.35: ao 1.11 bloom 0.13 composite 0.11 |
| town_day | t2_on | 5.53 | 4.83 | 1.18 | 0.54 | frame 5.53 ms | world 4.83 (shadow 1.18) | post 0.54: ao 0.42 bloom 0.07 composite 0.04 |
| town_day | t3_d8 | 5.35 | 4.54 | 0.78 | 0.6 | frame 5.35 ms | world 4.54 (shadow 0.78) | post 0.60: ao 0.44 bloom 0.10 composite 0.06 |
| town_day | t3_off | 5.63 | 3.94 | 0.39 | 1.35 | frame 5.63 ms | world 3.94 (shadow 0.39) | post 1.35: ao 1.10 bloom 0.13 composite 0.12 |
| town_day | t3_on | 5.4 | 4.75 | 1.17 | 0.49 | frame 5.40 ms | world 4.75 (shadow 1.17) | post 0.49: ao 0.38 bloom 0.07 composite 0.04 |
| woods_mixed_noon | t1_d8 | 5.95 | 5.32 | 0.7 | 0.52 | frame 5.95 ms | world 5.32 (shadow 0.70) | post 0.52: ao 0.43 bloom 0.06 composite 0.03 |
| woods_mixed_noon | t1_off | 4.89 | 4.12 | 0.3 | 0.64 | frame 4.89 ms | world 4.12 (shadow 0.30) | post 0.64: ao 0.51 bloom 0.08 composite 0.05 |
| woods_mixed_noon | t1_on | 7.01 | 6.38 | 1.16 | 0.55 | frame 7.01 ms | world 6.38 (shadow 1.16) | post 0.55: ao 0.47 bloom 0.05 composite 0.03 |
| woods_mixed_noon | t2_d8 | 6.19 | 5.49 | 0.72 | 0.58 | frame 6.19 ms | world 5.49 (shadow 0.72) | post 0.58: ao 0.49 bloom 0.06 composite 0.03 |
| woods_mixed_noon | t2_off | 4.84 | 4.09 | 0.3 | 0.62 | frame 4.84 ms | world 4.09 (shadow 0.30) | post 0.62: ao 0.49 bloom 0.08 composite 0.05 |
| woods_mixed_noon | t2_on | 6.91 | 6.33 | 1.1 | 0.49 | frame 6.91 ms | world 6.33 (shadow 1.10) | post 0.49: ao 0.41 bloom 0.05 composite 0.03 |
| woods_mixed_noon | t3_d8 | 5.93 | 5.3 | 0.71 | 0.53 | frame 5.93 ms | world 5.30 (shadow 0.71) | post 0.53: ao 0.44 bloom 0.06 composite 0.03 |
| woods_mixed_noon | t3_off | 4.88 | 4.14 | 0.3 | 0.6 | frame 4.88 ms | world 4.14 (shadow 0.30) | post 0.60: ao 0.47 bloom 0.08 composite 0.05 |
| woods_mixed_noon | t3_on | 6.59 | 6.02 | 1.06 | 0.48 | frame 6.59 ms | world 6.02 (shadow 1.06) | post 0.48: ao 0.41 bloom 0.05 composite 0.03 |
