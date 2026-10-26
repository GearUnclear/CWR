# Gate item 5 (off equals baseline), A2

Cells: max channel difference / pixels over 2 levels (1920x1080; bands sky 0-422, horizon 422-591,
ground 591-1080). gate = A2 `__off` (`remastered,<keys>=0`; no switch is read yet, so this is the
plain remastered look) against the A1 build's `__on` (`remastered`), both this session
(20261018-024320-ab-A2, 20261018-022516-ab-A1build) with the fixed per-view clock and held clouds.
A2 on/A1 on = the same across builds for the `__on` shots. A1 noise and A2 noise = off against on
in one game of that build. A1 boots = the A1 build's `__on` against A1's own run of the same code
(tmp/integration/A1/20261018-015544-ab-A1), an earlier boot.

## sky

| view | gate | A2 on / A1 on | A1 noise | A2 noise | A1 boots |
|---|---|---|---|---|---|
| airbeams_pinewood_morn | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| airhaze_valley_0515 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| atmosphere_coastline_sunset | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| clouds_coast_oc_up | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| combat_ffh | 109 / 212,820 | 107 / 66,295 | 98 / 189,875 | 103 / 188,811 | 100 / 62,709 |
| hfrt_hill_1730 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| lighttiles_town_night | 202 / 228,075 | 181 / 228,903 | 169 / 310,664 | 154 / 135,551 | 168 / 309,596 |
| materials_town_night_rain | 190 / 98,457 | 191 / 175,375 | 198 / 144,373 | 154 / 166,222 | 150 / 149,165 |
| night_road_flat_drive_moonless | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| perf_sunrise_ridge | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| shade_lowsun_fp | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| shadows_woods_pine2_evening | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| ssr_bay_offshore | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| temporal_forest_edge_golden | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| texsharp_forest_edge | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| water_coastline_sunset | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |

## horizon

| view | gate | A2 on / A1 on | A1 noise | A2 noise | A1 boots |
|---|---|---|---|---|---|
| airbeams_pinewood_morn | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| airhaze_valley_0515 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 26 / 91 |
| atmosphere_coastline_sunset | 8 / 15 | 5 / 6 | 0 / 0 | 8 / 8 | 5 / 9 |
| clouds_coast_oc_up | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| combat_ffh | 130 / 23,870 | 134 / 19,550 | 94 / 23,326 | 113 / 14,920 | 106 / 17,793 |
| hfrt_hill_1730 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| lighttiles_town_night | 147 / 45,387 | 176 / 45,182 | 187 / 57,146 | 110 / 41,111 | 197 / 64,462 |
| materials_town_night_rain | 212 / 67,886 | 206 / 119,964 | 217 / 119,544 | 146 / 101,074 | 181 / 85,165 |
| night_road_flat_drive_moonless | 124 / 22 | 119 / 23 | 37 / 10 | 38 / 15 | 73 / 13 |
| perf_sunrise_ridge | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| shade_lowsun_fp | 14 / 9 | 104 / 1,005 | 11 / 7 | 104 / 1,005 | 104 / 1,009 |
| shadows_woods_pine2_evening | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| ssr_bay_offshore | 183 / 8,874 | 44 / 905 | 183 / 8,879 | 185 / 8,865 | 120 / 4,483 |
| temporal_forest_edge_golden | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| texsharp_forest_edge | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| water_coastline_sunset | 34 / 63 | 0 / 0 | 34 / 63 | 34 / 63 | 44 / 108 |

## ground

| view | gate | A2 on / A1 on | A1 noise | A2 noise | A1 boots |
|---|---|---|---|---|---|
| airbeams_pinewood_morn | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| airhaze_valley_0515 | 1 / 0 | 0 / 0 | 1 / 0 | 1 / 0 | 59 / 60 |
| atmosphere_coastline_sunset | 23 / 53 | 22 / 31 | 0 / 0 | 23 / 24 | 21 / 33 |
| clouds_coast_oc_up | 12 / 896 | 11 / 66 | 12 / 896 | 12 / 1,069 | 12 / 106 |
| combat_ffh | 58 / 1,131 | 58 / 393 | 59 / 1,384 | 20 / 610 | 57 / 284 |
| hfrt_hill_1730 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| lighttiles_town_night | 99 / 56,146 | 155 / 32,018 | 161 / 50,031 | 102 / 47,070 | 209 / 30,295 |
| materials_town_night_rain | 209 / 45,296 | 211 / 176,019 | 183 / 139,436 | 133 / 145,279 | 120 / 135,018 |
| night_road_flat_drive_moonless | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| perf_sunrise_ridge | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| shade_lowsun_fp | 0 / 0 | 1 / 0 | 0 / 0 | 1 / 0 | 1 / 0 |
| shadows_woods_pine2_evening | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| ssr_bay_offshore | 10 / 24,563 | 4 / 386 | 9 / 19,672 | 8 / 7,480 | 6 / 2,187 |
| temporal_forest_edge_golden | 174 / 543 | 174 / 543 | 174 / 543 | 11 / 1 | 174 / 542 |
| texsharp_forest_edge | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| water_coastline_sunset | 3 / 51 | 0 / 0 | 3 / 51 | 3 / 51 | 3 / 100 |

## Pixel-identical views

Views where A1 boot-to-boot is identical in every band (the repeatable views): 5 (airbeams_pinewood_morn, hfrt_hill_1730, perf_sunrise_ridge, shadows_woods_pine2_evening, texsharp_forest_edge).

Views where A2 (off and on) is identical to the A1 build in every band: 5 (airbeams_pinewood_morn, hfrt_hill_1730, perf_sunrise_ridge, shadows_woods_pine2_evening, texsharp_forest_edge).

Repeatable views that are not identical across builds: none.
