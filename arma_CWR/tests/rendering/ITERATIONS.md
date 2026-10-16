# GL33 lighting iteration record

Branch: `graphics/lighting-improvements`. PR #210 remains a **draft**.
This record distinguishes a semantic correction from a demonstrated visual win.
No game assets, captured screenshots, or executable binaries belong in this tree.

## Alphabetical milestone progress

Current numbered stage: **S (19 of 703)**. The requested target is **AAA (703)**
using A–Z, AA–AZ, and onward. This is a longer-running iteration milestone,
separate from the 15 screenshot deliveries over 10 hours; neither is complete.
F2 is a subvariant of F. Diagnostics, time-of-day repeats, camera angles, and
scene surveys are validation subruns, not additional numbered iterations.

### Historical checkpoint notes (superseded by the later entries)

E is published on `graphics/lighting-improvements` at
[`ab100b1`](https://github.com/GearUnclear/CWR/commit/ab100b1ac4c6a9e179ef128fd859e9ec57180b8f)
(remote connector and CI SHA verified; not fetched into the local shallow
checkout). All CI jobs for run `38899289262` were verified green at 12:38 UTC
on 2026-10-16. F2 and G remain local. Northeast barn captures are complete for
both. G is useful calibration, **not a visual success**: the user judged it
"very lame" and requested bold, out-of-the-box ideas. G derives from F2's
D-based experiment, **not** from E's forced-sun separation.

H has completed three demo capture sets and independent code/visual review,
with 114 focused GLSL checks and 26 prior checks passing. Plaster highlight
recovery and grounding are stronger, but wheels/foliage are too dark and the
palette is too yellow/olive: retain the experimental direction, **not a clean
win**. H2 side-fill correction is provisionally retained. I completed warmed
comparisons with no convincing atmospheric gain and stays off/unproven.
H2 and J2 are subvariants. J/J2 contact shading was captured and reviewed, but its subtle benefit does not justify inclusion; keep it local/default-off. Its diagnostic work exposed an independent first-enable shadow framebuffer bug, fixed and verified in J2. The first scheduled
screenshot delivery (two fresh H images) was accepted at 12:42:52 UTC on
2026-10-16; this is separate from iteration completion.

## Experiments and disposition

- **A — baseline:** original vertex-evaluated sun Lambert lighting. Retained as
  the comparison point, not treated as a visual-quality target by definition.
- **B — interpolated normals:** evaluate mesh sun diffuse per fragment after
  interpolating and normalizing the normal. Synthetic curved-normal tests expose
  the difference; the actual demo comparison was subtle, not a compelling broad
  visual improvement.
- **C — ambient reduction:** reducing daylight ambient by 20% was rejected. Darkening the
  scene is not a substitute for correct light-source separation.
- **Existing shadow-map probe:** enabling the already-implemented depth-map path
  (normally off) improved directional shadows but crushed shadowed regions.
  No new shadow algorithm or strength adjustment was introduced in that probe;
  it was rejected unchanged.
- **D — sun-only shadowing:** shadow direct sunlight and sun specular while
  preserving true ambient, emissive, and local light. This left a semantic gap:
  the existing combined ambient upload also includes sun-derived
  `sun->Diffuse() * mat.forcedDiffuse`.
- **E — forced-sun separation:** carry that normal-independent sun component in
  reserved VS UBO slot 22 and subtract its occluded portion in the normal,
  detail, and grass mesh paths. Keep slot 23 reserved and texture matrices at
  slots 24 and 28. Disabled/unoccluded arithmetic, legacy prelit fallback, and
  alpha are preserved. This closes D's semantic gap but does **not** establish
  a visible ground-shadow improvement in the demonstrated scene.
- **F — deferred upper clamp, experimental:** investigate saturation erasing
  contrast before texture modulation. Diagnostic ground irradiance was roughly
  `(1.804, 1.757, 1.631)` lit and `(1.145, 1.176, 1.318)` occluded, so both clamp
  to white before albedo. Deferring the clamp recovered contrast but made terrain
  too bright and expanded specular/fog compatibility concerns. A narrower F2
  subvariant bounds base diffuse after texture0, before tint/detail/specular,
  and remains local. Neither F nor F2 is accepted by the E-scoped regression
  suite. F/F2 derive from D, without E forced-sun semantics.
- **G — daylight Lambert tuning, local:** on F2, reduce normal-dependent
  Lambert RGB by 20% in daylight for CPU/GPU rendering consumers, preserving
  raw light values, forcedDiffuse, specular, local light, emissive, and alpha.
  Northeast barn validation captures are complete. The earlier provisional
  preference is superseded by the user's artistic assessment: useful calibration,
  but not a visual success. The next experiment should pursue a bolder direction.

- **H — hemisphere and highlight shoulder, local:** based on G, orient true
  ambient using world-Y hemisphere coefficients; shape actual sun visibility
  for normal/forced diffuse; map environmental textured diffuse through a
  luminance-preserving shoulder. G's daylight Lambert scale remains. Three
  capture sets cover the hillside, northeast barn, and landscape. Independent
  reviews retain the direction but flag dark wheels/foliage and yellow/olive
  bias. H2 lifts the lateral horizon coefficients to `(0.70, 0.65, 0.52)`
  and is provisionally retained; no clean visual win is claimed.
- **I — opt-in height atmosphere, local:** based on H2, replace weather fog
  transmittance with a height-density blend when explicitly enabled. Warmed
  comparisons remove a woodland geometry/LOD confound but show no convincing
  atmospheric gain in either heading. Keep I off/unproven, not an improvement.
  Future wide-view captures wait 10 seconds after camera placement. J is
  compiling; no completed J stage is claimed.

Actual demo data was used for the visual investigation. The controlled GLSL
checks below are separate evidence, not a replacement for those comparisons.
Full official Remaster data, broad scene compatibility, and representative
hardware performance have **not** been verified. Do not infer acceptance for
retail/Guerrilla content or a player GPU from software-renderer results.

## Reproducible synthetic checks

From the repository root, with Python 3, `moderngl` (tested with 5.13.0), and an
EGL implementation supporting OpenGL 3.3:

```sh
python arma_CWR/tests/rendering/test_sunlight_gl33.py \
  --report /tmp/sunlight-results.json
python arma_CWR/tests/rendering/test_hemisphere_lighting_gl33.py \
  --report /tmp/current-material-results.json
```

The original 26 assertions in `test_sunlight_gl33.py` remain intact;
see the later uniform-block fixture correction for allocation changes. The current
H2 suite and historical E suite independently extract unmodified GLSL without
rewriting shaders or executing rewritten test source. E remains a separate
historical contract, including its old UBO layout and upper clamp. Its source
argument is required; do not run it against H2/current source.

To test a preserved revision rather than whatever is currently checked out:

```sh
# Substitute the desired verified revisions; store snapshots outside the tree.
git show "$E_REV:arma_CWR/engine/PoseidonGL33/EngineGL33_Shaders.cpp" > /tmp/E.cpp
git show "$D_REV:arma_CWR/engine/PoseidonGL33/EngineGL33_Shaders.cpp" > /tmp/D.cpp
python arma_CWR/tests/rendering/test_shadow_lighting_gl33.py \
  --source /tmp/E.cpp --baseline /tmp/D.cpp --report /tmp/shadow-E.json
```

For the historical E suite, `--source` is required. `--baseline` is
optional: when supplied, all forced-sun matrix cases additionally compare their
shadow-disabled and unoccluded outputs with that source. Reports identify source
path, SHA-256, renderer, GL version, individual cases, and scope.

### Recorded E validation, 2026-10-16

- Explicit preserved E source SHA-256:
  `a226161f6fa86e9fb3f4c8257c607880b0bdbe0fabef4864c49c7e6fc21c6a2f`.
- New independent suite: **270 checks passed**, with preserved D as the optional
  baseline, on `llvmpipe (LLVM 19.1.7, 256 bits)`, OpenGL 4.5 Core,
  Mesa `25.0.7-2+deb13u1`.
- Earlier E evidence also recorded 78 focused experimental checks and the
  original 26 checks passing. Those experimental scripts are not shipped and
  were not reused as executable test implementations here.
- The 270 count includes the static slot-layout/upload-wiring guard. Optional
  baseline comparisons are additional assertions, not separate counted rows.

Coverage includes direct/forced sunlight over normal orientation, visibility,
shadow enabled/disabled, and sun enabled/disabled; true ambient; emissive;
local ambient/diffuse; sun specular; material-product-equivalent UBO values;
legacy saturation; tint/detail; fog and shadow fading; night coefficients;
prelit fallback; and alpha preservation for normal/detail/grass.

Limitations: one synthetic triangle, one constant-depth shadow layer, one centre
pixel, and no MSAA/alpha-to-coverage, cascade transitions, spatial PCF edges,
spotlight cones, water/flat rendering acceptance, or C++ upload execution. The
slot-22 upload check is static; the material fixture models C++ products but is
not a runtime C++ integration test. Grass intentionally has its existing alpha
formula and no normal/detail night/specular/tint processing. Remaining release
acceptance includes same-scene demo comparisons, full Remaster checks, and
hardware frame-time measurements. A passing suite does not justify promoting
PR #210 out of draft on its own.


## H2 experimental regression guard

`test_hemisphere_lighting_gl33.py` extracts unmodified GLSL directly from the
selected C++ source. It is separate from the E suite and does not relax E's
expectations. It defaults to current production source. For reproducibility,
select a frozen H2 snapshot and optional G baseline:

```sh
python arma_CWR/tests/rendering/test_hemisphere_lighting_gl33.py \
  --source /tmp/H2.cpp --baseline /tmp/G.cpp --report /tmp/hemisphere-H2.json
```

The optional G baseline checks night and prelit TL identity. After adding
ABI/upload and material-order guards, 457 checks pass without it and 475 with
it on llvmpipe LLVM 19.1.7. The current production snapshot also passes 457
material checks; the unchanged original 26-check suite passes. Verified H2 source SHA-256:
`b8bb2592d3d4ccd5259001644056d1b9b88c87b5bb77206ed8ebfd05cf82e229`.
Original H fails the H2 horizon assertion as an intentional negative control.
Coverage includes ±Y/endpoints and horizon continuity, input-dependent cool
lateral balance, shoulder luminance monotonicity/chromaticity, direct and forced
sun visibility, additive local/emissive light and twilight clipping policy,
sun-off, alpha, TL fallback, and HDR tint/detail/specular/legacy-fog ordering
across normal/detail/grass. Static ABI/upload guards check ambient/daylight
slot22, forced sun slot23, preserved texture-matrix slots, and upload products.
The fixture sets I height-fog opt-in `fogParam.w=0` and camera altitude
`camPos.w=0`; it does not validate optional I atmosphere or J fullscreen contact
processing. This is synthetic single-triangle evidence, not C++ upload execution,
artist approval, cascaded shadow/MSAA coverage, or hardware timing.

## Local-only J/J2 contact regression guard

Contact AO remains local/unproven and is excluded from the candidate H2
checkpoint; this test is not a default command against H2 without AO.
`test_contact_gl33.py` independently compiles the extracted contact GLSL and
checks analytic box/ground depth for both clip conventions. Its 20 checks pass
against frozen J and J2: 14 geometry/compositing checks and six same-size nearest
D24S8 resolves at 32/64 pixels, requested samples 0/1/4. Actual sample count is
recorded because the tested driver rounds requested 1 up to 4. The explicit
zero-sample case proves single-sample coverage, rather than assuming it.

Use `--source /tmp/J2.cpp --report /tmp/contact-results.json` for reproducible
snapshot selection; the default source is production. This package validates
shader math and a direct-GL resolve, not engine C++ state restoration, initial
shadow framebuffer restoration, runtime integration, temporal motion, or player
GPU timing. Synthetic success does not close J/J2's runtime acceptance work.

## Isolated J2 framebuffer initialization checkpoint

The candidate checkpoint includes H2 material lighting, four G CPU lighting
files, and the isolated J2 shadow framebuffer initialization fix. It does not
include I height atmosphere or J contact AO. Research-stage numbering does not
imply those experiments are published or accepted.

`test_shadow_framebuffer_gl33.py` extracts and compiles exact production
allocator/RAII definitions and executes them on real EGL OpenGL. Eleven frozen
J2 checks pass: single/array first allocation, reuse, resize, failure, recovery,
and array layer-count change, with separate read/draw restoration and metadata
invalidation assertions. Logging/cache methods alone are stubbed. The test does
not execute the complete render entrypoint or prove CPU cache integration,
real-game state restoration, temporal stability, or hardware performance.
See README for compiler/header/library options and snapshot commands.

## Lighting reference

[Barn by Dg-505, photographed 12 May 2011](https://commons.wikimedia.org/wiki/File:Barn_-_panoramio_(1).jpg),
originally on Panoramio, is licensed [CC BY 3.0](https://creativecommons.org/licenses/by/3.0/).
Use it as qualitative direction: warm sunlight, cool but readable shade,
texture retained in bright plaster/roof surfaces, and layered distance. It is
not numerical noon calibration. Do not copy black foliage, infer new geometry,
or claim denser assets. Only attribution and source links are stored here; no
reference photograph is committed.

## Retained checkpoint

This checkpoint contains H2 lighting plus the independent J2 shadow initialization fix, not the I atmosphere or J contact-shading prototypes. The exact combined client built with Clang and rendered both hillside and barn fixtures. H2 preserves recovered plaster highlights with more readable side fill; the warm/olive balance and flat roof remain unfinished. The initialization fix preserves separate read/draw framebuffer bindings through allocation and failure; its runtime first-enable trace no longer showed the earlier unsupported-target skip.

Current material coverage: 457 checks (475 with G controls), original 26 checks, and 11 real-GL extracted allocator checks. Full official Remaster, cascade edges, all material paths and hardware performance remain unverified. Software performance comparisons were confounded by the engine’s existing adaptive LOD and are not isolated GPU-cost claims.

## H3 palette and K disposition

H3 is a subvariant, not another alphabetical stage. It changes lateral sky fill at approximately matched luminance in an illustrative input, not global exposure. Actual shaded/sunlit barn comparisons favor its cooler shade and retained texture, with reduced warmth on lit plaster as a known tradeoff. A clean source candidate based on c6b89fe changes only the three coefficients; it built and rendered both barn views.

K identifies authored tree/bush cutout sections correctly, but its leaf-transmission beauty effect remained unconvincing even at a diagnostic 4x strength. K2 contribution and orientation captures verified an active, small effect following broad foliage-card triangles. Preserve K/K2 locally; exclude them from the preferred stack rather than amplify card artifacts. A fixed-LOD QA override was used in those comparisons and is not part of this published checkpoint.

## Saved shadow controls

A separate usability checkpoint exposes depth-map shadows as a default-off Advanced option. Real menu interaction and restart verified persistence and Shadow Quality Off gating; it does not count as another artistic iteration or certify every shadow scene. The exact clean H3+option binary was built, with 479 focused assertions passing. No rejected rendering prototypes are included.

## L linear-material experiment (not retained)

L0 decoded ordinary albedo for selected opaque/cutout environment material paths,
not a full HDR pipeline. It lifted some vehicle detail but compressed terrain
detail contrast. L1 moved detail composition before decoding, restoring that
contrast; matched actual-engine comparisons still preferred the existing H3
sun/shadow separation overall. Both remain experimental, default-off, and
excluded from this published renderer. Existing higher MSAA/render-scale settings
improved edges independently and are not credited to L1.

## M procedural near-field grass (rejected first beauty pass)

M is the thirteenth research stage; M1 eligibility diagnostics and M2/M2b density
subtests are not extra stages. M2b rendered generated, asset-free opaque blades
through the normal mesh path with existing lighting and shadow settings. Matched
960x640 software-rendered off/on captures used the same binary, demo fixture and
fixed LOD. Both exited successfully, with 154/152/125 clumps in the three views.

Independent visual review rejected promotion: blades were nearly black, sparse
and repetitive. A suspected stray segment was subsequently identified as an
antenna-tip difference also present at a different position in the disabled
view; it is not established as a grass defect. The black blade material remains
a real failure, not evidence of improved lighting. Preserve the captures and
prototype while diagnosing material/normal behavior. Source checks and successful
GLSL compilation did not establish correct appearance. M2c replaced the white
texture asset with the renderer's white fallback without changing geometry or
palette; actual captures still showed pure-black blade interiors, so that
texture-path experiment did not resolve the defect.

The bounded occupancy checks cover authored static structures and Thing-derived
loose objects, not all moving actors. Runtime motion, shadow reception, hardware
cost and official Remaster acceptance remain open. No procedural-grass source,
game assets, screenshots or binaries are included in the published checkpoint.

### M2h diagnosis and retained local correction

Draw-time GPU readback showed nonzero lighting, white tint and the expected
opaque-white texture. A constant-red fragment diagnostic rendered correctly;
pre-texture lighting rendered olive green, while the normal final RGB was
nonfinite. A separate repeated EGL probe reproduced NaN sampling from constant
UVs at 16x anisotropy on Mesa 25.0.7 llvmpipe. Ordinary triangle UVs sampled white.
This is evidence about that tested driver, not a universal anisotropy defect.

M2h changed only generated triangle UV coordinates relative to M2c. Actual engine
captures in all three matched views then rendered olive-green blades instead of
black. Independent review retained this technical correction locally, but still
requested better groundcover. Selected camera-motion frames showed no obvious
clump jumps or black relapse; this is not exhaustive temporal acceptance.

A separate depth-map toggle comparison demonstrated darker grass under the net
while all sampled sunlit control blade pixels remained identical. The tested
vehicle-edge tuft did not change, so vehicle shadow reception was not established
there. Legacy projected shadows remained in the depth-map-disabled comparison.

M3 shorter bent blades and more jittered sites reduced the rigid-spike look, but
independent review still found tiny sparse sprouts rather than coherent grass.
Its greater draw/triangle ceiling is not justified by the observed art result.
M remains one research stage with diagnostic and art subtests; no grass prototype
code is included in this checkpoint. The standalone UV probe is published for
reproducibility, not as proof of full engine acceptance.

### M4 disposition

M4b increased local patch density and batched the shared blades. Patches became
visible at native output size, but independent review still found modest sparse
sprouts rather than compelling groundcover. In one ordered software-renderer
fixed-LOD comparison, mean frame times changed from 57.28/56.15/62.68 ms to
67.07/60.00/69.93 ms; a moving-camera window changed from 56.40 to 61.12 ms.
Instancing limited additional stationary draw calls to 8/8/7. These are local
measurements, not representative hardware costs or isolated universal estimates.
M4b is preserved locally and parked, not included in the published renderer.

Centered barrel spawning/movement/deletion exercised Thing exclusion and recovery:
9 sites suppressed after spawn, 3 at the moved destination, zero after deletion;
the final image exactly matched its baseline. The static index did not rebuild.
A road-area capture rejected all samples through nongrass/mixed-surface gating;
it did not exercise the roadway-specific rejection branch.

## N: analytic clear-day sky, retained N2 opt-in checkpoint

N is the fourteenth research stage. Its initial smooth background was rejected:
a gray/overcast appearance mismatched crisp sunlight, and near-white authored
sky modulation multiplied by accommodation caused broad highlight clipping.
N2 uses those inputs as bounded brightness/tint for an explicit artistic blue
profile, pale low-angle haze and a localized sunward aureole. The true horizon
matches existing fog color; terrain lighting and final exposure/gamma are unchanged.

Independent review of matched native-resolution gameplay and higher-angle views
retains N2 as a modest, coherent clear-day improvement. Repeated dome texture
bands are reduced while separate cloud wisps, sun and halo remain. The dome's
own baked texture contribution is intentionally replaced. Qualitative night
ON/OFF captures remained dark without a bright-sky regression; they are not
bitwise-equivalence or all-weather acceptance evidence.

The first clean candidate's full suite caught a raw depth-mask ownership
violation. Candidate v2 fixes this through scoped Core depth/stencil and color
write-state helpers without weakening the audit. Exact v2 demo captures rendered
all three scenes successfully and passed independent appearance review. Clean
captures use ordinary adaptive LOD, so foreground-detail differences are not
attributed to the sky; controlled comparisons used separately preserved QA builds.

Verification: 118 extracted-sky GLSL checks, 512 actual-GL helper restoration
cases, original 26 and material 475 rendering checks, and 11 allocator checks.
The full candidate no-game-data suite passed 3,021 cases, with 12 OpenAL-device
skips and 2,306,963 passing assertions. Its external build reused 198 previously
staged tracked synthetic fixture files; it was not a fresh CMake-only fixture
layout. Helper tests isolate actual state restoration, not complete caller cache
integration. The engine's first-draw touched-state verifier also reported success.

Default remains off; activation is `CWR_EXPERIMENTAL_SKY=1`. Hard activation
transitions, other weather/viewport/material combinations, extreme HDR behavior,
representative hardware cost and full official Remaster acceptance remain open.
No grass, fixed-LOD QA, game assets, screenshots or binaries are published with N2.

## Engineering follow-up: stable MSAA capability clamp (#212)

This is not another artistic stage. The previous target-reuse comparison used
requested MSAA before hardware clamping. On a max-4 device, a retained 8x request
therefore rebuilt the same 4x target every frame. Failed allocations also retried
unchanged requests and discarded the requested render scale.

The fix keys attempts on effective dimensions, normalized scale and supported
sample count. Success is reused; unchanged failure stays in a documented fallback
until an effective setting/size change or explicit reset permits another attempt.
Requested preferences remain intact. Defaults, preset policy, resolve filtering
and graphics-menu choices are unchanged.

149 executable checks pass, including five real-EGL allocation cases; the baseline
fails 77. The compiled production methods use an explicit class/SDL shim, and only
the real-GL mode exercises genuine allocations. Full engine evidence separately
shows 325 target-allocation log entries before versus two after: one initial target
and one configured 1440x960 4x-MSAA target. Both runs requested 8x and 150% scale;
settings remained saved. All three actual after-captures passed independent visual
regression review. No FPS gain or hardware-general cost is inferred from these runs.

The exact candidate GameDemo and full no-game-data target built. The suite passed
3,021 cases and 2,306,963 assertions with 12 OpenAL-device skips, reusing the same
previously staged synthetic-fixture layout described above. Actual GL context-loss
recovery and effective-AA menu labeling remain unverified/separate work.

## O / O2: opt-in comparison-filtered shadows (stage 15)

O changes only shadow-map sampling: hardware bilinear depth comparisons reduce
stepping in the existing nine-tap footprint. A dedicated comparison sampler and
shader-cache discriminator keep the legacy raw-depth mode separate. Depth storage,
raw diagnostic readback, bias, cascades, fades and material lighting remain unchanged.
The independent critic retained this as a modest edge-quality refinement, not a
whole-scene transformation. Selected paused-camera motion showed no obvious new
flashes or contact detachment; this is not quantified temporal stability.

O's initial nine-fetch comparison filter cost more in local software-renderer
measurements. O2 regroups the same bilinear kernel into four weighted hardware
fetches for positive reciprocal map sizes. Nonpositive diagnostic inputs preserve
the original fallback. O2 is a subtest, not an additional numbered stage.

Six alternating runs (legacy, O, O2, O2, O, legacy) used the same three scenes,
fixed-LOD QA override, 1x scale and no MSAA. Mean frame milliseconds averaged over
the paired runs were legacy 59.77/58.72/62.18, O 63.65/62.65/70.10, and O2
62.01/57.23/63.97. Draw calls matched at 883/892/877. O2 was cheaper than O in
this test, with mixed small differences against legacy. These are Mesa llvmpipe
results, not hardware-GPU performance promises. The QA override is not published.

Verification includes 504 direct O2 shader checks with legacy comparisons, plus
942 O-versus-O2 equivalence cases over D24 and D32F, including 600 additional raw
path bitwise checks. Maximum observed comparison RGBA error was 5.96e-8 against
3e-5 tolerance. D32F also uses an independent CPU oracle; D24 compares the two
shaders on identical quantized storage. Surviving-fragment alpha is tested, not
cutout/discard rasterization. These tests do not establish universal GPU identity.
The exact clean final build passed 3,021 cases / 2,306,963 assertions with 12 audio
skips, using the previously described staged synthetic fixtures. All 11 embedded
GLSL strings remained identical through the final log-only wording correction.

Default remains legacy. Opt in before launch with CWR_SHADOW_COMPARE_PCF=1 and
enable Depth-map Shadows separately. No UI default, game data or assets change.
Broader scenes, hardware cost, full Remaster content and long temporal behavior
remain unverified. Preserve the earlier O and O2 captures and binaries for comparison.

Final clean integration gate passed: three exact-candidate gameplay views at
1440x960 internal / 960x640 output, effective 4x MSAA and 150% scale, plus the
preserved clean O2 night capture. The reviewer found no obvious new contact,
resolve, seam or night-sky bleed issue. Adaptive LOD changes net/tree detail; these
high-AA frames do not expose the net-shadow stripes as clearly as the earlier
fixed-LOD comparisons, which remain the evidence for edge-quality improvement.

## N3: sunrise activation continuity (subtest of stage 14)

Actual matched upward-view captures exposed a large whole-sky change at the old
10-degree activation threshold: authored pink/cloud bands abruptly switched to
the darker analytic sky. Both ascending and descending time samples reproduced
it. N3 keeps the authored dome underneath and blends the analytic overlay using
smoothstep in sun-direction Y between 10 and 15 degrees. Zero weight does no GL
work; full weight preserves N2 analytic RGB. The always-present underlay protects
failure and discarded below-horizon pixels, but adds work at full activation.
Night/NVG guards and later cloud/celestial draw ordering remain unchanged.

The independent critic retained the 14-frame sampled sequence: nominal 9.9, 10,
11, 12.5, 14, 15 and 15.1 degrees in each direction. Candidate ascending/descending
images match at each sampled angle. The endpoint pop is removed and intermediate
mixed clouds look plausible, but a gentle brightness hump remains (slightly
brighter near 11 degrees, then darker toward 14-15). This is sampled continuity,
not a continuous-video or perfect-photometry claim. Exact fog matching applies to
the analytic contribution; the partial-weight composite also includes authored sky.

New Core blend ownership restores enable, all four factors and both equations;
RGB-only and depth protection remain separate scoped owners. Tests preserve the
118 prior sky checks and 512 prior state cases, adding 187 activation/compositing
checks and 800 blend cases. Seven optional QA-only source guards check forced
shader-failure routing; these alone do not prove executed fallback. Full local
suite passes again with the existing fixture-layout and OpenAL-skip limitations.

Actual QA failure injection compiled an invalid fragment shader through the real
compiler after the dome was flushed, then used the normal failed-state fallback.
All three gameplay views retained a coherent authored sky, with no blank region
or overlay remnant. Sky pixels differed slightly from the sky-disabled control;
this is qualitative fallback acceptance, not exact image identity. QA failure
injection, activation logs and fixed-LOD override are excluded from production.

A four-arm full-weight cost check measured exactly one extra draw per scene
(883/892/877 before, 884/893/878 after). Mean frame times varied substantially
between repeated runs, so this test does not isolate the added underlay cost and
supports neither a speedup nor a negligible-overhead claim. Hardware cost remains
open; the always-present underlay is retained for simple, reliable fallback.

Actual first-person NVG input was tested with confirmed NVGoggles possession and
normal N-key events, not a debug bypass. With NVG active, both sky-enabled and
sky-disabled groups visibly use the same green/vignetted fallback; toggling off
restores each group's normal sky. This is qualitative comparison, not pixel
identity; daylight NVG saturation remains present in both controls.

The exact clean N3 binary also rendered three midpoint dawn views at effective
4x MSAA / 150% scale, without the QA override. Independent review retained the
midpoint's blended cloud structure and grounded scene without an obvious resolve
or horizon regression. These integration images do not replace the controlled
14-frame transition sequence or establish all-weather acceptance.

## P / P1: terrain-only detail relief (stage 16)

P first established routing and signal, not a beauty improvement. An explicit
hardware base-terrain owner is captured into deferred draw state and then gated
by each section descriptor. PSDetail alone is not a terrain identifier. Default-off
cyan and detail-alpha display modes showed eligible terrain in three actual
views while sky, opaque objects, foliage, weapon and net/poles stayed separate.
The sampled detail-alpha signal varied locally and approached uniform gray at
distance. Display gamma/MSAA mean diagnostic gray is not raw numeric alpha.

P1 treats that authored detail alpha as an approximate small height field, not
new geometry or an authored normal map. Only direct Lambert sunlight uses the
perturbed normal. Ambient hemisphere, forced sun, specular, shadow visibility,
fog and alpha keep their original inputs. Nominal height scale is 0.04m, slope
cap 0.35, with smooth pixel-footprint fade from 0.06 to 0.35m and distance fade
from 15 to 60m. Degenerate/mirrored frames are guarded; constant-height and fully
faded paths return the original normal exactly. Default remains off.

Independent matched-still review retained a modest default-off candidate:
near-ground roughness is visible at native 960x640, strongest in first-person,
but it can look slightly etched/pebbled. No stronger beauty tuning was attempted.
Selected adjacent pairs from 16 small lateral and 24 forward steps per mode showed
no obvious new discontinuity, broad pulse, fade ring or contact detachment. This
is sampled-frame review, not zero shimmer or continuous-video acceptance.

Diagnostics passed 589,824 descriptor assertions across 196,608 configurations,
15 source ownership guards and 360 actual-GLSL checks. P1 adds 130 actual-GL checks
covering slope/finite/mirrored/degenerate behavior, reversed sun response, constant
and faded identity, excluded/default-off paths, protected lighting and surviving
alpha. The exact candidate full suite passed 3,021 cases / 2,306,963 assertions
with the same 12 audio skips and staged-fixture limitations described above.

P1 failed the broader dawn artistic gate: at approximately 12.5 degrees sun
height, all three views became conspicuously stippled/etched, like a pebble or
emboss filter. The original dawn palette and grounded shadows remained intact;
the new grain was the issue. Disposition: daytime KEEP experimentally, low-sun
REVISE; P1 is not published as an accepted feature. P2 will test a bounded
geometric-incidence fade rather than increase strength or change global exposure.

P2 (not published) added a smooth slope multiplier driven by the original
geometric sun incidence, zero at/below 0.15 and full at/above 0.45. It passed 175
actual-GL checks, preserving the 130 P1 cases plus 45 incidence checks, and the
full suite. Actual dawn review nevertheless found no meaningful correction:
one fixed-camera foreground region was pixel-identical to P1 and other near-ground
regions differed only slightly. Disposition: REVISE, not an improvement. Further
motion/performance testing was deferred pending actual incidence/upload diagnosis;
no stale-light or terrain-normal cause is assumed without measurement.

P1 source-cell crossing showed no new seam at the verified 50m boundary Z=5400,
X=6710.5. Forward/reverse images matched within each mode. Adjacent texture IDs
were not decoded, so this is not proof of mixed-material transition handling.
The low 0.45m camera also makes the existing etched/blocky appearance more obvious.

P2 diagnostic readback showed uploaded and current sun directions agreeing at
sampled dawn/day draws. Actual fragment incidence/fade views showed the sloping
foreground retaining the full local gate. Screenshot channels were gamma-affected,
not raw float measurements. This evidence did not support a stale-sun hypothesis.

P3 retains the local gate and adds the same smoothstep to world sun-direction Y
(0.15 to 0.45), explicitly an artistic low-sun safeguard. It genuinely suppresses
P1/P2's dawn etching while keeping the modest daytime gain. The critic retained
it as a default-off candidate, not a scene-wide transformation. Selected matched
motion pairs showed no obvious new pop, broad pulse or detached contact shadow;
this does not prove continuous-video or mixed-material stability.

Four alternating performance runs used one QA binary and unchanged settings.
Paired mean milliseconds in the three views were OFF 63.795/57.785/66.315 and ON
62.295/61.435/67.26. Calls stayed 884/893/878. Costs were mixed and view-dependent,
including a slower middle view; no zero-overhead, speedup or hardware-GPU claim.

P3 passed 205 actual-GL checks with preserved historical snapshots. The published
suite also supports a fresh checkout using the published N3 baseline: 195 checks
plus ten historical parity checks explicitly marked NOT RUN when old snapshots
are absent. An independent vector oracle replaces reliance on unpublished P2
output for elevation expectations. Original 26, current H3 material 457 and O2
shadow-comparison 504 checks also pass against the frozen P3 shader.

Final exact clean P3 integration passed independent review: paired daytime OFF/ON,
dawn ON and night ON, three views each, effective 4x MSAA / 150% scale with no QA
LOD override. Modest daylight foreground relief survives AA; dawn stays restrained
and night stays dark without obvious glow. Adaptive-LOD detail differences are
not relief gains. P3-specific cell crossings, mixed-material boundaries, sunrise
motion and representative hardware performance remain unverified.

The diagnostic suite's strict wiring guard now explicitly supports P3's factored
terrainEligible predicate and both zero resets. Current P3 rerun passes all
589,824 descriptor assertions, 15 guards and 360 actual-GLSL checks against the
published N3 baseline. No ownership or exclusion assertion was relaxed.


## Q: authored-cloud registration diagnostic (stage 17)

Q is a default-off diagnostic, not a cloud-shadow beauty improvement. Enable
CWR_CLOUD_REGISTRATION=1 before launch to display projected transmission on
explicitly owned, normal-bearing base terrain. Ordinary objects remain normally
shaded. It shares the exact legacy cloud-cell placement with the visible sky,
but projects original uncompressed authored quads along the sun direction.
No procedural cloud artwork or game assets are added.

The 512-square R16F field spans 4096m (8m texels), centered on a 256m grid.
Authored alpha and CloudsAlpha multiply transmission through overlapping quads.
Receiver height is accounted for. Unsupported geometry, missing resources,
invalid inputs and bounded enumeration failures leave ordinary shading intact.
ClampU/ClampV are routed through the render descriptor, not backend raw flags.
The first REPEAT-sampler candidate and the raw-flag integration revision are
superseded; their failed results remain preserved. Source filtering is linear,
without the visible sky's mip/anisotropic filtering, so this is an approximation.

Independent review inspected five matched OFF/diagnostic views and 24 additional
actual-engine images. Broad, soft lobes remain plausibly anchored to road and
woodland landmarks across camera translation, height change and recentering.
The return image equals the initial image exactly. Sampled phase progression is
coherent, with no visible discontinuity around 33. That is NOT a cloud-cell wrap:
separate actual captures subsequently straddled 25, 41.666667 and 50. The critic
found no visible reset in the six ground/sky pairs at those true boundaries. The metadata log's camera
field is not independent verification of the capture camera; scripted viewpoints
and their visible changes provide the camera evidence.

Disposition: KEEP diagnostic for further work, not beauty acceptance, exact
sky/ground correspondence, continuous-animation proof or hardware performance.
The grayscale override exposes a bright distant terrain edge; final beauty must
preserve ordinary fog and avoid that diagnostic appearance. No attenuation is
applied to production lighting yet. A clean full suite passed 3,021 cases and
2,306,966 assertions, with 12 OpenAL skips and the previously disclosed synthetic
fixture staging. Focused layout/projection, actual-GL and state-restoration tests
are separate synthetic evidence, not substitutes for engine captures.

P3's subsequent source-cell crossing check also passed at the verified 50m
boundary. Adjacent texture IDs remain unverified, so it is not mixed-material
acceptance. The very low camera still exposes etched/blocky detail.

Final-source focused reruns pass 1,269 native layout/projection checks, 240
actual-GL cases and three strict clamp guards, plus 2,112 scoped-state cases
and seven source guards. Those counts do not certify whole-engine failure paths.

The exact clean Q binary rendered five integration views at effective 4x MSAA /
150% scale. Independent review passed diagnostic routing and found no obvious
corruption or resolve failure. Phase was uncontrolled and adaptive LOD enabled,
so these are not exact-pattern comparisons. The wide-view fog/object mismatch
remains a diagnostic limitation, not accepted beauty lighting.

## R: bounded cloud sunlight experiment (stage 18, local)

R1 reuses Q's authored-cloud coverage but applies one separate factor capped at
12% additional attenuation to direct Lambert, forced sunlight and sun specular.
Ambient, local/emissive terms, PCF visibility, fog and surviving alpha are not
multiplied. Existing weather lighting remains, so this is additional artistic
modulation, not a physical energy correction. Default off, with separate cyan
receiver classification. Lit world Normal/Detail opaque/cutout meshes participate;
prelit, water, roads/decals, grass shader and alpha-blended meshes do not.

A camera-relative projected radius fades from 1400 to 1600m, inside every snapped
4096m map window. This prevents the outer fade envelope moving with 256m map
recentering. It does not prove raster-map temporal stability. Invalid/nonfinite
coordinates and samples return unity; all nonreceiver draws reset both controls.

Independent review of five paired actual views rejected R1 as an artistic gain:
mostly slight dimming, with cloud bands hard to distinguish beneath terrain texture
and fog. Three additional paired phases at the lower 100m road-landmark camera
confirmed insufficient visible gain. No obvious new horizon seam or sky change
was found; sky controls were pixel-identical. The diagnostic hard horizon does
not appear in ordinary beauty mode. Classification exposes excluded patches on
net/poles, road, vehicle markings and wheel/track details; actual material auditing
is separate, and absence of glaring mismatch at 12% is not full coverage approval.

R1 passes 474 actual-GL lighting cases plus 17,287 descriptor assertions, 2,030
extracted upload assertions and 20 source guards. Eleven deliberate mutations
are rejected. These isolate selected code with explicit harness/stub limitations,
not whole-engine acceptance. The unchanged full C++ suite passed with the prior
audio skips and fixture-staging caveats. Prior sources, tests and captures remain
preserved. R2 is a strength-only 40% calibration with the same GLSL/eligibility;
its explicit model passes the same tests. Independent review of its matched
normal views and three lower-camera phases recommends PARK: stronger modulation
is detectable, but ordinary gameplay mainly looks dimmer, not persuasively better.
All three phase sky images match OFF exactly. Further strength-only tuning is
not justified by these captures. Neither R1 nor R2 production changes are published.

A later read-only authored-section audit confirmed IsAlpha+IsAlphaOrdered on
Shed net/rope and M113 star/wheel-endcap sections, without depth-disable,
DisableSun or AlphaFog flags. R's transparent-phase exclusion therefore omitted
some genuinely lit solid-object sections, not only effects. Wooden poles are
opaque in the audited asset; their visible nonparticipation is not explained
by those section bits alone. No speculative classification is treated as proven.

## S: directional aerial color (stage 19, local candidate)

S leaves fog transmittance, lighting and alpha unchanged, but shifts distant
atmospheric color cooler away from the sun and warmer sunward. Nearfield color
is protected by a 120–350m fade. Activation occurs only with successful full N
sky, fading from 15 to 20 degrees sun elevation, with night/NVG fallback.

Initial independent review of three matched actual views retains a localized
experimental gain: westward mountains recede more coherently while near ground
and middle-distance trees stay distinct. Sampled foreground regions match OFF.
North/east change little; the existing sunward gray horizon band remains. The
central far ridge loses definition, so this is not universally better visibility.
Three paired night views pass qualitative review without daytime-blue leakage.
Actual NVG input and map toggles also pass: green NVG activates, the notebook/map
is visible without an S tint, and returning restores ordinary rendering. Daylight
NVG saturation is shared with OFF, not an improvement. No whole-image identity
claim is made for those checks.

Sixteen matched yaw/low-sun images retain coherent distance color. Base/return
images match within each mode, and 06:00 ON/OFF matches exactly. Actual sun-Y
samples are .24799727/.29411823/.34005442/.3856089 at 06:00/15/30/45.
Later samples gradually introduce cooler distance without a hard onset band;
the ridge can become powder-blue. These are sparse west-facing samples, not
continuous-motion or sunward low-sun acceptance.

The exact clean binary rendered three OFF/ON high-AA views at effective 4x MSAA
and 150% render scale. Independent review retained the localized westward gain
without obvious new resolve/material/horizon artifacts. Ordinary adaptive LOD
and uncontrolled cloud phase prevent exact whole-image comparisons. Near-field
lighting/fog density are not changed by this experiment.

Production scope includes explicit fog-enabled hardware world mesh families and
both ordinary hardware terrain sea (Normal/Detail) and the special Water shader.
Only the final fog RGB operand changes; water bump, lighting, specular and alpha
remain original. CPU-prelit/software water and unsupported materials retain the
legacy path. The shared helper's true-horizon agreement is not a claim that every
visible skyline pixel or the old sunward gray band is fixed.

Verification: 1,158 actual-GL checks plus 97,933 CPU assertions and 21 source
guards. These use exact extracted helpers/functions with recorded stub limits,
not full-engine failure execution. Full local suite passes 3,021 cases and
2,306,966 assertions with 12 OpenAL-device skips and prior fixture staging.
Corrected, linked-size original26/H3/shadow/terrain/cloud fixtures pass against
frozen Q/S as applicable. No R attenuation or QA metadata/LOD overrides belong
in the S production candidate. Representative GPU cost and full Remaster content
remain unverified.

## Regression fixture UBO sizing correction

Historical original26/H3/shadow fixtures allocated 432-byte PS buffers. That fits
E/N3, but later P3/Q/S linked Normal/Detail/Grass blocks require 448/480/496 bytes.
Their earlier later-source llvmpipe passes did not establish portable binding
validity. The fixtures now inspect the linked block size and zero-fill additional
storage, preserving every existing assertion. Original26's compile-only
Water/Flat/shadow checks were unaffected. The original fixture is preserved.
Terrain diagnostic's 448-byte buffer fit P3 but not Q/S; cloud diagnostic's
480-byte buffer fit Q but not S. Those allocations receive the same correction.
Reruns against frozen Q and S retain the original expectations rather than
weakening them. This is a verification fix, not another artistic stage.


S1 four-arm software timing used one fixed-LOD QA binary, identical clear-weather
phase, no MSAA and unchanged settings. Paired mean milliseconds for north/east/west
were OFF 60.405/95.835/60.200 and ON 71.670/108.245/66.855, with unchanged draw calls
901/766/727. The cost is material on llvmpipe; north ON repeats also varied notably.
This is not a representative GPU estimate, a speedup, or a negligible-cost claim.
S1 is retained as a default-off visual experiment with that cost disclosed.

A separate S2 fog-free-work shortcut is being investigated, not included here.
Its strict HDR bitwise-parity hypothesis failed despite tiny measured differences;
that report remains preserved rather than being called a pass. Performance and
bounded-numeric/actual-image gates are separate from S1's published contract.
