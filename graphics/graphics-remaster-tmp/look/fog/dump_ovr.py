"""Dump the fog shaders (and the remastered prelude) as override files:
    python dump_ovr.py <outdir> [name ...]
The engine reads <outdir>/<name>.glsl in place of a built-in shader of that
name when CWR_SHADER_OVERRIDE_DIR points there (run.ps1 -Override)."""
import sys
from pathlib import Path
sys.path.insert(0, r"D:\Arma_CWA_worktrees\look-fog\arma_CWR\tests\rendering\lighting_lab")
import glsl_extract as g  # noqa: E402

out = Path(sys.argv[1])
names = sys.argv[2:] or ["psFogInject", "psFogIntegrate", "psFogApply"]
out.mkdir(parents=True, exist_ok=True)
post = g.load_programs(r"D:\Arma_CWA_worktrees\look-fog\arma_CWR\engine\PoseidonGL33\EngineGL33_PostShaders.cpp")
lit = g.load_programs(r"D:\Arma_CWA_worktrees\look-fog\arma_CWR\engine\PoseidonGL33\EngineGL33_LightingShaders.cpp")
for n in names:
    src = post.get(n) or lit.get(n)
    (out / f"{n}.glsl").write_text(src, encoding="utf-8", newline="\n")
    print(n, len(src))
