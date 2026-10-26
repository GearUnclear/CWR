$env:VCPKG_ROOT='C:\dev\vcpkg'
$env:Path='C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64;'+$env:Path
Set-Location D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR
$log = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\B2\land-build.log'
cmake --build build/graphics-remaster-rwdi -j 12 *>&1 | Out-File -FilePath $log -Encoding utf8
"EXIT=$LASTEXITCODE" | Out-File -FilePath $log -Append -Encoding utf8
