# GL33 lighting regression checks

These optional tests compile the actual embedded GLSL and draw synthetic triangles
in an EGL OpenGL context. They require Python 3 and `moderngl` (tested with 5.13.0).
They do not need game assets. Run the original smoke suite and current H3-palette material
contract against production:

```sh
python arma_CWR/tests/rendering/test_sunlight_gl33.py
python arma_CWR/tests/rendering/test_hemisphere_lighting_gl33.py
```

For an explicit before/after comparison, save the unmodified shader source first:

```sh
git show 16259cc21188558c5dc5e0cc920f916c80acd9b0:arma_CWR/engine/PoseidonGL33/EngineGL33_Shaders.cpp > /tmp/baseline-shaders.cpp
python arma_CWR/tests/rendering/test_sunlight_gl33.py --baseline /tmp/baseline-shaders.cpp --report /tmp/sunlight-results.json
```

## Original 26-check smoke suite

- Eleven production vertex/fragment shader combinations compile and link.
- Normal, detail and grass paths each render five controlled cases: curved normals,
  planar normals, ambient only, sunlight disabled and prelit screen vertices.
- At the triangle centre, interpolated normals point toward the sun. New diffuse
  intensity is 0.7; vertex-interpolated Lambert intensity was 0.4. Other tested
  cases preserve the baseline, including existing alpha behavior.
- Float pixel assertions use tolerance 1e-5 and fail the process on mismatch.

## Original interpolation scope and remaining acceptance

The original B interpolation step changed **mesh sun diffuse**, not textures, vegetation density or weather. The following paragraph records that historical scope; H2 supersedes its clamp and shadow behavior.
CPU-lit TL terrain does not gain fragment lighting. Water and specular lighting
remain unchanged. Local lights are still evaluated per vertex; the combined mesh
RGB is now clamped after interpolation, so saturated lighting can differ as well.
Existing shadow attenuation still affects the combined lighting result.

The synthetic fixture deliberately disables shadows and fog. Compile/link checks
cover their shader source, but are not visual regression tests of those features.
There is no GPU-performance assertion. The recorded local run used software Mesa
llvmpipe, not a representative player GPU.

Before leaving draft, build both revisions and render the same demo scene with
identical camera, time, weather, configuration and frame timing. Inspect trees,
curved vehicles, flat buildings, alpha-cutout foliage, terrain, menu/HUD, water,
night/local lights, fog and cascaded shadows. Then repeat the relevant checks with
full official Remaster data. Demo renderer checks do not certify full-game or
Guerrilla content compatibility. Do not commit demo or retail assets.

## Current progress (2026-10-16)

The initial dependency/download interruptions were resolved. Both compiled-main
and modified demo clients now build and render the same controlled gameplay
fixture. This is not full Remaster acceptance. The interpolation-only comparison
was visually subtle; further shadow/light separation is documented in
[ITERATIONS.md](ITERATIONS.md), including rejected experiments and limitations.

## Candidate checkpoint versus historical contracts

This checkpoint contains the H3 palette on H2 material lighting, its four G CPU lighting files, and
the isolated J2 shadow-framebuffer initialization fix. I height atmosphere and
J/J2 contact AO remain local/unproven and are not part of that candidate.
The iteration record describes research history, not the set of published
features. The exact combined checkpoint compiled and rendered hillside and barn demo scenes. It remains a draft visual direction, not full-game acceptance.

- `test_hemisphere_lighting_gl33.py` is the current hemisphere/material contract and
  defaults to production source. `--source /tmp/H2.cpp` selects a frozen snapshot;
  optional `--baseline /tmp/G.cpp` adds night/TL identity comparisons. It checks
  hemisphere/shoulder behaviour, protected lighting, ABI/upload wiring,
  tint/detail/specular/fog ordering, alpha, and TL fallback. Frozen H2 passes
  457 checks, or 475 with G baseline comparisons.
- `test_shadow_lighting_gl33.py` preserves the 270 E assertions.
  It **requires `--source`** for a preserved E source. It is not a current-source
  test: E puts forced sun in slot22, whereas H2 uses slot22 for ambient/daylight
  and slot23 for forced sun. The different clamp/visibility expectations are
  deliberate and were not silently relaxed.
- `test_sunlight_gl33.py` is the original 26-check suite, unchanged.

Historical E reproduction (the commit must be available in your local clone):

```sh
git show ab100b1ac4c6a9e179ef128fd859e9ec57180b8f:arma_CWR/engine/PoseidonGL33/EngineGL33_Shaders.cpp > /tmp/E.cpp
python arma_CWR/tests/rendering/test_shadow_lighting_gl33.py \
  --source /tmp/E.cpp --report /tmp/shadow-E.json
```

The current fixture explicitly disables I's opt-in height fog with
`fogParam.w=0` and sets camera altitude `camPos.w=0`. It exercises existing fog,
not I's optional atmosphere. J's separate fullscreen contact pass is not run by
these material tests. ABI/upload assertions are static, not C++ execution.
No suite here establishes cascade-edge, MSAA, full-game, visual-quality, or
hardware-performance acceptance. A changed contract must be reviewed explicitly;
do not add silent skips or widen tolerances to make a new experiment pass.


## Shadow framebuffer initialization guard

`test_shadow_framebuffer_gl33.py` is eligible for the isolated J2 initialization
fix checkpoint. It extracts the exact `ShadowFramebufferBindings`, `EnsureTarget`
and `EnsureArrayTarget` definitions, compiles them without rewriting their
bodies, and executes them against real EGL OpenGL. It needs a C++17 compiler
and OpenGL development headers/library in addition to moderngl.

```sh
python arma_CWR/tests/rendering/test_shadow_framebuffer_gl33.py \
  --report /tmp/shadow-init-results.json
```

Use `--source /tmp/EngineGL33_ShadowDepth.J2.cpp` for a frozen source. Optional
`--compiler`, repeatable `--include-dir`, and `--gl-library` configure relocated
toolchains without editing the test. Build products live in a temporary directory
and are removed. Eleven checks pass on frozen J2: first allocation, reuse,
resize, zero-size failure and recovery for single/array targets, plus changing
array layer count. Every case checks distinct read/draw framebuffer restoration
and cached dimensions/layers, including invalidation after failed allocation.

Only logging and GL33Bind cache methods are stubbed. The fixture therefore does
not prove cache bookkeeping, complete engine render-entrypoint restoration,
game-runtime integration, temporal stability, or GPU performance. This is a
focused real-GL allocator test, not a full-engine C++ suite.

## Local-only contact experiment (not in the candidate checkpoint)

`test_contact_gl33.py` remains local/unpublished until contact AO is accepted.
Do not include it in default commands against the H2 checkpoint, which has no
contact shader. The test is self-contained. It extracts actual contact vertex and
fragment GLSL without rewriting it, generates analytic ground/box depth, and
executes a separate direct-OpenGL D24S8 resolve fixture. For a frozen source:

```sh
python arma_CWR/tests/rendering/test_contact_gl33.py \
  --source /tmp/J2.cpp --report /tmp/contact-results.json
```

Twenty checks pass on frozen J and J2 under llvmpipe: fourteen geometry checks
cover both zero-to-one and negative-one-to-one depth conventions, no false
flat-ground occlusion, bounded box contact, alpha/sky/night preservation,
analytically correct debug depth, offset viewport registration, and fractional
daylight compositing. Six resolve checks cover 32/64-pixel D24S8 attachments with
requested sample counts 0, 1 and 4. Actual sample counts are reported: on the
verified driver, requesting 1 yields 4, while requesting 0 proves a genuinely
single-sample source. Resolved depth matches 0.42 within 1e-6.

This does **not** execute the engine's C++ state guard, framebuffer allocation
or first-shadow-pass restoration, runtime draw hook, real game geometry, or
motion history. It does not certify silhouette halos, temporal stability,
material-aware AO, or hardware performance. The resolve test exercises the GL
API operation, not the production C++ call path. Unsupported contexts fail
explicitly rather than silently skipping MSAA coverage.

## H3 palette checkpoint

Only the three lateral ambient coefficients change from H2: `(0.70, 0.65, 0.52)` to `(0.60, 0.66, 0.63)`. Shade reads cooler while plaster detail remains. The lit facade also loses some warmth, so this remains a provisional artistic direction. The exact clean candidate built and rendered shaded and sunlit barn scenes; no experimental foliage or benchmark code is included.

The current test defaults to `--palette h3`. Use `--palette h2 --source /tmp/H2.cpp` to reproduce the preserved H2 contract. Both matched models pass 457 checks (475 with G controls); mismatched source/palette pairs fail. Historical E assertions remain separate and unchanged.

## Persistent depth-map option

Options → Graphics → Advanced → Depth-map Shadows now exposes the existing depth-map path. It defaults off and presets do not change it. Shadow Quality Off suppresses the backend without erasing the saved preference; restoring quality re-enables the opted-in path. Close the page to persist the choice. This remains an experimental option with a performance cost.

Verification: 479 assertions across 40 focused GraphicsConfig/GraphicsPage cases; exact H3+option demo build; actual menu toggle, Shadow Quality Off/restore, saved config, and process restart into gameplay with maps enabled without the test enable command. The complete local PoseidonTests target hit an untouched relative PackageMount.hpp include error in test_package_mount.cpp; the focused executable passed, not the full suite. New-head CI is separate.

## External-build verification follow-up (#211)

A separate one-line test include correction now lets PackageMount.hpp resolve directly from the source directory instead of relying on the in-tree build layout. The affected object and full PoseidonTests target compiled in the external build. PackageMount cases passed: 3 cases/18 assertions.

The no-game-data suite then passed 3,021 cases with 12 explicit OpenAL-device skips and 2,306,960 assertions. This required staging the repository synthetic fixtures into the external build output: the first run failed because the PAA fixture lookup expected fixtures/paa, while staging supplied fixtures/texture/paa. No fixture or gameplay files were changed by this commit. The full local test binary included frozen experimental M sources; it is not an exact published-head replay. Published-head CI remains a separate verification.

### UV sampling regression and driver diagnostic

Requires Python `moderngl` and an EGL/OpenGL 3.3 implementation. From the repository root:

```sh
python arma_CWR/tests/rendering/test_uv_sampling_gl33.py --report uv-sampling-report.json
```

The self-contained synthetic shader probe asserts finite opaque-white sampling with nondegenerate triangle UVs, using ordinary filtering and the supported maximum anisotropy capped at 16. Unsupported anisotropy cases are explicitly skipped; context/dependency errors are not successful skips. The JSON report includes driver/version, requested/applied anisotropy, sampler/texture settings and all repeated samples.

Constant-UV samples are recorded without asserting a driver-specific failure. Mesa 25.0.7 llvmpipe produced NaN with constant UV at 16x while nondegenerate UV remained white; other drivers may correctly sample both. This does not test engine GLSL, material distribution, full-game behavior or hardware performance. Run separately from timed rendering benchmarks to avoid contention.

### Experimental daytime sky shader (N3, retaining N2 contract)

Requires Python `numpy`, `moderngl` and EGL/OpenGL 3.3. From the repository root:

```sh
python arma_CWR/tests/rendering/test_sky_gl33.py --report sky-report.json
# To test a preserved variant rather than the current checkout:
python arma_CWR/tests/rendering/test_sky_gl33.py --model n2 --source path/to/N2/EngineGL33_ExperimentalSky.cpp --report sky-report.json
```

The fixture extracts both GLSL literals unchanged from the selected C++ file and preserves 118 named checks: horizon continuity, lower-hemisphere discard, finite/bounded output, cropped viewport, pitched/rolled camera ray registration, authored-color scaling, mission-value highlight bounds, blue zenith and haze. Reports include the exact source SHA256 and driver/version. No game assets are needed.

`clearSky=0` verifies the shader's horizon-color identity only. It does not execute the engine's weather/night/NVG activation or legacy fallback gates, C++ GL-state restoration, cloud ordering, camera motion or renderer integration. These need separate actual-engine validation. This is a default-off clear-day candidate, not all-weather acceptance or a hardware-performance benchmark.

Activation: set `CWR_EXPERIMENTAL_SKY=1` in the game process environment before launch. It is disabled by default and currently has no graphics-menu toggle. GL33 uses the analytic sky only in full daylight with sun elevation above 10 degrees, night vision off, supported world target/projection and positive sky-through. Unsupported conditions or shader failure retain the original dome. N3 always retains the authored dome underneath and smoothly increases analytic overlay weight from 10 to 15 degrees. Separate cloud/celestial passes remain. At full weight the analytic upper sky replaces the dome texture; below the horizon the underlay survives. Weather transitions, extreme HDR clipping, and a gentle intermediate brightness hump remain experimental limits.

### Scoped background write-state helpers

```sh
python arma_CWR/tests/rendering/test_background_state_gl33.py --report background-state-report.json
```

Requires ModernGL/EGL, a C++17 compiler, desktop OpenGL headers and linker library. `--depth-header` and `--pipeline-header` select frozen headers; `--compiler`, repeatable `--include-dir` and `--gl-library` override local build discovery. The exact headers are compiled unchanged into a temporary shared library. A temporary `glad/gl.h` adapter supplies system GL declarations; calls execute real GL, not state mocks.

512 cases cover all depth/stencil enable combinations, depth write masks and RGBA write masks through early-return destruction, explicit repeated restoration, inert restoration after later state changes, and nested scopes. Depth comparison and distinct front/back stencil functions, references, masks and operations must remain untouched. Ownership noncopyability is checked at compile time. This isolates helper behavior, not engine cache invalidation, caller gates, full render ordering or hardware performance.

## Frame-target capability clamp and bounded retry (#212)

`test_frame_target_gl33.py` extracts the exact four production frame-target methods
and their header member initializers, compiles them as C++17, and executes them.
The default 144 checks use counted GL/SDL stubs for deterministic capability and
failure transitions. Optional `--real-gl` adds five genuine EGL OpenGL allocation
checks on Linux, requiring `moderngl` and the desktop `libGL` runtime.

```sh
python arma_CWR/tests/rendering/test_frame_target_gl33.py --real-gl \
  --report /tmp/frame-target-results.json
```

`--source` and `--header` select preserved source/header snapshots together;
`--compiler` selects the C++17 compiler (default `CXX` or `clang++`). Reports hash
both files and the exact extracted code. The pre-fix snapshot fails 77 of 149
checks; the candidate passes all 149. Real Mesa llvmpipe reports `GL_MAX_SAMPLES=4`:
requesting 8x over 120 unchanged frames previously allocated 120 targets, versus
one after the fix. Resize, scale/capability changes, both framebuffer failure
stages, separate engine instances, and explicit reset are covered. The real-GL
zero-size failure attempts allocation and warns once, then recovers once after
size restoration without losing the requested 150% scale or 8x preference.

Failed effective requests remain suppressed until target size, normalized scale,
effective sample count, or explicit target reset changes. This intentionally does
not retry transient allocation failure every frame. The fallback's effective
MSAA/scale becomes off/1 while the user's requested settings are retained.
Defaults, presets, and Advanced UI choices are unchanged. Effective-versus-requested
AA labeling in the normal graphics UI remains separate follow-up work.

The test uses a class-state/SDL shim and, in real-GL mode, counted framebuffer
creation and no-op logging. It is not a full engine, full Remaster, visual-quality,
or representative-hardware performance test. The existing equal-size nearest
MSAA resolve then linear supersampling downsample remains unchanged.

Explicit target reset plus capability requery is covered; actual GL context-loss
recovery is not simulated. This is an engineering stability follow-up, not a new
artistic lighting iteration.

## Opt-in comparison-filtered shadows (O2)

```sh
python arma_CWR/tests/rendering/test_shadow_compare_gl33.py --source arma_CWR/engine/PoseidonGL33/EngineGL33_Shaders.cpp --baseline path/to/legacy/EngineGL33_Shaders.cpp --report shadow-compare-report.json
```

Requires NumPy, ModernGL, desktop GL and EGL. The current production source supports comparison PCF. Actual normal/detail/grass fragment shaders are extracted unchanged; only the supported compile-time define is prepended after `#version`. Synthetic vertex outputs and a depth32F array isolate fragment semantics. The CPU oracle evaluates nearest raw or bilinear LEQUAL comparisons across the unchanged nine-tap kernel, cascade blending/fallthrough and material visibility.

Checks cover nonuniform stepped/checkerboard depth, subtexel smoothness, map/depth bounds, layer isolation, equality, bias scaling, fog, daylight/night coefficients, sun-off, disabled shadows and alpha. With `--baseline`, every raw result is also compared bitwise to the selected legacy source. Sampler filters and comparison mode are queried from real GL; texture storage remains raw. Source guards cover shader-cache discrimination and sampler selection, not engine execution. This does not validate real vertex transforms, depth24 quantization, runtime cache/sampler lifecycle, temporal gameplay or hardware performance. Engine captures remain required.

For historical optimization reproduction, use the preserved O1 source as --source and add `--comparison-source path/to/optimized/EngineGL33_Shaders.cpp`. This preserves the original 504 checks when a legacy baseline is supplied and adds 942 actual-GL equivalence cases over D24 and D32F storage, deterministic random/nonuniform arrays, subtexel positions, borders, cascade/layer cases and nonpositive texel-size fallback. The D32F path also checks the independent nine-bilinear CPU oracle; D24 compares both shaders on identical quantized storage. 600 of these cases additionally require bitwise raw-path identity. Comparison output uses absolute RGBA tolerance 3e-5 to allow driver interpolation precision; passing one implementation is not a universal hardware guarantee. Reports identify baseline and optimized source SHA256 values. Alpha checks cover only surviving fragments, not cutout/discard rasterization or alpha-tested casters.

Set `CWR_SHADOW_COMPARE_PCF=1` before launching to opt in. Depth-map Shadows must also be enabled, with Shadow Quality above Off. The default remains legacy sampling. Production uses four weighted fetches for the existing nine-bilinear-tap footprint; this is not physically varying penumbra or PCSS. Historical baseline/comparison snapshots are optional and must be preserved separately.

### N3 smooth sky activation and blend restoration

```sh
python arma_CWR/tests/rendering/test_sky_gl33.py --model n3 --source arma_CWR/engine/PoseidonGL33/EngineGL33_ExperimentalSky.cpp --report sky-N3.json
python arma_CWR/tests/rendering/test_background_state_gl33.py --blend-header arma_CWR/engine/Poseidon/Graphics/Core/GLBlendState.hpp --report blend-N3.json
```

The explicit N3 sky contract preserves the original 118 checks and adds 187 checks: the exact pure C++ activation function is compiled, its finite guards and smooth thresholds are checked, and actual extracted GLSL is composited over a synthetic framebuffer at five overlay weights with RGB-only writes. Destination alpha is preserved for zero, fractional and full alpha. N3 requires a C++17 compiler (`--compiler`, default `CXX` or `clang++`); N2's original shader-only contract remains available with `--model n2`.

The optional blend header preserves the original 512 state cases and adds 800 cases across blend enable, four distinct factor profiles, all 25 pairs of RGB/alpha equations (add/subtract/reverse/min/max), early-return destruction, repeated explicit restoration, inert restoration after later state changes and nesting. Exact selected production headers compile unchanged against real GL declarations. This tests helper early-return behavior, not the complete engine's early-return or allocation-failure paths. Authored dome integration, all weather/low-sun conditions, long motion, state-cache behavior and hardware cost still require engine validation.

Optional source-only QA failure-route guards:

```sh
python arma_CWR/tests/rendering/test_sky_gl33.py --source path/to/clean/EngineGL33_ExperimentalSky.cpp --failure-qa-source path/to/ExperimentalSky.QA-failure.cpp --report sky-failure-guards.json
```

This exits without a GL context. Seven source checks cover QA-only injection isolation, zero-weight/compile ordering, real invalid fragment compilation, shader cleanup and failed-state fallback returns. They are not executed engine-failure or fallback-image tests; the QA source is preserved separately and is not production code.

### Experimental terrain direct-sun relief (P3, default off)

`CWR_TERRAIN_RELIEF=1` enables the nominal terrain-only prototype (.04 height
scale, .35 maximum slope). `CWR_TERRAIN_RELIEF=2` is an exaggerated diagnostic
(.08/.70), not the recommended visual setting. Unset or other values disable it.
Only explicitly owned, opaque, lit detail-textured terrain is eligible. The
perturbed normal affects direct-sun Lambert response, not geometry, ambient,
local/emissive light, or shadow-map visibility.

Separate `CWR_TERRAIN_DIAGNOSTIC=1` shows eligible terrain cyan;
`CWR_TERRAIN_DIAGNOSTIC=2` displays sampled detail alpha as grayscale. These are
routing/signal views, not relief or beauty modes. Leave this flag unset when
reviewing relief. Displayed grayscale is not raw numeric texture analysis.

P1's daytime result was retained experimentally but dawn looked conspicuously
etched. P2's local geometric-incidence fade did not sufficiently suppress tilted
terrain at dawn. Actual uploaded/current sun directions agreed: this was not a
stale-uniform diagnosis. P3 retains that local gate and adds a global sun-elevation
smoothstep (.15 to .45 in direction Y), explicitly an artistic low-sun safeguard,
not a physical lighting correction. Distance and pixel-footprint fades remain.
Actual day/dawn and sampled-motion reviews retained a modest default-off candidate.
See ITERATIONS.md for capture and cost evidence; do not infer visual or performance
acceptance from these automated tests.

From a repository checkout containing the published N3 commit:

```sh
git show a514a696700ce4acf1479d9c4daf42b211ca8a04:arma_CWR/engine/PoseidonGL33/EngineGL33_Shaders.cpp > /tmp/cwr-N3-shaders.cpp
cd arma_CWR
python arma_CWR/tests/rendering/test_terrain_relief_gl33.py \
  --baseline /tmp/cwr-N3-shaders.cpp --report /tmp/terrain-relief.json
```

The source defaults to the checkout shader and the model defaults to P3. This
reproducible mode runs 195 checks and explicitly reports ten unavailable
historical parity checks as NOT RUN, not PASS. The independent Python oracle
solves the tangent-plane gradient with linear algebra and applies the bounded
slope and fades; it neither rewrites current GLSL nor relies on unpublished P2
output. Requires Python numpy/moderngl and EGL/OpenGL 3.3.

To additionally reproduce all 205 checks, supply preserved experimental shaders
with `--p1-source /path/to/P1/EngineGL33_Shaders.cpp` and
`--p2-source /path/to/P2/EngineGL33_Shaders.cpp`. These enable six exact P1 and four
exact P2 parity comparisons. Historical `--model p1` and `--model p2` remain
explicit; sources/models are never silently auto-detected. The original full
205-check reports remain preserved separately from the fresh-checkout report.

The suite compiles unchanged extracted GLSL and its original helper with
synthetic planar geometry/detail ramps. Coverage includes finite/slope bounds,
constant height, mirrored/degenerate frames, fade continuity, independent
geometric/global elevation oracles, protected-light contribution,
disabled/excluded paths and surviving fragment alpha.

This is not actual-engine ownership/cache execution, proof of terrain art quality,
cutout rasterization, temporal stability, full Remaster validation, or hardware
performance. The diagnostic suite separately tests descriptor classification and
synthetic cyan/detail-alpha output. No game assets or capture images are included.

Current diagnostic suite, from `arma_CWR`, with the same published N3 baseline:

```sh
python arma_CWR/tests/rendering/test_terrain_diagnostic_gl33.py \
  --engine-root engine --baseline /tmp/cwr-N3-shaders.cpp \
  --report /tmp/terrain-diagnostic.json
```

Use a complete engine source tree, not a changed-files-only patch snapshot:
the exact descriptor compile also needs unchanged `RenderFlags.hpp`. Current
P3 passes all 589,824 descriptor assertions, 15 ownership/reset source guards
and 360 synthetic actual-GL diagnostic checks. The source guard recognizes
both the original inline eligibility expression and P3's explicitly factored
ownership expression with zero resets; it does not waive ownership conditions.

### Terrain-detail diagnostic contract (not a beauty effect)

```sh
python arma_CWR/tests/rendering/test_terrain_diagnostic_gl33.py --engine-root arma_CWR/engine --baseline /tmp/cwr-N3-shaders.cpp --report terrain-diagnostic.json
```

Requires C++17 (`--compiler`), NumPy, ModernGL and EGL/OpenGL 3.3. The coherent engine root must include the diagnostic source files and unchanged RenderFlags.hpp. Exact descriptor headers compile without mocks. 196,608 classification configurations yield 589,824 assertions including descriptor equality and nonowner reset. Fifteen source guards cover captured mesh provenance, scoped restoration/defaults, dedicated UBO slot and explicit zero upload; they do not execute engine draw-queue or state-cache transitions.

360 actual-fragment checks use synthetic vertex outputs and controlled detail-alpha textures. Mode0 is bitwise identical to the selected pre-diagnostic source. Modes1/2 produce cyan or sampled detail alpha only for normal-bearing Detail fragments; normal, grass and prelit exclusions preserve baseline results. Surviving-fragment alpha is retained. These are diagnostic validation tests, not proof that real terrain assets provide a useful relief signal, correct terrain eligibility in actual captures, a beauty improvement, alpha-discard coverage or hardware performance.

The current wiring guard explicitly supports the original P inline ownership test
and the P3 terrainEligible definition with both diagnostic and relief zero-reset
expressions. The current P3 suite passes the same assertion counts. Use the
published N3 baseline extracted above; historical local snapshots are optional.


### Authored-cloud registration diagnostic (Q, default-off experiment)

`CWR_CLOUD_REGISTRATION=1` requests a bounded transmission-map diagnostic. It is
not beauty attenuation: eligible terrain displays sampled transmission as
grayscale. The map uses original uncompressed authored-cloud quads, not their
camera-compressed sky display positions. Actual registration requires separate
engine capture review; synthetic checks cannot establish it.

From a checkout containing the published P3 commit:

```sh
git show e37b7280e65706f1b371e8aabdd77c2c9a4ac22c:arma_CWR/engine/Poseidon/World/Terrain/LandscapeRender.cpp > /tmp/cwr-P3-landscape.cpp
git show e37b7280e65706f1b371e8aabdd77c2c9a4ac22c:arma_CWR/engine/PoseidonGL33/EngineGL33_Shaders.cpp > /tmp/cwr-P3-shaders.cpp
cd arma_CWR
python tests/rendering/test_cloud_registration_gl33.py \
  --legacy-landscape /tmp/cwr-P3-landscape.cpp \
  --baseline /tmp/cwr-P3-shaders.cpp --report /tmp/cloud-registration.json
python tests/rendering/test_cloud_bindings_gl33.py --report /tmp/cloud-bindings.json
```

Current source paths default to this checkout; explicit `--engine-root` and
`--source` overrides support frozen complete trees. The state test additionally
needs a C++ compiler, OpenGL development headers/library and supports optional
`--include-dir`/`--gl-library` overrides. No private snapshots or game assets are
needed for these tests.

The suite compiles the unchanged shared layout helper alongside extracted legacy
expressions using a deterministic synthetic RNG (not actual engine random-value
distribution). 1,269 native checks cover exact visible-instance parity, negative
cells/phase wraps, ray-plane projection and receiver-height shear. 240 actual-EGL
checks cover synthetic AI88 intensity/alpha swizzle, weather-scaled multiplicative
overlap, asymmetric UV alpha orientation, out-of-range authored-clamp edges, exact map boundaries, diagnostic receiver sampling, outside-map neutrality and disabled,
prelit and material exclusions. Requires a C++17 compiler, Python numpy/moderngl
and EGL/OpenGL 3.3. Actual assets, visual registration, missing-texture/overflow
engine execution and hardware performance are not covered by this suite.

The separate test_cloud_bindings_gl33.py compiles the exact scoped binding owner
with real GL state. Its 2,112 cases exercise restoration and synthetic failure
exits; seven explicitly source-only guards are not whole-engine failure tests.

Three source guards require the canonical descriptor sampler route, reject either unclamped axis, and require matching clamp sampler S/T.
They do not execute engine missing-texture or unsupported-face failure branches.


### Linked uniform-block sizing in regression fixtures

Current fixtures allocate at least the linked PSConstants block size and zero
all added slots. Historical 432-byte Normal/Detail/Grass buffers fit E/N3, but
P3/Q/S declare 448/480/496 bytes. Earlier later-source llvmpipe passes with shorter
buffers were not portable evidence. Assertions are unchanged; historical sources
and original26 compile-only checks are not reclassified as failures.

The terrain diagnostic (previously448 bytes) and cloud diagnostic (previously480)
use the same rule. Tests select exact frozen source explicitly; the original26
suite now accepts --source while retaining all26 assertions.


### Directional aerial color regression checks

`CWR_AERIAL_COLOR=1` opts into directional fog scattering color, not increased
fog density or changed transmittance. Also set `CWR_EXPERIMENTAL_SKY=1`; it requires a successful, fully active
analytic sky overlay. Activation fades between sun elevations 15–20 degrees;
mesh color weight fades over 120–350 m. Night, NVG and excluded draws upload
zero control. The analytic sky and supported mesh families share the same
luminance-preserving cool/warm directional color helper.

From `arma_CWR` in a fresh checkout, obtain the published pre-S shader baseline
(no experimental snapshot or game asset is required):

```sh
git show 2eb8e25cf19d44471f74ecf5d6552a8c762f006c:arma_CWR/engine/PoseidonGL33/EngineGL33_Shaders.cpp > /tmp/cwr-Q-shaders.cpp
python tests/rendering/test_aerial_color_gl33.py \
  --baseline /tmp/cwr-Q-shaders.cpp --report /tmp/aerial-color.json
python tests/rendering/test_aerial_eligibility.py --report /tmp/aerial-gates.json
```

The baseline commit must be available in Git; a shallow clone may need to fetch
that commit first. The GL suite requires Python `numpy`, `moderngl` and an
EGL/OpenGL 3.3 implementation. The CPU suite requires a C++17 GNU-style compiler
(`--compiler` or `CXX`). Sources default to the checkout; explicit `--source`,
`--sky-source` (GL) and `--engine-root` (CPU) support coherent frozen sources.
Both reports include tested source hashes; the GL report includes driver details.

- **1,158 actual-GL checks:** Normal, Detail, Grass and Water; independent color
  and luminance oracle; unchanged foreground and fog transmittance against Q;
  Water bump/specular slot c4; surviving-fragment alpha; exact nearfield,
  disabled and no-fog identity; distance continuity; matched true-horizon
  analytic sky/mesh color. Newly appended UBO slots are explicitly zeroed.
- **97,933 CPU assertions plus 21 strict source guards:** 50,688 receiver rows,
  31 named materials, 99 sequential upload/reset cases, a 2,001-sample activation
  oracle and 144 fresh-process environment combinations. Exact production
  headers and complete extracted functions execute in a recording shell;
  external scene, GL and upload dependencies are stubs, not a full engine.

These are S-specific contracts against published Q, not the parked R cloud-sun
attenuation experiment. Synthetic checks do not establish atmospheric artistic
quality, physical scattering accuracy, cutout rasterization, continuous-motion
stability, full Remaster compatibility or hardware performance. Engine capture
and visual review evidence is separate; shader identity is not proof of actual
runtime gating or successful framebuffer rendering.
