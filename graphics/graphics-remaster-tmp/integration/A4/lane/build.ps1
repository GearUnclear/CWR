# A4 lane build in the scratch copy D:\Arma_CWA_worktrees\gr-A4 (stages to dist/gr-A4-rwdi).
param([switch]$Configure, [string]$Log = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\A4\lane\build.log')
$env:VCPKG_ROOT='C:\dev\vcpkg'
$env:Path='C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64;'+$env:Path
Set-Location D:\Arma_CWA_worktrees\gr-A4\arma_CWR
$t0 = Get-Date
& {
    if ($Configure) {
        cmake --preset win-x64-clang-rwdi -B build/gr-A4-rwdi
        "configure exit: $LASTEXITCODE"
    }
    cmake --build build/gr-A4-rwdi -j 8
    "build exit: $LASTEXITCODE"
    "elapsed: $((Get-Date) - $t0)"
} *>&1 | ForEach-Object { "$_" } | Out-File -FilePath $Log -Encoding utf8
