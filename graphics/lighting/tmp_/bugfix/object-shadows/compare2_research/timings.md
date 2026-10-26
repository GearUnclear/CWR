| label | before GPU frame | before world | before shadow | before CPU frame | after GPU frame | after world | after shadow | after CPU frame |
|---|---|---|---|---|---|---|---|---|
| forest_edge_golden__boot | 9.56 | 8.84 | 0.05 | 15.23 | 5.65 | 5.01 | 0.26 | 8.12 |
| forest_edge_golden__dbgsunvis | 5.82 | 5.29 | 0.03 | 9.52 | 5.43 | 4.75 | 0.28 | 8.57 |
| forest_edge_golden__remastered | 5.82 | 5.29 | 0.03 | 9.52 | 5.43 | 4.75 | 0.28 | 8.57 |
| noon_fields__dbgsunvis | 7.81 | 7.31 | 0.04 | 18.42 | 7.80 | 7.29 | 0.07 | 12.51 |
| noon_fields__remastered | 7.81 | 7.31 | 0.04 | 18.42 | 7.80 | 7.29 | 0.07 | 12.51 |
| town_day__dbgsunvis | 4.40 | 2.95 | 0.09 | 5.22 | 4.62 | 2.97 | 0.44 | 6.33 |
| town_day__remastered | 4.40 | 2.95 | 0.09 | 5.22 | 4.62 | 2.97 | 0.44 | 6.33 |

| view | before shadow pass median (n) | after shadow pass median (n) | before GPU frame median | after GPU frame median |
|---|---|---|---|---|
| forest_edge_golden | 0.03 (1) | 0.28 (1) | 5.82 (1) | 5.43 (1) |
| noon_fields | 0.04 (1) | 0.07 (1) | 7.81 (1) | 7.80 (1) |
| town_day | 0.09 (1) | 0.44 (1) | 4.40 (1) | 4.62 (1) |

Raw triGpuTimes summaries:

- before `forest_edge_golden__boot`: `frame 9.56 ms | world 8.84 (shadow 0.05) | post 0.54: ao 0.41 bloom 0.08 composite 0.05"` cpu frame 15.23
- before `forest_edge_golden__remastered`: `frame 5.82 ms | world 5.29 (shadow 0.03) | post 0.38: ao 0.31 bloom 0.05 composite 0.03"` cpu frame 9.52
- before `forest_edge_golden__dbgsunvis`: `frame 5.82 ms | world 5.29 (shadow 0.03) | post 0.38: ao 0.31 bloom 0.05 composite 0.03"` cpu frame 9.52
- before `noon_fields__remastered`: `frame 7.81 ms | world 7.31 (shadow 0.04) | post 0.39: ao 0.32 bloom 0.05 composite 0.03"` cpu frame 18.42
- before `noon_fields__dbgsunvis`: `frame 7.81 ms | world 7.31 (shadow 0.04) | post 0.39: ao 0.32 bloom 0.05 composite 0.03"` cpu frame 18.42
- before `town_day__remastered`: `frame 4.40 ms | world 2.95 (shadow 0.09) | post 1.09: ao 0.82 bloom 0.15 composite 0.12"` cpu frame 5.22
- before `town_day__dbgsunvis`: `frame 4.40 ms | world 2.95 (shadow 0.09) | post 1.09: ao 0.82 bloom 0.15 composite 0.12"` cpu frame 5.22
- after `forest_edge_golden__boot`: `frame 5.65 ms | world 5.01 (shadow 0.26) | post 0.48: ao 0.37 bloom 0.07 composite 0.04"` cpu frame 8.12
- after `forest_edge_golden__remastered`: `frame 5.43 ms | world 4.75 (shadow 0.28) | post 0.50: ao 0.38 bloom 0.08 composite 0.05"` cpu frame 8.57
- after `forest_edge_golden__dbgsunvis`: `frame 5.43 ms | world 4.75 (shadow 0.28) | post 0.50: ao 0.38 bloom 0.08 composite 0.05"` cpu frame 8.57
- after `noon_fields__remastered`: `frame 7.80 ms | world 7.29 (shadow 0.07) | post 0.41: ao 0.34 bloom 0.05 composite 0.03"` cpu frame 12.51
- after `noon_fields__dbgsunvis`: `frame 7.80 ms | world 7.29 (shadow 0.07) | post 0.41: ao 0.34 bloom 0.05 composite 0.03"` cpu frame 12.51
- after `town_day__remastered`: `frame 4.62 ms | world 2.97 (shadow 0.44) | post 1.30: ao 1.03 bloom 0.14 composite 0.13"` cpu frame 6.33
- after `town_day__dbgsunvis`: `frame 4.62 ms | world 2.97 (shadow 0.44) | post 1.30: ao 1.03 bloom 0.14 composite 0.13"` cpu frame 6.33
