# B11 mutation check

| mutation | build | failed cases | failed assertions | cases |
|---|---|---|---|---|
| kept cap 4x (the research's) | ok | 2 | 20 | Terrain detail: everything fades out by 300 m, where the frame gate ends |
| frame constants ignore the gate | ok | 1 | 1 | Terrain detail: its frame constants are zero when the switch or its gate is off |
| shader evaluates to 400 m | ok | 1 | 1 |  |
| no near fade | ok | 1 | 3 |  |
| anti-tiling on magnified detail | ok | 1 | 3 |  |
| luma sharpen not planned | ok | 1 | 4 | Post stages: with TAA off the luma sharpen runs the Sharpen stage alone |
| luma sharpen also on a TAA frame | ok | 0 | 0 |  |
| composite into the frame target under the luma sharpen | ok | 1 | 1 | Post stages: the luma sharpen reads the composite and writes the frame target |
| terrainDetail not landed | ok | 2 | 4 |  |
| no land flag in ApplyPassState | ok | 1 | 1 |  |
| sea flagged as land | ok | 1 | 1 |  |
| detail slot declared before the padding | ok | 1 | 10 | Remaster LitConstants GLSL block matches the LitConst slot layout |
| the stock albedo expression changed | ok | 1 | 1 |  |

Restored build: exit 0; tests: All tests passed (47736 assertions in 200 test cases)
