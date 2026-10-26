$env:VCPKG_ROOT='C:/dev/vcpkg'
$env:Path='C:/Program Files/LLVM/bin;C:/Program Files/CMake/bin;C:/Users/Dane/AppData/Local/Microsoft/WinGet/Packages/Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:/Users/Dane/AppData/Local/Microsoft/WinGet/Packages/Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe/ccache-4.13.6-windows-x86_64;'+$env:Path
Set-Location D:/Arma_CWA_worktrees/look-fix-horizon/arma_CWR
$marker = 'D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/fix/horizon/scratch/build.done'
Remove-Item $marker -ErrorAction SilentlyContinue
cmake --build build/look-fix-horizon-rwdi -j 8 *> D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/fix/horizon/scratch/build_mine.log
"exit=$LASTEXITCODE" | Out-File -Encoding ascii $marker
