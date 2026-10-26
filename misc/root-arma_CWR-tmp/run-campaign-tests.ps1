param([string]$Label, [string]$Render = 'dummy', [string[]]$Cases)
$ErrorActionPreference = 'Stop'
$repo = 'D:\Arma_CWA\arma_CWR'
Set-Location -LiteralPath $repo
$root = Join-Path $repo "tmp/acceptance-20261005-$Label"
[void][IO.Directory]::CreateDirectory($root)
$results = @()
foreach ($case in $Cases) {
    $name = [IO.Path]::GetFileName($case).Replace('.test.sqf','').Replace('.seq','')
    $out = Join-Path $root $name
    [void][IO.Directory]::CreateDirectory($out)
    $runner = $null
    try {
        $runner = Start-Process -FilePath "$repo/engine/Trident/target/debug/tri.exe" -ArgumentList @('test','--render',$Render,'-j1','--retries','0','--output-dir',$out,$case) -WorkingDirectory $repo -WindowStyle Hidden -RedirectStandardOutput "$out/runner.log" -RedirectStandardError "$out/runner-error.log" -PassThru
        $runner.WaitForExit()
        $exitCode = $runner.ExitCode
        $results += [pscustomobject]@{Case=$case; ExitCode=$exitCode; Artifacts=$out}
        $results | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath "$root/results.json"
        Write-Output "$name : exit $exitCode"
        Get-Content -LiteralPath "$out/runner.log" -Tail 8
    } finally {
        if ($runner) {
            Get-CimInstance Win32_Process -Filter "Name = 'PoseidonGame.exe'" | Where-Object ParentProcessId -EQ $runner.Id | ForEach-Object {Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue}
        }
    }
}
if (@($results | Where-Object ExitCode -NE 0).Count) {exit 1}
exit 0
