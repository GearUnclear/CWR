param([string]$Case='classic-cold',[string]$Profile='portrait-final-classic',[string]$Test='tests/integration/scripting/guerrilla_portrait_dossier.test.sqf',[int]$Width=800,[int]$Height=600,[double]$Gamma=1.2)
$ErrorActionPreference='Stop'
$workspace=(Get-Location).Path
$profileRoot=Join-Path $workspace ('tmp/'+$Profile)
New-Item -ItemType Directory -Path $profileRoot -Force | Out-Null
$env:POSEIDON_USER_DIR=$profileRoot
@"
monitor=0;
windowMode=1;
resolutionWidth=$Width;
resolutionHeight=$Height;
refreshRate=0;
displayStyle=0;
ultrawideClamp=1;
"@ | Set-Content -LiteralPath (Join-Path $profileRoot 'display.cfg')
$graphics=Join-Path $profileRoot 'graphics.cfg'
if (Test-Path -LiteralPath $graphics) { (Get-Content -LiteralPath $graphics -Raw) -replace 'gamma=[^;]+;',("gamma="+$Gamma.ToString([cultureinfo]::InvariantCulture)+';') | Set-Content -LiteralPath $graphics }
else { ('gamma='+$Gamma.ToString([cultureinfo]::InvariantCulture)+';') | Set-Content -LiteralPath $graphics }
$before=@{}
Get-ChildItem -LiteralPath $profileRoot -Recurse -Filter portrait.png | ForEach-Object {$before[$_.FullName]=$_.LastWriteTimeUtc.Ticks}
$out='tmp/portrait-'+$Case
$runTest='tmp/portrait-run-'+$Case+'.test.sqf'
Copy-Item -LiteralPath $Test -Destination $runTest
$testConfig=Get-Content -LiteralPath ($Test -replace '\.sqf$','.toml') -Raw
$extra='"--width", "'+$Width+'", "--height", "'+$Height+'"'
$testConfig=[regex]::Replace($testConfig,'extra_args = \[(.*?)\]',{param($m) 'extra_args = ['+$m.Groups[1].Value+', '+$extra+']'})
[IO.File]::WriteAllText((Join-Path $workspace ($runTest -replace '\.sqf$','.toml')),$testConfig)
$Test=$runTest
$tri='D:/Arma_CWA/arma_CWR/engine/Trident/target/debug/tri.exe'
$game=Join-Path $workspace 'dist/x64-win-rwdi/PoseidonGame.exe'
$peak=0L
$start=Get-Date
try {
    $runner=Start-Process -FilePath $tri -ArgumentList @('test','--game-dir','dist/x64-win-rwdi','--data-dir','tmp/portrait-classic','--output-dir',$out,'--retries','0','-j','1',$Test) -WindowStyle Hidden -PassThru -RedirectStandardOutput ('build/portrait-'+$Case+'.log') -RedirectStandardError ('build/portrait-'+$Case+'.err')
    $handle=$runner.Handle
    while (!$runner.HasExited) {
        Get-Process PoseidonGame -ErrorAction SilentlyContinue | Where-Object {$_.Path -eq $game} | ForEach-Object {$peak=[Math]::Max($peak,$_.PeakWorkingSet64)}
        Start-Sleep -Milliseconds 500
        $runner.Refresh()
    }
    $runner.WaitForExit()
    $code=$runner.ExitCode
} finally { Get-Process PoseidonGame -ErrorAction SilentlyContinue | Where-Object {$_.Path -eq $game} | Stop-Process -Force }
$after=Get-ChildItem -LiteralPath $profileRoot -Recurse -Filter portrait.png
$changed=@($after | Where-Object {$before.ContainsKey($_.FullName) -and $before[$_.FullName] -ne $_.LastWriteTimeUtc.Ticks}).Count
[pscustomobject]@{case=$Case;exit=$code;elapsedSeconds=((Get-Date)-$start).TotalSeconds;peakWorkingSetMiB=[Math]::Round($peak/1MB,1);before=$before.Count;after=@($after).Count;rewritten=$changed;resolution="$Width x $Height";gamma=$Gamma} | ConvertTo-Json | Set-Content -LiteralPath ('build/portrait-'+$Case+'-metrics.json')
Get-Content -LiteralPath ('build/portrait-'+$Case+'-metrics.json')
Get-Content -LiteralPath ('build/portrait-'+$Case+'.log') -Tail 8
exit $code
