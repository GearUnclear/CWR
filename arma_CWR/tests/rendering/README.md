# GL33 sunlight regression checks

These optional tests compile the actual embedded GLSL and draw synthetic triangles
in an EGL OpenGL context. They require Python 3 and `moderngl` (tested with 5.13.0).
They do not need game assets. Run:

```sh
python arma_CWR/tests/rendering/test_sunlight_gl33.py
```

For an explicit before/after comparison, save the unmodified shader source first:

```sh
git show 16259cc21188558c5dc5e0cc920f916c80acd9b0:arma_CWR/engine/PoseidonGL33/EngineGL33_Shaders.cpp > /tmp/baseline-shaders.cpp
python arma_CWR/tests/rendering/test_sunlight_gl33.py --baseline /tmp/baseline-shaders.cpp --report /tmp/sunlight-results.json
```

## What is checked

- Eleven production vertex/fragment shader combinations compile and link.
- Normal, detail and grass paths each render five controlled cases: curved normals,
  planar normals, ambient only, sunlight disabled and prelit screen vertices.
- At the triangle centre, interpolated normals point toward the sun. New diffuse
  intensity is 0.7; vertex-interpolated Lambert intensity was 0.4. Other tested
  cases preserve the baseline, including existing alpha behavior.
- Float pixel assertions use tolerance 1e-5 and fail the process on mismatch.

## Scope and remaining acceptance

This changes **mesh sun diffuse**, not textures, vegetation density or weather.
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

As of 2026-10-16, local shader tests pass; the full engine build and demo A/B run
are **not completed**. Dependency and asset jobs were interrupted by an execution
service approval cancellation, including after the user authorized resumption.
