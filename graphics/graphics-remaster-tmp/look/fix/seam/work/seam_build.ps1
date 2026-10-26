param([string]$Target = '')
$env:VCPKG_ROOT='C:/dev/vcpkg'
$env:Path='C:/Program Files/LLVM/bin;C:/Program Files/CMake/bin;C:/Users/Dane/AppData/Local/Microsoft/WinGet/Packages/Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:/Users/Dane/AppData/Local/Microsoft/WinGet/Packages/Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe/ccache-4.13.6-windows-x86_64;'+$env:Path
Set-Location 'D:/Arma_CWA_worktrees/look-fix-seam/arma_CWR'
$w = 'D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/fix/seam/work'
"running $(Get-Date -Format s) target=$Target" | Out-File -Encoding ascii "$w/mybuild_status.txt"
$t0 = Get-Date
if ($Target) { cmake --build build/look-fix-seam-rwdi -j 8 --target $Target 2>&1 | Out-File -Encoding utf8 "$w/mybuild.log" }
else { cmake --build build/look-fix-seam-rwdi -j 8 2>&1 | Out-File -Encoding utf8 "$w/mybuild.log" }
$code = $LASTEXITCODE
"done $(Get-Date -Format s) exit=$code minutes=$([math]::Round(((Get-Date)-$t0).TotalMinutes,1)) target=$Target" | Out-File -Encoding ascii "$w/mybuild_status.txt"
