# fable-3: measure one of my static runs against the references.
#   powershell -NoProfile -File measure.ps1 -Tag static-det1 [-Moving]
# Writes fable-3/out/<Tag>_motion.txt (agent 1 kit, --static for held camera), <Tag>_curtain.txt
# (agent 4 curtain4.py with the references) and <Tag>_band.txt (bandmetric).
param([Parameter(Mandatory = $true)][string]$Tag, [switch]$Moving)
$root = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\video'
$me = "$root\fogwind\fable-3"
$mkv = "$me\runs\$Tag\$Tag.mkv"
if (-not (Test-Path $mkv)) { throw "no clip $mkv" }
$kit = "$root\fogwind\1\kit"
$out = "$me\out"
New-Item -ItemType Directory -Force -Path $out | Out-Null
if ($Moving) {
    & python "$kit\fogmotion.py" $mkv --from 15 --json "$out\${Tag}_motion.json" 2>&1 | Out-File "$out\${Tag}_motion.txt" -Encoding utf8
} else {
    & python "$kit\fogmotion.py" $mkv --from 15 --static --json "$out\${Tag}_motion.json" 2>&1 | Out-File "$out\${Tag}_motion.txt" -Encoding utf8
}
& python "$root\mist\tools\bandmetric.py" $mkv --from 15 2>&1 | Out-File "$out\${Tag}_band.txt" -Encoding utf8
if (-not $Moving) {
    $refs = @(
        "$root\mist\3\runs\static-base\static-base.mkv:base",
        "$root\mist\3\runs\static-frozen\static-frozen.mkv:frozen",
        "$root\mist\3\runs\static-percell\static-percell.mkv:d91",
        "$root\fogwind\4\runs\static-q4\static-q4.mkv:q4",
        "$root\fogwind\4\runs\static-q2b\static-q2b.mkv:q2b",
        "$root\fogwind\2\runs\static-Act25\static-Act25.mkv:Act25",
        "${mkv}:$Tag")
    & python "$root\fogwind\4\tools\curtain4.py" @refs 2>&1 | Out-File "$out\${Tag}_curtain.txt" -Encoding utf8
}
Get-Content "$out\${Tag}_band.txt"
Get-Content "$out\${Tag}_motion.txt"
if (-not $Moving) { Get-Content "$out\${Tag}_curtain.txt" }
