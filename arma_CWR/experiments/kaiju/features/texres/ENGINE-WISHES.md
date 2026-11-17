# Texture resolution: wishes for the engine and the pipeline

Evidence, runs and pictures: `D:\Arma_CWA_worktrees\kaiju-notes\features\texres\NOTES.md`.
The standard for the artists: `TEXTURE-BUDGET.md` in the same folder. Line numbers are from the
cryptid worktree, the source of the test binary.

## Engine, in order of worth

1. **Mip level request for big objects.** `Poseidon\World\Scene\Object.cpp:880`, `:920`, `:927`:
   the level to keep loaded is chosen from the squared distance to the object's position (for
   Dedushka, the ground between the feet) and from the least dense face on each sheet
   (`Graphics\Rendering\Draw\DrawPoly.cpp:20-118`, `Shape.cpp:1800-1838`). Measured: a camera 10 m
   from the chest, 35 m from the feet, got 16 texels per metre where the screen had room for 110.
   Use the distance to the bounding sphere's surface, as the landscape does for its own shapes
   (`World\Terrain\LandscapeRender.cpp:1182-1183`, `:1317-1318`). Three lines.
   Data-only cure, tested: one hidden triangle per sheet whose three corners share one UV point
   (`gen_texres.py`, `LodBuilder.add_mip_anchors`). The engine then keeps the whole sheet loaded.
2. **More than 7 mip levels.** `#define MAX_MIPMAPS 7` (`Graphics\Textures\TextureBank.hpp:10`) and
   `generateMipmaps(rgba, 8)` (`Graphics\Textures\PAAEncoder.cpp:104`). A 4096 sheet stops at
   64 x 64. Not shown to be visible in stills; low urgency.
3. **A detail layer for a flagged model.** `psDetail` (`PoseidonGL33\EngineGL33_Shaders.cpp:319-430`)
   already multiplies the base by a tiling texture at 32 times the UV; only the landscape sets the
   flag (`Core\Types.hpp:356`). About 8 lines to switch it on for a model with a named property
   (stock detail texture, no fade), 60 to 80 lines for an own texture, scale and distance fade.
   My attempt to show it with a shader override (`detail_demo\psNormal.glsl`) loaded and changed
   nothing on screen; cause not found.
4. **PoseidonTools PAA writer.** It writes a broken file for a level of 16 MiB or more (DXT5 4096,
   ARGB8888 2048: `PAAEncoder.cpp:37-42`, `:124`) and raw ARGB4444 / ARGB1555 / AI88 that the
   engine's reader refuses (`PAAEncoder.cpp:69-85` against `Rendering\Font\Pactext.cpp:875-960`).
   My DXT5 2048 panel drew opaque (its alpha band did not show through); why was not traced.

## Pipeline (`experiments\kaiju\addon\export_dedushka_p3d.py`)

1. `MAX_TEX = 1024` (line 68) to 4096.
2. Write each visual LOD's faces sorted by texture: `.paa` faces draw in file order
   (`Shape\ShapeSetup.cpp:747-753`, `Shape.cpp:1150-1173`), so every run is a draw call.
3. Add a mip anchor triangle per sheet to LOD0 and LOD1 (see 1 above).
4. Run `check_texture_budget.py <model.p3d> <texture folder>` after export.
