# timings (r16)

| view | label | GPU frame ms | world | shadow (incl. sky maps) | post | raw |
|---|---|---|---|---|---|---|
| forest_edge_golden | classic | 3.62 | 3.51 | None | None | frame 3.62 ms | world 3.51 |
| forest_edge_golden | dbg_final | 7.59 | 7.11 | 1.47 | 0.37 | frame 7.59 ms | world 7.11 (shadow 1.47) | post 0.37: ao 0.29 bloom 0.05 composite 0.03 |
| forest_edge_golden | final | 7.58 | 7.11 | 1.48 | 0.35 | frame 7.58 ms | world 7.11 (shadow 1.48) | post 0.35: ao 0.28 bloom 0.05 composite 0.03 |
| forest_edge_golden | final_phys | 7.65 | 7.13 | 1.48 | 0.41 | frame 7.65 ms | world 7.13 (shadow 1.48) | post 0.41: ao 0.34 bloom 0.05 composite 0.03 |
| forest_edge_golden | remastered | 4.96 | 4.41 | 0.24 | 0.4 | frame 4.96 ms | world 4.41 (shadow 0.24) | post 0.40: ao 0.30 bloom 0.06 composite 0.04 |
| lowsun_fp | classic | 3.98 | 3.67 | None | None | frame 3.98 ms | world 3.67 |
| lowsun_fp | dbg_final | 7.8 | 7.09 | 1.36 | 0.62 | frame 7.80 ms | world 7.09 (shadow 1.36) | post 0.62: ao 0.33 bloom 0.05 shafts 0.21 composite 0.03 |
| lowsun_fp | final | 7.96 | 7.2 | 1.43 | 0.67 | frame 7.96 ms | world 7.20 (shadow 1.43) | post 0.67: ao 0.38 bloom 0.05 shafts 0.22 composite 0.03 |
| lowsun_fp | final_phys | 7.86 | 7.16 | 1.38 | 0.61 | frame 7.86 ms | world 7.16 (shadow 1.38) | post 0.61: ao 0.31 bloom 0.05 shafts 0.22 composite 0.03 |
| lowsun_fp | remastered | 5.91 | 4.78 | 0.11 | 0.96 | frame 5.91 ms | world 4.78 (shadow 0.11) | post 0.96: ao 0.45 bloom 0.09 shafts 0.34 composite 0.09 |
| noon_fields | classic | 6.88 | 6.78 | None | None | frame 6.88 ms | world 6.78 |
| noon_fields | dbg_final | 11.2 | 10.72 | 1.3 | 0.38 | frame 11.20 ms | world 10.72 (shadow 1.30) | post 0.38: ao 0.31 bloom 0.05 composite 0.03 |
| noon_fields | final | 11.25 | 10.78 | 1.32 | 0.38 | frame 11.25 ms | world 10.78 (shadow 1.32) | post 0.38: ao 0.30 bloom 0.05 composite 0.03 |
| noon_fields | final_phys | 11.2 | 10.74 | 1.31 | 0.36 | frame 11.20 ms | world 10.74 (shadow 1.31) | post 0.36: ao 0.28 bloom 0.05 composite 0.03 |
| noon_fields | remastered | 7.94 | 7.5 | 0.07 | 0.35 | frame 7.94 ms | world 7.50 (shadow 0.07) | post 0.35: ao 0.28 bloom 0.05 composite 0.03 |
| overcast_broken | classic | 9.49 | 9.4 | None | None | frame 9.49 ms | world 9.40 |
| overcast_broken | dbg_final | 17.25 | 16.58 | 1.41 | 0.58 | frame 17.25 ms | world 16.58 (shadow 1.41) | post 0.58: ao 0.50 bloom 0.05 composite 0.03 |
| overcast_broken | final | 17.55 | 16.86 | 1.46 | 0.6 | frame 17.55 ms | world 16.86 (shadow 1.46) | post 0.60: ao 0.52 bloom 0.05 composite 0.03 |
| overcast_broken | final_phys | 17.45 | 16.78 | 1.45 | 0.57 | frame 17.45 ms | world 16.78 (shadow 1.45) | post 0.57: ao 0.50 bloom 0.05 composite 0.03 |
| overcast_broken | remastered | 10.16 | 9.5 | 0.09 | 0.57 | frame 10.16 ms | world 9.50 (shadow 0.09) | post 0.57: ao 0.50 bloom 0.05 composite 0.03 |
| overcast_close | classic | 4.98 | 4.71 | None | None | frame 4.98 ms | world 4.71 |
| overcast_close | dbg_final | 6.97 | 6.47 | 1.51 | 0.4 | frame 6.97 ms | world 6.47 (shadow 1.51) | post 0.40: ao 0.31 bloom 0.05 composite 0.03 |
| overcast_close | final | 6.99 | 6.5 | 1.56 | 0.39 | frame 6.99 ms | world 6.50 (shadow 1.56) | post 0.39: ao 0.31 bloom 0.05 composite 0.03 |
| overcast_close | final_phys | 7.02 | 6.49 | 1.53 | 0.43 | frame 7.02 ms | world 6.49 (shadow 1.53) | post 0.43: ao 0.35 bloom 0.05 composite 0.03 |
| overcast_close | remastered | 6.28 | 5.06 | 0.44 | 0.96 | frame 6.28 ms | world 5.06 (shadow 0.44) | post 0.96: ao 0.74 bloom 0.12 composite 0.10 |
| town_day | classic | 3.02 | 2.78 | None | None | frame 3.02 ms | world 2.78 |
| town_day | dbg_final | 6.99 | 6.41 | 1.43 | 0.45 | frame 6.99 ms | world 6.41 (shadow 1.43) | post 0.45: ao 0.37 bloom 0.05 composite 0.03 |
| town_day | final | 6.65 | 6.12 | 1.38 | 0.41 | frame 6.65 ms | world 6.12 (shadow 1.38) | post 0.41: ao 0.34 bloom 0.05 composite 0.03 |
| town_day | final_phys | 6.9 | 6.32 | 1.41 | 0.45 | frame 6.90 ms | world 6.32 (shadow 1.41) | post 0.45: ao 0.37 bloom 0.05 composite 0.03 |
| town_day | remastered | 5.23 | 3.64 | 0.36 | 1.25 | frame 5.23 ms | world 3.64 (shadow 0.36) | post 1.25: ao 1.04 bloom 0.11 composite 0.10 |
| town_night | classic | 3.72 | 3.58 | None | None | frame 3.72 ms | world 3.58 |
| town_night | dbg_final | 9.39 | 8.78 | 1.34 | 0.45 | frame 9.39 ms | world 8.78 (shadow 1.34) | post 0.45: ao 0.37 bloom 0.05 composite 0.03 |
| town_night | final | 9.36 | 8.78 | 1.37 | 0.43 | frame 9.36 ms | world 8.78 (shadow 1.37) | post 0.43: ao 0.35 bloom 0.05 composite 0.03 |
| town_night | final_phys | 9.46 | 8.81 | 1.34 | 0.5 | frame 9.46 ms | world 8.81 (shadow 1.34) | post 0.50: ao 0.42 bloom 0.05 composite 0.03 |
| town_night | remastered | 6.11 | 5.42 | None | 0.51 | frame 6.11 ms | world 5.42 | post 0.51: ao 0.41 bloom 0.06 composite 0.04 |
| town_street | classic | 1.05 | 0.95 | None | None | frame 1.05 ms | world 0.95 |
| town_street | dbg_final | 7.18 | 6.35 | 1.36 | 0.74 | frame 7.18 ms | world 6.35 (shadow 1.36) | post 0.74: ao 0.41 bloom 0.05 shafts 0.25 composite 0.03 |
| town_street | final | 7.25 | 6.43 | 1.42 | 0.72 | frame 7.25 ms | world 6.43 (shadow 1.42) | post 0.72: ao 0.38 bloom 0.05 shafts 0.27 composite 0.03 |
| town_street | final_phys | 7.23 | 6.4 | 1.41 | 0.73 | frame 7.23 ms | world 6.40 (shadow 1.41) | post 0.73: ao 0.39 bloom 0.05 shafts 0.26 composite 0.03 |
| town_street | remastered | 5.2 | 3.52 | 0.36 | 1.48 | frame 5.20 ms | world 3.52 (shadow 0.36) | post 1.48: ao 0.56 bloom 0.12 shafts 0.71 composite 0.09 |
| woods_mixed_noon | classic | 3.4 | 3.28 | None | None | frame 3.40 ms | world 3.28 |
| woods_mixed_noon | dbg_final | 7.19 | 6.51 | 1.68 | 0.58 | frame 7.19 ms | world 6.51 (shadow 1.68) | post 0.58: ao 0.49 bloom 0.06 composite 0.04 |
| woods_mixed_noon | final | 6.96 | 6.37 | 1.66 | 0.49 | frame 6.96 ms | world 6.37 (shadow 1.66) | post 0.49: ao 0.41 bloom 0.05 composite 0.03 |
| woods_mixed_noon | final_phys | 7.0 | 6.37 | 1.64 | 0.53 | frame 7.00 ms | world 6.37 (shadow 1.64) | post 0.53: ao 0.45 bloom 0.05 composite 0.03 |
| woods_mixed_noon | remastered | 4.82 | 4.07 | 0.29 | 0.62 | frame 4.82 ms | world 4.07 (shadow 0.29) | post 0.62: ao 0.50 bloom 0.07 composite 0.04 |
| woods_overcast | classic | 3.29 | 3.16 | None | None | frame 3.29 ms | world 3.16 |
| woods_overcast | dbg_final | 6.85 | 6.22 | 1.63 | 0.53 | frame 6.85 ms | world 6.22 (shadow 1.63) | post 0.53: ao 0.44 bloom 0.05 composite 0.03 |
| woods_overcast | final | 7.0 | 6.39 | 1.66 | 0.51 | frame 7.00 ms | world 6.39 (shadow 1.66) | post 0.51: ao 0.42 bloom 0.05 composite 0.03 |
| woods_overcast | final_phys | 6.84 | 6.25 | 1.62 | 0.49 | frame 6.84 ms | world 6.25 (shadow 1.62) | post 0.49: ao 0.40 bloom 0.05 composite 0.03 |
| woods_overcast | remastered | 4.75 | 4.03 | 0.28 | 0.58 | frame 4.75 ms | world 4.03 (shadow 0.28) | post 0.58: ao 0.46 bloom 0.07 composite 0.04 |
| woods_pine2_evening | classic | 3.66 | 3.57 | None | None | frame 3.66 ms | world 3.57 |
| woods_pine2_evening | dbg_final | 9.39 | 8.53 | 1.64 | 0.77 | frame 9.39 ms | world 8.53 (shadow 1.64) | post 0.77: ao 0.42 bloom 0.05 shafts 0.27 composite 0.03 |
| woods_pine2_evening | final | 9.35 | 8.54 | 1.62 | 0.72 | frame 9.35 ms | world 8.54 (shadow 1.62) | post 0.72: ao 0.36 bloom 0.05 shafts 0.29 composite 0.03 |
| woods_pine2_evening | final_phys | 9.45 | 8.59 | 1.65 | 0.77 | frame 9.45 ms | world 8.59 (shadow 1.65) | post 0.77: ao 0.40 bloom 0.05 shafts 0.29 composite 0.04 |
| woods_pine2_evening | remastered | 6.72 | 5.7 | 0.37 | 0.91 | frame 6.72 ms | world 5.70 (shadow 0.37) | post 0.91: ao 0.43 bloom 0.06 shafts 0.38 composite 0.04 |
