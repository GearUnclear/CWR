param([string]$Log = 'build.log', [string]$Target = '')
$env:VCPKG_ROOT = 'C:\dev\vcpkg'
$env:Path = 'C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64;' + $env:Path
Set-Location 'D:\Arma_CWA_worktrees\look-light\arma_CWR'
$logPath = Join-Path 'C:\Users\Dane\AppData\Local\Temp\claude\D--Arma-CWA\0cfe8c80-fc33-4f8e-a86e-52fa1175350d\scratchpad\ll' $Log
if ($Target) { & cmake --build build/look-light-rwdi -j 8 --target $Target *>&1 | Out-File -Encoding utf8 $logPath }
else { & cmake --build build/look-light-rwdi -j 8 *>&1 | Out-File -Encoding utf8 $logPath }
"exit $LASTEXITCODE" | Out-File -Append -Encoding utf8 $logPath
