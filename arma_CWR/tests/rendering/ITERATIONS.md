# GL33 lighting iteration record

Branch: `graphics/lighting-improvements`. PR #210 remains a **draft**.
This record distinguishes a semantic correction from a demonstrated visual win.
No game assets, captured screenshots, or executable binaries belong in this tree.

## Alphabetical milestone progress

Current numbered stage: **K (11 of 703)**. The requested target is **AAA (703)**
using A–Z, AA–AZ, and onward. This is a longer-running iteration milestone,
separate from the 15 screenshot deliveries over 10 hours; neither is complete.
F2 is a subvariant of F. Diagnostics, time-of-day repeats, camera angles, and
scene surveys are validation subruns, not additional numbered iterations.

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

The original 26 checks in `test_sunlight_gl33.py` remain untouched. The current
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
