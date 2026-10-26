# Reviewer mutations that need a rebuild of PoseidonTests (accessor contracts).
$ErrorActionPreference = 'Continue'
$env:VCPKG_ROOT='C:\dev\vcpkg'
$env:Path='C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64;'+$env:Path
$root = 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR'
$gl = "$root\engine\PoseidonGL33"
$bak = 'C:\Users\Dane\AppData\Local\Temp\claude\D--Arma-CWA\0cfe8c80-fc33-4f8e-a86e-52fa1175350d\scratchpad\orig'
$tests = "$root\build\graphics-remaster-rwdi\tests\unit\engine\Poseidon\PoseidonTests.exe"
$enc = New-Object Text.UTF8Encoding($false)

function Run-Tests {
    Set-Location $root
    $b = cmake --build build/graphics-remaster-rwdi -j 12 --target PoseidonTests 2>&1
    "build exit: $LASTEXITCODE"
    if ($LASTEXITCODE -ne 0) { $b | Select-String -Pattern 'error' | Select-Object -First 5 | ForEach-Object { $_.Line } }
    Set-Location D:\
    $r = & $tests "[remaster]" --reporter compact 2>&1
    "tests exit: $LASTEXITCODE"
    $r | Select-String -Pattern ': failed:' | ForEach-Object { $_.Line.Substring(0, [Math]::Min(260, $_.Line.Length)) }
    $r | Select-Object -Last 2
}
function Restore([string[]]$names) {
    foreach ($n in $names) {
        Copy-Item (Join-Path $bak $n) (Join-Path $gl $n) -Force
        "restored ${n}: " + ((Get-FileHash (Join-Path $gl $n)).Hash -eq (Get-FileHash (Join-Path $bak $n)).Hash)
    }
}

"=== R3 fogColorToward moved from the Sky piece to the head of the Lights piece (join unchanged)"
try {
    $sky = [IO.File]::ReadAllText("$gl\EngineGL33_LightingShadersSky.cpp")
    $i = $sky.IndexOf("`n// Fog toward the sun")
    $j = $sky.IndexOf(')";', $i)
    $fog = $sky.Substring($i, $j - $i)
    [IO.File]::WriteAllText("$gl\EngineGL33_LightingShadersSky.cpp", $sky.Substring(0, $i) + $sky.Substring($j), $enc)
    $lights = [IO.File]::ReadAllText("$gl\EngineGL33_LightingShadersLights.cpp")
    $anchor = 'const char kRemasterLightsGLSL[] = R"('
    $k = $lights.IndexOf($anchor) + $anchor.Length
    [IO.File]::WriteAllText("$gl\EngineGL33_LightingShadersLights.cpp", $lights.Substring(0, $k) + $fog + $lights.Substring($k), $enc)
    Run-Tests
} finally { Restore @('EngineGL33_LightingShadersSky.cpp', 'EngineGL33_LightingShadersLights.cpp') }

"=== R4 the shadowMap row dropped from kLitSamplers (the old separate bind)"
try {
    $h = [IO.File]::ReadAllText("$gl\EngineGL33_Lighting.hpp")
    $row = '    {"shadowMap", kShadowMapUnit},' + "`n"
    if (-not $h.Contains($row)) { "MISSING anchor" }
    [IO.File]::WriteAllText("$gl\EngineGL33_Lighting.hpp", $h.Replace($row, ''), $enc)
    Run-Tests
} finally { Restore @('EngineGL33_Lighting.hpp') }

"=== restored state"
Set-Location $root
cmake --build build/graphics-remaster-rwdi -j 12 2>&1 | Select-Object -Last 2
"full rebuild exit: $LASTEXITCODE"
Set-Location D:\
& $tests "[remaster]" 2>&1 | Select-Object -Last 2
"tests (restored) exit: $LASTEXITCODE"
