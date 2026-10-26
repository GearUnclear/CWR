# Runs a list of video clips, one run-captures.ps1 call per clip (so the BMP
# twins are deleted after each clip), on this lane's own data junction.
param([string]$Junction, [string]$Tag, [string[]]$Clips)
$ErrorActionPreference = 'Continue'
$engine = 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR'
$runs = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video\runs'
$Clips = @($Clips | ForEach-Object { $_ -split '[,\s]+' } | Where-Object { $_ })
Set-Location $engine
foreach ($c in $Clips) {
    $out = Join-Path $runs "$Tag-$c"
    "$(Get-Date -Format HH:mm:ss) start $c"
    & powershell -NoProfile -ExecutionPolicy Bypass -File tests\rendering\captures\run-captures.ps1 -GameDir dist/graphics-remaster-rwdi -DataJunction $Junction -OutDir $out -Tests "tests/rendering/captures/video/$c.test.sqf" *> (Join-Path $runs "$Tag-$c.log")
    & powershell -NoProfile -ExecutionPolicy Bypass -File D:\Arma_CWA\arma_CWR\tools\stop-owned-games.ps1 -DataDir (Join-Path $engine "packages\$Junction") *>> (Join-Path $runs "$Tag-$c.log")
    $n = @(Get-ChildItem -Recurse -Filter *.png (Join-Path $out 'tri') -ErrorAction SilentlyContinue | Where-Object { $_.Name -match '__(off|on)__f\d{4}\.png$' }).Count
    "$(Get-Date -Format HH:mm:ss) done $c frames=$n"
}
"$(Get-Date -Format HH:mm:ss) lane finished"
