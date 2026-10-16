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
