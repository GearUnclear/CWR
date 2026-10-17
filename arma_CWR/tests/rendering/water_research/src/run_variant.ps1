# Shoots the research water views with one override dir (or stock when -Id stock).
#   powershell -NoProfile -File run_variant.ps1 -Id B -Junction RemasterUD-water2 [-Tag r2]
# Output goes to <git root>\tmp\water_research\runs\<Id>[-<Tag>] (gitignored).
param(
    [Parameter(Mandatory)][string]$Id,
    [string]$Junction = 'RemasterUD-water',
    [string]$Tag = ''
)
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path                  # tests\rendering\water_research
$engine = (Resolve-Path (Join-Path $root '..\..\..')).Path                   # arma_CWR
$gitRoot = (Resolve-Path (Join-Path $engine '..')).Path
$name = if ($Tag) { "$Id-$Tag" } else { $Id }
$out = Join-Path $gitRoot "tmp\water_research\runs\$name"
if (Test-Path -LiteralPath $out) { Remove-Item -LiteralPath $out -Recurse -Force }
$args = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', "$engine\tests\rendering\captures\run-captures.ps1",
    '-Tests', 'tests/rendering/water_research/abel_water.test.sqf', '-DataJunction', $Junction,
    '-OutDir', $out, '-TimeoutSec', '2400')
if ($Id -ne 'stock') { $args += @('-ShaderOverrideDir', (Join-Path $root "shaders_$Id")) }
Push-Location $engine
try { & powershell @args 2>&1 | Select-Object -Last 6 } finally { Pop-Location }
Get-ChildItem -LiteralPath (Join-Path $out 'tri') -Recurse -Filter game_stdout.log | ForEach-Object {
    Select-String -LiteralPath $_.FullName -Pattern 'override loaded|compile error|link error' | ForEach-Object {
        $l = $_.Line; $i = $l.IndexOf('GL33:'); if ($i -ge 0) { $l.Substring($i) } else { $l }
    }
}
