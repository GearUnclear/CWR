# Shoots the research water views with one override dir (or stock when -Id stock).
#   powershell -NoProfile -File run_variant.ps1 -Id B -Junction RemasterUD-water2 [-Tag r2]
param(
    [Parameter(Mandatory)][string]$Id,
    [string]$Junction = 'RemasterUD-water',
    [string]$Tag = ''
)
$ErrorActionPreference = 'Stop'
$root = 'D:\Arma_CWA_worktrees\lighting\tmp\research\water'
$engine = 'D:\Arma_CWA_worktrees\lighting\arma_CWR'
$name = if ($Tag) { "$Id-$Tag" } else { $Id }
$out = Join-Path $root "runs\$name"
if (Test-Path -LiteralPath $out) { Remove-Item -LiteralPath $out -Recurse -Force }
$args = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', "$engine\tests\rendering\captures\run-captures.ps1",
    '-Tests', 'tests/rendering/research/water/abel_water.test.sqf', '-DataJunction', $Junction,
    '-OutDir', $out, '-TimeoutSec', '2400')
if ($Id -ne 'stock') { $args += @('-ShaderOverrideDir', (Join-Path $root "shaders_$Id")) }
Push-Location $engine
try { & powershell @args 2>&1 | Select-Object -Last 6 } finally { Pop-Location }
Get-ChildItem -LiteralPath (Join-Path $out 'tri') -Recurse -Filter game_stdout.log | ForEach-Object {
    Select-String -LiteralPath $_.FullName -Pattern 'override loaded|compile error|link error' | ForEach-Object {
        $l = $_.Line; $i = $l.IndexOf('GL33:'); if ($i -ge 0) { $l.Substring($i) } else { $l }
    }
}
