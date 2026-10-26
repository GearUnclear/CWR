#requires -Version 5.1
# Integration gate for the occupier-forces branch (orchestrator only).
# Usage: powershell -NoProfile -File gate.ps1 -Phase 1 [-SkipBuild] [-SkipInstall] [-ExtraTests a,b]
param(
    [Parameter(Mandatory)][string]$Phase,
    [switch]$SkipBuild,
    [switch]$SkipInstall,
    [switch]$SkipUnit,
    [switch]$SkipTri,
    [switch]$Visible,
    [string[]]$ExtraTests = @()
)
$ErrorActionPreference = 'Continue'
$env:VCPKG_ROOT = 'C:\dev\vcpkg'
# SDL3 hint: test game windows open without taking focus from the owner's desktop.
$env:SDL_WINDOW_ACTIVATE_WHEN_SHOWN = '0'
$env:Path = "C:\Program Files\LLVM\bin;C:\Program Files\CMake\bin;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe;C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64;" + $env:Path
$pwsh = 'C:\dev\vcpkg\downloads\tools\powershell-core-7.6.2-windows\pwsh.exe'
$root = 'D:\Arma_CWA\arma_CWR'
$shadow = 'D:/Arma_CWA/tmp/remaster-shadow-67'
$log = "D:\Arma_CWA\tmp\of-gate-phase$Phase.log"
Set-Location $root
"=== GATE phase $Phase $(Get-Date -Format s) ===" | Tee-Object -FilePath $log

function Step($name, [scriptblock]$body) {
    "`n--- $name $(Get-Date -Format T) ---" | Tee-Object -FilePath $log -Append
    & $body 2>&1 | Tee-Object -FilePath $log -Append
    "--- $name exit=$LASTEXITCODE ---" | Tee-Object -FilePath $log -Append
}

if (-not $SkipBuild) {
    Step 'build' { cmake --build build/win-x64-clang-rwdi --target PoseidonGame PoseidonTools PoseidonTests -- -j 5 | Select-Object -Last 15 }
}
if (-not $SkipUnit) {
    Step 'unit' { & build\win-x64-clang-rwdi\tests\unit\engine\Poseidon\PoseidonTests.exe "[guerrilla],[counterattack],[campaignend],[save],[journal],[occupierforces]" | Select-Object -Last 40 }
}
if (-not $SkipInstall) {
    Step 'setup' { & $pwsh -File ./setup-guerrilla.ps1 -GameDir $shadow -LoBoDir "$shadow/@LoBo" -Tools './dist/x64-win-rwdi/PoseidonTools.exe' | Select-Object -Last 15 }
    Step 'install-missions' { & $pwsh -File guerrilla-mode/install-missions.ps1 -GameDir $shadow | Select-Object -Last 15 }
}
if (-not $SkipTri) {
    $tests = @(
        'tests/integration/ui/remaster',
        'tests/integration/scripting/guerrilla_town_defenders.test.sqf',
        'tests/integration/scripting/guerrilla_counter_attack_live.test.sqf',
        'tests/integration/scripting/guerrilla_counter_attack_offscreen.test.sqf',
        'tests/integration/scripting/guerrilla_counter_attack_unseen_spawn.test.sqf',
        'tests/integration/scripting/guerrilla_counter_attack_watched_base.test.sqf',
        'tests/integration/scripting/guerrilla_native_capture.test.sqf',
        'tests/integration/scripting/guerrilla_offscreen_capture.test.sqf',
        'tests/integration/scripting/guerrilla_offscreen_idle.test.sqf',
        'tests/integration/scripting/guerrilla_offscreen_convoy.test.sqf',
        'tests/integration/scripting/guerrilla_offscreen_save_reload.seq',
        'tests/integration/scripting/qrf_reference_mission.test.sqf',
        'tests/integration/scripting/guerrilla_civ_shakedown.test.sqf'
    ) + @($ExtraTests | ForEach-Object { $_ -split ',' } | Where-Object { $_ })
    if ($Visible) {
        Step 'tri' { & .\target\debug\tri.exe test --retries 3 -j2 --game-dir dist/x64-win-rwdi --data-dir $shadow @tests }
    } else {
        # Default: tri and its games run on a private desktop (tmp/tri-hidden-desktop.ps1), so no window
        # reaches the owner's desktop. -Visible turns it off.
        $triLog = "D:\Arma_CWA\tmp\of-gate-phase$Phase-tri.log"
        $triCmd = 'target\debug\tri.exe test --retries 3 -j2 --game-dir dist/x64-win-rwdi --data-dir ' + $shadow + ' ' + ($tests -join ' ')
        Step 'tri' {
            powershell -NoProfile -File D:\Arma_CWA\tmp\tri-hidden-desktop.ps1 -Log $triLog -WorkDir $root -TimeoutSec 21600 -Cmd $triCmd
            $code = $LASTEXITCODE
            Get-Content $triLog
            $global:LASTEXITCODE = $code
        }
    }
}
Step 'human-suite' { python -m unittest tests.contracts.test_human_suite }
"=== GATE phase $Phase done $(Get-Date -Format s) ===" | Tee-Object -FilePath $log -Append
