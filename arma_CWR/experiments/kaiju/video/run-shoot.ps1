# Runs one Dedushka video shoot test (stepped frame capture). Copies the integration's script set
# kj\ into the shoot mission (video\DedushkaShoot.Eden), writes the take's kjd_params.sqf, then hands
# over to the pipeline's run-tri.ps1 (mutex UD-kaiju-tri, hidden desktop, one game, --nosound is in
# the toml). While the game runs a janitor deletes the BMP twin of every frame (6 MB each) so a long
# take does not fill the disk. Windows PowerShell 5.1 or later.
#
#   powershell -NoProfile -File run-shoot.ps1 -Name kv_probe [-Tag p1] [-TimeoutSec 5400]
#
# -Name   the test: video\<Name>.test.sqf + .test.toml (+ optional <Name>.params.sqf: the take).
# Output: kaiju-notes\video\runs\<tag>-<stamp>\ (game_stdout.log, PNG frames); logs in kaiju-notes\video\logs.
param(
    [Parameter(Mandatory)][string]$Name,
    [string]$Tag = '',
    [int]$TimeoutSec = 5400,
    [int]$MaxWaitSec = 7200,
    [string]$NotesDir = 'D:\Arma_CWA_worktrees\kaiju-notes\video'
)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
if (-not $Tag) { $Tag = $Name }
$gitRoot = [IO.Path]::GetFullPath((Join-Path $here '..\..\..\..'))
$free = (Get-PSDrive D).Free / 1GB
if ($free -lt 15) { Write-Host ("stopping: D: has {0:N1} GB free (floor 15)" -f $free); exit 126 }
if (-not (Test-Path -LiteralPath (Join-Path $gitRoot 'tmp\@kj_dedushka\addons\uddedushka.pbo'))) { Write-Host "missing mount tmp\@kj_dedushka"; exit 2 }
$test = Join-Path $here "$Name.test.sqf"
if (-not (Test-Path -LiteralPath $test)) { Write-Host "no test $test"; exit 2 }
$mdir = Join-Path $here 'DedushkaShoot.Eden'
# the script set: one source (integration\kj), copied whole, never edited here
$kj = Join-Path $mdir 'kj'
if (Test-Path -LiteralPath $kj) { Remove-Item -LiteralPath $kj -Recurse -Force }
Copy-Item -LiteralPath (Join-Path $here '..\integration\kj') -Destination $kj -Recurse
# the take
$paramFile = Join-Path $mdir 'kjd_params.sqf'
$take = Join-Path $here "$Name.params.sqf"
$text = "// written by run-shoot.ps1 for $Name`r`nKJD_params = `"$Name`";`r`n"
if (Test-Path -LiteralPath $take) { $text += [IO.File]::ReadAllText($take) + "`r`n" }
[IO.File]::WriteAllText($paramFile, $text, (New-Object Text.UTF8Encoding($false)))
$gameDir = Join-Path $gitRoot 'arma_CWR\dist\kaiju-rwdi'
$runTri = [IO.Path]::GetFullPath((Join-Path $here '..\run-tri.ps1'))
$stamp = Get-Date -Format 'MMdd-HHmmss'
$out = Join-Path $NotesDir "runs\$Tag-$stamp"
[void][IO.Directory]::CreateDirectory($out)
# janitor: the BMP twins, once they are 4 s old (the PNG beside each is the frame we keep)
$jan = Start-Job -ArgumentList $out -ScriptBlock {
    param($dir)
    while ($true) {
        Start-Sleep -Seconds 5
        $cut = (Get-Date).AddSeconds(-4)
        Get-ChildItem -LiteralPath $dir -Recurse -Filter *.bmp -ErrorAction SilentlyContinue | Where-Object { $_.LastWriteTime -lt $cut } | Remove-Item -Force -ErrorAction SilentlyContinue
        if (((Get-PSDrive D).Free / 1GB) -lt 15) { Set-Content -LiteralPath (Join-Path $dir 'DISK-LOW.txt') -Value (Get-Date -Format s) }
    }
}
try {
    powershell -NoProfile -File $runTri -Test "experiments/kaiju/video/$Name.test.sqf" -Tag $Tag -OutDir $out -LogDir (Join-Path $NotesDir 'logs') -GameDir $gameDir -TimeoutSec $TimeoutSec -MaxWaitSec $MaxWaitSec
    $code = $LASTEXITCODE
}
finally {
    Stop-Job $jan -ErrorAction SilentlyContinue; Remove-Job $jan -Force -ErrorAction SilentlyContinue
    Get-ChildItem -LiteralPath $out -Recurse -Filter *.bmp -ErrorAction SilentlyContinue | Remove-Item -Force -ErrorAction SilentlyContinue
}
$n = @(Get-ChildItem -LiteralPath $out -Recurse -Filter *.png -ErrorAction SilentlyContinue).Count
$glog = Get-ChildItem -LiteralPath $out -Recurse -Filter game_stdout.log -ErrorAction SilentlyContinue | Select-Object -First 1
$ended = $false
if ($glog) { $ended = [bool](Select-String -LiteralPath $glog.FullName -Pattern 'KV END' -Quiet) }
$verdict = if ($ended -and $code -eq 0) { 'RAN TO ITS END' } elseif ($ended) { "ran to its end, tri exit $code" } else { "DID NOT REACH ITS END (tri exit $code)" }
Add-Content -LiteralPath (Join-Path $NotesDir 'logs\runs.txt') -Value ("{0} {1} {2} : {3}, {4} frames, {5}" -f (Get-Date -Format s), $Name, $Tag, $verdict, $n, $out) -Encoding utf8
Write-Host "KVRUN $Tag : $verdict, $n frames"
Write-Host "KVOUT=$out"
exit $code
