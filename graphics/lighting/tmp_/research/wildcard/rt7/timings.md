# timings (rt7)

| view | label | GPU frame ms | world | shadow (incl. sky maps) | post | raw |
|---|---|---|---|---|---|---|
| lowsun_fp | t1_final | 8.0 | 7.24 | 1.42 | 0.66 | frame 8.00 ms | world 7.24 (shadow 1.42) | post 0.66: ao 0.37 bloom 0.05 shafts 0.22 composite 0.03 |
| lowsun_fp | t1_k16 | 6.72 | 5.98 | 0.97 | 0.64 | frame 6.72 ms | world 5.98 (shadow 0.97) | post 0.64: ao 0.32 bloom 0.05 shafts 0.24 composite 0.03 |
| lowsun_fp | t1_off | 5.94 | 4.14 | 0.15 | 1.54 | frame 5.94 ms | world 4.14 (shadow 0.15) | post 1.54: ao 0.66 bloom 0.14 shafts 0.59 composite 0.15 |
| lowsun_fp | t2_final | 7.78 | 7.08 | 1.32 | 0.6 | frame 7.78 ms | world 7.08 (shadow 1.32) | post 0.60: ao 0.31 bloom 0.05 shafts 0.22 composite 0.03 |
| lowsun_fp | t2_k16 | 6.7 | 5.96 | 0.95 | 0.64 | frame 6.70 ms | world 5.96 (shadow 0.95) | post 0.64: ao 0.32 bloom 0.05 shafts 0.24 composite 0.03 |
| lowsun_fp | t2_off | 4.75 | 3.87 | 0.08 | 0.75 | frame 4.75 ms | world 3.87 (shadow 0.08) | post 0.75: ao 0.35 bloom 0.07 shafts 0.30 composite 0.04 |
| lowsun_fp | t3_final | 7.76 | 7.05 | 1.3 | 0.61 | frame 7.76 ms | world 7.05 (shadow 1.30) | post 0.61: ao 0.31 bloom 0.05 shafts 0.22 composite 0.03 |
| lowsun_fp | t3_k16 | 6.69 | 5.91 | 0.92 | 0.68 | frame 6.69 ms | world 5.91 (shadow 0.92) | post 0.68: ao 0.36 bloom 0.05 shafts 0.24 composite 0.03 |
| lowsun_fp | t3_off | 4.76 | 3.87 | 0.08 | 0.75 | frame 4.76 ms | world 3.87 (shadow 0.08) | post 0.75: ao 0.35 bloom 0.07 shafts 0.30 composite 0.04 |
| overcast_close | t1_final | 6.79 | 6.31 | 1.55 | 0.38 | frame 6.79 ms | world 6.31 (shadow 1.55) | post 0.38: ao 0.31 bloom 0.05 composite 0.03 |
| overcast_close | t1_k16 | 6.16 | 5.56 | 1.12 | 0.48 | frame 6.16 ms | world 5.56 (shadow 1.12) | post 0.48: ao 0.39 bloom 0.06 composite 0.03 |
| overcast_close | t1_off | 6.04 | 4.83 | 0.43 | 0.96 | frame 6.04 ms | world 4.83 (shadow 0.43) | post 0.96: ao 0.74 bloom 0.12 composite 0.10 |
| overcast_close | t2_final | 7.0 | 6.47 | 1.49 | 0.43 | frame 7.00 ms | world 6.47 (shadow 1.49) | post 0.43: ao 0.35 bloom 0.05 composite 0.03 |
| overcast_close | t2_k16 | 6.25 | 5.7 | 1.1 | 0.43 | frame 6.25 ms | world 5.70 (shadow 1.10) | post 0.43: ao 0.33 bloom 0.06 composite 0.04 |
| overcast_close | t2_off | 3.99 | 3.34 | 0.18 | 0.5 | frame 3.99 ms | world 3.34 (shadow 0.18) | post 0.50: ao 0.37 bloom 0.08 composite 0.05 |
| overcast_close | t3_final | 6.61 | 6.12 | 1.43 | 0.39 | frame 6.61 ms | world 6.12 (shadow 1.43) | post 0.39: ao 0.32 bloom 0.05 composite 0.03 |
| overcast_close | t3_k16 | 6.07 | 5.52 | 1.06 | 0.44 | frame 6.07 ms | world 5.52 (shadow 1.06) | post 0.44: ao 0.34 bloom 0.06 composite 0.03 |
| overcast_close | t3_off | 5.57 | 4.43 | 0.41 | 0.94 | frame 5.57 ms | world 4.43 (shadow 0.41) | post 0.94: ao 0.72 bloom 0.10 composite 0.11 |
| town_day | t1_final | 6.98 | 6.41 | 1.44 | 0.44 | frame 6.98 ms | world 6.41 (shadow 1.44) | post 0.44: ao 0.37 bloom 0.05 composite 0.03 |
| town_day | t1_k16 | 5.89 | 5.3 | 1.08 | 0.45 | frame 5.89 ms | world 5.30 (shadow 1.08) | post 0.45: ao 0.36 bloom 0.06 composite 0.03 |
| town_day | t1_off | 4.87 | 3.39 | 0.34 | 1.17 | frame 4.87 ms | world 3.39 (shadow 0.34) | post 1.17: ao 0.97 bloom 0.11 composite 0.09 |
| town_day | t2_final | 6.85 | 6.31 | 1.42 | 0.41 | frame 6.85 ms | world 6.31 (shadow 1.42) | post 0.41: ao 0.34 bloom 0.05 composite 0.03 |
| town_day | t2_k16 | 6.02 | 5.4 | 1.1 | 0.48 | frame 6.02 ms | world 5.40 (shadow 1.10) | post 0.48: ao 0.39 bloom 0.06 composite 0.03 |
| town_day | t2_off | 3.33 | 2.59 | 0.21 | 0.55 | frame 3.33 ms | world 2.59 (shadow 0.21) | post 0.55: ao 0.41 bloom 0.09 composite 0.05 |
| town_day | t3_final | 6.68 | 6.15 | 1.37 | 0.41 | frame 6.68 ms | world 6.15 (shadow 1.37) | post 0.41: ao 0.34 bloom 0.05 composite 0.03 |
| town_day | t3_k16 | 5.89 | 5.3 | 1.07 | 0.45 | frame 5.89 ms | world 5.30 (shadow 1.07) | post 0.45: ao 0.36 bloom 0.06 composite 0.03 |
| town_day | t3_off | 5.02 | 3.48 | 0.34 | 1.24 | frame 5.02 ms | world 3.48 (shadow 0.34) | post 1.24: ao 1.03 bloom 0.11 composite 0.10 |
| town_street | t1_final | 7.17 | 6.36 | 1.38 | 0.72 | frame 7.17 ms | world 6.36 (shadow 1.38) | post 0.72: ao 0.38 bloom 0.05 shafts 0.26 composite 0.03 |
| town_street | t1_k16 | 6.28 | 5.37 | 1.08 | 0.8 | frame 6.28 ms | world 5.37 (shadow 1.08) | post 0.80: ao 0.39 bloom 0.06 shafts 0.31 composite 0.04 |
| town_street | t1_off | 4.03 | 2.71 | 0.26 | 1.16 | frame 4.03 ms | world 2.71 (shadow 0.26) | post 1.16: ao 0.47 bloom 0.09 shafts 0.52 composite 0.08 |
| town_street | t2_final | 7.26 | 6.41 | 1.42 | 0.77 | frame 7.26 ms | world 6.41 (shadow 1.42) | post 0.77: ao 0.42 bloom 0.05 shafts 0.26 composite 0.03 |
| town_street | t2_k16 | 6.25 | 5.34 | 1.06 | 0.81 | frame 6.25 ms | world 5.34 (shadow 1.06) | post 0.81: ao 0.40 bloom 0.06 shafts 0.31 composite 0.04 |
| town_street | t2_off | 4.68 | 2.97 | 0.34 | 1.49 | frame 4.68 ms | world 2.97 (shadow 0.34) | post 1.49: ao 0.85 bloom 0.10 shafts 0.44 composite 0.10 |
| town_street | t3_final | 7.21 | 6.41 | 1.41 | 0.71 | frame 7.21 ms | world 6.41 (shadow 1.41) | post 0.71: ao 0.38 bloom 0.05 shafts 0.26 composite 0.03 |
| town_street | t3_k16 | 6.27 | 5.33 | 1.07 | 0.83 | frame 6.27 ms | world 5.33 (shadow 1.07) | post 0.83: ao 0.42 bloom 0.06 shafts 0.31 composite 0.04 |
| town_street | t3_off | 3.55 | 2.37 | 0.22 | 1.04 | frame 3.55 ms | world 2.37 (shadow 0.22) | post 1.04: ao 0.44 bloom 0.08 shafts 0.43 composite 0.08 |
| woods_mixed_noon | t1_final | 7.01 | 6.35 | 1.61 | 0.56 | frame 7.01 ms | world 6.35 (shadow 1.61) | post 0.56: ao 0.47 bloom 0.06 composite 0.03 |
| woods_mixed_noon | t1_k16 | 6.29 | 5.67 | 1.22 | 0.52 | frame 6.29 ms | world 5.67 (shadow 1.22) | post 0.52: ao 0.42 bloom 0.06 composite 0.03 |
| woods_mixed_noon | t1_off | 4.98 | 4.23 | 0.3 | 0.61 | frame 4.98 ms | world 4.23 (shadow 0.30) | post 0.61: ao 0.49 bloom 0.08 composite 0.05 |
| woods_mixed_noon | t2_final | 6.58 | 6.01 | 1.57 | 0.48 | frame 6.58 ms | world 6.01 (shadow 1.57) | post 0.48: ao 0.40 bloom 0.05 composite 0.03 |
| woods_mixed_noon | t2_k16 | 6.17 | 5.55 | 1.21 | 0.52 | frame 6.17 ms | world 5.55 (shadow 1.21) | post 0.52: ao 0.42 bloom 0.06 composite 0.03 |
| woods_mixed_noon | t2_off | 4.99 | 4.25 | 0.3 | 0.61 | frame 4.99 ms | world 4.25 (shadow 0.30) | post 0.61: ao 0.49 bloom 0.08 composite 0.05 |
| woods_mixed_noon | t3_final | 6.85 | 6.27 | 1.59 | 0.48 | frame 6.85 ms | world 6.27 (shadow 1.59) | post 0.48: ao 0.39 bloom 0.05 composite 0.03 |
| woods_mixed_noon | t3_k16 | 6.32 | 5.64 | 1.24 | 0.58 | frame 6.32 ms | world 5.64 (shadow 1.24) | post 0.58: ao 0.48 bloom 0.06 composite 0.03 |
| woods_mixed_noon | t3_off | 4.94 | 4.23 | 0.29 | 0.58 | frame 4.94 ms | world 4.23 (shadow 0.29) | post 0.58: ao 0.46 bloom 0.08 composite 0.04 |
