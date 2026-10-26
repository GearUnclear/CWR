Local exposure cost = composite section (lum pyramid + grid splat/blur + adaptation + composite) minus the shipped composite, same run and view. 960x640 window, 1.5x render scale (1440x960 frame), 4x MSAA. Every run shared the GPU with up to three other research games, so the minimum is the best estimate of the real cost.

| view | shots | composite delta min ms | median ms | max ms |
|---|---|---|---|---|
| lowsun_fp | 27 | +0.31 | +0.92 | +1.63 |
| sunrise_ridge | 27 | +0.31 | +0.88 | +1.68 |
| coastline_sunset | 27 | +0.03 | +0.66 | +1.92 |
| town_night | 16 | +0.64 | +1.12 | +1.68 |
| noon_fields | 11 | +0.25 | +1.09 | +1.24 |
| forest_edge_golden | 16 | +0.35 | +0.74 | +1.07 |
| tank_gunner_interior | 16 | +0.25 | +0.27 | +0.27 |
| ALL | 140 | +0.03 | +0.80 | +1.92 |
