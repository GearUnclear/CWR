param([string]$Label)
$env:VCPKG_ROOT='C:\dev\vcpkg'; $env:Path='C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64;'+$env:Path
Set-Location D:\Arma_CWA_worktrees\lighting\arma_CWR
$log = "D:\Arma_CWA_worktrees\lighting\tmp\bugfix\integ_build_$Label.log"
cmake --build build/lighting-rwdi -j 12 *> $log
"BUILD EXIT $LASTEXITCODE"
if ($LASTEXITCODE -ne 0) { Select-String -Path $log -Pattern 'error' | Select-Object -First 20 | ForEach-Object { $_.Line } }
