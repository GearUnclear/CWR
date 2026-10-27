<#
.SYNOPSIS
    Runs the Classic | Remastered lighting capture set through Trident and
    collects the images and timings into one folder.

.DESCRIPTION
    1. Runs tri on the capture tests in this folder (all three by default)
       through the hidden-desktop wrapper (tmp\tri-hidden-desktop.ps1, a
       local untracked script), so no game window reaches the desktop.
    2. Always stops the games this run started, scoped by data dir: the tests
       pin no data_dir, this script passes `--data-dir <arma_CWR>\packages\<DataJunction>`,
       tri launches every game with `-C` set to it, and
       tools\stop-owned-games.ps1 -DataDir matches exactly that junction path.
       Parallel runs (several at once) each pass their own
       -DataJunction, so one run's cleanup never stops another's games. Never
       a global Stop-Process.
    3. Runs make_pairs.py: copies <view>__classic.png / <view>__remastered.png
       out of the tri tree, builds <view>__pair.png, contact_sheet.png and
       timings.csv / timings.json / timings.md from the UDCAP log lines.

    Output: <worktree>\tmp\captures\<yyyyMMdd-HHmmss>\ (gitignored: /tmp/ at
    the git root). tri's own tree is in its tri\ subfolder.

.PARAMETER Tests
    Test files to run, relative to arma_CWR. Default: the three capture tests.
.PARAMETER GameDir
    Build/dist dir with PoseidonGame.exe, relative to arma_CWR or absolute.
    Default dist/x64-win-rwdi (the win-x64-clang-rwdi preset).
.PARAMETER Tri
    tri.exe to use. Default: target\debug\tri.exe of this tree, else of the
    main checkout.
.PARAMETER HiddenDesktop
    The hidden-desktop wrapper. Default: tmp\tri-hidden-desktop.ps1 of this
    tree, else of the main checkout. Not needed with -Visible.
.PARAMETER StopGames
    The scoped game cleanup script. Default: tools\stop-owned-games.ps1 of
    this tree, else of the main checkout.
.PARAMETER DataJunction
    Name of the junction under arma_CWR\packages that the games use as their
    data dir. Default RemasterUD. Any other name is created on demand as a
    junction to -ShadowDir; give every parallel run its own name.
.PARAMETER ShadowDir
    The setup-prepared Remaster shadow a new junction points at. Default:
    tmp\remaster-shadow-67 of this tree, else of the main checkout.
.PARAMETER ShaderOverrideDir
    Sets CWR_SHADER_OVERRIDE_DIR for the games of this run (absolute path):
    every <name>.glsl there replaces the built-in shader of that name.
.PARAMETER Visible
    Run tri directly instead of on the hidden desktop (game windows show).
.PARAMETER CollectOnly
    Skip the run; rebuild pairs and tables from an existing output folder
    (pass it with -OutDir).

.EXAMPLE
    powershell -NoProfile -File tests\rendering\captures\run-captures.ps1
.EXAMPLE
    powershell -NoProfile -File tests\rendering\captures\run-captures.ps1 -Tests tests/rendering/captures/eden_lighting.test.sqf
#>
[CmdletBinding()]
param(
    [string[]]$Tests = @(
        'tests/rendering/captures/abel_lighting.test.sqf',
        'tests/rendering/captures/eden_lighting.test.sqf',
        'tests/rendering/captures/sinai_lobo_lighting.test.sqf'
    ),
    [string]$GameDir = 'dist/x64-win-rwdi',
    [string]$Tri = '',
    [string]$HiddenDesktop = '',
    [string]$StopGames = '',
    [string]$OutRoot = '',
    [string]$OutDir = '',
    [int]$Retries = 1,
    [int]$TimeoutSec = 5400,
    [string]$Python = 'python',
    [string]$DataJunction = 'RemasterUD',
    [string]$ShadowDir = '',
    [string]$ShaderOverrideDir = '',
    [switch]$Visible,
    [switch]$CollectOnly
)

$ErrorActionPreference = 'Stop'
# `powershell -File ... -Tests a,b` delivers "a,b" as one string; split it here.
$Tests = @($Tests | ForEach-Object { $_ -split '[,\s]+' } | Where-Object { $_ })
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$engineRoot = (Resolve-Path (Join-Path $here '..\..\..')).Path          # arma_CWR
$gitRoot = (Resolve-Path (Join-Path $engineRoot '..')).Path             # worktree root

# Defaults that were machine paths are resolved here, relative to this tree.
# A linked worktree has no tmp\ of its own and may lack a tri build, so each
# default falls back to the same path in the main checkout (the git common
# dir's parent). Pass the parameter to use anything else.
$mainRoot = $gitRoot
try {
    $common = & git -C $engineRoot rev-parse --path-format=absolute --git-common-dir 2>$null
    if ($LASTEXITCODE -eq 0 -and $common) { $mainRoot = Split-Path -Parent ($common | Select-Object -First 1).Trim() }
} catch { }
function Get-FirstExisting([string[]]$Candidates) {
    foreach ($c in $Candidates) { if ($c -and (Test-Path -LiteralPath $c)) { return $c } }
    return $Candidates[0]
}
if (-not $Tri) { $Tri = Get-FirstExisting @((Join-Path $engineRoot 'target\debug\tri.exe'), (Join-Path $mainRoot 'arma_CWR\target\debug\tri.exe')) }
if (-not $HiddenDesktop) { $HiddenDesktop = Get-FirstExisting @((Join-Path $gitRoot 'tmp\tri-hidden-desktop.ps1'), (Join-Path $mainRoot 'tmp\tri-hidden-desktop.ps1')) }
if (-not $StopGames) { $StopGames = Get-FirstExisting @((Join-Path $engineRoot 'tools\stop-owned-games.ps1'), (Join-Path $mainRoot 'arma_CWR\tools\stop-owned-games.ps1')) }
if (-not $ShadowDir) { $ShadowDir = Get-FirstExisting @((Join-Path $gitRoot 'tmp\remaster-shadow-67'), (Join-Path $mainRoot 'tmp\remaster-shadow-67')) }
if (-not $OutRoot) { $OutRoot = Join-Path $gitRoot 'tmp\captures' }
if (-not $OutDir) { $OutDir = Join-Path $OutRoot (Get-Date -Format 'yyyyMMdd-HHmmss') }
$triOut = Join-Path $OutDir 'tri'
if ($DataJunction -notmatch '^[A-Za-z0-9_-]+$') { Write-Error "-DataJunction must be a plain name: $DataJunction"; exit 2 }
$junction = Join-Path $engineRoot "packages\$DataJunction"

New-Item -ItemType Directory -Force -Path $triOut | Out-Null
Write-Host "output: $OutDir"

if (-not $CollectOnly -and $DataJunction -ne 'RemasterUD' -and -not (Test-Path -LiteralPath $junction)) {
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $junction) | Out-Null
    & cmd /c mklink /J "$junction" "$ShadowDir" | Out-Null
}
if ($ShaderOverrideDir) {
    if (-not (Test-Path -LiteralPath $ShaderOverrideDir)) { Write-Error "shader override dir not found: $ShaderOverrideDir"; exit 2 }
    $env:CWR_SHADER_OVERRIDE_DIR = (Resolve-Path -LiteralPath $ShaderOverrideDir).Path
    Write-Host "shader override: $env:CWR_SHADER_OVERRIDE_DIR"
} else {
    Remove-Item Env:\CWR_SHADER_OVERRIDE_DIR -ErrorAction SilentlyContinue
}

$exit = 0
if (-not $CollectOnly) {
    # ---- preflight: fail early with a readable reason, not a tri spawn error
    $gameDirAbs = if ([IO.Path]::IsPathRooted($GameDir)) { $GameDir } else { Join-Path $engineRoot $GameDir }
    $problems = @()
    if (-not (Test-Path -LiteralPath (Join-Path $gameDirAbs 'PoseidonGame.exe'))) { $problems += "no PoseidonGame.exe in $gameDirAbs (is the build finished?)" }
    if (-not (Test-Path -LiteralPath $Tri)) { $problems += "tri not found: $Tri" }
    if (-not (Test-Path -LiteralPath (Join-Path $junction 'dtaExt'))) {
        $problems += "packages\$DataJunction is missing or not the Remaster shadow. Create it with: cmd /c mklink /J `"$junction`" `"$ShadowDir`""
    }
    if (-not $Visible -and -not (Test-Path -LiteralPath $HiddenDesktop)) { $problems += "hidden-desktop wrapper not found: $HiddenDesktop (or pass -Visible)" }
    foreach ($t in $Tests) {
        if (-not (Test-Path -LiteralPath (Join-Path $engineRoot $t))) { $problems += "test not found: $t" }
    }
    if ($problems) { $problems | ForEach-Object { Write-Error $_ -ErrorAction Continue }; exit 2 }

    $triLog = Join-Path $OutDir 'tri.log'
    # The hidden-desktop wrapper hands this line to `cmd /c "<line> > log 2>&1"`,
    # whose quote stripping makes embedded quotes fragile: keep paths unquoted.
    foreach ($p in @($Tri, $triOut, $GameDir, $triLog, $junction) + $Tests) {
        if ($p -match '\s') { Write-Error "path with whitespace is not supported here: $p"; exit 2 }
    }
    $testArgs = ($Tests | ForEach-Object { $_ -replace '\\', '/' }) -join ' '
    $triCmd = "$Tri test --retries $Retries -j1 --timeout 120 --game-dir $GameDir --data-dir $junction --output-dir $triOut $testArgs"
    "cmd: $triCmd" | Out-File -FilePath (Join-Path $OutDir 'run.txt') -Encoding utf8
    Write-Host "running: $triCmd"
    try {
        if ($Visible) {
            Push-Location $engineRoot
            try {
                & cmd /c "$triCmd > `"$triLog`" 2>&1"
                $exit = $LASTEXITCODE
            } finally { Pop-Location }
        } else {
            & powershell -NoProfile -ExecutionPolicy Bypass -File $HiddenDesktop -Log $triLog -WorkDir $engineRoot -Cmd $triCmd -TimeoutSec $TimeoutSec
            $exit = $LASTEXITCODE
        }
    } finally {
        # Games this run started carry `-C <this run's junction>`; stop only
        # those (a no-op when tri already reaped them).
        if (Test-Path -LiteralPath $StopGames) {
            & powershell -NoProfile -ExecutionPolicy Bypass -File $StopGames -DataDir $junction
        } else {
            Write-Warning "stop-owned-games.ps1 not found at $StopGames; check for stray games yourself."
        }
    }
    Write-Host "tri exit code: $exit (log: $triLog)"
}

# ---- collect: pairs, contact sheet, timings
& $Python (Join-Path $here 'make_pairs.py') $OutDir --tri-dir $triOut
$collectExit = $LASTEXITCODE

# The BMP twin of every capture (TriScreenshot writes PNG + BMP) adds nothing.
Get-ChildItem -LiteralPath $triOut -Recurse -Filter '*.bmp' -ErrorAction SilentlyContinue | Remove-Item -Force -ErrorAction SilentlyContinue

Write-Host "done: $OutDir"
if ($exit -ne 0) { exit $exit }
exit $collectExit
