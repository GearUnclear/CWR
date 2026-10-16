# GL33 lighting iteration record

Branch: `graphics/lighting-improvements`. PR #210 remains a **draft**.
This record distinguishes a semantic correction from a demonstrated visual win.
No game assets, captured screenshots, or executable binaries belong in this tree.

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
  experiment is under review. Neither F nor F2 is accepted by this record or by
  the E-scoped regression suite.

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
python arma_CWR/tests/rendering/test_shadow_lighting_gl33.py \
  --report /tmp/shadow-lighting-results.json
```

The original 26 checks in `test_sunlight_gl33.py` remain untouched. The additional
suite independently extracts and compiles production raw GLSL; it does not modify
shader strings, import experimental scripts, or execute rewritten test source.
It is intentionally a semantic-E guard, including E's upper clamp. A later clamp
experiment may fail it and needs explicit review rather than relaxed assertions.

To test a preserved revision rather than whatever is currently checked out:

```sh
# Substitute the desired verified revisions; store snapshots outside the tree.
git show "$E_REV:arma_CWR/engine/PoseidonGL33/EngineGL33_Shaders.cpp" > /tmp/E.cpp
git show "$D_REV:arma_CWR/engine/PoseidonGL33/EngineGL33_Shaders.cpp" > /tmp/D.cpp
python arma_CWR/tests/rendering/test_shadow_lighting_gl33.py \
  --source /tmp/E.cpp --baseline /tmp/D.cpp --report /tmp/shadow-E.json
```

`--source` defaults to production `EngineGL33_Shaders.cpp`. `--baseline` is
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
