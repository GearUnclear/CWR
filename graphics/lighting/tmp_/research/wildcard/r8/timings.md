# timings (r8)

| view | label | GPU frame ms | world | shadow (incl. sky maps) | post | raw |
|---|---|---|---|---|---|---|
| forest_edge_golden | B_ground | 6.84 | 6.27 | 1.25 | 0.44 | frame 6.84 ms | world 6.27 (shadow 1.25) | post 0.44: ao 0.34 bloom 0.06 composite 0.04 |
| forest_edge_golden | B_nobounce | 6.37 | 5.77 | 0.78 | 0.45 | frame 6.37 ms | world 5.77 (shadow 0.78) | post 0.45: ao 0.35 bloom 0.07 composite 0.04 |
| forest_edge_golden | C_gain | 6.75 | 6.2 | 1.24 | 0.41 | frame 6.75 ms | world 6.20 (shadow 1.24) | post 0.41: ao 0.32 bloom 0.06 composite 0.04 |
| forest_edge_golden | classic | 4.95 | 4.79 | None | None | frame 4.95 ms | world 4.79 |
| forest_edge_golden | dbg_bounceB | 7.0 | 6.42 | 1.27 | 0.43 | frame 7.00 ms | world 6.42 (shadow 1.27) | post 0.43: ao 0.33 bloom 0.06 composite 0.04 |
| forest_edge_golden | dbg_visB | 7.31 | 6.73 | 1.45 | 0.43 | frame 7.31 ms | world 6.73 (shadow 1.45) | post 0.43: ao 0.33 bloom 0.07 composite 0.04 |
| forest_edge_golden | remastered | 5.09 | 4.49 | 0.24 | 0.45 | frame 5.09 ms | world 4.49 (shadow 0.24) | post 0.45: ao 0.35 bloom 0.07 composite 0.04 |
| lowsun_fp | B_ground | 5.08 | 4.33 | 0.96 | 0.64 | frame 5.08 ms | world 4.33 (shadow 0.96) | post 0.64: ao 0.34 bloom 0.05 shafts 0.22 composite 0.03 |
| lowsun_fp | B_nobounce | 5.54 | 4.8 | 0.67 | 0.64 | frame 5.54 ms | world 4.80 (shadow 0.67) | post 0.64: ao 0.31 bloom 0.05 shafts 0.22 composite 0.07 |
| lowsun_fp | C_gain | 7.46 | 6.62 | 1.22 | 0.73 | frame 7.46 ms | world 6.62 (shadow 1.22) | post 0.73: ao 0.36 bloom 0.05 shafts 0.22 composite 0.11 |
| lowsun_fp | classic | 2.34 | 2.21 | None | None | frame 2.34 ms | world 2.21 |
| lowsun_fp | dbg_bounceB | 4.95 | 4.23 | 0.89 | 0.62 | frame 4.95 ms | world 4.23 (shadow 0.89) | post 0.62: ao 0.33 bloom 0.05 shafts 0.21 composite 0.03 |
| lowsun_fp | dbg_visB | 5.06 | 4.34 | 1.04 | 0.61 | frame 5.06 ms | world 4.34 (shadow 1.04) | post 0.61: ao 0.32 bloom 0.05 shafts 0.21 composite 0.03 |
| lowsun_fp | remastered | 4.57 | 3.52 | 0.06 | 0.94 | frame 4.57 ms | world 3.52 (shadow 0.06) | post 0.94: ao 0.39 bloom 0.05 shafts 0.22 composite 0.29 |
| noon_fields | B_ground | 11.98 | 11.51 | 0.92 | 0.37 | frame 11.98 ms | world 11.51 (shadow 0.92) | post 0.37: ao 0.30 bloom 0.05 composite 0.03 |
| noon_fields | B_nobounce | 8.76 | 8.27 | 0.49 | 0.38 | frame 8.76 ms | world 8.27 (shadow 0.49) | post 0.38: ao 0.31 bloom 0.05 composite 0.03 |
| noon_fields | C_gain | 9.35 | 8.87 | 1.0 | 0.37 | frame 9.35 ms | world 8.87 (shadow 1.00) | post 0.37: ao 0.30 bloom 0.05 composite 0.03 |
| noon_fields | classic | 9.63 | 9.19 | None | None | frame 9.63 ms | world 9.19 |
| noon_fields | dbg_bounceB | 9.5 | 9.01 | 1.03 | 0.39 | frame 9.50 ms | world 9.01 (shadow 1.03) | post 0.39: ao 0.31 bloom 0.05 composite 0.03 |
| noon_fields | dbg_visB | 9.93 | 9.45 | 1.46 | 0.37 | frame 9.93 ms | world 9.45 (shadow 1.46) | post 0.37: ao 0.30 bloom 0.05 composite 0.03 |
| noon_fields | remastered | 8.24 | 7.73 | 0.07 | 0.4 | frame 8.24 ms | world 7.73 (shadow 0.07) | post 0.40: ao 0.32 bloom 0.05 composite 0.03 |
| overcast_broken | B_ground | 11.27 | 10.58 | 1.08 | 0.59 | frame 11.27 ms | world 10.58 (shadow 1.08) | post 0.59: ao 0.52 bloom 0.05 composite 0.03 |
| overcast_broken | B_nobounce | 12.65 | 11.98 | 0.51 | 0.58 | frame 12.65 ms | world 11.98 (shadow 0.51) | post 0.58: ao 0.50 bloom 0.05 composite 0.03 |
| overcast_broken | C_gain | 11.28 | 10.61 | 1.08 | 0.58 | frame 11.28 ms | world 10.61 (shadow 1.08) | post 0.58: ao 0.50 bloom 0.05 composite 0.03 |
| overcast_broken | classic | 9.42 | 9.32 | None | None | frame 9.42 ms | world 9.32 |
| overcast_broken | dbg_bounceB | 11.23 | 10.6 | 1.13 | 0.54 | frame 11.23 ms | world 10.60 (shadow 1.13) | post 0.54: ao 0.47 bloom 0.05 composite 0.03 |
| overcast_broken | dbg_visB | 11.49 | 10.88 | 1.18 | 0.52 | frame 11.49 ms | world 10.88 (shadow 1.18) | post 0.52: ao 0.44 bloom 0.05 composite 0.03 |
| overcast_broken | remastered | 10.21 | 9.59 | 0.08 | 0.52 | frame 10.21 ms | world 9.59 (shadow 0.08) | post 0.52: ao 0.45 bloom 0.05 composite 0.03 |
| overcast_close | B_ground | 5.2 | 4.56 | 1.21 | 0.52 | frame 5.20 ms | world 4.56 (shadow 1.21) | post 0.52: ao 0.42 bloom 0.06 composite 0.04 |
| overcast_close | B_nobounce | 5.27 | 4.62 | 0.73 | 0.5 | frame 5.27 ms | world 4.62 (shadow 0.73) | post 0.50: ao 0.37 bloom 0.08 composite 0.05 |
| overcast_close | C_gain | 5.67 | 4.96 | 1.26 | 0.56 | frame 5.67 ms | world 4.96 (shadow 1.26) | post 0.56: ao 0.44 bloom 0.07 composite 0.05 |
| overcast_close | classic | 5.02 | 4.74 | None | None | frame 5.02 ms | world 4.74 |
| overcast_close | dbg_bounceB | 8.03 | 6.86 | 1.66 | 0.96 | frame 8.03 ms | world 6.86 (shadow 1.66) | post 0.96: ao 0.75 bloom 0.12 composite 0.10 |
| overcast_close | dbg_visB | 6.68 | 5.92 | 1.48 | 0.6 | frame 6.68 ms | world 5.92 (shadow 1.48) | post 0.60: ao 0.45 bloom 0.09 composite 0.06 |
| overcast_close | remastered | 6.2 | 4.95 | 0.44 | 1.01 | frame 6.20 ms | world 4.95 (shadow 0.44) | post 1.01: ao 0.78 bloom 0.12 composite 0.10 |
| town_day | B_ground | 4.91 | 4.27 | 1.13 | 0.5 | frame 4.91 ms | world 4.27 (shadow 1.13) | post 0.50: ao 0.40 bloom 0.06 composite 0.04 |
| town_day | B_nobounce | 7.26 | 5.6 | 1.04 | 1.35 | frame 7.26 ms | world 5.60 (shadow 1.04) | post 1.35: ao 1.11 bloom 0.14 composite 0.10 |
| town_day | C_gain | 6.08 | 5.29 | 1.22 | 0.61 | frame 6.08 ms | world 5.29 (shadow 1.22) | post 0.61: ao 0.48 bloom 0.08 composite 0.05 |
| town_day | classic | 2.51 | 2.36 | None | None | frame 2.51 ms | world 2.36 |
| town_day | dbg_bounceB | 5.43 | 4.76 | 1.16 | 0.52 | frame 5.43 ms | world 4.76 (shadow 1.16) | post 0.52: ao 0.40 bloom 0.07 composite 0.04 |
| town_day | dbg_visB | 5.66 | 4.99 | 1.38 | 0.52 | frame 5.66 ms | world 4.99 (shadow 1.38) | post 0.52: ao 0.40 bloom 0.07 composite 0.04 |
| town_day | remastered | 3.47 | 2.68 | 0.22 | 0.59 | frame 3.47 ms | world 2.68 (shadow 0.22) | post 0.59: ao 0.44 bloom 0.09 composite 0.06 |
| town_street | B_ground | 4.87 | 3.96 | 1.06 | 0.81 | frame 4.87 ms | world 3.96 (shadow 1.06) | post 0.81: ao 0.44 bloom 0.05 shafts 0.29 composite 0.03 |
| town_street | B_nobounce | 5.66 | 4.46 | 0.78 | 1.05 | frame 5.66 ms | world 4.46 (shadow 0.78) | post 1.05: ao 0.50 bloom 0.08 shafts 0.41 composite 0.06 |
| town_street | C_gain | 6.28 | 5.21 | 1.21 | 0.95 | frame 6.28 ms | world 5.21 (shadow 1.21) | post 0.95: ao 0.44 bloom 0.07 shafts 0.38 composite 0.06 |
| town_street | classic | 1.63 | 1.48 | None | None | frame 1.63 ms | world 1.48 |
| town_street | dbg_bounceB | 6.65 | 5.53 | 1.24 | 0.99 | frame 6.65 ms | world 5.53 (shadow 1.24) | post 0.99: ao 0.47 bloom 0.08 shafts 0.40 composite 0.05 |
| town_street | dbg_visB | 7.25 | 6.09 | 1.46 | 1.02 | frame 7.25 ms | world 6.09 (shadow 1.46) | post 1.02: ao 0.46 bloom 0.08 shafts 0.42 composite 0.05 |
| town_street | remastered | 5.59 | 3.73 | 0.38 | 1.65 | frame 5.59 ms | world 3.73 (shadow 0.38) | post 1.65: ao 0.61 bloom 0.14 shafts 0.80 composite 0.10 |
| woods_mixed_noon | B_ground | 6.96 | 6.36 | 1.1 | 0.51 | frame 6.96 ms | world 6.36 (shadow 1.10) | post 0.51: ao 0.43 bloom 0.05 composite 0.03 |
| woods_mixed_noon | B_nobounce | 6.1 | 5.48 | 0.72 | 0.52 | frame 6.10 ms | world 5.48 (shadow 0.72) | post 0.52: ao 0.44 bloom 0.06 composite 0.03 |
| woods_mixed_noon | C_gain | 6.88 | 6.29 | 1.09 | 0.5 | frame 6.88 ms | world 6.29 (shadow 1.09) | post 0.50: ao 0.42 bloom 0.05 composite 0.03 |
| woods_mixed_noon | classic | 3.23 | 3.11 | None | None | frame 3.23 ms | world 3.11 |
| woods_mixed_noon | dbg_bounceB | 7.09 | 6.45 | 1.11 | 0.55 | frame 7.09 ms | world 6.45 (shadow 1.11) | post 0.55: ao 0.47 bloom 0.05 composite 0.03 |
| woods_mixed_noon | dbg_visB | 7.72 | 7.08 | 1.36 | 0.54 | frame 7.72 ms | world 7.08 (shadow 1.36) | post 0.54: ao 0.45 bloom 0.06 composite 0.03 |
| woods_mixed_noon | remastered | 5.41 | 4.32 | 0.36 | 0.92 | frame 5.41 ms | world 4.32 (shadow 0.36) | post 0.92: ao 0.76 bloom 0.09 composite 0.07 |
| woods_pine2_evening | B_ground | 7.47 | 6.63 | 1.13 | 0.76 | frame 7.47 ms | world 6.63 (shadow 1.13) | post 0.76: ao 0.39 bloom 0.05 shafts 0.29 composite 0.04 |
| woods_pine2_evening | B_nobounce | 6.67 | 5.82 | 0.74 | 0.77 | frame 6.67 ms | world 5.82 (shadow 0.74) | post 0.77: ao 0.40 bloom 0.05 shafts 0.29 composite 0.03 |
| woods_pine2_evening | C_gain | 7.48 | 6.65 | 1.14 | 0.75 | frame 7.48 ms | world 6.65 (shadow 1.14) | post 0.75: ao 0.39 bloom 0.05 shafts 0.29 composite 0.03 |
| woods_pine2_evening | classic | 3.9 | 3.81 | None | None | frame 3.90 ms | world 3.81 |
| woods_pine2_evening | dbg_bounceB | 7.48 | 6.66 | 1.14 | 0.73 | frame 7.48 ms | world 6.66 (shadow 1.14) | post 0.73: ao 0.38 bloom 0.05 shafts 0.28 composite 0.03 |
| woods_pine2_evening | dbg_visB | 7.78 | 6.94 | 1.35 | 0.75 | frame 7.78 ms | world 6.94 (shadow 1.35) | post 0.75: ao 0.39 bloom 0.05 shafts 0.28 composite 0.03 |
| woods_pine2_evening | remastered | 6.75 | 5.73 | 0.38 | 0.92 | frame 6.75 ms | world 5.73 (shadow 0.38) | post 0.92: ao 0.41 bloom 0.06 shafts 0.39 composite 0.05 |
