# Lighting capture timings

GPU times are the smoothed `triGpuTimes` averages (EngineGL33 timestamp queries, ~90 frames on the view
before the shot); CPU is `triPerfStats` over the same frames. Classic has no post pass, so its post columns
are empty. Delta = Remastered minus Classic.

| view | mode | frame | world | shadow | post | ao | bloom | shafts | composite | CPU fps | lighting |
|---|---|---|---|---|---|---|---|---|---|---|---|
| dm1900on | classic | missing |  |  |  |  |  |  |  |  |  |
| dm1900on | remastered |  |  |  |  |  |  |  |  |  | OK |
| dm1900off | classic | missing |  |  |  |  |  |  |  |  |  |
| dm1900off | remastered |  |  |  |  |  |  |  |  |  | OK |
| dm1900cl | classic |  |  |  |  |  |  |  |  |  | OK |
| dm1900cl | remastered | missing |  |  |  |  |  |  |  |  |  |
| dm1910on | classic | missing |  |  |  |  |  |  |  |  |  |
| dm1910on | remastered |  |  |  |  |  |  |  |  |  | OK |
| dm1910off | classic | missing |  |  |  |  |  |  |  |  |  |
| dm1910off | remastered |  |  |  |  |  |  |  |  |  | OK |
| dm1910cl | classic |  |  |  |  |  |  |  |  |  | OK |
| dm1910cl | remastered | missing |  |  |  |  |  |  |  |  |  |
| dm1925on | classic | missing |  |  |  |  |  |  |  |  |  |
| dm1925on | remastered |  |  |  |  |  |  |  |  |  | OK |
| dm1925off | classic | missing |  |  |  |  |  |  |  |  |  |
| dm1925off | remastered |  |  |  |  |  |  |  |  |  | OK |
| dm1925cl | classic |  |  |  |  |  |  |  |  |  | OK |
| dm1925cl | remastered | missing |  |  |  |  |  |  |  |  |  |
| dm1940on | classic | missing |  |  |  |  |  |  |  |  |  |
| dm1940on | remastered |  |  |  |  |  |  |  |  |  | OK |
| dm1940off | classic | missing |  |  |  |  |  |  |  |  |  |
| dm1940off | remastered |  |  |  |  |  |  |  |  |  | OK |
| dm1940cl | classic |  |  |  |  |  |  |  |  |  | OK |
| dm1940cl | remastered | missing |  |  |  |  |  |  |  |  |  |
| dm1955on | classic | missing |  |  |  |  |  |  |  |  |  |
| dm1955on | remastered |  |  |  |  |  |  |  |  |  | OK |
| dm1955off | classic | missing |  |  |  |  |  |  |  |  |  |
| dm1955off | remastered |  |  |  |  |  |  |  |  |  | OK |
| dm1955cl | classic |  |  |  |  |  |  |  |  |  | OK |
| dm1955cl | remastered | missing |  |  |  |  |  |  |  |  |  |
| dm2015on | classic | missing |  |  |  |  |  |  |  |  |  |
| dm2015on | remastered |  |  |  |  |  |  |  |  |  | OK |
| dm2015off | classic | missing |  |  |  |  |  |  |  |  |  |
| dm2015off | remastered |  |  |  |  |  |  |  |  |  | OK |
| dm2015cl | classic |  |  |  |  |  |  |  |  |  | OK |
| dm2015cl | remastered | missing |  |  |  |  |  |  |  |  |  |
