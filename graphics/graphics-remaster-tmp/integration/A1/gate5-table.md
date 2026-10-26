# Gate item 5 (off equals baseline), A1

Cells: max channel difference / pixels over 2 levels (1920x1080; bands sky 0-422, horizon 422-591,
ground 591-1080). gate = A1 `__off` (`remastered,<keys>=0`) against the A0 build's `__on`
(`remastered`), both with the fixed per-view sim clock; A0 noise = the A0 build's two shots of the
view in one game (identical settings); A1 noise = A1 off against A1 on in one game (20261018-015544); boots = A1 boot 2
against A1 boot 1 (20261018-012542), `__on`, both with the clouds held and the fixed clock.

## sky

| view | gate | A0 noise | A1 noise | A1 boots |
|---|---|---|---|---|
| airbeams_pinewood_morn | 16 / 286,353 | 8 / 5,355 | 0 / 0 | 0 / 0 |
| airhaze_valley_0515 | 17 / 279,482 | 10 / 44,971 | 0 / 0 | 0 / 0 |
| atmosphere_coastline_sunset | 13 / 166,719 | 5 / 3,112 | 0 / 0 | 0 / 0 |
| clouds_coast_oc_up | 87 / 666,183 | 28 / 195,071 | 0 / 0 | 0 / 0 |
| combat_ffh | 111 / 187,030 | 107 / 149,690 | 116 / 172,083 | 100 / 64,251 |
| hfrt_hill_1730 | 9 / 101,836 | 4 / 651 | 0 / 0 | 0 / 0 |
| lighttiles_town_night | 178 / 217,587 | 150 / 193,689 | 175 / 178,233 | 173 / 219,516 |
| materials_town_night_rain | 208 / 370,849 | 190 / 179,869 | 158 / 185,592 | 168 / 128,707 |
| night_road_flat_drive_moonless | 7 / 16,976 | 3 / 368 | 0 / 0 | 0 / 0 |
| perf_sunrise_ridge | 15 / 237,309 | 10 / 70,747 | 0 / 0 | 0 / 0 |
| shade_lowsun_fp | 11 / 161,014 | 5 / 3,814 | 0 / 0 | 0 / 0 |
| shadows_woods_pine2_evening | 11 / 42,463 | 3 / 26 | 0 / 0 | 0 / 0 |
| ssr_bay_offshore | 14 / 224,156 | 3 / 75 | 0 / 0 | 0 / 0 |
| temporal_forest_edge_golden | 12 / 179,054 | 4 / 2,283 | 0 / 0 | 0 / 0 |
| texsharp_forest_edge | 14 / 230,719 | 4 / 1,882 | 0 / 0 | 0 / 0 |
| water_coastline_sunset | 9 / 125,289 | 4 / 130 | 0 / 0 | 0 / 0 |

## horizon

| view | gate | A0 noise | A1 noise | A1 boots |
|---|---|---|---|---|
| airbeams_pinewood_morn | 4 / 805 | 3 / 1 | 0 / 0 | 0 / 0 |
| airhaze_valley_0515 | 27 / 378 | 14 / 56 | 0 / 0 | 39 / 367 |
| atmosphere_coastline_sunset | 5 / 9 | 0 / 0 | 5 / 9 | 5 / 15 |
| clouds_coast_oc_up | 69 / 198,796 | 37 / 52,562 | 0 / 0 | 0 / 0 |
| combat_ffh | 112 / 18,990 | 99 / 17,074 | 94 / 16,369 | 97 / 16,945 |
| hfrt_hill_1730 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| lighttiles_town_night | 203 / 54,196 | 143 / 46,799 | 131 / 41,793 | 204 / 66,105 |
| materials_town_night_rain | 219 / 98,764 | 178 / 68,483 | 132 / 87,195 | 210 / 62,659 |
| night_road_flat_drive_moonless | 86 / 22 | 120 / 23 | 78 / 17 | 49 / 13 |
| perf_sunrise_ridge | 2 / 0 | 2 / 0 | 0 / 0 | 0 / 0 |
| shade_lowsun_fp | 12 / 6 | 23 / 4 | 104 / 1,009 | 104 / 1,009 |
| shadows_woods_pine2_evening | 2 / 0 | 2 / 0 | 0 / 0 | 0 / 0 |
| ssr_bay_offshore | 185 / 8,745 | 158 / 8,711 | 185 / 8,740 | 169 / 6,429 |
| temporal_forest_edge_golden | 3 / 321 | 1 / 0 | 0 / 0 | 0 / 0 |
| texsharp_forest_edge | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| water_coastline_sunset | 21 / 33 | 21 / 33 | 44 / 52 | 33 / 22 |

## ground

| view | gate | A0 noise | A1 noise | A1 boots |
|---|---|---|---|---|
| airbeams_pinewood_morn | 2 / 0 | 2 / 0 | 0 / 0 | 0 / 0 |
| airhaze_valley_0515 | 68 / 222 | 25 / 15 | 0 / 0 | 68 / 164 |
| atmosphere_coastline_sunset | 21 / 33 | 0 / 0 | 21 / 33 | 22 / 62 |
| clouds_coast_oc_up | 83 / 131,139 | 15 / 8,487 | 12 / 506 | 0 / 0 |
| combat_ffh | 57 / 681 | 57 / 1,100 | 57 / 713 | 3 / 5 |
| hfrt_hill_1730 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| lighttiles_town_night | 173 / 55,916 | 106 / 54,842 | 106 / 19,644 | 208 / 63,209 |
| materials_town_night_rain | 209 / 97,359 | 82 / 115,858 | 97 / 133,130 | 218 / 40,349 |
| night_road_flat_drive_moonless | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| perf_sunrise_ridge | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| shade_lowsun_fp | 2 / 0 | 1 / 0 | 1 / 0 | 1 / 0 |
| shadows_woods_pine2_evening | 2 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| ssr_bay_offshore | 15 / 101,315 | 25 / 287,275 | 13 / 43,600 | 4 / 295 |
| temporal_forest_edge_golden | 3 / 2 | 16 / 5 | 12 / 1 | 12 / 1 |
| texsharp_forest_edge | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| water_coastline_sunset | 3 / 34 | 3 / 34 | 3 / 48 | 3 / 27 |
