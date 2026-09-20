<# Launch a prepared Classic/LoBo installation using a built UD executable. #>
#requires -Version 7.0
param(
    [Parameter(Mandatory)][string]$GameDir,
    [Parameter(Mandatory)][string]$LoBoDir,
    [Parameter(Mandatory)][string]$GameExe,
    [string]$Mission
)
$ErrorActionPreference = 'Stop'
$GameDir = (Resolve-Path -LiteralPath $GameDir).ProviderPath
$LoBoDir = (Resolve-Path -LiteralPath $LoBoDir).ProviderPath
$GameExe = (Resolve-Path -LiteralPath $GameExe).ProviderPath
foreach ($relative in @('bin/remaster.cpp','bin/resource-extra.cpp','gmcore/init.sqs','fonts/cwr_body.ttf')) {
    $path = $GameDir
    foreach ($part in $relative.Split('/')) {
        $child = @(Get-ChildItem -LiteralPath $path | Where-Object Name -IEQ $part)
        if ($child.Count -ne 1) { throw "Run setup-guerrilla.ps1 first; missing or ambiguous $relative" }
        $path = $child[0].FullName
    }
}
$mods = @($LoBoDir)
$deps = Join-Path $LoBoDir 'deps/@LoBo_Deps'
if (Test-Path -LiteralPath $deps -PathType Container) { $mods += $deps }
$argsList = @('--window','--no-splash','--mod',($mods -join ';'))
if ($Mission) { $argsList += @('--test-mission', $Mission) }
# ArgumentList preserves paths containing spaces without manually quoting shell code.
$info = [Diagnostics.ProcessStartInfo]::new($GameExe)
$info.WorkingDirectory = $GameDir
$info.UseShellExecute = $false
foreach ($arg in $argsList) { $info.ArgumentList.Add($arg) }
$process = [Diagnostics.Process]::Start($info)
Write-Output "Started UD (PID $($process.Id))."
