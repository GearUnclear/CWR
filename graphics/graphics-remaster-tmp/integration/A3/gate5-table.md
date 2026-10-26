# Gate item 5 (off equals baseline), A3

Cells: max channel difference / pixels over 2 levels (1920x1080; bands sky 0-422, horizon 422-591,
ground 591-1080). gate = A3 `__off` (`remastered,<keys>=0`; no switch is read yet, so this is the
plain remastered look) against the A2 build's `__on` (`remastered`), both this session
(20261018-040749-ab-A3, 20261018-040811-ab-A2build) with the fixed per-view clock and held clouds.
A3 on/A2 on = the same across builds for the `__on` shots. A2 noise and A3 noise = off against on
in one game of that build. A2 boots = the A2 build's `__on` against A2's own run of the same code
(tmp/integration/A2/20261018-024320-ab-A2), an earlier boot.

## sky

| view | gate | A3 on / A2 on | A2 noise | A3 noise | A2 boots |
|---|---|---|---|---|---|
| airbeams_pinewood_morn | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| airhaze_valley_0515 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| atmosphere_coastline_sunset | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| clouds_coast_oc_up | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| combat_ffh | 129 / 168,201 | 129 / 70,892 | 102 / 167,442 | 116 / 153,083 | 98 / 48,597 |
| hfrt_hill_1730 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| lighttiles_town_night | 186 / 231,665 | 195 / 254,258 | 128 / 85,266 | 195 / 270,878 | 159 / 90,772 |
| materials_town_night_rain | 194 / 178,996 | 191 / 81,993 | 27 / 54,448 | 170 / 179,821 | 190 / 124,741 |
| night_road_flat_drive_moonless | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| perf_sunrise_ridge | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| shade_lowsun_fp | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| shadows_woods_pine2_evening | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| ssr_bay_offshore | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| temporal_forest_edge_golden | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| texsharp_forest_edge | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| water_coastline_sunset | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |

## horizon

| view | gate | A3 on / A2 on | A2 noise | A3 noise | A2 boots |
|---|---|---|---|---|---|
| airbeams_pinewood_morn | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| airhaze_valley_0515 | 38 / 265 | 39 / 279 | 27 / 377 | 26 / 58 | 26 / 344 |
| atmosphere_coastline_sunset | 5 / 9 | 6 / 20 | 5 / 6 | 6 / 30 | 5 / 6 |
| clouds_coast_oc_up | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| combat_ffh | 135 / 20,482 | 132 / 26,818 | 105 / 19,975 | 92 / 16,540 | 98 / 15,981 |
| hfrt_hill_1730 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| lighttiles_town_night | 200 / 61,528 | 216 / 63,583 | 151 / 41,318 | 131 / 49,383 | 202 / 47,899 |
| materials_town_night_rain | 199 / 126,402 | 204 / 64,826 | 101 / 49,195 | 199 / 125,141 | 205 / 92,543 |
| night_road_flat_drive_moonless | 91 / 20 | 91 / 21 | 80 / 15 | 50 / 16 | 119 / 20 |
| perf_sunrise_ridge | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| shade_lowsun_fp | 104 / 1,007 | 24 / 7 | 104 / 1,006 | 104 / 1,008 | 24 / 14 |
| shadows_woods_pine2_evening | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| ssr_bay_offshore | 188 / 61,336 | 186 / 62,071 | 87 / 2,755 | 183 / 8,872 | 186 / 62,056 |
| temporal_forest_edge_golden | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| texsharp_forest_edge | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| water_coastline_sunset | 26 / 6 | 0 / 0 | 26 / 39 | 26 / 6 | 34 / 63 |

## ground

| view | gate | A3 on / A2 on | A2 noise | A3 noise | A2 boots |
|---|---|---|---|---|---|
| airbeams_pinewood_morn | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| airhaze_valley_0515 | 54 / 171 | 47 / 167 | 69 / 164 | 54 / 35 | 68 / 357 |
| atmosphere_coastline_sunset | 21 / 33 | 23 / 89 | 22 / 31 | 23 / 122 | 22 / 31 |
| clouds_coast_oc_up | 15 / 930 | 12 / 78 | 14 / 802 | 16 / 1,237 | 12 / 144 |
| combat_ffh | 55 / 593 | 57 / 1,500 | 20 / 857 | 57 / 1,059 | 57 / 684 |
| hfrt_hill_1730 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| lighttiles_town_night | 205 / 53,157 | 206 / 53,516 | 132 / 20,491 | 125 / 21,646 | 182 / 43,934 |
| materials_town_night_rain | 218 / 207,770 | 203 / 64,815 | 148 / 119,986 | 158 / 192,647 | 210 / 140,104 |
| night_road_flat_drive_moonless | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| perf_sunrise_ridge | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| shade_lowsun_fp | 1 / 0 | 0 / 0 | 1 / 0 | 1 / 0 | 0 / 0 |
| shadows_woods_pine2_evening | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| ssr_bay_offshore | 42 / 735,248 | 43 / 739,381 | 27 / 333,601 | 6 / 2,172 | 43 / 739,381 |
| temporal_forest_edge_golden | 174 / 543 | 174 / 542 | 174 / 543 | 2 / 0 | 174 / 543 |
| texsharp_forest_edge | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 | 0 / 0 |
| water_coastline_sunset | 3 / 7 | 0 / 0 | 3 / 33 | 3 / 7 | 3 / 51 |

## Pixel-identical views

Views where A2 boot-to-boot is identical in every band (the repeatable views): 5 (airbeams_pinewood_morn, hfrt_hill_1730, perf_sunrise_ridge, shadows_woods_pine2_evening, texsharp_forest_edge).

Views where A3 (off and on) is identical to the A2 build in every band: 5 (airbeams_pinewood_morn, hfrt_hill_1730, perf_sunrise_ridge, shadows_woods_pine2_evening, texsharp_forest_edge).

Repeatable views that are not identical across builds: none.
