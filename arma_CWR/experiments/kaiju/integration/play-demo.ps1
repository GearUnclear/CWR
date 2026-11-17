# Starts the kaiju game for a person to PLAY the Dedushka demo by hand: a normal visible window,
# WITH sound, on the kaiju Remaster shadow, with tmp\@kj_dedushka mounted, straight into
# DedushkaDemo.Eden with KJD_manual = true (every beat and every camera on the action menu).
# Windows PowerShell 5.1 or later.
#
#   powershell -NoProfile -File play-demo.ps1                          # creature A (Dedushka), dusk
#   powershell -NoProfile -File play-demo.ps1 -Creature D -Time night  # the Hound, by night
#   powershell -NoProfile -File play-demo.ps1 -DryRun                  # make the copy, print the command line, start nothing
#   powershell -NoProfile -File play-demo.ps1 -Proof                   # agents only: see "The proof" below
#
# -Creature  A (UDDedushka, the Starved Bear) or D (UDDedushkaHound).
# -Time      dusk (the mission's own 19:10), day (12:00) or night (23:30).
# -Width, -Height  the window (default 1600 x 900, the size every test ran at).
# -Mortal    leave the player killable (default: the player cannot be hurt, as in every test run;
#            see "What to expect").
#
# How the mission is loaded (read from the engine, not guessed):
# - apps\cwr\Game\GameApplication.cpp: --test-mission <folder> copies the mission folder into the
#   game's temp dir and boots straight into it (World::StartIntro -> StartAutoTest -> DisplayMission):
#   no menu, no briefing. It is the switch arma_CWR\run-game.ps1 -Mission uses for hand play and the
#   one tri uses for every kaiju test. The folder name must keep its island suffix (.Eden).
# - The single mission LIST (UI\OptionsUIImpl.cpp, GetSPMissionLookupDirs) only shows folders and pbos
#   under <data dir>\Missions and <mod>\Missions. The shadow's Missions folder is game data and the
#   mount is a built addon, so nothing is put there; the demo is loaded by path instead.
# - The tracked mission is never edited. The launcher copies DedushkaDemo.Eden to
#   <git root>\tmp\kj-play\DedushkaDemo.Eden (tmp\ is ignored by git), copies the one script set
#   integration\kj into it, and writes the take into the COPY's kjd_params.sqf, the file init.sqs
#   reads on its first line (the same hook run-kj.ps1 -Params uses).
# - Switches (engine\Poseidon\Foundation\Platform\AppConfig.cpp): -C <data dir>, --window,
#   --no-splash, --lang, --test-mission, --render gl33, --no-strict (this build is RelWithDebInfo,
#   where strict is on by default and any error-level log line would end the game), --width,
#   --height, --mods-dir, --mod, --log-file. NOT passed: --nosound (you hear it), --harness and
#   --autotest (those are the test runner's).
#
# What to expect (all from the engine source, see the notes file for the lines):
# - You stand as an unarmed East soldier 170 m south east of Morton's main row. Open the action
#   menu: "Dedushka: play the whole demo", wake, walk in, roar, the beam, the cameras, and so on.
# - "Dedushka: a heavy force kills him (WEAPONS FIRE)" is a real fight with real guns: loud.
# - A mission started with --test-mission runs in the engine's test mode: a script error closes
#   the game (exit code 2), and so does the end of the mission or the player's death. That is why
#   the player is made unhurtable by default (as the tests do); -Mortal turns that off.
# - Close it with the window's close button or Alt+F4, or from another shell by its pid:
#   powershell -NoProfile -File D:\Arma_CWA\arma_CWR\tools\stop-owned-games.ps1 -Pid <pid>
# - Logs of a hand-play run: <git root>\tmp\kj-play\logs\play-<stamp>.log (engine log; the KJ lines
#   are the script set's), .stdout.log and .stderr.log beside it.
# - It does not take the kaiju test turn (the mutex UD-kaiju-tri): if an agent's hidden kaiju test
#   is running you are told, and both run at once (slower, nothing breaks).
#
# The proof (-Proof). The launcher starts ITSELF again on a hidden desktop (the pipeline's
# tmp\tri-hidden-desktop.ps1), so the game is started by the very same lines as in hand play
# (the same command line, the same Start-Process), with three differences that the inner call
# gets through the internal switches -Silent, -Probe and -LogDir:
#   --nosound is added; the logs go to the run folder; and the copy of the mission (a second
#   copy, tmp\kj-play-proof) additionally gets a probe script (kjd_playproof.sqs, started from
#   the copy's kjd_params.sqf) that logs the take, the creature and the action menu as the engine
#   holds it for the player (triActionMenuText), and then picks menu entry 1 ("Dedushka: wake")
#   through the menu's own handler kjd_act.sqs.
# The outer call waits while the owner's hold file exists (as run-tri.ps1 does), holds the kaiju
# turn (mutex UD-kaiju-tri) for the whole of it, points the game's
# user, cache and temp dirs at the run folder through the environment (as tri does) so the
# owner's profile is not touched, reads the engine log, stops the game by its own pid
# (stop-owned-games.ps1 -Pid) and prints "KJPLAY RESULT: n checks failed: [...]".
# Run folder: <ProofDir>\runs\play-proof-<tag>-<stamp>.
# What the proof does NOT run: the visible window, the sound, the owner's own profile.
param(
    [ValidateSet('A', 'D')][string]$Creature = 'A',
    [ValidateSet('dusk', 'day', 'night')][string]$Time = 'dusk',
    [int]$Width = 1600,
    [int]$Height = 900,
    [switch]$Mortal,
    [switch]$DryRun,
    [switch]$Proof,
    [string]$ProofDir = 'D:\Arma_CWA_worktrees\kaiju-notes\finishing\retest',
    [int]$ProofSec = 240,
    [int]$MaxWaitSec = 7200,
    [string]$Wrapper = 'D:\Arma_CWA\tmp\tri-hidden-desktop.ps1',
    [string]$StopOwned = 'D:\Arma_CWA\arma_CWR\tools\stop-owned-games.ps1',
    [string]$OwnerHold = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\perf-squad\queue\OWNER-HOLD.txt',
    # internal, set by -Proof on the inner call
    [switch]$Silent,
    [switch]$Probe,
    [string]$LogDir = ''
)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$gitRoot = [IO.Path]::GetFullPath((Join-Path $here '..\..\..\..'))
$exe = Join-Path $gitRoot 'arma_CWR\dist\kaiju-rwdi\PoseidonGame.exe'
$shadow = Join-Path $gitRoot 'tmp\remaster-shadow-kaiju'
$modsDir = Join-Path $gitRoot 'tmp'
$mount = Join-Path $modsDir '@kj_dedushka'
$srcMission = Join-Path $here 'DedushkaDemo.Eden'
$srcKj = Join-Path $here 'kj'
foreach ($f in @($exe, (Join-Path $shadow 'BIN'), (Join-Path $mount 'addons\uddedushka.pbo'), (Join-Path $srcMission 'mission.sqm'), (Join-Path $srcKj 'start.sqs'))) {
    if (-not (Test-Path -LiteralPath $f)) { Write-Host "missing: $f"; exit 2 }
}
$free = (Get-PSDrive D).Free / 1GB
if ($free -lt 15) { Write-Host ("stopping: D: has {0:N1} GB free (floor 15)" -f $free); exit 126 }
$class = if ($Creature -eq 'D') { 'UDDedushkaHound' } else { 'UDDedushka' }
$playRoot = Join-Path $gitRoot $(if ($Probe) { 'tmp\kj-play-proof' } else { 'tmp\kj-play' })
$copy = Join-Path $playRoot 'DedushkaDemo.Eden'
$tag = "$($Creature.ToLower())-$Time"
function Quote-Arg([string]$v) { if ($v -match '[\s"]') { '"' + ($v -replace '"', '\"') + '"' } else { $v } }

if (-not $Proof) {
    # ============================ hand play (and the proof's inner call) ============================
    $hour = switch ($Time) { 'day' { '12' } 'night' { '23.5' } default { '' } }
    $utf8 = New-Object Text.UTF8Encoding($false)
    # ---- the copy: the tracked mission is only read
    if (Test-Path -LiteralPath $copy) { Remove-Item -LiteralPath $copy -Recurse -Force }
    [void][IO.Directory]::CreateDirectory($copy)
    Get-ChildItem -LiteralPath $srcMission -Force | Where-Object { $_.Name -ne 'kj' } | ForEach-Object {
        Copy-Item -LiteralPath $_.FullName -Destination $copy -Recurse -Force
    }
    Copy-Item -LiteralPath $srcKj -Destination (Join-Path $copy 'kj') -Recurse
    $take = "// written by play-demo.ps1 into its own copy of the mission: creature $Creature, $Time`r`n"
    $take += "KJD_manual = true;`r`n"
    $take += "KJD_class = `"$class`";`r`n"
    if ($hour) { $take += "KJD_hour = $hour;`r`n" }
    if (-not $Mortal) { $take += "player allowDammage false;`r`n" }
    $take += "KJD_params = `"play $Creature $Time`";`r`n"
    if ($Probe) {
        $take += "[] exec `"kjd_playproof.sqs`";`r`n"
        $probeText = @(
            '; written by play-demo.ps1 -Proof into its own copy of the mission (never into the tracked one).',
            '; It only looks, then picks menu entry 1 through the menu''s own handler.',
            '_t = time',
            '#wait',
            '~0.5',
            '? time > _t + 120 : goto "late"',
            '? isNil "KJD_menu" : goto "wait"',
            '~3',
            'logInfo format ["KJPLAY proof: time %1, manual %2, class asked %3, creature %4, alive %5, state %6, ready %7, hour %8, entries in KJD_menu %9", time, KJD_manual, KJD_class, typeOf kaiju, alive kaiju, KJ_state, KJ_ready, daytime, count KJD_menu]',
            'logInfo "KJPLAY menu begin"',
            'logInfo triActionMenuText',
            'logInfo "KJPLAY menu end"',
            '_t2 = time',
            '[player, player, 1] exec "kjd_act.sqs"',
            '#woke',
            '~0.5',
            '? (KJ_state != "idle") and (time < _t2 + 90) : goto "woke"',
            'logInfo format ["KJPLAY wake: state %1 after %2 s, beats done %3, player alive %4", KJ_state, time - _t2, KJD_done, alive player]',
            'logInfo "KJPLAY PROOF END"',
            'exit',
            '#late',
            'logInfo "KJPLAY late: the menu script never ran"',
            'logInfo "KJPLAY PROOF END"',
            'exit'
        ) -join "`r`n"
        [IO.File]::WriteAllText((Join-Path $copy 'kjd_playproof.sqs'), $probeText + "`r`n", $utf8)
    }
    [IO.File]::WriteAllText((Join-Path $copy 'kjd_params.sqf'), $take, $utf8)

    # ---- the command line
    if (-not $LogDir) { $LogDir = Join-Path $playRoot 'logs' }
    $logBase = Join-Path $LogDir ("play-" + (Get-Date -Format 'MMdd-HHmmss'))
    $gameArgs = @('-C', (Quote-Arg $shadow), '--window', '--no-splash', '--lang', 'English',
        '--test-mission', (Quote-Arg $copy), '--render', 'gl33', '--no-strict',
        '--width', "$Width", '--height', "$Height",
        '--mods-dir', (Quote-Arg $modsDir), '--mod', '@kj_dedushka',
        '--log-file', (Quote-Arg "$logBase.log"))
    if ($Silent) { $gameArgs += '--nosound' }
    Write-Host "mission copy: $copy  (creature $class, $Time, KJD_manual = true)"
    Write-Host ("command line: " + (Quote-Arg $exe) + ' ' + ($gameArgs -join ' '))
    if ($DryRun) { Write-Host 'dry run: nothing started'; exit 0 }
    [void][IO.Directory]::CreateDirectory($LogDir)
    $busy = @(Get-CimInstance Win32_Process -Filter "Name='PoseidonGame.exe'" -ErrorAction SilentlyContinue |
        Where-Object { $_.CommandLine -and $_.CommandLine -like "*$shadow*" })
    if ($busy) { Write-Host ("note: a game is already running on the kaiju shadow (pid " + (($busy | ForEach-Object ProcessId) -join ', ') + ", probably an agent's hidden test). Both will run at once.") }
    $p = Start-Process -FilePath $exe -ArgumentList $gameArgs -WorkingDirectory $shadow `
        -RedirectStandardOutput "$logBase.stdout.log" -RedirectStandardError "$logBase.stderr.log" -PassThru
    Write-Host "started PoseidonGame pid $($p.Id): open the action menu for the beats and the cameras."
    Write-Host "  engine log $logBase.log"
    Write-Host "  stderr     $logBase.stderr.log"
    Write-Host "  to stop it from a shell: powershell -NoProfile -File $StopOwned -Pid $($p.Id)"
    exit 0
}

# ====================================== the proof (outer call) ======================================
# hidden desktop, silent, the kaiju turn held, the game closed by its own pid
foreach ($f in @($Wrapper, $StopOwned)) { if (-not (Test-Path -LiteralPath $f)) { Write-Host "missing: $f"; exit 2 } }
$copy = Join-Path $gitRoot 'tmp\kj-play-proof\DedushkaDemo.Eden'      # where the inner call (-Probe) makes its copy
$runDir = Join-Path $ProofDir ("runs\play-proof-$tag-" + (Get-Date -Format 'MMdd-HHmmss'))
$userDir = Join-Path $runDir 'user'
[void][IO.Directory]::CreateDirectory($userDir)
# the same gate as the pipeline's run-tri.ps1: no kaiju game while the owner's hold file exists
$waited = 0
while (Test-Path -LiteralPath $OwnerHold) {
    if ($waited % 60 -eq 0) { Write-Host ("[{0}] waiting: OWNER-HOLD" -f (Get-Date -Format HH:mm:ss)) }
    Start-Sleep -Seconds 15
    $waited += 15
    if ($waited -gt $MaxWaitSec) { Write-Host "gave up after $MaxWaitSec s of waiting: OWNER-HOLD"; exit 125 }
}
$mutex = New-Object System.Threading.Mutex($false, 'UD-kaiju-tri')
$got = $false
Write-Host ("[{0}] waiting for the kaiju turn (mutex UD-kaiju-tri)" -f (Get-Date -Format HH:mm:ss))
try { $got = $mutex.WaitOne([TimeSpan]::FromSeconds($MaxWaitSec)) } catch [System.Threading.AbandonedMutexException] { $got = $true }
if (-not $got) { Write-Host "gave up after $MaxWaitSec s of waiting for the kaiju turn"; exit 125 }
$script:fails = @()
$gamePid = 0
$ended = 'the probe finished'
$engineLog = ''
$launchLog = Join-Path $runDir 'launcher.console.log'
$launch = ''
$wnd = $null
# (a log may still be held open for writing by the running game)
function Read-Shared([string]$path) {
    if (-not $path -or -not (Test-Path -LiteralPath $path)) { return '' }
    $fs = New-Object IO.FileStream($path, [IO.FileMode]::Open, [IO.FileAccess]::Read, [IO.FileShare]::ReadWrite)
    try { $sr = New-Object IO.StreamReader($fs); return $sr.ReadToEnd() } finally { $fs.Dispose() }
}
try {
    Write-Host ("[{0}] kaiju turn taken" -f (Get-Date -Format HH:mm:ss))
    # the launcher itself, in hand-play mode, on the hidden desktop
    $inner = 'powershell -NoProfile -File ' + (Quote-Arg $PSCommandPath) + " -Creature $Creature -Time $Time -Width $Width -Height $Height" +
        $(if ($Mortal) { ' -Mortal' } else { '' }) + ' -Silent -Probe -LogDir ' + (Quote-Arg $runDir)
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = (Get-Command powershell.exe).Source
    $psi.Arguments = '-NoProfile -File "' + $Wrapper + '" -Log "' + $launchLog + '" -WorkDir "' + $here + '" -TimeoutSec 120 -Cmd "' + ($inner -replace '"', '\"') + '"'
    $psi.UseShellExecute = $false
    $psi.CreateNoWindow = $true
    foreach ($k in 'POSEIDON_USER_DIR', 'POSEIDON_CACHE_DIR', 'POSEIDON_TEMP_DIR') { $psi.EnvironmentVariables[$k] = $userDir }
    $wp = [System.Diagnostics.Process]::Start($psi)
    if (-not $wp.WaitForExit(150000)) { Write-Host 'the inner launcher did not return within 150 s' }
    $launch = Read-Shared $launchLog
    $launch -split "`r?`n" | Where-Object { $_ } | ForEach-Object { Write-Host "  inner: $_" }
    $pm = [regex]::Match($launch, 'started PoseidonGame pid (\d+)')
    $lm = [regex]::Match($launch, 'engine log (\S+)')
    if ($lm.Success) { $engineLog = $lm.Groups[1].Value }
    if ($pm.Success) {
        # the pid must be a PoseidonGame whose command line names the proof's own copy of the mission
        $g = Get-CimInstance Win32_Process -Filter ("ProcessId=" + $pm.Groups[1].Value) -ErrorAction SilentlyContinue
        if ($g -and ($g.Name -eq 'PoseidonGame.exe') -and $g.CommandLine -and ($g.CommandLine -like "*$copy*")) { $gamePid = [int]$g.ProcessId; Write-Host "game pid $gamePid (hidden desktop)" }
    }
    if (-not $gamePid) { $ended = 'the inner launcher started no game that could be found' }
    else {
        $t0 = Get-Date
        $sawEnd = $false
        $alive = $true
        $wnd = $null
        while (((Get-Date) - $t0).TotalSeconds -lt $ProofSec) {
            Start-Sleep -Seconds 2
            # a process on another desktop shows no window to this one: 0 here means nothing reached the owner's desktop
            if (($null -eq $wnd) -and (((Get-Date) - $t0).TotalSeconds -gt 8)) { $gp = Get-Process -Id $gamePid -ErrorAction SilentlyContinue; if ($gp) { $wnd = [int64]$gp.MainWindowHandle } }
            if ((Read-Shared $engineLog) -match 'KJPLAY PROOF END') { $sawEnd = $true; break }
            if (-not (Get-Process -Id $gamePid -ErrorAction SilentlyContinue)) { $alive = $false; break }
        }
        if (-not $sawEnd) { $ended = if ($alive) { "no end line within $ProofSec s" } else { 'the game ended by itself before the end line' } }
        # closed by its own pid, never by name
        if (Get-Process -Id $gamePid -ErrorAction SilentlyContinue) {
            & powershell -NoProfile -File $StopOwned -Pid $gamePid | ForEach-Object { Write-Host "  $_" }
            Start-Sleep -Seconds 2
        }
    }
}
finally {
    Start-Sleep -Seconds 4
    Write-Host ("[{0}] kaiju turn released" -f (Get-Date -Format HH:mm:ss))
    $mutex.ReleaseMutex()
    $mutex.Dispose()
}

# ---- the verdict, from the logs of the run
$log = Read-Shared $engineLog
$stdoutLog = $engineLog -replace '\.log$', '.stdout.log'
# (the mod line is printed before the engine log file is opened: it is only in the stdout capture)
$console = if ($engineLog) { Read-Shared $stdoutLog } else { '' }
function Check([bool]$ok, [string]$what) { if ($ok) { Write-Host "KJPLAY ok: $what" } else { Write-Host "KJPLAY CHECK FAILED: $what"; $script:fails += $what } }
Check ($ended -eq 'the probe finished') "the probe ran to its end ($ended)"
Check ($launch -match 'command line: .* --nosound') 'the inner call was silent (--nosound on its command line)'
Check (($null -ne $wnd) -and ($wnd -eq 0)) "the running game showed no window on the owner's desktop (main window handle seen from here: $wnd)"
Check ($log -match 'Test mission: .*DedushkaDemo\.Eden') 'the mission was loaded by path (--test-mission)'
Check ($console -match "Loaded mod: '@kj_dedushka'") 'the mod @kj_dedushka was mounted'
Check ($log -match "KJ ready at [\d.]+: creature $class, native true") "the script set came up on the kaiju binary with creature $class"
$m = [regex]::Match($log, 'KJPLAY proof: time ([\d.]+), manual (\w+), class asked (\w+), creature (\w+), alive (\w+), state (\w*), ready (\w+), hour ([\d.]+), entries in KJD_menu (\d+)')
Check $m.Success 'the probe logged the take'
if ($m.Success) {
    Check ($m.Groups[2].Value -eq 'true') 'KJD_manual is true'
    Check (($m.Groups[4].Value -eq $class) -and ($m.Groups[5].Value -eq 'true')) "the creature exists and is alive ($($m.Groups[4].Value))"
    Check ($m.Groups[6].Value -eq 'dormant') "he waits dormant for the menu, no timeline started (state $($m.Groups[6].Value))"
    $h = [double]$m.Groups[8].Value
    $wantH = switch ($Time) { 'day' { 12 } 'night' { 23.5 } default { 19.17 } }
    Check ([math]::Abs($h - $wantH) -lt 0.2) ("the hour is the take's ({0:N2}, asked {1})" -f $h, $Time)
}
$titles = @()
$menuFile = Join-Path $copy 'kjd_menu.sqs'
if (Test-Path -LiteralPath $menuFile) {
    $tm = [regex]::Match([IO.File]::ReadAllText($menuFile), '(?m)^_titles\s*=\s*\[(.*)\]\s*$')
    if ($tm.Success) { $titles = @([regex]::Matches($tm.Groups[1].Value, '"([^"]*)"') | ForEach-Object { $_.Groups[1].Value }) }
}
$menu = ''
$mm = [regex]::Match($log, '(?s)KJPLAY menu begin(.*?)KJPLAY menu end')
if ($mm.Success) { $menu = $mm.Groups[1].Value }
$found = @($titles | Where-Object { $menu.Contains($_) })
Check (($titles.Count -gt 0) -and ($found.Count -eq $titles.Count)) "the player's action menu holds every entry of kjd_menu.sqs ($($found.Count) of $($titles.Count))"
$w = [regex]::Match($log, 'KJPLAY wake: state (\w*) after ([\d.]+) s, beats done (\[[^\]]*\]), player alive (\w+)')
Check ($w.Success -and ($w.Groups[1].Value -eq 'idle')) ("menu entry 1 (wake) through kjd_act.sqs woke him" + $(if ($w.Success) { " (state $($w.Groups[1].Value) after $($w.Groups[2].Value) s)" } else { '' }))
Check (-not ($log -match 'Script error')) 'no script error in the log'
Check (($gamePid -ne 0) -and -not (Get-Process -Id $gamePid -ErrorAction SilentlyContinue)) "the game (pid $gamePid) was found and is gone after the stop by its pid"
$result = "KJPLAY RESULT: $($script:fails.Count) checks failed: [" + ($script:fails -join '; ') + ']'
Write-Host $result
@("play-demo.ps1 -Proof -Creature $Creature -Time $Time", "game pid $gamePid, $ended", $result) + @($launch -split "`r?`n" | Where-Object { $_ -match '^command line' }) | Set-Content -LiteralPath (Join-Path $runDir 'proof.txt') -Encoding utf8
Write-Host "KJOUT=$runDir"
if ($script:fails.Count -gt 0) { exit 1 }
exit 0
