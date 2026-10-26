$env:VCPKG_ROOT='C:\dev\vcpkg'
$env:Path='C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64;'+$env:Path
Set-Location D:\Arma_CWA_worktrees\look-director\arma_CWR
$t0 = Get-Date
cmake --build build/look-director-rwdi -j 8 --target PoseidonTests 2>&1 | Select-Object -Last 25
"exit: $LASTEXITCODE  time: $((Get-Date) - $t0)"

