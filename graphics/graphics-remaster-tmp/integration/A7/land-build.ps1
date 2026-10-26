# A7 landing build in the graphics-remaster worktree.
$env:VCPKG_ROOT='C:\dev\vcpkg'
$env:Path='C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64;'+$env:Path
Set-Location D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR
$t0 = Get-Date
cmake --build build/graphics-remaster-rwdi -j 12
"build exit: $LASTEXITCODE"
"elapsed: $((Get-Date) - $t0)"
