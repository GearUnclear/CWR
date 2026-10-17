# Lighting lab

An offline, headless harness that renders a synthetic but OFP-like scene through
the engine's **real GLSL** (extracted from `engine/PoseidonGL33/EngineGL33_Shaders.cpp`)
with the uniform blocks packed exactly as `EngineGL33` packs them. Use it to compare
lighting changes as PNGs, before and after, without game data or a game build.

Requirements: Python 3, `moderngl` (5.13 tested), `numpy`, `Pillow`, and Mesa
(llvmpipe) with EGL. No display is needed. If EGL fails, run under `xvfb-run -a`.

## Quick start

```sh
cd arma_CWR/tests/rendering/lighting_lab
python run_lab.py --out ../../../../tmp/lighting_lab/classic          # 12 images + contact_sheet.png + manifest.json
python run_lab.py --out OUT --source HEAD~5                            # shaders from a git revision
python run_lab.py --out OUT --source some/other/EngineGL33_Shaders.cpp # shaders from any file
python run_lab.py --out OUT --camera ground --lighting golden,noon     # a subset
python compare.py BEFORE_DIR AFTER_DIR --out CMP_DIR --diff            # BEFORE | AFTER (| diff) pairs + sheet
```

Each run prints the time per image (about 25 to 60 ms on llvmpipe at 960x540 after a
first frame of about 0.5 to 1 s that builds the GPU resources). The exit code is 2 if
any frame produced non-finite values.

## Files

| File | Role |
|---|---|
| `glsl_extract.py` | Pulls `R"(...)"` / `R"delim(...)delim"` shader strings out of C++ by variable name, from a path or `git show <rev>:<path>`. `--dump DIR` writes `vsTransform.glsl`, `psNormal.glsl`, ... in the engine's override naming. |
| `lighting.py` | Python port of `LightSun::Recalculate`, `Scene::MainLightChanged`, `Engine::InitDraw` (accom = brightness, default 1.6), `CreateMaterial*` + `TexMaterial::Combine`, `UploadVSMaterialConstants`, `DoSkyLighting`, the sun-object colours and the night-eye coefficients. Lighting presets live here. |
| `textures.py` | Deterministic, tileable numpy textures: grass, dirt, detail (grey, alpha 0.5 +- 0.15), plaster with window, roof tiles, olive paint, bark, foliage RGBA cutout, rock, crate, uniform, skin, rubber, sky (clear to overcast), sun disc/halo, horizon band. |
| `scene.py` | World-space geometry (X east, Y up, Z north, left-handed, metres): terrain, 2 houses, an APC, 26 trees, a soldier, rocks, crates, the sky dome, the sun billboards and the far-plane horizon polygon. Camera presets live here. |
| `presets.py` | Camera x lighting combinations. |
| `gl_common.py` | Context creation, `glClipControl` through ctypes, matrices built like `ConvertMatrix` / `ConvertProjectionMatrix` / `Camera::Adjust`, texture and VAO helpers, block and sampler bindings. |
| `engine_state.py` | CPU shadows of `VSConstants` (280 floats), `PSConstants` (108 floats) and `WorldInstances`, using the C++ slot numbers, plus the C++ upload functions (`upload_frame_constants`, `upload_vs_material_constants`, `upload_vs_texgen_constants`, `set_alpha_test`, ...). |
| `classic.py` | The renderer for the current engine pipeline. |
| `run_lab.py` | CLI: renders presets with a renderer module and writes PNGs, the contact sheet and `manifest.json`. |
| `compare.py` | Side-by-side BEFORE/AFTER pairs per image name, an overall sheet and `compare.json` metrics. |
| `remaster.py` | The renderer for the remastered lighting (`--renderer remaster`, see below). |
| `postfx.py` | The remastered post pass: runs the `EngineGL33_PostFX.cpp` shaders with its constants. |
| `shadowpass.py` | A one-tier sun shadow map of the object casters, filling the engine's cascade constants. |
| `native.py`, `lab_native.cpp` | ctypes bridge that builds the engine's pure remastered C++ (`Graphics/Remaster/*.cpp`: light rig, horizon map, cloud noise, crown ellipsoid) into a shared library under the repo-root `tmp/` and calls it. Needs `clang++` or `g++`. |
| `rawgl.py` | The few raw GL calls moderngl lacks: depth texture arrays, comparison samplers, sampler-unit uniforms. |

## What the classic renderer reproduces

Draw order and state follow `Landscape::Draw` / `DrawRect` and the GL33 backend:

1. Clear to the fog colour (`World.cpp`: `InitDraw(clear, FogColor())`).
2. Sky dome: per-vertex colour from `DoSkyLighting` (`(SunSkyColor * cos^6 + SkyColor) * accom`, packed to 8 bits) times the sky texture, through `s_psNormalGLSL`. Depth, fog and culling are off.
3. Sun halo and sun disc: `Sun{,Halo}ObjectColor * accom`, alpha times SkyThrough, alpha blended with an alpha test at ref 1, no depth.
4. Terrain: `s_vsTransformGLSL` + `s_psDetailGLSL`, `TGDetail` texgen (`texCtrl {0,1,0,0}`, `texMat1` = scale 32), detail texture on unit 1, one texture per 50 m square with `uv = world_xz / 50`.
5. Horizon (`Landscape::DrawHorizont`): an 8-point far-plane band with an MSInShadow (ambient-only) material, alpha blended, depth read-only and fogged.
6. Opaque objects: `s_vsTransformGLSL` + `s_psNormalGLSL`, culling back faces with `glFrontFace(GL_CW)`.
7. Cutout foliage: same shaders with the alpha test on at ref `0xC0` and no alpha-to-coverage.
8. Gamma: `s_vsGammaGLSL` + `s_psGammaGLSL` with `invGamma = 1/1.2`, reading an **RGBA8** scene target, so output is clamped as in the engine's default framebuffer.

Uniforms follow the engine:
- Rendering is camera-relative: the world matrix holds `-cameraPos`, the view matrix has zero translation and the `camPos` uniform is 0.
- The projection uses `fov 0.7`, `cNear = clamp(0.067/fov, 0.07, 0.2)`, the far plane at the fog end and `glClipControl(LOWER_LEFT, ZERO_TO_ONE)`.
- `PSConstants` holds the fog colour (c0), alphaRef (c1), shadows off (c2), white `constColor` (c3) and `rgbEyeCoef` (c7).
- Material constants are `diffuse = sunDiffuse * mat.diffuse` and `ambient = sunAmbient * mat.ambient + sunDiffuse * mat.forcedDiffuse`. Materials are `accom * white`, with these exceptions:
  - Foliage adds `forcedDiffuse = 0.5 * accom` (`scene.FOLIAGE_MATERIAL`; the engine's MSFullLighted variant is a one-line switch).
  - The vehicle has specular 0.2, power 20.
- Fog is linear, from 100 m to 900 m.

`manifest.json` records for every image:
- the sun, ambient, fog and sky values
- the exact VS material constants of each mesh
- the linked block sizes (1120 / 432 / 16384 bytes)
- whether clip control was active
- the count of non-finite values from a second render into an RGBA32F target.

## Presets

- Cameras:
  - `ground`: 1.8 m eye height. The house is at about 28 m, the APC at about 15 m and the sun is to the left, so lit and shaded faces both show.
  - `overview`: 40 m up, looking toward azimuth 235 (the sun azimuth) across the hills.
  - `closeup`: the APC from 6 m, at a front three-quarter angle.
- Lighting:
  - `noon`: elevation 60
  - `afternoon`: elevation 30
  - `golden`: elevation 7
  - `overcast`: elevation 45, skyThrough 0.2.

All four use azimuth 235, brightness 1.6 and gamma 1.2.

## Comparing a lighting change

- **Engine source change:** `run_lab.py --source <old rev>` and `run_lab.py` on the working tree, then `compare.py`.
- **GLSL-only experiment:** `glsl_extract.py --dump DIR`, edit `DIR/psNormal.glsl` (and the others), then `run_lab.py --shader-override-dir DIR`. The same directory works with the game's `CWR_SHADER_OVERRIDE_DIR`.
- **New pipeline (new uniforms, new passes, post-processing):** write a renderer module that exposes
  `render(ctx, scene, preset, width, height) -> PIL.Image` (and optionally `configure(**opts)`
  and `NAME`), then run `run_lab.py --renderer mymodule` or `--renderer path/to/mymodule.py`.
  Reuse `scene`, `lighting`, `gl_common` and `engine_state` (copy `classic.draw_scene` as a
  starting point). Keep the output image names (`<camera>_<lighting>.png`) so that
  `compare.py` pairs them.

## The remastered renderer

`--renderer remaster` draws the same scene through the remastered lighting
(`engine/Poseidon/Graphics/Remaster/README.md`): the engine's remastered GLSL
(`EngineGL33_LightingShaders.cpp`), its post pass (`EngineGL33_PostFX.cpp`),
and its C++ light rig, terrain horizon map, cloud noise and crown ellipsoid
through `native.py`. `LitConstants`, the crown constants (VS slots 22-23) and
the post and lighting constants (`constexpr float k...` in
`EngineGL33_Lighting.cpp` and `EngineGL33_PostFX.cpp`) are read from or mirror
the engine, so a change to either shows up here without editing the lab.

```sh
python run_lab.py --renderer remaster --lighting noon,afternoon,lowsun,golden,broken,overcast --out OUT
python run_lab.py --renderer remaster --tuning sun=1.1,bloom=0 --out OUT    # LightingTuning overrides
python run_lab.py --renderer remaster --debug-view 1 --no-post --out OUT    # 1 sun / 2 sky visibility, 3 normals, 4 AO
python compare.py CLASSIC_OUT OUT --out CMP --labels CLASSIC REMASTERED
```

Extra presets for it: `lowsun` (14 degrees, azimuth 195) and `broken`
(40 degrees, skyThrough 0.6, cloud shadows). The terrain horizon map covers a
3.2 km window around the scene at 12.5 m.

It adds to what the classic renderer reproduces: a sun shadow map of the object
casters (one tier instead of the engine's cascades), terrain hill shadows and
valley sky occlusion, cloud shadows, foliage crown normals, the sun-glow fog and
the post pass (AO, bloom, sun shafts) on a depth texture. It still does not
cover water, local lights, MSAA or alpha-to-coverage, cockpits, particles or
night vision.

## What this validates, and what it does not

It **does** validate that:
- The engine's shader text compiles and links (GLSL 330 core, as Mesa sees it) and behaves as intended for representative inputs.
- The uniform-block layout and packing match the C++ slot map.
- The sun, ambient, fog and material maths are as ported, including the VS clamp of lit colour before texturing and the 8-bit framebuffer.
- Relative changes in tone, contrast, fog and sky between two pipelines show up on a fixed, deterministic scene. Output is bit-exact across runs on the same Mesa.

It does **not**:
- Use game assets. Textures, models, the sky and horizon textures and every colour are procedural stand-ins, so absolute colours are not those of CWA.
- Reproduce GPU or driver differences. llvmpipe is the only rasterizer, with no anisotropic filtering (the engine uses 16x) and no MSAA or alpha-to-coverage.
- Cover these engine features:
  - classic shadows (no cascades, no per-poly or stencil shadows; the remastered renderer has one shadow-map tier)
  - clouds, stars, the moon and water
  - local and night lights (the light count is 0)
  - LOD, grass layers (`CanGrass()` is false in GL33 anyway), roads, decals, particles, cockpit and UI.
- Replicate the CPU TL path for the sky and sun. Their colours are computed exactly on the CPU, but they are transformed on the GPU by a lab-only `PRELIT_VS` instead of the engine's CPU projection + `s_vsScreenGLSL`. The sky pass's own clip range (near 10 m, far 4000 m) and the camera-height user clip plane are approximated by a hemisphere 500 m out.
- Model exact original data. The UV layouts and texture contents of `obloha.p3d` / `horizont.p3d` and the sun/halo shapes are guesses. `Scene::_skyColor` is read from pixel (1,1) of the lab sky texture, which is laid out horizon-first.
