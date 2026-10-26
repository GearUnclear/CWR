import pathlib

p = pathlib.Path(r"D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR\engine\Poseidon\Graphics\Remaster\research\INTEGRATION-PLAN.md")
s = p.read_text(encoding="utf-8")

# 3.3 as built
a33 = """79 vec4 is 1,264 bytes, far below the GL minimum uniform block size. Combat's reprojection rows
(prototype slots 59-64) are not needed once its shadow becomes a mask pass (B4).
"""
assert a33 in s
s = s.replace(a33, a33 + """
As built in A2: `LitConst` itself is unchanged (`Count` 58): growing it would change the block's
GLSL and the bytes of every remastered program, and A2 changes none. The rows above are a separate
table, `LitConstPlanned` in `EngineGL33_Lighting.hpp` (`constexpr int`, comment per row; a
`static_assert` that they start at `LitConst::Count` and that 79 slots fit the 16 KB GL minimum). The
step that lands a feature moves its row into `LitConst`, declares it in `LitConstantsBlock()` and raises
`Count`; the layout test then checks it. std140 members are contiguous, so a slot that lands before a
lower planned one (A5's `Taa` at 59 before B7's `MatClass` at 58) declares the lower ones as padding
members.
""")

# 3.4 as built
a34 = """`InitPixelShaders` binds sampler names to units from one table (today `litSamplers[]` with three
rows; every prototype appended a row). Every unit that can be sampled gets a complete stand-in
texture at `InitLightingResources`, the same rule F3 applied to unit 2, and a new
`[ShadowSampler]` audit case covers unit 6.
"""
assert a34 in s
s = s.replace(a34, a34 + """
As built in A2: the constants are in `EngineGL33_Lighting.hpp`, with `kShadowMapUnit` 2 for the
cascade array and the four units in use; one `static_assert` checks that every unit is within 2 to 15,
not 7, and distinct. `kLitSamplers` (name, unit) is the one table `InitPixelShaders` binds from, for
classic and remastered programs alike; it took the `shadowMap` row, which was a literal 2 in
`EngineGL33_Shaders.cpp`. The other literal 2s for the shadow unit (`EngineGL33_Lighting.cpp`,
`EngineGL33_ShadowDepth.cpp` `EnsureArrayTarget`, `EngineGL33_Shaders.cpp` `UploadPSShadowConstants`)
are `kShadowMapUnit` now, and the two F3 audits in `test_gl_state_cache_audit.cpp` look for
`GL33Bind::ActiveUnit(kShadowMapUnit);`. Not in the table: `BindLightingTextures` still binds units 2
to 5 by name (each unit has its own stand-in and texture target), and the post pass keeps its own
units (`{0, 2, 3, 4, 5}` in `EndWorldScene`), which are A3's. `GL33Bind` caches units 0 to 7 only
(`kUnits = 8` in `GL33BindCache.cpp`), so units 8 to 15 bind on every call. The `[ShadowSampler]` case
for unit 6 belongs to the step that first samples it (A5 or B1).
""")

# 3.5 as built
a35 = """`s_glslPostCommon` holds depth
  reconstruction (`viewZ`, `viewPos` with `projInfo`, `depthMap`, `worldRect`), interleaved
  gradient noise, cascade lookup and the terrain and cloud visibility helpers that volumetric,
  ssr and combat each rewrote."""
i35 = s.index("### 3.6 Post pipeline stages")
add35 = """As built in A2 (corrections to the bullets above):

- There is no single `s_psRemasterCommon` string any more: the LitConstants block sits between the
  `PSConstants` header and the samplers, so the prelude is ten pieces (`FragmentPiece`: header,
  LitConstants, inputs, shadows, terrain and clouds, sky, lights, surface, shade, finish), joined by
  the table `kFragmentPieces`. `RemasterFragmentCommon()` returns the join and
  `RemasterFragmentPiece()` one piece. The new files are `EngineGL33_LightingShadersSky.cpp`,
  `EngineGL33_LightingShadersLights.cpp` and `EngineGL33_LightingShadersSurface.cpp`; every piece
  after the header starts with the blank line before it, so the join is byte for byte the tip's
  `s_psRemasterCommon`.
- "PoseidonTests links PoseidonGL33" holds only for objects without engine code: the first A2 build
  pulled `EngineGL33_PostFX.cpp.obj` in through the post accessors, which pulled
  `EngineGL33_Lifecycle.cpp.obj`, whose `CreateEngineGL33` collides with the test stub
  (`test_stubs.cpp:101`, lld-link duplicate symbol). So the post GLSL moved to
  `EngineGL33_PostShaders.cpp` (no GL code, the `kPostShaders` table, `PostShaderName` and
  `PostShaderSource`), and `EngineGL33_PostFX.cpp` keeps the pass code. A post pass added later puts
  its GLSL in a GL-free file (`EngineGL33_PostShaders.cpp` or its own `EngineGL33_Post<Name>Shaders.cpp`)
  and adds its row to `kPostShaders`; its pass code goes where 3.6 says.
- `s_glslPostCommon` holds only the depth reconstruction AO uses (`viewZ(vec2)`, `viewPos`), and the
  pass declares the four inputs it reads (`depthTex`, `projInfo`, `depthMap`, `worldRect`): AO declares
  its own `aoParams` and `screen` between them, so a shared declaration block would reorder AO's
  uniforms and change its bytes. Interleaved gradient noise, the cascade lookup and the projection
  helpers join it with their first post consumer (A3's linear depth, A5, B6, B8); the noise is still
  written inline in AO, the shafts and the composite. The terrain and cloud helpers need no rewrite:
  the TerrainClouds piece takes positions, so a post pass pastes `LitConstantsBlock()`, the samplers
  and `RemasterFragmentPiece(FragmentPiece::TerrainClouds)`.
- `s_glslLitConstantsBlock` is pasted into the prelude only; no vertex or post shader reads the rig
  yet (A7 is the first).
- Tests: the compile case goes through `RemasterVertexSource`, `RemasterFragmentCommon` plus each
  `RemasterFragmentMain`, and `PostShaderSource`; pieces do not compile alone, so "every `s_ps*` /
  `s_vs*` symbol" became a separate case that every raw string of `EngineGL33_LightingShaders*.cpp`
  and `EngineGL33_Post*.cpp` sits inside a compiled program. The layout case checks every
  `uniform LitConstants {` in `engine/PoseidonGL33/*.{cpp,hpp}` member by member and that there is one.
  Two more cases pin the assembly tables as the lighting lab reads them (`kFragmentPieces`,
  `kPostShaders` parsed from the source give exactly what the accessors return) and the concern of each
  piece; one checks the unit table. The lab's `glsl_extract.py` assembles the same way
  (`load_programs`), so `--dump` writes `psRemasterCommon.glsl` whole, and it now decodes
  `git show` output as UTF-8 (it used the console code page, which mangled the tip's comments).

"""
s = s[:i35] + add35 + s[i35:]

# hot spot row
a6 = "| `EngineGL33_PostFX.cpp` | F5, combat, ssr, volumetric, materials, night, shadows, texsharp, temporal | A3 turns it into stages; every feature adds its own `EngineGL33_Post*.cpp`. Only A3 and A5 touch the stage list itself; later steps register a stage. |"
assert a6 in s
s = s.replace(a6, "| `EngineGL33_PostFX.cpp` | F5, combat, ssr, volumetric, materials, night, shadows, texsharp, temporal | A3 turns it into stages; every feature adds its own `EngineGL33_Post*.cpp`, with its GLSL in a GL-free file and a `kPostShaders` row (A2, 3.5). Only A3 and A5 touch the stage list itself; later steps register a stage. |")
p.write_text(s, encoding="utf-8", newline="\n")
print("ok")
