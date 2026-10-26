$env:VCPKG_ROOT='C:/dev/vcpkg'
$env:Path='C:/Program Files/LLVM/bin;C:/Program Files/CMake/bin;C:/Users/Dane/AppData/Local/Microsoft/WinGet/Packages/Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:/Users/Dane/AppData/Local/Microsoft/WinGet/Packages/Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe/ccache-4.13.6-windows-x86_64;'+$env:Path
$out = 'D:/Arma_CWA_worktrees/graphics-remaster/tmp/look/director/r5_build'
Remove-Item "$out/build.done" -ErrorAction SilentlyContinue
Set-Location 'D:/Arma_CWA_worktrees/look-director/arma_CWR'
$sw = [Diagnostics.Stopwatch]::StartNew()
cmake --build build/look-director-rwdi -j 16 *>&1 | Out-File -Encoding utf8 "$out/build.log"
$code = $LASTEXITCODE
[IO.File]::WriteAllText("$out/build.done", "exit=$code secs=$([int]$sw.Elapsed.TotalSeconds)")
