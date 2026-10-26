# Volumetric track: GPU timings per shot

Smoothed `triGpuTimes` averages over the 60-90 settle frames before each shot, 1440x960 render target
(960x640 window, 150% scale), 4x MSAA. The `shafts` column is the volumetric pass (march, temporal
resolve when on, two blur passes) when a vol variant is active, else the old screen-space shafts.
**Caveat:** every run shared the GPU with up to three other agents' games (tri global slots), so the
frame and world columns swing by 2-5x between shots of the same view; compare the `shafts` column.

| run | shot | frame | world | shadow | post | ao | shafts / vol | composite |
|---|---|---|---|---|---|---|---|---|
| probe1 | grove_sun__classic | 3.21 | 3.08 |  |  |  |  |  |
| probe1 | grove_sun__remastered | 10.84 | 10.27 | 0.00 | 0.43 | 0.20 | 0.15 | 0.03 |
| probe1 | grove_sun__vol_a | 22.43 | 20.24 | 0.00 | 2.05 | 0.28 | 1.59 | 0.13 |
| probe1 | grove_sun__vol_a_dbg | 20.49 | 18.54 | 0.00 | 1.81 | 0.32 | 1.29 | 0.15 |
| probe1 | grove_sun__classic | 3.97 | 3.85 |  |  |  |  |  |
| probe1 | grove_sun__remastered | 8.75 | 7.92 | 0.00 | 0.70 | 0.47 | 0.15 | 0.03 |
| probe1 | grove_sun__vol_a | 17.97 | 16.12 | 0.00 | 1.71 | 0.34 | 1.13 | 0.20 |
| probe1 | grove_sun__vol_a_dbg | 17.67 | 15.69 | 0.00 | 1.84 | 0.63 | 0.79 | 0.36 |
| probe1 | grove_sun__vol_t | 18.63 | 16.82 | 0.00 | 1.67 | 0.49 | 1.08 | 0.06 |
| probe1 | grove_sun__vol_b | 17.59 | 15.79 | 0.00 | 1.68 | 0.49 | 0.90 | 0.25 |
| probe1 | grove_sun__vol_m3 | 17.54 | 15.91 | 0.00 | 1.50 | 0.44 | 0.92 | 0.09 |
| probe1 | sunrise_ridge__classic | 46.01 | 45.89 |  |  |  |  |  |
| probe1 | sunrise_ridge__remastered | 40.83 | 39.80 | 0.00 | 0.90 | 0.67 | 0.15 | 0.03 |
| probe1 | sunrise_ridge__vol_a | 35.77 | 33.43 | 0.00 | 2.20 | 1.06 | 0.46 | 0.63 |
| probe1 | sunrise_ridge__vol_a_dbg | 41.20 | 38.26 | 0.00 | 2.81 | 1.55 | 0.49 | 0.72 |
| probe1 | sunrise_ridge__vol_t | 34.66 | 32.53 | 0.00 | 2.00 | 0.63 | 1.26 | 0.06 |
| probe1 | sunrise_ridge__vol_b | 48.69 | 46.51 | 0.00 | 2.05 | 0.79 | 0.63 | 0.59 |
| probe1 | grove_mist__classic | 6.55 | 6.43 |  |  |  |  |  |
| probe1 | grove_mist__remastered | 7.79 | 7.20 | 0.00 | 0.46 | 0.23 | 0.15 | 0.03 |
| probe1 | grove_mist__vol_a | 8.54 | 6.78 | 0.00 | 1.62 | 0.43 | 1.01 | 0.13 |
| probe1 | grove_mist__vol_a_dbg | 9.12 | 7.52 | 0.00 | 1.46 | 0.31 | 1.02 | 0.08 |
| probe1 | grove_mist__vol_t | 7.97 | 6.24 | 0.00 | 1.59 | 0.41 | 1.08 | 0.05 |
| probe1 | town_night__classic | 10.50 | 10.35 |  |  |  |  |  |
| probe1 | town_night__remastered | 12.84 | 12.26 |  | 0.41 | 0.34 |  | 0.03 |
| probe1 | town_night__vol_a | 13.22 | 12.52 |  | 0.54 | 0.41 | 0.04 | 0.04 |
| probe1 | town_night__vol_a_dbg | 13.02 | 12.32 |  | 0.54 | 0.42 | 0.04 | 0.04 |
| probe1 | town_night__vol_l3 | 13.97 | 13.32 |  | 0.50 | 0.37 | 0.04 | 0.04 |
| probe2 | pine_eve__classic | 6.03 | 5.91 |  |  |  |  |  |
| probe2 | pine_eve__remastered | 7.19 | 6.68 | 1.44 | 0.38 | 0.15 | 0.15 | 0.03 |
| probe2 | pine_eve__va | 16.52 | 14.28 | 1.21 | 2.12 | 0.20 | 1.81 | 0.06 |
| probe2 | pine_eve__va_dbg | 34.09 | 31.74 | 0.66 | 2.23 | 0.16 | 1.96 | 0.06 |
| probe2 | pine_eve__vb | 27.07 | 25.18 | 0.40 | 1.77 | 0.15 | 1.51 | 0.06 |
| probe2 | pine_eve__vc | 24.66 | 22.83 | 0.35 | 1.70 | 0.18 | 1.42 | 0.06 |
| probe2 | pine_eve__vc_dbg | 26.75 | 24.85 | 0.23 | 1.78 | 0.22 | 1.46 | 0.05 |
| probe2 | pine_eve__vt | 29.79 | 27.73 | 0.75 | 1.93 | 0.31 | 1.51 | 0.06 |
| probe2 | pine_low__classic | 27.26 | 27.15 |  |  |  |  |  |
| probe2 | pine_low__remastered | 31.74 | 31.22 | 0.23 | 0.40 | 0.17 | 0.15 | 0.03 |
| probe2 | pine_low__va | 32.54 | 30.56 | 0.23 | 1.87 | 0.17 | 1.59 | 0.06 |
| probe2 | pine_low__va_dbg | 33.76 | 31.56 | 0.31 | 2.08 | 0.16 | 1.83 | 0.05 |
| probe2 | pine_low__vb | 34.15 | 31.94 | 0.37 | 2.10 | 0.16 | 1.83 | 0.06 |
| probe2 | pine_low__vc | 24.65 | 22.85 | 0.20 | 1.68 | 0.32 | 1.26 | 0.06 |
| probe2 | pine_low__vc_dbg | 22.40 | 20.74 | 0.21 | 1.54 | 0.23 | 1.21 | 0.05 |
| probe2 | pine_low__vt | 20.20 | 18.44 | 0.21 | 1.64 | 0.33 | 1.20 | 0.06 |
| probe2 | pine_mist__classic | 10.76 | 10.64 |  |  |  |  |  |
| probe2 | pine_mist__remastered | 8.06 | 7.42 | 0.23 | 0.50 | 0.27 | 0.15 | 0.03 |
| probe2 | pine_mist__va | 11.00 | 9.52 | 0.22 | 1.35 | 0.30 | 0.94 | 0.05 |
| probe2 | pine_mist__va_dbg | 10.88 | 8.81 | 0.26 | 1.93 | 0.30 | 1.53 | 0.05 |
| probe2 | pine_mist__vb | 12.19 | 10.14 | 0.19 | 1.91 | 0.20 | 1.60 | 0.07 |
| probe2 | pine_mist__vc | 13.33 | 11.45 | 0.19 | 1.75 | 0.27 | 1.37 | 0.06 |
| probe2 | pine_mist__vc_dbg | 12.92 | 11.11 | 0.22 | 1.67 | 0.21 | 1.36 | 0.05 |
| probe2 | pine_mist__vt | 13.28 | 10.89 | 0.22 | 2.25 | 0.40 | 1.73 | 0.07 |
| probe2 | sunrise_ridge__classic | 26.83 | 26.70 |  |  |  |  |  |
| probe2 | sunrise_ridge__remastered | 40.97 | 40.22 | 0.38 | 0.62 | 0.39 | 0.15 | 0.03 |
| probe2 | sunrise_ridge__va | 40.54 | 38.19 | 0.32 | 2.22 | 0.55 | 1.57 | 0.05 |
| probe2 | sunrise_ridge__vb | 41.76 | 39.16 | 0.10 | 2.47 | 0.69 | 1.68 | 0.06 |
| probe2 | sunrise_ridge__vc | 37.77 | 35.40 | 0.12 | 2.26 | 0.32 | 1.84 | 0.05 |
| probe2 | town_night__classic | 10.81 | 10.66 |  |  |  |  |  |
| probe2 | town_night__remastered | 12.07 | 11.40 |  | 0.51 | 0.43 |  | 0.03 |
| probe2 | town_night__va | 14.07 | 13.44 |  | 0.47 | 0.31 | 0.07 | 0.04 |
| probe2 | town_night__va_dbg | 13.91 | 13.27 |  | 0.49 | 0.34 | 0.07 | 0.04 |
| probe2 | town_night__vl3 | 13.68 | 12.94 |  | 0.57 | 0.41 | 0.07 | 0.04 |
| probe3 | pinewood_morn__classic | 6.11 | 5.97 |  |  |  |  |  |
| probe3 | pinewood_morn__remastered | 6.16 | 5.44 | 0.26 | 0.57 | 0.25 | 0.15 | 0.12 |
| probe3 | pinewood_morn__va | 13.44 | 11.08 | 1.78 | 2.22 | 0.17 | 1.94 | 0.06 |
| probe3 | pinewood_morn__va_dbg | 15.58 | 13.45 | 1.50 | 1.99 | 0.18 | 1.72 | 0.05 |
| probe3 | pinewood_morn__va2 | 15.81 | 13.62 | 1.48 | 2.06 | 0.33 | 1.62 | 0.05 |
| probe3 | pinewood_morn__va3 | 17.33 | 15.19 | 2.05 | 2.00 | 0.17 | 1.72 | 0.06 |
| probe3 | pinewood_morn__vak | 16.13 | 14.04 | 1.67 | 1.95 | 0.21 | 1.64 | 0.06 |
| probe3 | pine_eve__classic | 22.28 | 22.16 |  |  |  |  |  |
| probe3 | pine_eve__remastered | 15.43 | 14.80 | 0.29 | 0.51 | 0.28 | 0.15 | 0.03 |
| probe3 | pine_eve__va2 | 21.36 | 19.72 | 0.27 | 1.52 | 0.36 | 1.07 | 0.05 |
| probe3 | pine_eve__va3 | 27.56 | 25.55 | 0.23 | 1.90 | 0.36 | 1.44 | 0.05 |
| probe3 | pine_eve__vak | 24.34 | 22.69 | 0.26 | 1.53 | 0.21 | 1.22 | 0.05 |
| probe3 | pine_mist__classic | 17.21 | 17.09 |  |  |  |  |  |
| probe3 | pine_mist__remastered | 16.27 | 15.75 | 0.19 | 0.38 | 0.15 | 0.15 | 0.03 |
| probe3 | pine_mist__va2 | 18.24 | 16.01 | 0.96 | 2.10 | 0.16 | 1.84 | 0.06 |
| probe3 | pine_mist__va3 | 19.09 | 16.72 | 0.68 | 2.24 | 0.19 | 1.94 | 0.06 |
| probe3 | forest_edge_golden__classic | 15.96 | 15.37 |  |  |  |  |  |
| probe3 | forest_edge_golden__remastered | 14.10 | 13.66 | 0.29 | 0.30 | 0.23 |  | 0.03 |
| probe3 | forest_edge_golden__va | 20.52 | 18.09 | 1.39 | 2.31 | 0.18 | 2.03 | 0.06 |
| probe3 | forest_edge_golden__va2 | 17.78 | 15.31 | 0.72 | 2.34 | 0.19 | 2.03 | 0.06 |
| probe3 | lowsun_fp__classic | 13.71 | 13.59 |  |  |  |  |  |
| probe3 | lowsun_fp__remastered | 19.36 | 16.99 | 0.48 | 2.26 | 0.17 | 0.22 | 1.82 |
| probe3 | lowsun_fp__va | 18.98 | 16.36 | 0.54 | 2.51 | 0.17 | 2.24 | 0.06 |
| probe3 | noon_fields__classic | 33.14 | 33.03 |  |  |  |  |  |
| probe3 | noon_fields__remastered | 44.59 | 44.23 | 0.07 | 0.25 | 0.17 |  | 0.03 |
| probe3 | noon_fields__va | 33.26 | 31.21 | 0.46 | 1.94 | 0.17 | 1.67 | 0.05 |
| probe3 | town_night__classic | 12.37 | 12.20 |  |  |  |  |  |
| probe3 | town_night__remastered | 15.58 | 15.14 |  | 0.27 | 0.20 |  | 0.03 |
| probe3 | town_night__va | 19.84 | 19.12 |  | 0.45 | 0.31 | 0.04 | 0.04 |
| probe3 | town_night__va_dbg | 22.23 | 21.62 |  | 0.36 | 0.21 | 0.05 | 0.04 |
| probe3 | town_night__vl3 | 21.88 | 21.32 |  | 0.39 | 0.24 | 0.05 | 0.04 |
| probe4 | lowsun_fp__classic | 5.97 | 5.87 |  |  |  |  |  |
| probe4 | lowsun_fp__remastered | 11.47 | 9.31 | 0.23 | 2.06 | 0.17 | 0.22 | 1.62 |
| probe4 | lowsun_fp__va | 16.94 | 14.59 | 0.60 | 2.24 | 0.27 | 1.86 | 0.06 |
| probe4 | lowsun_fp__vah | 14.87 | 12.49 | 0.33 | 2.28 | 0.23 | 1.89 | 0.10 |
| probe4 | pine_eve__classic | 29.19 | 29.08 |  |  |  |  |  |
| probe4 | pine_eve__remastered | 31.97 | 31.45 | 0.37 | 0.39 | 0.15 | 0.15 | 0.04 |
| probe4 | pine_eve__va | 29.79 | 27.34 | 0.26 | 2.33 | 0.23 | 1.99 | 0.07 |
| probe4 | pine_eve__vah | 30.73 | 28.51 | 0.34 | 2.10 | 0.20 | 1.75 | 0.10 |
| probe4 | pine_eve__va3h | 32.09 | 29.58 | 0.56 | 2.38 | 0.25 | 1.99 | 0.10 |
| probe4 | pinewood_morn__classic | 15.29 | 15.15 |  |  |  |  |  |
| probe4 | pinewood_morn__remastered | 9.17 | 8.41 | 0.88 | 0.61 | 0.37 | 0.15 | 0.04 |
| probe4 | pinewood_morn__va | 8.75 | 7.21 | 0.78 | 1.40 | 0.29 | 0.99 | 0.07 |
| probe4 | pinewood_morn__vah | 11.69 | 9.68 | 1.03 | 1.87 | 0.57 | 1.15 | 0.10 |
| probe4 | pinewood_morn__va3h | 10.22 | 8.22 | 0.93 | 1.84 | 0.32 | 1.37 | 0.10 |
| probe4 | pine_mist__classic | 11.56 | 11.43 |  |  |  |  |  |
| probe4 | pine_mist__remastered | 6.78 | 6.00 | 0.25 | 0.65 | 0.41 | 0.15 | 0.04 |
| probe4 | pine_mist__va | 11.91 | 10.33 | 0.19 | 1.44 | 0.25 | 1.10 | 0.05 |
| probe4 | pine_mist__vah | 11.97 | 9.82 | 0.19 | 2.01 | 0.40 | 1.47 | 0.10 |
| probe4 | pine_mist__va3h | 13.53 | 11.72 | 0.19 | 1.68 | 0.34 | 1.20 | 0.09 |
| probe4 | sunrise_ridge__classic | 11.69 | 11.56 |  |  |  |  |  |
| probe4 | sunrise_ridge__remastered | 26.05 | 24.95 | 0.07 | 0.97 | 0.74 | 0.15 | 0.04 |
| probe4 | sunrise_ridge__va | 37.60 | 34.99 | 0.27 | 2.49 | 0.97 | 1.41 | 0.06 |
| probe4 | sunrise_ridge__vah | 45.44 | 43.21 | 0.20 | 2.10 | 0.35 | 1.61 | 0.10 |
| probe4 | town_night__classic | 16.33 | 16.18 |  |  |  |  |  |
| probe4 | town_night__remastered | 18.17 | 17.40 |  | 0.60 | 0.52 |  | 0.03 |
| probe4 | town_night__va | 18.25 | 17.54 |  | 0.55 | 0.40 | 0.05 | 0.05 |
| probe4 | town_night__va_dbg | 16.76 | 16.00 |  | 0.59 | 0.45 | 0.05 | 0.04 |
| probe4 | town_night__vl3 | 13.87 | 13.00 |  | 0.70 | 0.54 | 0.05 | 0.05 |
| probe5 | lowsun_fp__classic | 5.03 | 4.92 |  |  |  |  |  |
| probe5 | lowsun_fp__remastered | 5.77 | 4.73 | 0.12 | 0.93 | 0.22 | 0.22 | 0.44 |
| probe5 | lowsun_fp__va | 14.06 | 12.18 | 0.56 | 1.77 | 0.32 | 1.33 | 0.07 |
| probe5 | lowsun_fp__vah | 11.38 | 9.87 | 0.40 | 1.40 | 0.29 | 0.95 | 0.10 |
| probe5 | pine_eve__classic | 23.62 | 23.50 |  |  |  |  |  |
| probe5 | pine_eve__remastered | 26.14 | 25.58 | 0.38 | 0.43 | 0.19 | 0.15 | 0.04 |
| probe5 | pine_eve__va | 29.47 | 27.52 | 0.40 | 1.82 | 0.20 | 1.51 | 0.06 |
| probe5 | pine_eve__vah | 30.95 | 28.78 | 0.23 | 2.05 | 0.16 | 1.73 | 0.11 |
| probe5 | pine_eve__va3h | 28.06 | 26.03 | 0.28 | 1.91 | 0.23 | 1.52 | 0.11 |
| probe5 | pinewood_morn__classic | 15.05 | 14.92 |  |  |  |  |  |
| probe5 | pinewood_morn__remastered | 13.35 | 12.74 | 1.43 | 0.46 | 0.20 | 0.15 | 0.06 |
| probe5 | pinewood_morn__vah | 14.43 | 12.24 | 1.28 | 2.05 | 0.20 | 1.69 | 0.11 |
| probe5 | pinewood_morn__va3h | 15.66 | 13.39 | 1.53 | 2.12 | 0.18 | 1.78 | 0.11 |
| probe5 | pine_mist__classic | 5.29 | 5.16 |  |  |  |  |  |
| probe5 | pine_mist__remastered | 10.85 | 10.29 | 0.19 | 0.43 | 0.19 | 0.15 | 0.04 |
| probe5 | pine_mist__va | 7.84 | 6.32 | 0.24 | 1.39 | 0.36 | 0.92 | 0.05 |
| probe5 | pine_mist__vah | 12.61 | 10.63 | 0.32 | 1.84 | 0.29 | 1.40 | 0.11 |
| probe5 | sunrise_ridge__classic | 13.01 | 12.88 |  |  |  |  |  |
| probe5 | sunrise_ridge__remastered | 33.00 | 32.25 | 0.18 | 0.61 | 0.37 | 0.15 | 0.04 |
| probe5 | sunrise_ridge__va | 37.28 | 34.87 | 0.16 | 2.27 | 0.31 | 1.85 | 0.06 |
| probe5 | sunrise_ridge__vah | 38.06 | 35.45 | 0.23 | 2.48 | 0.97 | 1.35 | 0.11 |
| probe5 | town_night__classic | 16.22 | 16.07 |  |  |  |  |  |
| probe5 | town_night__remastered | 17.35 | 16.88 |  | 0.31 | 0.23 |  | 0.03 |
| probe5 | town_night__van | 17.79 | 17.12 |  | 0.51 | 0.37 | 0.05 | 0.05 |
| probe5 | town_night__van_dbg | 13.19 | 11.97 |  | 1.05 | 0.92 | 0.05 | 0.04 |
| probe5 | town_night__vl2 | 17.03 | 16.43 |  | 0.39 | 0.24 | 0.05 | 0.05 |
| probe6 | hl_side__classic | 4.22 | 4.07 |  |  |  |  |  |
| probe6 | hl_side__remastered | 6.71 | 6.31 |  | 0.28 | 0.20 |  | 0.03 |
| probe6 | hl_side__vn | 7.45 | 6.95 |  | 0.37 | 0.20 | 0.07 | 0.05 |
| probe6 | hl_side__vns | 8.00 | 6.25 |  | 1.61 | 0.20 | 1.13 | 0.22 |
| probe6 | hl_side__vns2 | 8.28 | 6.28 |  | 1.87 | 0.20 | 1.18 | 0.44 |
| probe6 | hl_side__vnsm | 7.31 | 5.52 |  | 1.63 | 0.19 | 0.96 | 0.38 |
| probe6 | hl_side__vns2_dbg | 7.17 | 5.29 |  | 1.64 | 0.20 | 1.06 | 0.23 |
| probe6 | hl_front__classic | 7.70 | 7.55 |  |  |  |  |  |
| probe6 | hl_front__remastered | 7.33 | 6.75 |  | 0.44 | 0.36 |  | 0.03 |
| probe6 | hl_front__vns | 7.29 | 6.37 |  | 0.77 | 0.36 | 0.17 | 0.19 |
| probe6 | hl_front__vns2 | 13.49 | 11.36 |  | 1.99 | 0.52 | 0.83 | 0.59 |
| probe6 | hl_front__vnsm | 11.94 | 10.74 |  | 1.04 | 0.29 | 0.36 | 0.35 |
| probe6 | pinewood_morn__classic | 20.07 | 19.93 |  |  |  |  |  |
| probe6 | pinewood_morn__remastered | 14.70 | 14.03 | 1.61 | 0.53 | 0.27 | 0.15 | 0.05 |
| probe6 | pinewood_morn__vah | 14.07 | 12.07 | 1.26 | 1.85 | 0.30 | 1.40 | 0.10 |
| probe6 | pinewood_morn__vg25 | 21.15 | 18.73 | 1.63 | 2.28 | 0.19 | 1.90 | 0.14 |
| probe6 | pinewood_morn__vg4 | 20.48 | 17.73 | 2.03 | 2.60 | 0.36 | 2.05 | 0.14 |
| probe6 | pinewood_morn__va3g | 22.36 | 19.50 | 1.75 | 2.71 | 0.19 | 2.34 | 0.14 |
| probe6 | pinewood_morn__vg25_dbg | 21.22 | 18.36 | 2.00 | 2.72 | 0.19 | 2.35 | 0.13 |
| probe6 | pinewood_mist__classic | 13.73 | 13.59 |  |  |  |  |  |
| probe6 | pinewood_mist__remastered | 7.70 | 6.99 | 1.09 | 0.56 | 0.31 | 0.15 | 0.05 |
| probe6 | pinewood_mist__va | 13.03 | 10.18 | 1.13 | 2.71 | 0.18 | 2.34 | 0.14 |
| probe6 | pinewood_mist__vah | 12.71 | 10.11 | 1.08 | 2.45 | 0.19 | 2.07 | 0.14 |
| probe6 | pinewood_mist__vg25 | 12.67 | 9.75 | 1.34 | 2.77 | 0.19 | 2.38 | 0.14 |
| probe6 | pinewood_mist__va3g | 13.19 | 10.52 | 1.20 | 2.52 | 0.20 | 2.13 | 0.14 |
| probe6 | pine_eve__classic | 34.85 | 34.73 |  |  |  |  |  |
| probe6 | pine_eve__remastered | 27.55 | 26.99 | 0.51 | 0.45 | 0.15 | 0.15 | 0.09 |
| probe6 | pine_eve__vah | 42.36 | 39.82 | 1.22 | 2.43 | 0.17 | 2.07 | 0.14 |
| probe6 | pine_eve__vg25 | 11.05 | 9.25 | 0.15 | 1.68 | 0.31 | 1.19 | 0.13 |
| probe6 | pine_eve__va3g | 17.13 | 14.97 | 0.15 | 2.03 | 0.21 | 1.64 | 0.13 |
| probe6 | lowsun_fp__classic | 14.84 | 14.73 |  |  |  |  |  |
| probe6 | lowsun_fp__remastered | 19.26 | 17.11 | 0.50 | 2.04 | 0.18 | 0.22 | 1.59 |
| probe6 | lowsun_fp__vah | 19.29 | 16.82 | 0.50 | 2.37 | 0.20 | 1.98 | 0.14 |
| probe6 | lowsun_fp__vg25 | 19.09 | 16.73 | 0.47 | 2.25 | 0.18 | 1.88 | 0.14 |
| probe6 | sunrise_ridge__classic | 40.01 | 39.88 |  |  |  |  |  |
| probe6 | sunrise_ridge__remastered | 26.72 | 26.06 | 0.26 | 0.54 | 0.30 | 0.15 | 0.04 |
| probe6 | sunrise_ridge__va | 36.42 | 34.15 | 0.12 | 2.14 | 0.36 | 1.59 | 0.13 |
| probe6 | sunrise_ridge__vglo | 42.93 | 40.70 | 0.11 | 2.09 | 0.55 | 1.35 | 0.13 |
| probe6 | sunrise_ridge__vg25 | 43.25 | 40.92 | 0.08 | 2.21 | 0.30 | 1.72 | 0.13 |
| probe6 | pinewood_pan__sp_p00 | 12.23 | 10.39 | 1.21 | 1.69 | 0.22 | 1.28 | 0.13 |
| probe6 | pinewood_pan__sp_p08 | 12.23 | 10.39 | 1.21 | 1.69 | 0.22 | 1.28 | 0.13 |
| probe6 | pinewood_pan__sp_p16 | 12.23 | 10.39 | 1.21 | 1.69 | 0.22 | 1.28 | 0.13 |
| probe6 | pinewood_pan__sp_p24 | 12.23 | 10.39 | 1.21 | 1.69 | 0.22 | 1.28 | 0.13 |
| probe6 | pinewood_pan__tt_p00 | 8.28 | 6.27 | 0.58 | 1.86 | 0.24 | 1.43 | 0.14 |
| probe6 | pinewood_pan__tt_p08 | 8.28 | 6.27 | 0.58 | 1.86 | 0.24 | 1.43 | 0.14 |
| probe6 | pinewood_pan__tt_p16 | 8.28 | 6.27 | 0.58 | 1.86 | 0.24 | 1.43 | 0.14 |
| probe6 | pinewood_pan__tt_p24 | 8.28 | 6.27 | 0.58 | 1.86 | 0.24 | 1.43 | 0.14 |
| probe7 | olive_morn__classic | 3.07 | 2.94 |  |  |  |  |  |
| probe7 | olive_morn__remastered | 5.95 | 5.01 | 1.16 | 0.82 | 0.34 | 0.15 | 0.27 |
| probe7 | olive_morn__vg25 | 10.76 | 8.49 | 1.35 | 2.13 | 0.27 | 1.68 | 0.13 |
| probe7 | olive_morn__vg4 | 10.77 | 9.17 | 1.11 | 1.46 | 0.19 | 1.10 | 0.13 |
| probe7 | olive_morn__vg25_dbg | 10.90 | 9.05 | 1.23 | 1.71 | 0.20 | 1.35 | 0.12 |
| probe7 | town_eve__classic | 5.51 | 5.39 |  |  |  |  |  |
| probe7 | town_eve__remastered | 7.51 | 6.84 | 0.31 | 0.54 | 0.30 | 0.15 | 0.04 |
| probe7 | town_eve__vg25 | 10.52 | 8.20 | 1.58 | 2.19 | 0.17 | 1.84 | 0.14 |
| probe7 | town_eve__vg4 | 8.50 | 6.55 | 0.37 | 1.82 | 0.19 | 1.45 | 0.13 |
| probe7 | town_eve__vg25_dbg | 11.08 | 8.65 | 0.25 | 2.29 | 0.17 | 1.94 | 0.13 |
| probe7 | hl_front__classic | 9.69 | 9.55 |  |  |  |  |  |
| probe7 | hl_front__remastered | 5.75 | 5.26 |  | 0.34 | 0.26 |  | 0.03 |
| probe7 | hl_front__vnsm | 8.47 | 7.57 |  | 0.75 | 0.18 | 0.16 | 0.36 |
| probe7 | hl_front__vl4 | 8.50 | 7.16 |  | 1.19 | 0.20 | 0.33 | 0.62 |
| probe7 | hl_front__vl4f | 7.12 | 5.79 |  | 1.19 | 0.25 | 0.31 | 0.58 |
| probe7 | hl_front__vl4_dbg | 4.85 | 3.72 |  | 0.99 | 0.41 | 0.30 | 0.23 |
| probe7 | hl_side__remastered | 1.96 | 1.58 |  | 0.26 | 0.18 |  | 0.03 |
| probe7 | hl_side__vl4 | 4.40 | 3.58 |  | 0.71 | 0.19 | 0.12 | 0.35 |
| probe7 | hl_side__vl4f | 3.90 | 3.29 |  | 0.48 | 0.18 | 0.12 | 0.13 |
| probe7 | pinewood_morn__remastered | 10.46 | 9.88 | 1.21 | 0.43 | 0.19 | 0.15 | 0.04 |
| probe7 | pinewood_morn__vg25 | 12.45 | 10.44 | 1.37 | 1.87 | 0.20 | 1.49 | 0.14 |
| probe7 | pinewood_morn__vg25c | 15.28 | 13.42 | 1.93 | 1.72 | 0.18 | 1.35 | 0.14 |
| probe7 | pinewood_morn__vg25n | 15.44 | 13.48 | 1.43 | 1.81 | 0.18 | 1.45 | 0.13 |
| probe7 | forest_edge_golden__classic | 16.60 | 15.96 |  |  |  |  |  |
| probe7 | forest_edge_golden__remastered | 15.01 | 14.59 | 0.92 | 0.29 | 0.20 |  | 0.03 |
| probe7 | forest_edge_golden__vg25 | 12.37 | 10.63 | 0.62 | 1.61 | 0.23 | 1.19 | 0.13 |
| probe8 | town_eve__classic | 4.26 | 4.14 |  |  |  |  |  |
| probe8 | town_eve__remastered | 4.66 | 4.01 | 0.13 | 0.53 | 0.29 | 0.15 | 0.04 |
| probe8 | town_eve__vg25 | 3.82 | 2.67 | 0.43 | 1.03 | 0.34 | 0.51 | 0.12 |
| probe8 | town_eve__vg25e | 5.22 | 3.52 | 0.26 | 1.57 | 0.25 | 1.14 | 0.13 |
| probe8 | town_eve__vg4e | 8.01 | 6.42 | 0.25 | 1.46 | 0.19 | 1.08 | 0.13 |
| probe8 | pinewood_morn__classic | 14.68 | 14.55 |  |  |  |  |  |
| probe8 | pinewood_morn__remastered | 6.65 | 5.91 | 0.63 | 0.60 | 0.36 | 0.15 | 0.04 |
| probe8 | pinewood_morn__vg25e | 8.09 | 6.64 | 0.83 | 1.30 | 0.58 | 0.54 | 0.13 |
| probe8 | pinewood_morn__vg4e | 5.63 | 4.36 | 0.37 | 1.13 | 0.45 | 0.51 | 0.12 |
| probe8 | pine_eve__classic | 17.23 | 17.12 |  |  |  |  |  |
| probe8 | pine_eve__remastered | 29.56 | 29.04 | 0.38 | 0.40 | 0.16 | 0.15 | 0.04 |
| probe8 | pine_eve__vg25e | 20.82 | 19.20 | 0.21 | 1.50 | 0.19 | 1.13 | 0.13 |
| probe8 | pinewood_mist__classic | 5.64 | 5.50 |  |  |  |  |  |
| probe8 | pinewood_mist__remastered | 5.23 | 4.63 | 1.05 | 0.45 | 0.21 | 0.15 | 0.04 |
| probe8 | pinewood_mist__vg25e | 8.24 | 6.30 | 1.42 | 1.79 | 0.20 | 1.41 | 0.13 |
| probe8 | sunrise_ridge__classic | 40.58 | 40.46 |  |  |  |  |  |
| probe8 | sunrise_ridge__remastered | 33.62 | 32.86 | 0.17 | 0.63 | 0.40 | 0.15 | 0.04 |
| probe8 | sunrise_ridge__va | 41.56 | 39.47 | 0.10 | 1.97 | 0.34 | 1.45 | 0.13 |
| probe8 | sunrise_ridge__vg25e | 45.83 | 43.57 | 0.37 | 2.13 | 0.45 | 1.49 | 0.13 |
| probe8 | lowsun_fp__classic | 12.97 | 12.86 |  |  |  |  |  |
| probe8 | lowsun_fp__remastered | 12.95 | 11.34 | 0.32 | 1.50 | 0.30 | 0.22 | 0.93 |
| probe8 | lowsun_fp__vg25e | 14.02 | 11.85 | 0.62 | 2.05 | 0.22 | 1.65 | 0.13 |
| probe8 | town_night_fog__classic | 16.72 | 16.57 |  |  |  |  |  |
| probe8 | town_night_fog__remastered | 17.41 | 16.98 |  | 0.28 | 0.20 |  | 0.03 |
| probe8 | town_night_fog__vl4 | 18.27 | 17.65 |  | 0.46 | 0.25 | 0.05 | 0.12 |
| probe8 | town_night_fog__vl4_dbg | 18.08 | 17.51 |  | 0.42 | 0.21 | 0.05 | 0.11 |
| probe9 | lowsun_fp__classic | 1.93 | 1.82 |  |  |  |  |  |
| probe9 | lowsun_fp__remastered | 5.39 | 4.15 | 0.05 | 1.13 | 0.17 | 0.22 | 0.69 |
| probe9 | lowsun_fp__vg25e | 5.49 | 4.44 | 0.11 | 0.94 | 0.35 | 0.41 | 0.12 |
| probe9 | lowsun_fp__vg25em3 | 8.96 | 7.66 | 0.06 | 1.18 | 0.27 | 0.74 | 0.13 |
| probe9 | lowsun_fp__vg4em3 | 6.11 | 4.95 | 0.08 | 1.05 | 0.32 | 0.56 | 0.12 |
| probe9 | pine_eve__classic | 8.01 | 7.89 |  |  |  |  |  |
| probe9 | pine_eve__remastered | 21.09 | 20.57 | 0.21 | 0.40 | 0.16 | 0.15 | 0.04 |
| probe9 | pine_eve__vg25e | 25.11 | 23.28 | 0.21 | 1.72 | 0.17 | 1.36 | 0.14 |
| probe9 | pine_eve__vg25em3 | 11.38 | 9.96 | 0.45 | 1.30 | 0.34 | 0.78 | 0.13 |
| probe9 | pine_eve__vg4em3 | 8.85 | 7.52 | 0.29 | 1.21 | 0.35 | 0.69 | 0.13 |
| probe9 | pinewood_morn__classic | 5.17 | 5.03 |  |  |  |  |  |
| probe9 | pinewood_morn__remastered | 4.57 | 3.84 | 0.34 | 0.58 | 0.35 | 0.15 | 0.04 |
| probe9 | pinewood_morn__vg4e | 10.09 | 8.72 | 1.00 | 1.23 | 0.29 | 0.76 | 0.13 |
| probe9 | pinewood_morn__vg25em3 | 7.69 | 6.38 | 0.77 | 1.16 | 0.35 | 0.64 | 0.13 |
| probe9 | pinewood_morn__vg4em3 | 6.96 | 5.51 | 0.52 | 1.30 | 0.30 | 0.82 | 0.13 |
| probe9 | town_eve__classic | 3.19 | 3.07 |  |  |  |  |  |
| probe9 | town_eve__remastered | 4.11 | 3.44 | 0.25 | 0.55 | 0.31 | 0.15 | 0.04 |
| probe9 | town_eve__vg25e | 4.55 | 3.31 | 0.30 | 1.11 | 0.41 | 0.53 | 0.12 |
| probe9 | town_eve__vg25em3 | 6.75 | 5.26 | 0.26 | 1.37 | 0.21 | 0.97 | 0.13 |
| probe9 | town_eve__vg4em3 | 5.43 | 4.27 | 0.25 | 1.03 | 0.25 | 0.60 | 0.13 |
