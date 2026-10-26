# Reviewer mutations for A3: apply one, build PoseidonTests only, run the gate
# tags, record failures, restore the original bytes exactly.
$ErrorActionPreference = 'Continue'
$env:VCPKG_ROOT='C:\dev\vcpkg'
$env:Path='C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64;'+$env:Path
$root = 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR'
$rmd = 'engine\Poseidon\Graphics\Remaster'
$core = 'engine\Poseidon\Graphics\Core'
$gl = 'engine\PoseidonGL33'
$tests = "$root\build\graphics-remaster-rwdi\tests\unit\engine\Poseidon\PoseidonTests.exe"
$tags = '[remaster],[Shaders],[tri],[triSetLighting],[GL33],[gpu],[ShadowSampler],[DebugGroup],[ShutdownOrder],[UBORestoreGuard],[StateOwnership],[StateCacheAudit],[BufferUsageAudit],[IboBindAudit],[Conventions],[Settings],[GraphicsConfig],[GraphicsApply],[scene],[Shape],[PostStages]'
$nl = "`n"
$muts = @(
    @{ id = 'X1 shafts ignore the optics overlay (glow)'; f = "$rmd\PostStages.cpp";
       old = 'if (in.worldDepth && in.glow && in.sunShafts > 0.0f'; new = 'if (in.worldDepth && in.sunShafts > 0.0f' },
    @{ id = 'X2 no landed mask on the plan'; f = "$rmd\PostStages.cpp";
       old = '    plan.runs &= LandedMask();'; new = '' },
    @{ id = 'X3 summary stops at the composite'; f = "$core\GpuTimings.cpp";
       old = 'for (int i = static_cast<int>(Section::Resolve); i <= static_cast<int>(Section::DebugView); ++i)';
       new = 'for (int i = static_cast<int>(Section::Resolve); i <= static_cast<int>(Section::Composite); ++i)' },
    @{ id = 'X4 the registry caps debug at 4 again'; f = "$rmd\FeatureSwitches.cpp";
       old = '&LT::debugView, 0, 5),'; new = '&LT::debugView, 0, 4),' },
    @{ id = 'X6 checkerboard parity swapped'; f = "$gl\EngineGL33_PostShaders.cpp";
       old = '((ij.x + ij.y) & 1) == 0 ? zNear : zFar'; new = '((ij.x + ij.y) & 1) == 1 ? zNear : zFar' },
    @{ id = 'X8 composite only when an effect stage runs'; f = "$rmd\PostStages.cpp";
       old = '    plan.runs |= Bit(PostStage::Composite);';
       new = '    if (plan.runs & (Bit(PostStage::Ao) | Bit(PostStage::Bloom) | Bit(PostStage::Shafts)))' + $nl + '        plan.runs |= Bit(PostStage::Composite);' }
)
foreach ($m in $muts) {
    "=== $($m.id)"
    $p = Join-Path $root $m.f
    $orig = [IO.File]::ReadAllBytes($p)
    $s = [Text.Encoding]::UTF8.GetString($orig)
    # The working files are CRLF (core.autocrlf true): match with CRLF too.
    $old = $m.old
    $new = $m.new
    if ($s.Contains("`r`n")) { $new = $new.Replace("`n", "`r`n") }
    $n = ([regex]::Matches($s, [regex]::Escape($old))).Count
    if ($n -ne 1) { "anchor count $n in $($m.f): skipped"; continue }
    $idx = $s.IndexOf($old)
    $mut = $s.Substring(0, $idx) + $new + $s.Substring($idx + $old.Length)
    $hasBom = $orig.Length -ge 3 -and $orig[0] -eq 0xEF -and $orig[1] -eq 0xBB -and $orig[2] -eq 0xBF
    [IO.File]::WriteAllText($p, $mut, (New-Object Text.UTF8Encoding($hasBom)))
    try {
        Set-Location $root
        $b = cmake --build build/graphics-remaster-rwdi -j 12 --target PoseidonTests 2>&1
        "build exit: $LASTEXITCODE"
        if ($LASTEXITCODE -ne 0) { $b | Select-String -Pattern 'error' | Select-Object -First 5 | ForEach-Object { $_.Line } }
        Set-Location D:\
        $r = & $tests $tags --reporter compact 2>&1
        "tests exit: $LASTEXITCODE"
        $r | Select-String -Pattern ': failed:' | ForEach-Object { ($_.Line -split ': failed:')[0] } | Sort-Object -Unique | ForEach-Object { "  failed at $_" }
        $r | Select-Object -Last 2
    } finally {
        [IO.File]::WriteAllBytes($p, $orig)
        (Get-Item $p).LastWriteTime = Get-Date
        "restored $($m.f)"
    }
}
"=== restored state"
Set-Location $root
cmake --build build/graphics-remaster-rwdi -j 12 2>&1 | Select-Object -Last 2
"full rebuild exit: $LASTEXITCODE"
Set-Location D:\
& $tests $tags 2>&1 | Select-Object -Last 2
"tests (restored) exit: $LASTEXITCODE"
