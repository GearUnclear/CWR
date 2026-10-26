param([string]$Target = '')
$env:VCPKG_ROOT='C:/dev/vcpkg'
$env:Path='C:/Program Files/LLVM/bin;C:/Program Files/CMake/bin;C:/Users/Dane/AppData/Local/Microsoft/WinGet/Packages/Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:/Users/Dane/AppData/Local/Microsoft/WinGet/Packages/Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe/ccache-4.13.6-windows-x86_64;'+$env:Path
$dir = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\look\fix\plate'
Remove-Item "$dir\BUILD_MINE.txt" -ErrorAction SilentlyContinue
Set-Location 'D:\Arma_CWA_worktrees\look-fix-plate\arma_CWR'
$t0 = Get-Date
if ($Target) { & cmake --build build/look-fix-plate-rwdi -j 8 --target $Target *> "$dir\build_mine.log" } else { & cmake --build build/look-fix-plate-rwdi -j 8 *> "$dir\build_mine.log" }
"exit=$LASTEXITCODE minutes=$([math]::Round(((Get-Date)-$t0).TotalMinutes,1)) $(Get-Date -Format s)" | Out-File -FilePath "$dir\BUILD_MINE.txt" -Encoding ascii
