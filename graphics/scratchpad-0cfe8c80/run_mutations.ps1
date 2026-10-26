$ErrorActionPreference = 'Continue'
$env:VCPKG_ROOT = 'C:\dev\vcpkg'
$env:Path = 'C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64;' + $env:Path
$py = 'C:\Users\Dane\AppData\Local\Temp\claude\D--Arma-CWA\0cfe8c80-fc33-4f8e-a86e-52fa1175350d\scratchpad\mutate.py'
$out = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\A6\mutations3.txt'
$exe = 'D:\Arma_CWA_worktrees\gr-A6\arma_CWR\build\gr-A6-rwdi\tests\unit\engine\Poseidon\PoseidonTests.exe'
"A6 mutations: each applied alone, PoseidonTests rebuilt, tags [FrameLights],[FeatureSwitches],[remaster],[PerfCounters]" | Out-File $out -Encoding utf8
foreach ($m in 'naive_rect', 'spot_cone', 'no_fade', 'mirror_unfaded', 'tile_overwrite', 'rank_brightness', 'no_night', 'no_budget', 'budget_hard', 'not_landed', 'per_draw_in_tiles') {
    python $py $m | Out-File $out -Append -Encoding utf8
    Push-Location 'D:\Arma_CWA_worktrees\gr-A6\arma_CWR'
    cmake --build build/gr-A6-rwdi -j 8 --target PoseidonTests *> $null
    "build exit $LASTEXITCODE" | Out-File $out -Append -Encoding utf8
    Pop-Location
    Push-Location 'D:\'
    $res = & $exe "[FrameLights],[FeatureSwitches],[remaster],[PerfCounters]" 2>&1
    Pop-Location
    "== $m" | Out-File $out -Append -Encoding utf8
    ($res | Select-String -Pattern '^Frame lights:|^Feature switches:|^Remaster|^Post|test cases|All tests passed' | ForEach-Object { $_.Line } | Select-Object -Unique) | Out-File $out -Append -Encoding utf8
    python $py restore | Out-File $out -Append -Encoding utf8
}
Push-Location 'D:\Arma_CWA_worktrees\gr-A6\arma_CWR'
cmake --build build/gr-A6-rwdi -j 8 --target PoseidonTests *> $null
"final rebuild exit $LASTEXITCODE" | Out-File $out -Append -Encoding utf8
Pop-Location
"done" | Out-File $out -Append -Encoding utf8
