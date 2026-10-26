| label | before GPU frame | before world | before shadow | before CPU frame | after GPU frame | after world | after shadow | after CPU frame |
|---|---|---|---|---|---|---|---|---|
| backlit_hornbeam__dbgsunvis | 6.72 | 6.02 | 0.00 | 11.73 | 8.60 | 8.02 | 0.49 | 12.94 |
| backlit_hornbeam__remastered | 6.72 | 6.02 | 0.00 | 11.73 | 8.60 | 8.02 | 0.49 | 12.94 |
| forest_edge_golden__boot | 4.61 | 4.18 | 0.03 | 6.82 | 4.06 | 3.61 | 0.15 | 7.57 |
| forest_edge_golden__classic | 5.59 | 5.47 |  | 5.57 | 8.96 | 8.83 |  | 7.06 |
| forest_edge_golden__dbgsunvis | 11.81 | 11.00 | 0.03 | 11.94 | 11.22 | 10.74 | 0.83 | 12.53 |
| forest_edge_golden__remastered | 6.15 | 5.72 | 0.03 | 8.96 | 8.85 | 8.41 | 0.19 | 10.49 |
| forest_edge_golden__t2 | 9.51 | 8.96 | 0.11 | 9.97 | 10.55 | 10.00 | 0.29 | 11.66 |
| forest_edge_golden__t3 | 11.81 | 11.00 | 0.03 | 11.94 | 11.22 | 10.74 | 0.83 | 12.53 |
| noon_fields__classic | 21.18 | 21.08 |  | 26.82 | 18.92 | 18.81 |  | 23.10 |
| noon_fields__dbgsunvis | 27.77 | 27.40 | 0.28 | 29.59 | 30.82 | 30.47 | 0.53 | 30.17 |
| noon_fields__remastered | 21.38 | 20.98 | 0.11 | 26.36 | 25.25 | 24.86 | 0.60 | 27.52 |
| noon_fields__t2 | 29.14 | 28.75 | 0.27 | 27.96 | 25.60 | 25.25 | 0.62 | 29.00 |
| noon_fields__t3 | 27.77 | 27.40 | 0.28 | 29.59 | 30.82 | 30.47 | 0.53 | 30.17 |
| pine_stand_noon__dbgsunvis | 6.37 | 5.84 | 0.00 | 10.72 | 8.04 | 7.58 | 0.25 | 10.56 |
| pine_stand_noon__remastered | 6.37 | 5.84 | 0.00 | 10.72 | 8.04 | 7.58 | 0.25 | 10.56 |
| town_day__classic | 3.48 | 3.35 |  | 6.41 | 4.35 | 4.22 |  | 7.28 |
| town_day__dbgsunvis | 3.78 | 3.24 | 0.03 | 8.35 | 4.29 | 3.87 | 0.14 | 9.58 |
| town_day__remastered | 4.45 | 3.85 | 0.03 | 9.07 | 7.91 | 7.50 | 0.14 | 11.78 |
| town_day__t2 | 4.19 | 3.71 | 0.03 | 8.98 | 6.37 | 5.90 | 0.14 | 10.71 |
| town_day__t3 | 3.78 | 3.24 | 0.03 | 8.35 | 4.29 | 3.87 | 0.14 | 9.58 |
| town_night__classic | 13.81 | 13.62 |  | 13.62 | 11.08 | 10.92 |  | 12.03 |
| town_night__remastered | 7.06 | 6.14 |  | 10.44 | 9.10 | 8.67 |  | 11.07 |

| view | before shadow pass median (n) | after shadow pass median (n) | before GPU frame median | after GPU frame median |
|---|---|---|---|---|
| backlit_hornbeam | 0.00 (1) | 0.49 (1) | 6.72 (1) | 8.60 (1) |
| forest_edge_golden | 0.03 (3) | 0.29 (3) | 9.51 (3) | 10.55 (3) |
| noon_fields | 0.27 (3) | 0.60 (3) | 27.77 (3) | 25.60 (3) |
| pine_stand_noon | 0.00 (1) | 0.25 (1) | 6.37 (1) | 8.04 (1) |
| town_day | 0.03 (3) | 0.14 (3) | 4.19 (3) | 6.37 (3) |
| town_night |  |  | 7.06 (1) | 9.10 (1) |

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
- after `forest_edge_golden__boot`: `frame 4.06 ms | world 3.61 (shadow 0.15) | post 0.31: ao 0.24 bloom 0.05 composite 0.03"` cpu frame 7.57
- after `forest_edge_golden__remastered`: `frame 8.85 ms | world 8.41 (shadow 0.19) | post 0.29: ao 0.21 bloom 0.05 composite 0.03"` cpu frame 10.49
- after `forest_edge_golden__t2`: `frame 10.55 ms | world 10.00 (shadow 0.29) | post 0.42: ao 0.34 bloom 0.05 composite 0.03"` cpu frame 11.66
- after `forest_edge_golden__t3`: `frame 11.22 ms | world 10.74 (shadow 0.83) | post 0.34: ao 0.26 bloom 0.05 composite 0.03"` cpu frame 12.53
- after `forest_edge_golden__dbgsunvis`: `frame 11.22 ms | world 10.74 (shadow 0.83) | post 0.34: ao 0.26 bloom 0.05 composite 0.03"` cpu frame 12.53
- after `forest_edge_golden__classic`: `frame 8.96 ms | world 8.83"` cpu frame 7.06
- after `noon_fields__remastered`: `frame 25.25 ms | world 24.86 (shadow 0.60) | post 0.28: ao 0.20 bloom 0.05 composite 0.03"` cpu frame 27.52
- after `noon_fields__t2`: `frame 25.60 ms | world 25.25 (shadow 0.62) | post 0.24: ao 0.17 bloom 0.05 composite 0.03"` cpu frame 29.0
- after `noon_fields__t3`: `frame 30.82 ms | world 30.47 (shadow 0.53) | post 0.24: ao 0.17 bloom 0.05 composite 0.03"` cpu frame 30.17
- after `noon_fields__dbgsunvis`: `frame 30.82 ms | world 30.47 (shadow 0.53) | post 0.24: ao 0.17 bloom 0.05 composite 0.03"` cpu frame 30.17
- after `noon_fields__classic`: `frame 18.92 ms | world 18.81"` cpu frame 23.1
- after `town_night__remastered`: `frame 9.10 ms | world 8.67 | post 0.27: ao 0.20 bloom 0.05 composite 0.03"` cpu frame 11.07
- after `town_night__classic`: `frame 11.08 ms | world 10.92"` cpu frame 12.03
- after `town_day__remastered`: `frame 7.91 ms | world 7.50 (shadow 0.14) | post 0.28: ao 0.20 bloom 0.05 composite 0.03"` cpu frame 11.78
- after `town_day__t2`: `frame 6.37 ms | world 5.90 (shadow 0.14) | post 0.33: ao 0.25 bloom 0.05 composite 0.03"` cpu frame 10.71
- after `town_day__t3`: `frame 4.29 ms | world 3.87 (shadow 0.14) | post 0.29: ao 0.21 bloom 0.05 composite 0.03"` cpu frame 9.58
- after `town_day__dbgsunvis`: `frame 4.29 ms | world 3.87 (shadow 0.14) | post 0.29: ao 0.21 bloom 0.05 composite 0.03"` cpu frame 9.58
- after `town_day__classic`: `frame 4.35 ms | world 4.22"` cpu frame 7.28
- after `backlit_hornbeam__remastered`: `frame 8.60 ms | world 8.02 (shadow 0.49) | post 0.46: ao 0.23 bloom 0.05 shafts 0.15 composite 0.03"` cpu frame 12.94
- after `backlit_hornbeam__dbgsunvis`: `frame 8.60 ms | world 8.02 (shadow 0.49) | post 0.46: ao 0.23 bloom 0.05 shafts 0.15 composite 0.03"` cpu frame 12.94
- after `pine_stand_noon__remastered`: `frame 8.04 ms | world 7.58 (shadow 0.25) | post 0.36: ao 0.28 bloom 0.05 composite 0.03"` cpu frame 10.56
- after `pine_stand_noon__dbgsunvis`: `frame 8.04 ms | world 7.58 (shadow 0.25) | post 0.36: ao 0.28 bloom 0.05 composite 0.03"` cpu frame 10.56
