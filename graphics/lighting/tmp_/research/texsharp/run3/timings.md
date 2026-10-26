# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| noon_fields | classic | 2.90 | 2.67 |  |  |  |  |  |  | 246.90 | OK |
| noon_fields | remastered | 29.46 | 29.11 | 0.18 | 0.25 | 0.18 | 0.05 |  | 0.03 | 40.60 | OK |
| noon_fields | delta | +26.56 | +26.44 |  |  |  |  |  |  |  |  |
| overcast_broken | classic | 33.31 | 33.21 |  |  |  |  |  |  | 27.10 | OK |
| overcast_broken | remastered | 38.26 | 37.68 | 0.03 | 0.48 | 0.41 | 0.05 |  | 0.03 | 21.90 | OK |
| overcast_broken | delta | +4.95 | +4.47 |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 3.40 | 3.29 |  |  |  |  |  |  | 100.40 | OK |
| sinai_desert_noon | remastered | 16.04 | 14.90 | 0.04 | 1.01 | 0.94 | 0.05 |  | 0.03 | 48.60 | OK |
| sinai_desert_noon | delta | +12.64 | +11.61 |  |  |  |  |  |  |  |  |
