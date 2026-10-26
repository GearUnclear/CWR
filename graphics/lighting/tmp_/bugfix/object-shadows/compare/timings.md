| label | before GPU frame | before world | before shadow | before CPU frame | after GPU frame | after world | after shadow | after CPU frame |
|---|---|---|---|---|---|---|---|---|
| backlit_hornbeam__dbgsunvis | 6.72 | 6.02 | 0.00 | 11.73 | 12.68 | 12.11 | 0.79 | 16.57 |
| backlit_hornbeam__remastered | 6.72 | 6.02 | 0.00 | 11.73 | 12.68 | 12.11 | 0.79 | 16.57 |
| forest_edge_golden__boot | 4.61 | 4.18 | 0.03 | 6.82 | 6.01 | 5.55 | 0.15 | 9.65 |
| forest_edge_golden__classic | 5.59 | 5.47 |  | 5.57 | 11.02 | 10.89 |  | 8.28 |
| forest_edge_golden__dbgsunvis | 11.81 | 11.00 | 0.03 | 11.94 | 12.06 | 11.57 | 0.77 | 14.07 |
| forest_edge_golden__remastered | 6.15 | 5.72 | 0.03 | 8.96 | 7.87 | 7.31 | 0.44 | 11.44 |
| forest_edge_golden__t2 | 9.51 | 8.96 | 0.11 | 9.97 | 10.99 | 10.42 | 0.46 | 13.39 |
| forest_edge_golden__t3 | 11.81 | 11.00 | 0.03 | 11.94 | 12.06 | 11.57 | 0.77 | 14.07 |
| noon_fields__classic | 21.18 | 21.08 |  | 26.82 | 22.66 | 22.54 |  | 25.15 |
| noon_fields__dbgsunvis | 27.77 | 27.40 | 0.28 | 29.59 | 19.15 | 18.60 | 0.08 | 27.28 |
| noon_fields__remastered | 21.38 | 20.98 | 0.11 | 26.36 | 18.64 | 18.12 | 0.09 | 25.93 |
| noon_fields__t2 | 29.14 | 28.75 | 0.27 | 27.96 | 23.89 | 23.16 | 0.11 | 27.87 |
| noon_fields__t3 | 27.77 | 27.40 | 0.28 | 29.59 | 19.15 | 18.60 | 0.08 | 27.28 |
| pine_stand_noon__dbgsunvis | 6.37 | 5.84 | 0.00 | 10.72 | 10.93 | 9.96 | 0.18 | 17.90 |
| pine_stand_noon__remastered | 6.37 | 5.84 | 0.00 | 10.72 | 10.93 | 9.96 | 0.18 | 17.90 |
| town_day__classic | 3.48 | 3.35 |  | 6.41 | 3.33 | 3.20 |  | 6.90 |
| town_day__dbgsunvis | 3.78 | 3.24 | 0.03 | 8.35 | 4.07 | 3.48 | 0.14 | 8.51 |
| town_day__remastered | 4.45 | 3.85 | 0.03 | 9.07 | 4.95 | 4.45 | 0.14 | 9.91 |
| town_day__t2 | 4.19 | 3.71 | 0.03 | 8.98 | 4.25 | 3.72 | 0.14 | 9.14 |
| town_day__t3 | 3.78 | 3.24 | 0.03 | 8.35 | 4.07 | 3.48 | 0.14 | 8.51 |
| town_night__classic | 13.81 | 13.62 |  | 13.62 | 9.40 | 9.25 |  | 13.82 |
| town_night__remastered | 7.06 | 6.14 |  | 10.44 | 10.34 | 9.81 |  | 11.41 |

| view | before shadow pass median (n) | after shadow pass median (n) | before GPU frame median | after GPU frame median |
|---|---|---|---|---|
| backlit_hornbeam | 0.00 (1) | 0.79 (1) | 6.72 (1) | 12.68 (1) |
| forest_edge_golden | 0.03 (3) | 0.46 (3) | 9.51 (3) | 10.99 (3) |
| noon_fields | 0.27 (3) | 0.09 (3) | 27.77 (3) | 19.15 (3) |
| pine_stand_noon | 0.00 (1) | 0.18 (1) | 6.37 (1) | 10.93 (1) |
| town_day | 0.03 (3) | 0.14 (3) | 4.19 (3) | 4.25 (3) |
| town_night |  |  | 7.06 (1) | 10.34 (1) |

Raw triGpuTimes summaries:

- before `forest_edge_golden__boot`: `frame 4.61 ms | world 4.18 (shadow 0.03) | post 0.30: ao 0.22 bloom 0.05 composite 0.03"` cpu frame 6.82
- before `forest_edge_golden__remastered`: `frame 6.15 ms | world 5.72 (shadow 0.03) | post 0.29: ao 0.22 bloom 0.05 composite 0.03"` cpu frame 8.96
- before `forest_edge_golden__t2`: `frame 9.51 ms | world 8.96 (shadow 0.11) | post 0.43: ao 0.35 bloom 0.05 composite 0.03"` cpu frame 9.97
- before `forest_edge_golden__t3`: `frame 11.81 ms | world 11.00 (shadow 0.03) | post 0.67: ao 0.59 bloom 0.05 composite 0.03"` cpu frame 11.94
- before `forest_edge_golden__dbgsunvis`: `frame 11.81 ms | world 11.00 (shadow 0.03) | post 0.67: ao 0.59 bloom 0.05 composite 0.03"` cpu frame 11.94
- before `forest_edge_golden__classic`: `frame 5.59 ms | world 5.47"` cpu frame 5.57
- before `noon_fields__remastered`: `frame 21.38 ms | world 20.98 (shadow 0.11) | post 0.28: ao 0.21 bloom 0.05 composite 0.03"` cpu frame 26.36
- before `noon_fields__t2`: `frame 29.14 ms | world 28.75 (shadow 0.27) | post 0.28: ao 0.20 bloom 0.05 composite 0.03"` cpu frame 27.96
- before `noon_fields__t3`: `frame 27.77 ms | world 27.40 (shadow 0.28) | post 0.26: ao 0.18 bloom 0.05 composite 0.03"` cpu frame 29.59
- before `noon_fields__dbgsunvis`: `frame 27.77 ms | world 27.40 (shadow 0.28) | post 0.26: ao 0.18 bloom 0.05 composite 0.03"` cpu frame 29.59
- before `noon_fields__classic`: `frame 21.18 ms | world 21.08"` cpu frame 26.82
- before `town_night__remastered`: `frame 7.06 ms | world 6.14 | post 0.61: ao 0.54 bloom 0.05 composite 0.03"` cpu frame 10.44
- before `town_night__classic`: `frame 13.81 ms | world 13.62"` cpu frame 13.62
- before `town_day__remastered`: `frame 4.45 ms | world 3.85 (shadow 0.03) | post 0.45: ao 0.37 bloom 0.05 composite 0.03"` cpu frame 9.07
- before `town_day__t2`: `frame 4.19 ms | world 3.71 (shadow 0.03) | post 0.33: ao 0.26 bloom 0.05 composite 0.03"` cpu frame 8.98
- before `town_day__t3`: `frame 3.78 ms | world 3.24 (shadow 0.03) | post 0.40: ao 0.32 bloom 0.05 composite 0.03"` cpu frame 8.35
- before `town_day__dbgsunvis`: `frame 3.78 ms | world 3.24 (shadow 0.03) | post 0.40: ao 0.32 bloom 0.05 composite 0.03"` cpu frame 8.35
- before `town_day__classic`: `frame 3.48 ms | world 3.35"` cpu frame 6.41
- before `backlit_hornbeam__remastered`: `frame 6.72 ms | world 6.02 (shadow 0.00) | post 0.59: ao 0.36 bloom 0.05 shafts 0.15 composite 0.03"` cpu frame 11.73
- before `backlit_hornbeam__dbgsunvis`: `frame 6.72 ms | world 6.02 (shadow 0.00) | post 0.59: ao 0.36 bloom 0.05 shafts 0.15 composite 0.03"` cpu frame 11.73
- before `pine_stand_noon__remastered`: `frame 6.37 ms | world 5.84 (shadow 0.00) | post 0.40: ao 0.32 bloom 0.05 composite 0.03"` cpu frame 10.72
- before `pine_stand_noon__dbgsunvis`: `frame 6.37 ms | world 5.84 (shadow 0.00) | post 0.40: ao 0.32 bloom 0.05 composite 0.03"` cpu frame 10.72
- after `forest_edge_golden__boot`: `frame 6.01 ms | world 5.55 (shadow 0.15) | post 0.32: ao 0.24 bloom 0.05 composite 0.03"` cpu frame 9.65
- after `forest_edge_golden__remastered`: `frame 7.87 ms | world 7.31 (shadow 0.44) | post 0.42: ao 0.34 bloom 0.05 composite 0.03"` cpu frame 11.44
- after `forest_edge_golden__t2`: `frame 10.99 ms | world 10.42 (shadow 0.46) | post 0.43: ao 0.36 bloom 0.05 composite 0.03"` cpu frame 13.39
- after `forest_edge_golden__t3`: `frame 12.06 ms | world 11.57 (shadow 0.77) | post 0.35: ao 0.28 bloom 0.05 composite 0.03"` cpu frame 14.07
- after `forest_edge_golden__dbgsunvis`: `frame 12.06 ms | world 11.57 (shadow 0.77) | post 0.35: ao 0.28 bloom 0.05 composite 0.03"` cpu frame 14.07
- after `forest_edge_golden__classic`: `frame 11.02 ms | world 10.89"` cpu frame 8.28
- after `noon_fields__remastered`: `frame 18.64 ms | world 18.12 (shadow 0.09) | post 0.41: ao 0.34 bloom 0.05 composite 0.03"` cpu frame 25.93
- after `noon_fields__t2`: `frame 23.89 ms | world 23.16 (shadow 0.11) | post 0.62: ao 0.54 bloom 0.05 composite 0.03"` cpu frame 27.87
- after `noon_fields__t3`: `frame 19.15 ms | world 18.60 (shadow 0.08) | post 0.44: ao 0.37 bloom 0.05 composite 0.03"` cpu frame 27.28
- after `noon_fields__dbgsunvis`: `frame 19.15 ms | world 18.60 (shadow 0.08) | post 0.44: ao 0.37 bloom 0.05 composite 0.03"` cpu frame 27.28
- after `noon_fields__classic`: `frame 22.66 ms | world 22.54"` cpu frame 25.15
- after `town_night__remastered`: `frame 10.34 ms | world 9.81 | post 0.36: ao 0.28 bloom 0.05 composite 0.03"` cpu frame 11.41
- after `town_night__classic`: `frame 9.40 ms | world 9.25"` cpu frame 13.82
- after `town_day__remastered`: `frame 4.95 ms | world 4.45 (shadow 0.14) | post 0.36: ao 0.28 bloom 0.05 composite 0.03"` cpu frame 9.91
- after `town_day__t2`: `frame 4.25 ms | world 3.72 (shadow 0.14) | post 0.39: ao 0.31 bloom 0.05 composite 0.03"` cpu frame 9.14
- after `town_day__t3`: `frame 4.07 ms | world 3.48 (shadow 0.14) | post 0.45: ao 0.37 bloom 0.05 composite 0.03"` cpu frame 8.51
- after `town_day__dbgsunvis`: `frame 4.07 ms | world 3.48 (shadow 0.14) | post 0.45: ao 0.37 bloom 0.05 composite 0.03"` cpu frame 8.51
- after `town_day__classic`: `frame 3.33 ms | world 3.20"` cpu frame 6.9
- after `backlit_hornbeam__remastered`: `frame 12.68 ms | world 12.11 (shadow 0.79) | post 0.45: ao 0.22 bloom 0.05 shafts 0.15 composite 0.03"` cpu frame 16.57
- after `backlit_hornbeam__dbgsunvis`: `frame 12.68 ms | world 12.11 (shadow 0.79) | post 0.45: ao 0.22 bloom 0.05 shafts 0.15 composite 0.03"` cpu frame 16.57
- after `pine_stand_noon__remastered`: `frame 10.93 ms | world 9.96 (shadow 0.18) | post 0.81: ao 0.73 bloom 0.05 composite 0.03"` cpu frame 17.9
- after `pine_stand_noon__dbgsunvis`: `frame 10.93 ms | world 9.96 (shadow 0.18) | post 0.81: ao 0.73 bloom 0.05 composite 0.03"` cpu frame 17.9
