# xp-farshade: cheaper programs for far geometry (status: partial, not yet built or measured)

## What the audit number really was
The "remastered programs cost 1.75x Classic on far objects" figure was taken under the per-draw vertex upload stall that W1-9 removed. On the A2 anchor the valley pass1 is 2.42 ms and broken_high 3.50 ms (ground 2.87 and 5.53 ms). EXP-L16's FAR600 job (flat shading beyond 600 m, objects only) is the ceiling for any far-object shader trick: about -0.2 ms (valley) and -0.6 ms (broken_high) of pass1 at 1080p, three to four times that at render scale 2. The static cause S1 found still holds: the lit program is one huge shader (75 vec4 temps, a 28-entry local-memory table, PCSS loops, the dome and sun-disc paths), and its register footprint is set by its worst path even when a draw never takes it.

## What I built (behind POSEIDON_XP_FARSHADE, default off, unset = the tip's programs untouched)
Twins of the four remastered lit programs (objects, land, grass, sea), compiled from the same source with defines added after #version, linked with VSTransform only:
- level 1, far twin, used on a TL mesh whose whole bounding sphere (box radius x1.25 + 20 m, every instance of a run) lies past 300 m and past every shadow tier by eye depth: no shadow-map code (returns exactly 1 there), no relief or cavity (0 past 60/80/200 m), no terrain detail (0 past 300 m), vLitMode a constant (VSTransform writes 1), the wet tables as selects instead of local memory. Exact by construction.
- level 2, adds the lean twin on every other VSTransform lit draw (constant vLitMode, wet tables as selects). Exact by construction.
- level 3, adds the fog tier in the far twin: the fog colour (sky table lookups) only where the fog mix takes any of it. Rounding level (W1-5b measured 1 to 2 levels for the same gate).
Program swap: XpDecideMesh in BeginMeshTL picks the twin, XpSyncProgram after ApplyPipeline binds it and keeps the per-program uniforms (surfClass, decal mode) on the bound program; any non-mesh draw goes back to the base program. POSEIDON_XP_FARSHADE_DUMP=<dir> writes the driver binaries for offline comparison. Unit test: the 12 twin sources compile under glslang. Offline tool tools/nvcompile.ps1 + nvstats.py compiles base/lean/far/farfog through the driver (hidden window, no game) and prints vec4 temps, lmem and texture ops.

Files: EngineGL33.hpp, EngineGL33_LightingShaders.cpp, EngineGL33_LightingShadersSurface.cpp, EngineGL33_Mesh.cpp, EngineGL33_Queue.cpp, EngineGL33_Shaders.cpp, test_remaster_lighting.cpp. Formatted with clang-format 22.1.8; 21.1.8 pending.

## Status
The build, the unit gate and the driver compile were still waiting on the machine lock (INTEG-6's timed job) when this lane was told to report. Nothing queued, nothing measured, no stills. Job files are ready to generate (tools/mkjobs.py): same-exe stills L2 vs off and L3 vs off on valley_0515, broken_high, sunrise_ridge, noon_fields, town_night_rain; timed quick L2 and L3 vs off on valley_0515 and broken_high with --scale2.

## Next steps
1. Let build1 finish; unit gate (-Kind test); freeze dist sq-xp-farshade-r1-rwdi.
2. Read nvbin stats: if the far and lean twins do not drop temps and lmem clearly, stop here.
3. Dry-run and queue the stills jobs first, then the two quick jobs.
Queue note for the orchestrator: 15 stale EXP-A quick jobs from 2026-10-31 sit at prio 60 and will run before every XP lane's prio 60 to 69 job.

## Expectation and risks
Ceiling about 0.2 to 0.6 ms GPU per long view, likely less; worth it only as a free exact default. Risks: an untested program swap path (uniform state on twins, decal batches, night-eye reselection), the shape box not bounding land-fitted or animated vertices (covered by the margin), driver rounding making the twins differ by 1 level. Nothing temporal changes.
