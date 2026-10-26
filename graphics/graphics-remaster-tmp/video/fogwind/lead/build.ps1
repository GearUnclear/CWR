param([string]$Root = 'D:\Arma_CWA_worktrees\fog-8\arma_CWR', [string]$Name = 'fog-8', [string]$Target = '', [int]$J = 8)
$env:VCPKG_ROOT = 'C:\dev\vcpkg'
$env:Path = 'C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64;' + $env:Path
Set-Location $Root
$bdir = "build/$Name-rwdi"
if (-not (Test-Path "$bdir/build.ninja")) {
    cmake --preset win-x64-clang-rwdi -B $bdir
    if ($LASTEXITCODE -ne 0) { Write-Output "CONFIGURE FAILED"; exit 1 }
}
if ($Target -ne '') { cmake --build $bdir -j $J --target $Target } else { cmake --build $bdir -j $J }
Write-Output "BUILD EXIT $LASTEXITCODE"
