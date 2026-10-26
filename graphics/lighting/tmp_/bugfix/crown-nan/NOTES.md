# crown-nan: NaN lighting normals on tree crowns in the remastered shader

Scratch worktree `D:\Arma_CWA_worktrees\fix-crown-nan` (detached at 3c53141, the tip of
`claude/youthful-lovelace-8imbdh`), build `build/fix-crown-nan-rwdi`, game
`dist/fix-crown-nan-rwdi`. Nothing committed, no branch. The change is `fix.patch`
(`git diff claude/youthful-lovelace-8imbdh`, 5 files, 200 lines).

## Root cause (measured in game, not inferred)

The wildcard track named two suspects: leaf cards with zero mesh normals, and degenerate crown
ellipsoids giving inf/NaN `vCrown`. I separated them with a diagnostic shader override on the
branch-tip exe (`ovr_diag/`, run `run_diag_before/`), debug view 3 repainted per foliage fragment:
red = the vertex carries a zero mesh normal, yellow = the crown constants are not finite,
green = `vCrown` is NaN/inf with finite constants, blue = `vNormalW` is NaN with a non-zero mesh normal.

Result on forest_edge_golden, woods_mixed_noon, woods_pine2_evening and town_day: **only red**.
No yellow, green or blue pixel in any of the four views. The crown ellipsoid is not the cause in
the shipped game; zero mesh normals are.

The chain, with line numbers at the branch tip (`arma_CWR/engine/PoseidonGL33/EngineGL33_LightingShaders.cpp`):

1. `:139` `vec3 worldNormal = normalize(mat3(worldArr[gl_InstanceID]) * normal);` Many CWA tree
   models (the Malden town birch, several pines and broadleaves) carry leaf cards whose mesh normal
   is (0,0,0). `normalize` of a zero vector is undefined in GLSL; on this NVIDIA driver it is NaN.
2. The NaN is interpolated, so a single zero-normal vertex poisons its whole triangle. A second
   probe on the branch-tip exe (`ovr_nanprobe_before/`, only the fragment shader overridden, the
   shipped vertex shader untouched) confirmed that every flagged crown fragment has a NaN
   `vNormalW` and a NaN `nLight`, and that the NaN area is larger than the zero-normal area
   (woods_pine2_evening debug shots: 2,767 pure-red zero-normal pixels in `run_diag_before`,
   62,115 pure-blue NaN pixels in `run_nanprobe_before`).
3. `:518` normalises again, but `max(dot(NaN, NaN), 1e-12)` only guards a zero length, not NaN.
4. `:536-537` `card` is NaN, so `nLight = normalize(card * 0.3 + cn * 0.7)` is NaN.
5. From there everything the crown gets depends on how the driver treats NaN in `clamp`/`max`,
   which GLSL leaves undefined: `:556` sun term `clamp(NaN)`, `:560` `skyIrradiance(NaN)` (its
   `clamp(n.y)` picks one end of the sky hemisphere), `:561` moon term, and `:547` the shadow lookup,
   whose NaN uv fails every bounds test (`:368`) so `:380` returns 1: those leaves are never
   shadowed. In the debug normal view the NaN crowns render black (`town_day__before_after.png`,
   bottom left).

`CrownEllipsoid` (`arma_CWR/engine/Poseidon/Graphics/Remaster/LightRig.cpp:140-149`) is still
fragile: a NaN or infinite bound, or an extent that overflows float, gives NaN/inf constants (the
new unit test shows 21 failing assertions on the old code). That never happened in these views,
so it is hardening, not the cause.

## The fix

GLSL (`EngineGL33_LightingShaders.cpp`):

- VSTransform: the world normal is normalised with the guarded form
  `n *= inversesqrt(max(dot(n, n), 1e-12))`, so a zero mesh normal stays zero instead of NaN.
  This also makes the classic N.L in `vColor` (material alpha) a defined 0 for those vertices.
- `shadeRemastered`: the crown normal `cn` is computed once; a foliage fragment whose own normal
  is zero (`dot(n, n) < 0.25`; after the guarded normalisation `n` is either unit or near zero)
  takes `cn` as its normal, for the sun term, the sky, the moon, local lights and the shadow
  lookup's normal offset. The crown blend `card * 0.3 + cn * 0.7` uses the same guarded
  normalisation, so a fragment with neither normal nor crown frame gets a zero normal (horizon
  sky, wrapped sun term 0.26) instead of NaN. Healthy leaves compute exactly what they did before.

C++ (`LightRig.cpp`, `CrownEllipsoid`): an axis whose lower bound or extent is not finite falls
back to a 5 cm half extent centred on the origin; the other axes keep their fit. Header comment
and the Remaster README's Foliage row updated.

Why it is correct: a leaf card authored without a normal has no orientation of its own, and the
crown ellipsoid is exactly the normal the remastered model already uses for 70% of every leaf's
lighting normal. Taking it outright is the natural limit of the existing blend, removes all
driver-dependent NaN behaviour, and lets those leaves receive sun shadows like every other leaf.
Cost: one compare and a select per foliage fragment.

## Tests

Added to `arma_CWR/tests/unit/engine/Poseidon/Graphics/test_remaster_lighting.cpp`:

- `Remaster crown ellipsoid stays finite for degenerate boxes` [Graphics][remaster]: NaN min, NaN
  max, infinite max, infinite both ways, overflowing extent, inverted box, point box; centre and
  inverse half extents must be finite and in (0, 20]; a broken axis must not disturb the others.
- `Remaster lighting normals survive leaves without a mesh normal` [Graphics][Shaders][remaster]:
  a source contract (no GPU in the unit tests): no remastered vertex shader calls `normalize(`,
  `shadeRemastered` calls no `normalize(`, and a zero-normal foliage fragment takes the crown
  normal.

Before the fix (new tests built against the unchanged source, `tests_before.log`): 2 of 2 test
cases failed, 21 of 96 assertions (NaN/inf centres and inverse extents for NaN, infinite and
overflowing boxes; `normalize(` in `s_vsTransformRemasterGLSL` and in `shadeRemastered`).
After (`tests_after.log`): all 96 assertions in 2 test cases pass.

Tag runs after the fix (`tests_tags_after.log`), PoseidonTests.exe from the D: drive:
[remaster] 303 assertions / 15 cases, [gpu], [GL33] 104 cases, [Shaders], [DebugGroup],
[ShutdownOrder], [UBORestoreGuard], [StateOwnership], [StateCacheAudit], [BufferUsageAudit],
[IboBindAudit], [Conventions]: all pass. [graphics]: 353 of 354 pass; the one failure is the hidden
`DDSConverter: generate DDS fixture from synthetic_dxt5 DXT5` ([.fixture]), which expects 64 px but
its fixture `tests/fixtures/paa/synthetic_dxt5.paa` is 32x32 (header bytes `2000 2000`); untouched
by this diff. Full suite (`tests_full_after.log`): 3056 passed, 6 failed, 4 skipped; the 6 are the
DDS fixture case and five localisation tests that read `arma_CWR\packages\Remaster\...`, a junction
this scratch worktree does not have.

clang-format 22.1.8 `--dry-run --Werror` exits 0 on all four touched C++ files.

## In-game evidence

All runs through `run-captures.ps1`, hidden desktop, junction `RemasterUD-fix-crown-nan`, 960x640,
150% render scale, 4x MSAA, presented frame. Research test `tests/crown_nan_abel.test.sqf`
(generator `scripts/gen_crown_test.py`): per view a Remastered frame, the debug normal view
(debug 3), and a "late" Remastered frame 30 frames after it. BEFORE = `dist\lighting-rwdi`
(branch tip, not rebuilt), AFTER = `dist/fix-crown-nan-rwdi`.

| run | exe | override | what it shows |
|---|---|---|---|
| `run_diag_before` | tip | `ovr_diag` | cause classification (only zero mesh normals) |
| `run_nanprobe_before` | tip | `ovr_nanprobe_before` (FS only) | shipped VS gives NaN `vNormalW` and NaN `nLight` on those crowns |
| `run_check_after` | fix | `ovr_check_after` | 0 NaN pixels left; repaired leaves shaded by the crown's sun term |
| `run_matprobe`, `run_termprobe` | fix | probes | on the repaired leaves kd, ka and forced diffuse are all non-zero (kd and forced about equal), the sun is on, emissive is about 0, and the direct share of their light reads 0.55 to 0.78 (display-encoded, so approximate) |
| `run_before`, `run_after` | tip / fix | none | first image set (first and debug shots only) |
| `run2_nanpaint_before` | tip | `ovr_nanpaint_before` | NaN fragments painted magenta in the normal frame: the NaN mask |
| `run2_before`, `run2_after` | tip / fix | none | the evidence below |

Numbers (`measure.md`, display luma 0-255, mask = magenta pixels of `run2_nanpaint_before` in the
same shot):

| view | shot | NaN-normal px (shipped) | their luma before / after | mean abs change on them | px changed > 4 levels | mean abs change elsewhere |
|---|---|---|---|---|---|---|
| forest_edge_golden | first | 0 | - | - | 0 | 0.012 |
| forest_edge_golden | late | 38,552 (6.3%) | 42.3 / 44.0 | 2.36 | 12,150 | 0.020 |
| woods_mixed_noon | late | 20,256 (3.3%) | 52.8 / 53.2 | 1.78 | 10,355 | 0.004 |
| woods_pine2_evening | late | 68,868 (11.2%) | 49.6 / 49.8 | 2.39 | 23,236 | 0.008 |
| town_day | late | 70,435 (11.5%) | 52.5 / 51.2 | 2.13 | 40,868 | 0.035 |

What the images show:

- `<view>__before_after.png` (late frame on top, debug normal view below): BEFORE the NaN crowns
  are black in the normal view; AFTER they show the smooth ellipsoid gradient of a volume
  (town_day birch, the pines). Everything that was not NaN is unchanged (elsewhere 0.004-0.035
  levels mean, which is run-to-run noise such as the animated soldier).
- `<view>__diff_x8.png` (AFTER minus BEFORE luma x8, red brighter, blue darker): the repaired
  crowns now have a lit sun side and a darker far side: forest_edge_golden's conifers brighten on
  their sunward side (left in the frame, the sun is behind the camera's left shoulder), the town_day
  birch brightens on its sun side (left) and darkens on the far side (crown mean -1.3 levels), the
  evening pines brighten or darken by side. The red/blue blob on
  the soldier's head is pose noise between runs, not foliage.
- `<view>__crop.png`: BEFORE | AFTER | the shipped NaN mask, cropped to the affected crowns.
- `<view>__diag.png`: BEFORE classification (red = zero mesh normal) | AFTER check (red brightness
  = the repaired leaves' sun term, green would be any NaN left: none).

On screen the change is real but modest: 2 levels mean on the affected crowns, up to tens of levels
on individual leaves. Half of a foliage material's sun is the normal-independent forced diffuse and
the wrapped terminator keeps the far side partly lit, so the crown's volume reads as a gradient,
not as a hard lit/unlit split. No haze, no hue shift, no darkening of the frame (late-frame means
move by -0.15 to +0.11 levels).

forest_edge_golden's first frame has **no** NaN fragments at all: the conifers there are first
drawn with a lower LOD whose leaves have valid normals (visibly different, blobbier trees), and by
the late frame they have switched to the detailed LOD that carries the zero-normal leaf cards (the
first and late BEFORE frames differ by 20 levels on those crowns). That is why the first image set
(`run_before`/`run_after`) showed no change at forest_edge_golden; the evidence uses the late frame.
I did not establish what triggers the LOD switch (time, frame-rate driven LOD or the debug toggle).

GPU timings in `run2_before/timings.md` and `run2_after/timings.md` are single samples on a GPU
shared with other agents (forest_edge_golden 3.08 vs 3.29 ms, woods_mixed_noon 4.19 vs 3.51 ms):
noise, not a cost measurement. The shader change is a few ALU operations.

## Risks

- Leaves that were NaN are now shadowed. In the shipped exe their shadow lookup always returned
  "lit". With object shadows on (they are off at start-up, a separate bug: the shadow pass costs
  0.03 ms in these runs), those crowns will now take tree shadows and self-shadow like their
  healthy neighbours (`sunShadow` floor 0.45). That is the intended look, but it is new on them.
- The shadow lookup for a repaired leaf offsets along the crown normal, not a card normal; acne
  risk is the same as for healthy leaves (same offset scale), not tested with object shadows on.
- Non-foliage geometry with zero normals (none seen in these views) now gets a zero normal:
  horizon sky, no direct sun, instead of driver-defined NaN behaviour.
- The contract test is a source check; it pins the guarded forms, not the pixels. The pixels are
  covered only by the in-game runs above.
- Classic shaders are untouched (`EngineGL33_Shaders.cpp:124` still has the same `normalize`),
  since the remastered lighting replaces Classic.

## Files

- `fix.patch`: the change. `tests_before.log`, `tests_after.log`, `tests_tags_after.log`,
  `tests_graphics_after.log`, `tests_full_after.log`: unit-test output.
- `measure.md`, `*__before_after.png`, `*__crop.png`, `*__diff_x8.png`, `*__diag.png`.
- `ovr_*`: the research shader overrides; `scripts/`: their generators and the image scripts
  (`compare_crown2.py` makes the final sheets and `measure.md` from the `run2_*` runs;
  `compare_crown.py` and `diff_crown.py` are the first-pass versions, of which only the
  `*__diag.png` sheets survive; the `run_*` runs used the same test without the late shot);
  `tests/`: the research capture test (also in the scratch worktree under
  `arma_CWR/tests/rendering/research/crown-nan/`, git-excluded).
- `run*/`: every capture run (tri tree, logs, timings). `build*.log`: build logs.
