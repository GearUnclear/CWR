# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| overcast_broken | classic | 22.79 | 22.68 |  |  |  |  |  |  | 31.80 | OK |
| overcast_broken | remastered | 18.56 | 17.86 | 0.10 | 0.60 | 0.52 | 0.05 |  | 0.03 | 37.50 | OK |
| overcast_broken | delta | -4.23 | -4.82 |  |  |  |  |  |  |  |  |
| sinai_desert_noon | classic | 3.11 | 3.00 |  |  |  |  |  |  | 142.40 | OK |
| sinai_desert_noon | remastered | 14.13 | 13.09 | 0.11 | 0.92 | 0.85 | 0.05 |  | 0.03 | 44.70 | OK |
| sinai_desert_noon | delta | +11.02 | +10.09 |  |  |  |  |  |  |  |  |
