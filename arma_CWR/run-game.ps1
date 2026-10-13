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
    here). Default: the setup-prepared Remaster shadow, chosen by
    Get-UDDefaultDataDir (tools/runtime/UDDataDir.psm1): $env:UD_DATA_DIR,
    else the target of packages\RemasterUD, else
    D:\Arma_CWA\tmp\remaster-shadow-67 or D:\Arma_CWA\tmp\remaster-shadow.
    Only when none exists does it fall back, with a warning, to the legacy
    D:\Arma_CWA\ARMA Cold War Assault [Classic].

.PARAMETER Mod
    Mod folder(s) to mount, in mount order (later entries shadow earlier
    ones), passed to the engine as one ';'-separated --mod value. Alias:
    -Mods. An entry is an absolute path, or a relative path resolved against
    DataDir, then the repo root (D:\Arma_CWA, where @LoBo lives). Every entry
    is checked here, because an unresolvable --mod entry is fatal at startup
    and the detached game would only report it in the stderr log. This adds
    no @lobofixup and no --mods-dir; run-game-with-mods.ps1 does that.

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

.PARAMETER ExtraArgs
    Additional raw arguments appended to the game command line, for callers
    that wrap this script (run-game-with-mods.ps1 uses it for --mod /
    --mods-dir). Elements are passed through verbatim, so an element whose
    value contains a space must carry its own quotes.

.EXAMPLE
    .\run-game.ps1
        Build + launch to the main menu, capturing output to logs\.

.EXAMPLE
    .\run-game.ps1 -Mission 'Missions\Guerrilla.Demo'
        Build + jump straight into a Guerrilla Mode mission.

.EXAMPLE
    .\run-game.ps1 -SkipBuild
        Launch the existing build with no rebuild.

.EXAMPLE
    .\run-game.ps1 -SkipBuild -Mod '@LoBo' -Mission 'Missions\Guerrilla.Sinai'
        Mount the shadow's @LoBo and boot the Sinai template.
#>
param(
    [string]$DataDir,
    [string]$Preset = 'win-x64-clang-rwdi',
    [string]$Mission,
    [Alias('Mods')][string[]]$Mod = @(),
    [switch]$SkipBuild,
    [string]$LogDir = "$PSScriptRoot\logs",
    [switch]$NoLog,
    [string[]]$ExtraArgs = @()
)

$ErrorActionPreference = 'Stop'
$RepoRoot = $PSScriptRoot

if (-not $DataDir) {
    Import-Module (Join-Path $RepoRoot 'tools\runtime\UDDataDir.psm1') -Force
    $DataDir = Get-UDDefaultDataDir -EngineRoot $RepoRoot
}
if (-not (Test-Path -LiteralPath $DataDir -PathType Container)) {
    throw "DataDir not found: $DataDir"
}

# Start-Process joins -ArgumentList elements with spaces and quotes nothing,
# so a value this script adds must carry its own quotes (Windows argv rules:
# backslashes double only before a quote or the closing quote).
function ConvertTo-NativeArgument([string]$Value) {
    if ($Value -and $Value -notmatch '[\s"]') { return $Value }
    $escaped = [regex]::Replace($Value, '(\\*)"', { param($m) ($m.Groups[1].Value * 2) + '\"' })
    $escaped = [regex]::Replace($escaped, '(\\+)$', { param($m) $m.Groups[1].Value * 2 })
    return '"' + $escaped + '"'
}

$modPaths = @()
foreach ($entry in $Mod) {
    if (-not $entry) { continue }
    $candidates = if ([IO.Path]::IsPathRooted($entry)) { @($entry) } else {
        @((Join-Path $DataDir $entry), (Join-Path (Split-Path -Parent $RepoRoot) $entry)) }
    $found = $null
    foreach ($candidate in $candidates) {
        if (Test-Path -LiteralPath $candidate -PathType Container) { $found = [IO.Path]::GetFullPath($candidate); break }
    }
    if (-not $found) { throw "Mod '$entry' not found; tried: $($candidates -join ', ')" }
    $modPaths += $found
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
    $gameArgs += @('--test-mission', (ConvertTo-NativeArgument $Mission))
}
if ($modPaths) {
    # One scalar --mod option whose value is the ';'-joined list (AppConfig.cpp).
    $gameArgs += @('--mod', (ConvertTo-NativeArgument ($modPaths -join ';')))
}
if ($ExtraArgs) {
    $gameArgs += $ExtraArgs
}

Write-Host "Launching $exe (cwd=$DataDir)$(if ($Mission) { " -> $Mission" })"
if ($modPaths) { Write-Host "Mods (mount order): $($modPaths -join '; ')" }
# Start-Process globs -WorkingDirectory (PS 5.1 and 7), so a bracketed data dir
# ("[Classic]") must be wildcard-escaped or the launch fails to resolve (#12).
$wd = [Management.Automation.WildcardPattern]::Escape($DataDir)
if ($NoLog) {
    $process = Start-Process -FilePath $exe -ArgumentList $gameArgs -WorkingDirectory $wd -PassThru
    Write-Host "Started PoseidonGame (PID $($process.Id))."
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
    $gameArgs += @('--log-file', (ConvertTo-NativeArgument $engineLog))
    $process = Start-Process -FilePath $exe -ArgumentList $gameArgs -WorkingDirectory $wd `
        -RedirectStandardOutput $stdoutLog -RedirectStandardError $stderrLog -PassThru
    Write-Host "Started PoseidonGame (PID $($process.Id))."
    Write-Host "Logs:"
    Write-Host "  engine: $engineLog"
    Write-Host "  stdout: $stdoutLog"
    Write-Host "  stderr: $stderrLog"
}
