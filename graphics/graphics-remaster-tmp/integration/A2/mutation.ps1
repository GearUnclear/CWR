# Gate item 3: each new contract test must fail with its contract broken.
# One mutation at a time: apply it, build PoseidonTests only (the game in dist
# is not relinked), run [remaster], record the failing cases, restore the
# source byte for byte. A last rebuild and run checks the restored state.
$ErrorActionPreference = 'Continue'
$env:VCPKG_ROOT='C:\dev\vcpkg'
$env:Path='C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64;'+$env:Path
$root = 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR'
$gl = 'engine\PoseidonGL33'
$tests = "$root\build\graphics-remaster-rwdi\tests\unit\engine\Poseidon\PoseidonTests.exe"
$nl = "`n"
$muts = @(
    @{ id = 'M1 drop the cloudNoise row of kLitSamplers'; f = "$gl\EngineGL33_Lighting.hpp";
       old = '    {"cloudNoise", kCloudNoiseUnit},' + $nl; new = '' },
    @{ id = 'M2 a stale hand copy of the LitConstants block in EngineGL33_PostShaders.cpp'; f = "$gl\EngineGL33_PostShaders.cpp";
       old = 'static const char s_vsPostGLSL[]';
       new = 'static const char s_psStaleCopyGLSL[] = R"(' + $nl + 'layout(std140) uniform LitConstants {' + $nl + '    vec4 litSunDir;' + $nl + '    vec4 litSun;' + $nl + '};' + $nl + ')";' + $nl + $nl + 'static const char s_vsPostGLSL[]' },
    @{ id = 'M3 Sky and Lights swapped in kFragmentPieces'; f = "$gl\EngineGL33_LightingShaders.cpp";
       old = '    kRemasterSkyGLSL,          // Sky' + $nl + '    kRemasterLightsGLSL,       // Lights' + $nl;
       new = '    kRemasterLightsGLSL,       // Lights' + $nl + '    kRemasterSkyGLSL,          // Sky' + $nl },
    @{ id = 'M4 psPostAo without s_glslPostCommon'; f = "$gl\EngineGL33_PostShaders.cpp";
       old = '{s_psPostAoInputs, s_glslPostCommon, s_psPostAoMain}'; new = '{s_psPostAoInputs, s_psPostAoMain}' },
    @{ id = 'M5 litFogSun and litFogCap swapped in the GLSL block'; f = "$gl\EngineGL33_LightingShaders.cpp";
       old = '    vec4 litFogSun;       // 24: rgb sky-dome sun glow added to the fog (display), w enabled' + $nl + '    vec4 litFogCap;       // 25: rgb fog colour limit (the sky texture colour)' + $nl;
       new = '    vec4 litFogCap;       // 25: rgb fog colour limit (the sky texture colour)' + $nl + '    vec4 litFogSun;       // 24: rgb sky-dome sun glow added to the fog (display), w enabled' + $nl }
)
foreach ($m in $muts) {
    "=== $($m.id)"
    $p = Join-Path $root $m.f
    $orig = [IO.File]::ReadAllBytes($p)
    $s = [Text.Encoding]::UTF8.GetString($orig)
    if (-not $s.Contains($m.old)) { "MISSING anchor in $($m.f)"; continue }
    $idx = $s.IndexOf($m.old)
    $mut = $s.Substring(0, $idx) + $m.new + $s.Substring($idx + $m.old.Length)
    [IO.File]::WriteAllText($p, $mut, (New-Object Text.UTF8Encoding($false)))
    try {
        Set-Location $root
        $b = cmake --build build/graphics-remaster-rwdi -j 12 --target PoseidonTests 2>&1
        "build exit: $LASTEXITCODE"
        if ($LASTEXITCODE -ne 0) { $b | Select-String -Pattern 'error' | Select-Object -First 5 | ForEach-Object { $_.Line } }
        Set-Location D:\
        $r = & $tests "[remaster]" --reporter compact 2>&1
        "tests exit: $LASTEXITCODE"
        $r | Select-String -Pattern '^D:.*: failed:' | ForEach-Object { ($_.Line -split ': failed:')[0] } | Sort-Object -Unique | ForEach-Object { "  failed at $_" }
        $r | Select-Object -Last 2
    } finally {
        [IO.File]::WriteAllBytes($p, $orig)
        (Get-Item $p).LastWriteTime = Get-Date
        "restored $($m.f)"
    }
}
"=== restored state"
Set-Location $root
cmake --build build/graphics-remaster-rwdi -j 12 --target PoseidonTests 2>&1 | Select-Object -Last 1
"rebuild exit: $LASTEXITCODE"
Set-Location D:\
& $tests "[remaster]" 2>&1 | Select-Object -Last 2
"tests (restored) exit: $LASTEXITCODE"
