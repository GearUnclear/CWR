# Reviewer mutations that the tests see by reading the sources at run time
# (no rebuild). One at a time; each file restored byte for byte from the backup.
$ErrorActionPreference = 'Continue'
$gl = 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR\engine\PoseidonGL33'
$bak = 'C:\Users\Dane\AppData\Local\Temp\claude\D--Arma-CWA\0cfe8c80-fc33-4f8e-a86e-52fa1175350d\scratchpad\orig'
$tests = 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR\build\graphics-remaster-rwdi\tests\unit\engine\Poseidon\PoseidonTests.exe'
$nl = "`n"
$block = 'static const char s_psStaleBlockGLSL[] = R"(' + $nl + '// Remastered light rig and per-draw material, binding 3. Layout mirrors' + $nl
$muts = @(
    @{ id = 'R1 an orphan raw string in the Surface file (no table names it)'; f = 'EngineGL33_LightingShadersSurface.cpp';
       old = '} // namespace Poseidon::gl33lighting';
       new = 'const char kRemasterWetGLSL[] = R"(' + $nl + 'float wetness() { return 0.0; }' + $nl + ')";' + $nl + $nl + '} // namespace Poseidon::gl33lighting' },
    @{ id = 'R2 an exact hand copy of the LitConstants block in PostShaders'; f = 'EngineGL33_PostShaders.cpp';
       old = 'static const char s_vsPostGLSL[]'; new = 'COPY' },
    @{ id = 'R6 InitLightingResources parks the depth stand-in on the horizon unit'; f = 'EngineGL33_Lighting.cpp';
       old = '    GL33Bind::ActiveUnit(kShadowMapUnit);' + $nl + '    glBindTexture(GL_TEXTURE_2D_ARRAY, _dummyDepthArray);';
       new = '    GL33Bind::ActiveUnit(kTerrainHorizonUnit);' + $nl + '    glBindTexture(GL_TEXTURE_2D_ARRAY, _dummyDepthArray);' }
)
# R2: copy the shared block text verbatim out of LightingShaders.cpp.
$ls = [IO.File]::ReadAllText("$gl\EngineGL33_LightingShaders.cpp")
$a = $ls.IndexOf('static const char s_glslLitConstantsBlock[] = R"(')
$b = $ls.IndexOf(')";', $a)
$body = $ls.Substring($a + 'static const char s_glslLitConstantsBlock[] = R"('.Length, $b - $a - 'static const char s_glslLitConstantsBlock[] = R"('.Length)
foreach ($m in $muts) {
    "=== $($m.id)"
    $p = Join-Path $gl $m.f
    $orig = [IO.File]::ReadAllBytes($p)
    $s = [Text.Encoding]::UTF8.GetString($orig)
    if ($s.Replace("`r`n", "`n") -ne $s) { "file has CRLF" }
    if (-not $s.Contains($m.old)) { "MISSING anchor in $($m.f)"; continue }
    $new = $m.new
    if ($new -eq 'COPY') { $new = 'static const char s_psCopyBlockGLSL[] = R"(' + $body + ')";' + $nl + $nl + 'static const char s_vsPostGLSL[]' }
    $idx = $s.IndexOf($m.old)
    $mut = $s.Substring(0, $idx) + $new + $s.Substring($idx + $m.old.Length)
    [IO.File]::WriteAllText($p, $mut, (New-Object Text.UTF8Encoding($false)))
    try {
        Set-Location D:\
        $r = & $tests "[remaster],[ShadowSampler],[StateCacheAudit],[Graphics]" --reporter compact 2>&1
        "tests exit: $LASTEXITCODE"
        $r | Select-String -Pattern ': failed:' | ForEach-Object { $_.Line.Substring(0, [Math]::Min(260, $_.Line.Length)) }
        $r | Select-Object -Last 2
    } finally {
        Copy-Item (Join-Path $bak $m.f) $p -Force
        "restored $($m.f): " + ((Get-FileHash $p).Hash -eq (Get-FileHash (Join-Path $bak $m.f)).Hash)
    }
}
