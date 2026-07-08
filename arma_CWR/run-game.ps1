<#
.SYNOPSIS
    Builds (incrementally) and launches PoseidonGame against the CWA data directory.

.DESCRIPTION
    Wraps the manual steps normally needed to playtest a change: make sure the
    toolchain (Clang/CMake/Ninja/ccache/vcpkg) is on PATH for this session,
    incrementally build the PoseidonGame target so the exe reflects the
    current working tree, then launch it windowed with no splash screen
    against the game data directory.

    Building the *incremental* target only (not a full `cmake --build`) keeps
    this fast when only a few source files changed. Pass -SkipBuild to launch
    whatever is already in dist/ without touching the build at all.

    CONSOLE OUTPUT CAPTURE (on by default): every launch writes three files
    into <repo>\logs\ (gitignored), timestamped so runs never overwrite:
      game-<ts>.log         engine log (spdlog file sink via --log-file;
                            everything LogF/spdlog emits, same as console)
      game-<ts>.stdout.log  raw stdout (CLI help/version text)
      game-<ts>.stderr.log  raw stderr (crash-handler stack traces, minidump
                            paths, command-line errors - these bypass the
                            engine logger, so --log-file alone would miss
                            them)
    The game runs detached (Start-Process), so nothing floods the terminal
    or an AI-session transcript; inspect the files after (or during) the run
    instead. Pass -NoLog to launch with no capture at all.

.PARAMETER DataDir
    The Arma: Cold War Assault install (data) directory. This is the working
    directory the game runs from (AddOns/, Missions/, Campaigns/, etc. live
    here). Default: D:\Arma_CWA\ARMA Cold War Assault [Classic]

.PARAMETER Preset
    The CMake configure preset / build dir name under build/.
    Default: win-x64-clang-rwdi

.PARAMETER Mission
    Optional mission folder to boot straight into via --test-mission,
    resolved relative to DataDir (e.g. 'Missions\Guerrilla.Demo'). Omit to
    boot to the main menu, where GUERRILLA lets you pick island/faction
    interactively.

.PARAMETER SkipBuild
    Skip the cmake build step and launch whatever binary is already staged
    in dist/<preset-arch>/.

.PARAMETER LogDir
    Directory for the captured output files. Default: <repo>\logs
    (gitignored via /logs/ and *.log in .gitignore).

.PARAMETER NoLog
    Disable all output capture (no --log-file, no stdout/stderr
    redirection).

.EXAMPLE
    .\run-game.ps1
        Build + launch to the main menu, capturing output to logs\.

.EXAMPLE
    .\run-game.ps1 -Mission 'Missions\Guerrilla.Demo'
        Build + jump straight into a Guerrilla Mode mission.

.EXAMPLE
    .\run-game.ps1 -SkipBuild
        Launch the existing build with no rebuild.
#>
param(
    [string]$DataDir = 'D:\Arma_CWA\ARMA Cold War Assault [Classic]',
    [string]$Preset = 'win-x64-clang-rwdi',
    [string]$Mission,
    [switch]$SkipBuild,
    [string]$LogDir = "$PSScriptRoot\logs",
    [switch]$NoLog
)

$ErrorActionPreference = 'Stop'
$RepoRoot = $PSScriptRoot

if (-not (Test-Path -LiteralPath $DataDir -PathType Container)) {
    throw "DataDir not found: $DataDir"
}

if (-not $SkipBuild) {
    # Toolchain-not-on-PATH gotcha: PATH/VCPKG_ROOT were added to the User
    # env after some terminal sessions started, so add them defensively here
    # rather than assume this shell picked them up.
    if (-not $env:VCPKG_ROOT) { $env:VCPKG_ROOT = 'C:\dev\vcpkg' }
    $toolchainPaths = @(
        'C:\Program Files\LLVM\bin'
        'C:\Program Files\CMake\bin'
        'C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe'
        'C:\Users\Dane\AppData\Local\Microsoft\WinGet\Packages\Ccache.Ccache_Microsoft.Winget.Source_8wekyb3d8bbwe\ccache-4.13.6-windows-x86_64'
    )
    $env:Path = ($toolchainPaths -join ';') + ';' + $env:Path

    $buildDir = Join-Path $RepoRoot "build\$Preset"
    if (-not (Test-Path -LiteralPath $buildDir)) {
        Write-Host "No configured build dir at $buildDir - configuring first..."
        cmake --preset $Preset
    }
    cmake --build $buildDir --target PoseidonGame
    if ($LASTEXITCODE -ne 0) { throw "Build failed with exit code $LASTEXITCODE" }
}

$distArch = if ($Preset -match '^win-(.+)-clang-') { "$($Matches[1])-win-$($Preset.Split('-')[-1])" } else { $Preset }
$exe = Join-Path $RepoRoot "dist\$distArch\PoseidonGame.exe"
if (-not (Test-Path -LiteralPath $exe)) {
    throw "PoseidonGame.exe not found at $exe - build may have staged elsewhere; check dist\ manually."
}

$gameArgs = @('--window', '--no-splash')
if ($Mission) {
    $gameArgs += @('--test-mission', $Mission)
}

Write-Host "Launching $exe (cwd=$DataDir)$(if ($Mission) { " -> $Mission" })"
if ($NoLog) {
    Start-Process -FilePath $exe -ArgumentList $gameArgs -WorkingDirectory $DataDir
} else {
    # Full console-output capture (see .DESCRIPTION): the engine logger gets
    # a file sink (--log-file), and raw stdout/stderr are redirected too -
    # the crash handler and CLI-error paths write straight to stderr and
    # would otherwise be lost with a detached windowed process.
    New-Item -ItemType Directory -Force $LogDir | Out-Null
    $stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
    $engineLog = Join-Path $LogDir "game-$stamp.log"
    $stdoutLog = Join-Path $LogDir "game-$stamp.stdout.log"
    $stderrLog = Join-Path $LogDir "game-$stamp.stderr.log"
    $gameArgs += @('--log-file', $engineLog)
    Start-Process -FilePath $exe -ArgumentList $gameArgs -WorkingDirectory $DataDir `
        -RedirectStandardOutput $stdoutLog -RedirectStandardError $stderrLog
    Write-Host "Logs:"
    Write-Host "  engine: $engineLog"
    Write-Host "  stdout: $stdoutLog"
    Write-Host "  stderr: $stderrLog"
}
