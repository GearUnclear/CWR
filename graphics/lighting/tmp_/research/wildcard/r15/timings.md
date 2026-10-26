# timings (r15)

| view | label | GPU frame ms | world | shadow (incl. sky maps) | post | raw |
|---|---|---|---|---|---|---|
| forest_edge_golden | B_front | 7.12 | 6.61 | 1.04 | 0.39 | frame 7.12 ms | world 6.61 (shadow 1.04) | post 0.39: ao 0.31 bloom 0.05 composite 0.03 |
| forest_edge_golden | E_k24 | 9.12 | 8.64 | 1.48 | 0.36 | frame 9.12 ms | world 8.64 (shadow 1.48) | post 0.36: ao 0.29 bloom 0.05 composite 0.03 |
| forest_edge_golden | F_r64 | 9.71 | 9.21 | 1.44 | 0.38 | frame 9.71 ms | world 9.21 (shadow 1.44) | post 0.38: ao 0.30 bloom 0.05 composite 0.03 |
| forest_edge_golden | G_cal | 9.74 | 9.22 | 1.46 | 0.41 | frame 9.74 ms | world 9.22 (shadow 1.46) | post 0.41: ao 0.33 bloom 0.05 composite 0.03 |
| forest_edge_golden | classic | 3.64 | 3.53 | None | None | frame 3.64 ms | world 3.53 |
| forest_edge_golden | dbg_F | 9.65 | 9.17 | 1.45 | 0.37 | frame 9.65 ms | world 9.17 (shadow 1.45) | post 0.37: ao 0.29 bloom 0.05 composite 0.03 |
| forest_edge_golden | dbg_bounceF | 9.67 | 9.18 | 1.45 | 0.38 | frame 9.67 ms | world 9.18 (shadow 1.45) | post 0.38: ao 0.30 bloom 0.05 composite 0.03 |
| forest_edge_golden | remastered | 4.87 | 4.32 | 0.23 | 0.4 | frame 4.87 ms | world 4.32 (shadow 0.23) | post 0.40: ao 0.30 bloom 0.06 composite 0.04 |
| lowsun_fp | B_front | 6.78 | 6.02 | 1.05 | 0.66 | frame 6.78 ms | world 6.02 (shadow 1.05) | post 0.66: ao 0.37 bloom 0.05 shafts 0.22 composite 0.03 |
| lowsun_fp | E_k24 | 8.53 | 7.78 | 1.47 | 0.65 | frame 8.53 ms | world 7.78 (shadow 1.47) | post 0.65: ao 0.36 bloom 0.05 shafts 0.22 composite 0.03 |
| lowsun_fp | F_r64 | 8.36 | 7.56 | 1.59 | 0.71 | frame 8.36 ms | world 7.56 (shadow 1.59) | post 0.71: ao 0.41 bloom 0.05 shafts 0.22 composite 0.03 |
| lowsun_fp | G_cal | 8.51 | 7.75 | 1.65 | 0.66 | frame 8.51 ms | world 7.75 (shadow 1.65) | post 0.66: ao 0.36 bloom 0.05 shafts 0.22 composite 0.03 |
| lowsun_fp | classic | 5.01 | 4.6 | None | None | frame 5.01 ms | world 4.60 |
| lowsun_fp | dbg_F | 8.43 | 7.64 | 1.5 | 0.68 | frame 8.43 ms | world 7.64 (shadow 1.50) | post 0.68: ao 0.40 bloom 0.05 shafts 0.21 composite 0.03 |
| lowsun_fp | dbg_bounceF | 8.27 | 7.51 | 1.55 | 0.66 | frame 8.27 ms | world 7.51 (shadow 1.55) | post 0.66: ao 0.37 bloom 0.05 shafts 0.21 composite 0.03 |
| lowsun_fp | remastered | 4.72 | 3.79 | 0.07 | 0.81 | frame 4.72 ms | world 3.79 (shadow 0.07) | post 0.81: ao 0.36 bloom 0.06 shafts 0.27 composite 0.12 |
| noon_fields | B_front | 10.56 | 10.07 | 0.93 | 0.39 | frame 10.56 ms | world 10.07 (shadow 0.93) | post 0.39: ao 0.32 bloom 0.05 composite 0.03 |
| noon_fields | E_k24 | 11.89 | 11.39 | 1.36 | 0.39 | frame 11.89 ms | world 11.39 (shadow 1.36) | post 0.39: ao 0.32 bloom 0.05 composite 0.03 |
| noon_fields | F_r64 | 11.82 | 11.34 | 1.42 | 0.39 | frame 11.82 ms | world 11.34 (shadow 1.42) | post 0.39: ao 0.31 bloom 0.05 composite 0.03 |
| noon_fields | G_cal | 12.01 | 11.5 | 1.4 | 0.4 | frame 12.01 ms | world 11.50 (shadow 1.40) | post 0.40: ao 0.33 bloom 0.05 composite 0.03 |
| noon_fields | classic | 7.45 | 7.35 | None | None | frame 7.45 ms | world 7.35 |
| noon_fields | dbg_F | 11.79 | 11.28 | 1.39 | 0.41 | frame 11.79 ms | world 11.28 (shadow 1.39) | post 0.41: ao 0.34 bloom 0.05 composite 0.03 |
| noon_fields | dbg_bounceF | 11.94 | 11.42 | 1.35 | 0.42 | frame 11.94 ms | world 11.42 (shadow 1.35) | post 0.42: ao 0.34 bloom 0.05 composite 0.03 |
| noon_fields | remastered | 8.44 | 7.92 | 0.07 | 0.41 | frame 8.44 ms | world 7.92 (shadow 0.07) | post 0.41: ao 0.33 bloom 0.05 composite 0.03 |
| overcast_broken | B_front | 12.33 | 11.68 | 1.0 | 0.56 | frame 12.33 ms | world 11.68 (shadow 1.00) | post 0.56: ao 0.48 bloom 0.05 composite 0.03 |
| overcast_broken | E_k24 | 18.17 | 17.45 | 1.38 | 0.62 | frame 18.17 ms | world 17.45 (shadow 1.38) | post 0.62: ao 0.55 bloom 0.05 composite 0.03 |
| overcast_broken | F_r64 | 18.05 | 17.37 | 1.38 | 0.59 | frame 18.05 ms | world 17.37 (shadow 1.38) | post 0.59: ao 0.51 bloom 0.05 composite 0.03 |
| overcast_broken | G_cal | 18.17 | 17.42 | 1.4 | 0.66 | frame 18.17 ms | world 17.42 (shadow 1.40) | post 0.66: ao 0.58 bloom 0.05 composite 0.03 |
| overcast_broken | classic | 9.35 | 9.25 | None | None | frame 9.35 ms | world 9.25 |
| overcast_broken | dbg_F | 17.9 | 17.23 | 1.37 | 0.57 | frame 17.90 ms | world 17.23 (shadow 1.37) | post 0.57: ao 0.49 bloom 0.05 composite 0.03 |
| overcast_broken | dbg_bounceF | 18.08 | 17.42 | 1.4 | 0.56 | frame 18.08 ms | world 17.42 (shadow 1.40) | post 0.56: ao 0.49 bloom 0.05 composite 0.03 |
| overcast_broken | remastered | 10.48 | 9.86 | 0.12 | 0.53 | frame 10.48 ms | world 9.86 (shadow 0.12) | post 0.53: ao 0.45 bloom 0.05 composite 0.03 |
| overcast_close | B_front | 6.43 | 5.83 | 1.14 | 0.48 | frame 6.43 ms | world 5.83 (shadow 1.14) | post 0.48: ao 0.39 bloom 0.06 composite 0.03 |
| overcast_close | E_k24 | 7.26 | 6.73 | 1.51 | 0.43 | frame 7.26 ms | world 6.73 (shadow 1.51) | post 0.43: ao 0.35 bloom 0.05 composite 0.03 |
| overcast_close | F_r64 | 7.04 | 6.53 | 1.52 | 0.42 | frame 7.04 ms | world 6.53 (shadow 1.52) | post 0.42: ao 0.34 bloom 0.05 composite 0.03 |
| overcast_close | G_cal | 7.08 | 6.53 | 1.47 | 0.45 | frame 7.08 ms | world 6.53 (shadow 1.47) | post 0.45: ao 0.37 bloom 0.05 composite 0.03 |
| overcast_close | classic | 4.04 | 3.84 | None | None | frame 4.04 ms | world 3.84 |
| overcast_close | dbg_F | 6.89 | 6.38 | 1.5 | 0.41 | frame 6.89 ms | world 6.38 (shadow 1.50) | post 0.41: ao 0.34 bloom 0.05 composite 0.03 |
| overcast_close | dbg_bounceF | 6.94 | 6.44 | 1.48 | 0.41 | frame 6.94 ms | world 6.44 (shadow 1.48) | post 0.41: ao 0.33 bloom 0.05 composite 0.03 |
| overcast_close | remastered | 3.64 | 3.01 | 0.17 | 0.49 | frame 3.64 ms | world 3.01 (shadow 0.17) | post 0.49: ao 0.38 bloom 0.07 composite 0.04 |
| town_day | B_front | 6.24 | 5.62 | 1.09 | 0.5 | frame 6.24 ms | world 5.62 (shadow 1.09) | post 0.50: ao 0.42 bloom 0.05 composite 0.03 |
| town_day | E_k24 | 8.25 | 7.69 | 1.43 | 0.44 | frame 8.25 ms | world 7.69 (shadow 1.43) | post 0.44: ao 0.36 bloom 0.05 composite 0.03 |
| town_day | F_r64 | 8.29 | 7.71 | 1.45 | 0.47 | frame 8.29 ms | world 7.71 (shadow 1.45) | post 0.47: ao 0.39 bloom 0.05 composite 0.03 |
| town_day | G_cal | 8.25 | 7.66 | 1.42 | 0.47 | frame 8.25 ms | world 7.66 (shadow 1.42) | post 0.47: ao 0.39 bloom 0.05 composite 0.03 |
| town_day | classic | 2.3 | 2.18 | None | None | frame 2.30 ms | world 2.18 |
| town_day | dbg_F | 8.13 | 7.59 | 1.37 | 0.43 | frame 8.13 ms | world 7.59 (shadow 1.37) | post 0.43: ao 0.35 bloom 0.05 composite 0.03 |
| town_day | dbg_bounceF | 8.15 | 7.59 | 1.37 | 0.44 | frame 8.15 ms | world 7.59 (shadow 1.37) | post 0.44: ao 0.37 bloom 0.05 composite 0.03 |
| town_day | remastered | 4.02 | 3.2 | 0.26 | 0.59 | frame 4.02 ms | world 3.20 (shadow 0.26) | post 0.59: ao 0.43 bloom 0.10 composite 0.06 |
| town_street | B_front | 6.75 | 5.95 | 0.95 | 0.71 | frame 6.75 ms | world 5.95 (shadow 0.95) | post 0.71: ao 0.37 bloom 0.05 shafts 0.26 composite 0.03 |
| town_street | E_k24 | 9.77 | 8.94 | 1.38 | 0.74 | frame 9.77 ms | world 8.94 (shadow 1.38) | post 0.74: ao 0.40 bloom 0.05 shafts 0.26 composite 0.03 |
| town_street | F_r64 | 9.66 | 8.86 | 1.37 | 0.71 | frame 9.66 ms | world 8.86 (shadow 1.37) | post 0.71: ao 0.37 bloom 0.05 shafts 0.26 composite 0.03 |
| town_street | G_cal | 9.66 | 8.85 | 1.38 | 0.72 | frame 9.66 ms | world 8.85 (shadow 1.38) | post 0.72: ao 0.38 bloom 0.05 shafts 0.26 composite 0.03 |
| town_street | classic | 1.12 | 1.03 | None | None | frame 1.12 ms | world 1.03 |
| town_street | dbg_F | 9.72 | 8.89 | 1.41 | 0.74 | frame 9.72 ms | world 8.89 (shadow 1.41) | post 0.74: ao 0.40 bloom 0.05 shafts 0.26 composite 0.03 |
| town_street | dbg_bounceF | 9.79 | 8.96 | 1.44 | 0.75 | frame 9.79 ms | world 8.96 (shadow 1.44) | post 0.75: ao 0.42 bloom 0.05 shafts 0.26 composite 0.03 |
| town_street | remastered | 5.25 | 3.34 | 0.38 | 1.65 | frame 5.25 ms | world 3.34 (shadow 0.38) | post 1.65: ao 0.96 bloom 0.11 shafts 0.48 composite 0.10 |
| woods_mixed_noon | B_front | 11.14 | 10.52 | 1.15 | 0.53 | frame 11.14 ms | world 10.52 (shadow 1.15) | post 0.53: ao 0.46 bloom 0.05 composite 0.03 |
| woods_mixed_noon | E_k24 | 16.22 | 15.6 | 1.57 | 0.53 | frame 16.22 ms | world 15.60 (shadow 1.57) | post 0.53: ao 0.46 bloom 0.05 composite 0.03 |
| woods_mixed_noon | F_r64 | 15.96 | 15.32 | 1.51 | 0.55 | frame 15.96 ms | world 15.32 (shadow 1.51) | post 0.55: ao 0.47 bloom 0.05 composite 0.03 |
| woods_mixed_noon | G_cal | 15.94 | 15.31 | 1.49 | 0.55 | frame 15.94 ms | world 15.31 (shadow 1.49) | post 0.55: ao 0.47 bloom 0.05 composite 0.03 |
| woods_mixed_noon | classic | 2.51 | 2.42 | None | None | frame 2.51 ms | world 2.42 |
| woods_mixed_noon | dbg_F | 15.94 | 15.28 | 1.5 | 0.57 | frame 15.94 ms | world 15.28 (shadow 1.50) | post 0.57: ao 0.49 bloom 0.05 composite 0.03 |
| woods_mixed_noon | dbg_bounceF | 16.02 | 15.4 | 1.53 | 0.54 | frame 16.02 ms | world 15.40 (shadow 1.53) | post 0.54: ao 0.46 bloom 0.05 composite 0.03 |
| woods_mixed_noon | remastered | 4.74 | 3.95 | 0.27 | 0.67 | frame 4.74 ms | world 3.95 (shadow 0.27) | post 0.67: ao 0.55 bloom 0.07 composite 0.04 |
| woods_pine2_evening | B_front | 10.85 | 9.99 | 1.15 | 0.78 | frame 10.85 ms | world 9.99 (shadow 1.15) | post 0.78: ao 0.41 bloom 0.05 shafts 0.29 composite 0.03 |
| woods_pine2_evening | E_k24 | 14.72 | 13.79 | 1.64 | 0.83 | frame 14.72 ms | world 13.79 (shadow 1.64) | post 0.83: ao 0.44 bloom 0.05 shafts 0.29 composite 0.06 |
| woods_pine2_evening | F_r64 | 15.27 | 14.38 | 1.59 | 0.8 | frame 15.27 ms | world 14.38 (shadow 1.59) | post 0.80: ao 0.42 bloom 0.05 shafts 0.29 composite 0.04 |
| woods_pine2_evening | G_cal | 15.42 | 14.47 | 1.65 | 0.86 | frame 15.42 ms | world 14.47 (shadow 1.65) | post 0.86: ao 0.46 bloom 0.05 shafts 0.29 composite 0.06 |
| woods_pine2_evening | classic | 3.57 | 3.48 | None | None | frame 3.57 ms | world 3.48 |
| woods_pine2_evening | dbg_F | 15.28 | 14.34 | 1.61 | 0.85 | frame 15.28 ms | world 14.34 (shadow 1.61) | post 0.85: ao 0.43 bloom 0.05 shafts 0.28 composite 0.09 |
| woods_pine2_evening | dbg_bounceF | 15.3 | 14.39 | 1.64 | 0.83 | frame 15.30 ms | world 14.39 (shadow 1.64) | post 0.83: ao 0.42 bloom 0.05 shafts 0.28 composite 0.08 |
| woods_pine2_evening | remastered | 6.45 | 5.46 | 0.34 | 0.89 | frame 6.45 ms | world 5.46 (shadow 0.34) | post 0.89: ao 0.42 bloom 0.06 shafts 0.35 composite 0.06 |
