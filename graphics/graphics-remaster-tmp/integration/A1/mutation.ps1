# Gate item 3: the new contract tests must fail with the contract broken.
# Four mutations at once (each hits different cases), build PoseidonTests,
# run the new tags, restore the sources, rebuild.
$ErrorActionPreference = 'Continue'
$env:VCPKG_ROOT='C:\dev\vcpkg'
$env:Path='C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64;'+$env:Path
$root = 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR'
$a1 = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\A1'
$bak = Join-Path $a1 'mutation-bak'
New-Item -ItemType Directory -Force $bak | Out-Null
$muts = @(
    @{ f = 'engine\Poseidon\Graphics\Remaster\FeatureSwitches.cpp'; old = '    if (mode == LightingMode::Baseline)'; new = '    if (false && mode == LightingMode::Baseline)' },
    @{ f = 'engine\Poseidon\Graphics\Remaster\FeatureGates.hpp'; old = 'constexpr float kGateSunShadowMinSunDeg = 4.0f;'; new = 'constexpr float kGateSunShadowMinSunDeg = 2.0f;' },
    @{ f = 'engine\Poseidon\Core\Game\GameLoop.cpp'; old = '    if (gFixedSimStep > 0.0f)'; new = '    if (false && gFixedSimStep > 0.0f)' },
    @{ f = 'engine\Poseidon\Game\Commands\GameStateExtTestRender.cpp'; old = '    GEngine->PinCloudDrift(pin, x, z);'; new = '    GEngine->PinCloudDrift(false, x, z);' }
)
foreach ($m in $muts) {
    $p = Join-Path $root $m.f
    Copy-Item $p (Join-Path $bak ([IO.Path]::GetFileName($p))) -Force
    $s = [IO.File]::ReadAllText($p)
    if (-not $s.Contains($m.old)) { "MISSING anchor in $($m.f)"; continue }
    [IO.File]::WriteAllText($p, $s.Replace($m.old, $m.new))
    "mutated $($m.f)"
}
Set-Location $root
cmake --build build/graphics-remaster-rwdi -j 12 --target PoseidonTests | Select-Object -Last 3
"build (mutated) exit: $LASTEXITCODE"
Set-Location D:\
& "$root\build\graphics-remaster-rwdi\tests\unit\engine\Poseidon\PoseidonTests.exe" "[FeatureSwitches],[FeatureGates],[triSetFixedStep],[triPinCloudDrift],[triSetLighting]" --reporter compact 2>&1 | Select-String -Pattern 'failed|passed' | Select-Object -Last 40
"tests (mutated) exit: $LASTEXITCODE"
foreach ($m in $muts) {
    $p = Join-Path $root $m.f
    Copy-Item (Join-Path $bak ([IO.Path]::GetFileName($p))) $p -Force
    # a copy keeps the old timestamp, older than the mutated objects: touch it
    (Get-Item $p).LastWriteTime = Get-Date
    "restored $($m.f)"
}
Set-Location $root
cmake --build build/graphics-remaster-rwdi -j 12 --target PoseidonTests | Select-Object -Last 2
"rebuild exit: $LASTEXITCODE"
Set-Location D:\
& "$root\build\graphics-remaster-rwdi\tests\unit\engine\Poseidon\PoseidonTests.exe" "[FeatureSwitches],[FeatureGates],[triSetFixedStep],[triPinCloudDrift],[triSetLighting]" 2>&1 | Select-Object -Last 2
"tests (restored) exit: $LASTEXITCODE"
