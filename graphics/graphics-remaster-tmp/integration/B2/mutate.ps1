# B2 mutation check: each mutation breaks one contract of the change; the
# unit tests must turn red. Builds only PoseidonTests, restores every file.
$ErrorActionPreference = 'Stop'
$env:VCPKG_ROOT = 'C:\dev\vcpkg'
$env:Path = 'C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64;' + $env:Path
$root = 'D:\Arma_CWA_worktrees\gr-B2\arma_CWR'
$rig = "$root\engine\Poseidon\Graphics\Remaster\LightRig.cpp"
$sh = "$root\engine\PoseidonGL33\EngineGL33_LightingShaders.cpp"
$lit = "$root\engine\PoseidonGL33\EngineGL33_Lighting.cpp"
$fs = "$root\engine\Poseidon\Graphics\Remaster\FeatureSwitches.cpp"
$muts = @(
    @{ name = 'switch ignored'; file = $rig; old = 'const bool lowSunRig = tuning.features.lowSunRig > 0.0f;'; new = 'const bool lowSunRig = false;' },
    @{ name = 'fill without the switch'; file = $rig; old = 'const float fill = lowSunRig ? LowSunFill(elevation) : 0.0f;'; new = 'const float fill = LowSunFill(elevation);' },
    @{ name = 'hue without the switch'; file = $rig; old = 'const Rgb sunChroma = lowSunRig ? LowSunChroma'; new = 'const Rgb sunChroma = true ? LowSunChroma' },
    @{ name = 'no twilight limit'; file = $rig; old = 'if (fill > 0.0f)'; new = 'if (fill > 1e9f)' },
    @{ name = 'hue from 35 deg'; file = $rig; old = '0.4f * (1.0f - Smoothstep(6.0f, 25.0f, elevationDeg))'; new = '0.4f * (1.0f - Smoothstep(6.0f, 35.0f, elevationDeg))' },
    @{ name = 'anti-sun fill 1.0'; file = $rig; old = '(1.0f + kLowSunFillAntiSun * fill)) +'; new = '(1.0f + 1.0f * fill)) +' },
    @{ name = 'canopy ungated'; file = $sh; old = 'bool canopy = crown && !terrain && litHorizonSun.w > 0.5;'; new = 'bool canopy = crown && !terrain;' },
    @{ name = 'canopy on grass'; file = $sh; old = 'bool canopy = crown && !terrain && litHorizonSun.w > 0.5;'; new = 'bool canopy = crown && litHorizonSun.w > 0.5;' },
    @{ name = 'off core changed'; file = $sh; old = 'mix(canopy ? 0.45 : 0.6, 1.0,'; new = 'mix(0.45, 1.0,' },
    @{ name = 'canopy from the switch'; file = $lit; old = '_featureGates[rm::Feature::CrownCanopy].on ? 1.0f : 0.0f;'; new = '_lighting.features.crownCanopy > 0.0f ? 1.0f : 0.0f;' },
    @{ name = 'albedo view unmapped'; file = $rig; old = 'return debugView == kDebugViewAlbedo ? kShaderDebugAlbedo : 0.0f;'; new = 'return 0.0f;' },
    @{ name = 'shade not landed'; file = $fs; old = '"B2", true, &FS::lowSunRig'; new = '"B2", false, &FS::lowSunRig' }
)
$report = @()
foreach ($m in $muts) {
    $orig = [IO.File]::ReadAllText($m.file)
    if (-not $orig.Contains($m.old)) { $report += "$($m.name): PATTERN NOT FOUND"; continue }
    $idx = $orig.IndexOf($m.old)
    $mut = $orig.Substring(0, $idx) + $m.new + $orig.Substring($idx + $m.old.Length)
    [IO.File]::WriteAllText($m.file, $mut)
    try {
        Push-Location $root
        cmake --build build/gr-B2-rwdi --target PoseidonTests -j 8 *> "$env:TEMP\b2mut-build.log"
        $b = $LASTEXITCODE
        Pop-Location
        if ($b -ne 0) { $report += "$($m.name): BUILD FAILED"; continue }
        Push-Location D:\
        $out = & "$root\build\gr-B2-rwdi\tests\unit\engine\Poseidon\PoseidonTests.exe" "[remaster],[FeatureSwitches],[PostStages]" 2>&1
        Pop-Location
        $summary = ($out | Select-String -Pattern 'test cases:|All tests passed' | Select-Object -Last 1).Line
        $failed = $out | Select-String -Pattern '^[^\s].*(FAILED|failed)' | Select-Object -First 0
        $names = @()
        for ($i = 0; $i -lt $out.Count; $i++) {
            if ($out[$i] -match '^-{20,}' -and $i + 1 -lt $out.Count -and $out[$i + 1] -notmatch '^\s*$' -and $out[$i + 1] -notmatch '^-') { $names += $out[$i + 1].ToString().Trim() }
        }
        $names = $names | Select-Object -Unique
        $report += "$($m.name): $summary :: " + ($names -join ' | ')
    } finally {
        [IO.File]::WriteAllText($m.file, $orig)
    }
}
Push-Location $root
cmake --build build/gr-B2-rwdi --target PoseidonTests -j 8 *> "$env:TEMP\b2mut-build.log"
Pop-Location
$report | Out-File -FilePath 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\B2\mutations.txt' -Encoding utf8
$report
