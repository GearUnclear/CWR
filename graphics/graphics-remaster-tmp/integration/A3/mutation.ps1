# Gate item 3: each new contract test must fail with its contract broken.
# One mutation at a time: apply it, build PoseidonTests only (the game in dist
# is not relinked), run the A3 tags, record the failing cases, restore the
# source byte for byte. A last rebuild and run checks the restored state.
$ErrorActionPreference = 'Continue'
$env:VCPKG_ROOT='C:\dev\vcpkg'
$env:Path='C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64;'+$env:Path
$root = 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR'
$gl = 'engine\PoseidonGL33'
$rmd = 'engine\Poseidon\Graphics\Remaster'
$core = 'engine\Poseidon\Graphics\Core'
$tests = "$root\build\graphics-remaster-rwdi\tests\unit\engine\Poseidon\PoseidonTests.exe"
$tags = '[PostStages],[gpu],[remaster]'
$nl = "`n"
$muts = @(
    @{ id = 'M1 the linear depth runs on every pass (no reader asked)'; f = "$rmd\PostStages.cpp";
       old = '    plan.runs &= LandedMask();'; new = '    plan.runs |= Bit(PostStage::LinearDepth);' + $nl + '    plan.runs &= LandedMask();' },
    @{ id = 'M2 the early return asks the frame-gated strengths, not the knobs'; f = "$rmd\PostStages.cpp";
       old = 'const bool asked = in.ambientOcclusion > 0.0f || in.bloom > 0.0f || in.sunShafts > 0.0f || debugDepth;';
       new = 'const bool asked = (in.worldDepth && in.ambientOcclusion > 0.0f) || (in.glow && in.bloom > 0.0f) || (in.worldDepth && in.glow && in.sunShafts > 0.0f && in.shaftWeight > kShaftMinWeight) || debugDepth;' },
    @{ id = 'M3 the post section ends at the composite again'; f = "$core\GpuTimings.cpp";
       old = '{"post", Mark::WorldEnd, Mark::PostEnd},'; new = '{"post", Mark::WorldEnd, Mark::CompositeEnd},' },
    @{ id = 'M4 a stage section that does not fall back to an earlier stage'; f = "$core\GpuTimings.cpp";
       old = '            if (!IsPostStage(s))'; new = '            if (true)' },
    @{ id = 'M5 the shafts stage loses its case in RunPostStage'; f = "$gl\EngineGL33_PostFX.cpp";
       old = '        case rm::PostStage::Shafts:' + $nl + '            PostShafts(f);' + $nl + '            break;' + $nl; new = '' },
    @{ id = 'M6 the linear depth keeps the nearest sample everywhere'; f = "$gl\EngineGL33_PostShaders.cpp";
       old = '((ij.x + ij.y) & 1) == 0 ? zNear : zFar'; new = 'zNear' },
    @{ id = 'M7 the debug view stops reading the linear depth'; f = "$rmd\PostStages.cpp";
       old = '"debugview", "A3", true, true,'; new = '"debugview", "A3", true, false,' }
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
        $r = & $tests $tags --reporter compact 2>&1
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
& $tests $tags 2>&1 | Select-Object -Last 2
"tests (restored) exit: $LASTEXITCODE"
