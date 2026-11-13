# Builds the kj_roar addon (Dedushka's sounds) from nothing: synthesises every
# sound, checks each file with the engine's own loader, writes mp3 previews for
# the owner, packs kj_roar.pbo and copies it to the mount folder.
# Windows PowerShell 5.1 or later. No game is started.
#
#   powershell -NoProfile -File build-kj-roar.ps1 [-OutDir <folder>] [-NoInstall] [-NoSynth]
#
# -OutDir    every built file (wav masters, wss, pbo, logs). Default:
#            kaiju-notes\features\roar\build. Nothing binary goes into the repo.
# -Previews  where the mp3 previews go. Default: kaiju-notes\features\roar\previews.
# -NoSynth   reuse the masters already in <OutDir>\wav.
# -NoInstall do not copy the pbo to <git root>\tmp\@kj_roar (the mount folder,
#            gitignored, beside the Remaster shadow and @uddedushka; a test
#            mounts it with --mods-dir ".." --mod "@uddedushka;@kj_roar").
param(
    [string]$OutDir = 'D:\Arma_CWA_worktrees\kaiju-notes\features\roar\build',
    [string]$Previews = 'D:\Arma_CWA_worktrees\kaiju-notes\features\roar\previews',
    [switch]$NoSynth,
    [switch]$NoInstall,
    [string]$ModDir = '',
    [string]$Tools = 'D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe',
    [string]$Ffmpeg = 'C:\ffmpeg\bin\ffmpeg.exe',
    [string]$Python = 'python'
)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
# <git root>\arma_CWR\experiments\kaiju\features\roar\addon -> <git root>
$gitRoot = [IO.Path]::GetFullPath((Join-Path $here '..\..\..\..\..\..'))
if (-not $ModDir) { $ModDir = Join-Path $gitRoot 'tmp\@kj_roar' }
$OutDir = [IO.Path]::GetFullPath($OutDir)
[void][IO.Directory]::CreateDirectory($OutDir)
[void][IO.Directory]::CreateDirectory($Previews)
foreach ($f in @($Tools, (Join-Path $here 'kj_roar\config.cpp'), (Join-Path $here 'synth_sounds.py'))) {
    if (-not (Test-Path -LiteralPath $f)) { throw "missing input: $f" }
}

# 1. Synthesis: wav masters, .wss game files and synth.json (the numbers).
if (-not $NoSynth) {
    Write-Host '[kj_roar] synthesising'
    & $Python (Join-Path $here 'synth_sounds.py') $OutDir | ForEach-Object { Write-Host "    $_" }
    if ($LASTEXITCODE -ne 0) { throw 'synth_sounds.py failed' }
}
$report = Get-Content -LiteralPath (Join-Path $OutDir 'synth.json') -Raw | ConvertFrom-Json

# 2. Every sound the config names must exist, and the engine's loader must
# read it with the length the synthesis reported.
$config = [IO.File]::ReadAllText((Join-Path $here 'kj_roar\config.cpp'))
$named = [regex]::Matches($config, '\\kj_roar\\snd\\([A-Za-z0-9_]+)\.wss') | ForEach-Object { $_.Groups[1].Value } | Sort-Object -Unique
$check = @()
foreach ($n in $named) {
    $wss = Join-Path $OutDir "snd\$n.wss"
    if (-not (Test-Path -LiteralPath $wss)) { throw "config.cpp names snd\$n.wss but it was not built" }
    $out = & $Tools sound inspect $wss | Out-String
    if ($LASTEXITCODE -ne 0) { throw "sound inspect failed on $wss" }
    if ($out -notmatch 'Duration:\s*([0-9.]+)s') { throw "no duration in the inspect output of $wss" }
    $dur = [double]$Matches[1]
    $want = [double]$report.$n.seconds
    if ([math]::Abs($dur - $want) -gt 0.02) { throw "$n.wss: the engine reads $dur s, the synthesis made $want s" }
    $rate = if ($out -match 'Sample Rate:\s*(\d+)') { $Matches[1] } else { '?' }
    $check += ('{0,-12} {1,6:N2} s  {2} Hz 16 bit mono  engine loader ok' -f $n, $dur, $rate)
}
$check | ForEach-Object { Write-Host "    $_" }
$check | Out-File -FilePath (Join-Path $OutDir 'inspect.log') -Encoding utf8

# 3. Previews for the owner's ears (agents cannot hear): one mp3 per sound,
# and one file with all of them in order, a second of silence between.
if (Test-Path -LiteralPath $Ffmpeg) {
    $order = @('roar', 'roar_far', 'step', 'step_far', 'furnace', 'beam_charge', 'beam', 'beam_loop', 'exhale', 'static', 'birds')
    $i = 1
    $list = @()
    foreach ($n in $order) {
        $wav = Join-Path $OutDir "wav\$n.wav"
        if (-not (Test-Path -LiteralPath $wav)) { continue }
        $mp3 = Join-Path $Previews ('{0:D2}_{1}.mp3' -f $i, $n)
        & $Ffmpeg -hide_banner -loglevel error -y -i $wav -ar 44100 -ac 1 -codec:a libmp3lame -q:a 2 $mp3
        if ($LASTEXITCODE -ne 0) { throw "ffmpeg failed on $wav" }
        $list += $wav
        $i++
    }
    $inputs = @()
    $filter = ''
    for ($k = 0; $k -lt $list.Count; $k++) {
        $inputs += @('-i', $list[$k])
        $filter += "[${k}:a]aresample=44100,apad=pad_dur=1[a$k];"
    }
    $filter += (0..($list.Count - 1) | ForEach-Object { "[a$_]" }) -join ''
    $filter += "concat=n=$($list.Count):v=0:a=1[out]"
    & $Ffmpeg -hide_banner -loglevel error -y @inputs -filter_complex $filter -map '[out]' -codec:a libmp3lame -q:a 2 (Join-Path $Previews '00_all_in_order.mp3')
    if ($LASTEXITCODE -ne 0) { throw 'ffmpeg failed on the combined preview' }
    Write-Host "[kj_roar] previews in $Previews"
}
else { Write-Warning "no ffmpeg at ${Ffmpeg}; no previews written" }

# 4. Stage and pack.
$stage = Join-Path $OutDir 'stage\kj_roar'
if (Test-Path -LiteralPath $stage) { Remove-Item -LiteralPath $stage -Recurse -Force }
New-Item -ItemType Directory -Force -Path (Join-Path $stage 'snd') | Out-Null
Copy-Item -LiteralPath (Join-Path $here 'kj_roar\config.cpp') -Destination (Join-Path $stage 'config.cpp')
foreach ($n in $named) { Copy-Item -LiteralPath (Join-Path $OutDir "snd\$n.wss") -Destination (Join-Path $stage "snd\$n.wss") }
$addons = Join-Path $OutDir '@kj_roar\addons'
New-Item -ItemType Directory -Force -Path $addons | Out-Null
$pbo = Join-Path $addons 'kj_roar.pbo'
if (Test-Path -LiteralPath $pbo) { Remove-Item -LiteralPath $pbo -Force }
& $Tools pbo pack $stage $pbo | Out-File -FilePath (Join-Path $OutDir 'pack.log') -Encoding utf8
if ($LASTEXITCODE -ne 0) { throw "pbo pack failed (log: $OutDir\pack.log)" }
& $Tools pbo list $pbo
if ($LASTEXITCODE -ne 0) { throw "pbo list failed on $pbo" }

# 5. Mount copy.
if (-not $NoInstall) {
    $dst = Join-Path $ModDir 'addons'
    New-Item -ItemType Directory -Force -Path $dst | Out-Null
    Copy-Item -LiteralPath $pbo -Destination (Join-Path $dst 'kj_roar.pbo') -Force
    Write-Host "[kj_roar] installed to $dst"
}
Write-Host "[kj_roar] done: $pbo"
