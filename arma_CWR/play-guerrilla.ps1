<# Launch a prepared full official Remaster installation using a built UD
   executable. -LoBoDir is optional: leave it out to play the stock islands.

   The game starts detached. Output capture is on by default, exactly like
   run-game.ps1: each launch writes three timestamped files to -LogDir
   (default <arma_CWR>\logs, gitignored):
     game-<ts>.log         engine log (--log-file)
     game-<ts>.stdout.log  raw stdout
     game-<ts>.stderr.log  raw stderr; crash-handler stack traces, minidump
                           paths and CLI errors go here, bypassing the engine
                           log, so this is the file to read after a crash
   -NoLog launches with no capture. -Dev adds --dev (the dev panel, Ctrl+`,
   with its Console and Profile tabs). The script prints the PID; stop that
   process when you are done. #>
#requires -Version 7.0
param(
    [Parameter(Mandatory)][string]$GameDir,
    [string]$LoBoDir,
    [Parameter(Mandatory)][string]$GameExe,
    [string]$Mission,
    [switch]$Dev,
    [switch]$NoLog,
    [string]$LogDir = (Join-Path $PSScriptRoot 'logs')
)
$ErrorActionPreference = 'Stop'
Import-Module (Join-Path $PSScriptRoot 'tools/runtime/UDDataDir.psm1') -Force
$GameDir = Assert-UDRemasterDataDir -DataDir $GameDir
if ($LoBoDir) { $LoBoDir = (Resolve-Path -LiteralPath $LoBoDir).ProviderPath }
$GameExe = (Resolve-Path -LiteralPath $GameExe).ProviderPath
foreach ($relative in @('bin/remaster.cpp','bin/resource-extra.cpp','bin/ud-mainmenu.hpp','gmcore/init.sqs','fonts/cwr_body.ttf')) {
    $path = $GameDir
    foreach ($part in $relative.Split('/')) {
        $child = @(Get-ChildItem -LiteralPath $path | Where-Object Name -IEQ $part)
        if ($child.Count -ne 1) { throw "Run setup-guerrilla.ps1 first; missing or ambiguous $relative" }
        $path = $child[0].FullName
    }
}

# Start-Process joins its arguments with spaces and quotes nothing, so each
# argument is quoted here by the Windows argv rules: backslashes double only
# before a quote or before the closing quote.
function ConvertTo-NativeArgument([string]$Value) {
    if ($Value -and $Value -notmatch '[\s"]') { return $Value }
    $escaped = [regex]::Replace($Value, '(\\*)"', { param($m) ($m.Groups[1].Value * 2) + '\"' })
    $escaped = [regex]::Replace($escaped, '(\\+)$', { param($m) $m.Groups[1].Value * 2 })
    return '"' + $escaped + '"'
}

$argsList = @('--window','--no-splash')
if ($LoBoDir) {
    $mods = @($LoBoDir)
    $deps = Join-Path $LoBoDir 'deps/@LoBo_Deps'
    if (Test-Path -LiteralPath $deps -PathType Container) { $mods += $deps }
    $argsList += @('--mod',($mods -join ';'))
}
if ($Mission) { $argsList += @('--test-mission', $Mission) }
if ($Dev) { $argsList += '--dev' }

$start = @{
    FilePath         = $GameExe
    # Start-Process globs -WorkingDirectory, so a bracketed data dir (#12)
    # must be wildcard-escaped.
    WorkingDirectory = [Management.Automation.WildcardPattern]::Escape($GameDir)
    PassThru         = $true
}
if (-not $NoLog) {
    [void][IO.Directory]::CreateDirectory($LogDir)
    $LogDir = [IO.Path]::GetFullPath($LogDir)
    $stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
    $engineLog = Join-Path $LogDir "game-$stamp.log"
    $stdoutLog = Join-Path $LogDir "game-$stamp.stdout.log"
    $stderrLog = Join-Path $LogDir "game-$stamp.stderr.log"
    $argsList += @('--log-file', $engineLog)
    $start.RedirectStandardOutput = $stdoutLog
    $start.RedirectStandardError = $stderrLog
}
$start.ArgumentList = (@($argsList | ForEach-Object { ConvertTo-NativeArgument $_ }) -join ' ')
$process = Start-Process @start
Write-Output "Started UD (PID $($process.Id))."
if (-not $NoLog) {
    Write-Output "Logs:"
    Write-Output "  engine: $engineLog"
    Write-Output "  stdout: $stdoutLog"
    Write-Output "  stderr: $stderrLog"
}
