param(
    [Parameter(Mandatory=$true)][string]$TestPath,
    [Parameter(Mandatory=$true)][string]$OutputDir,
    [string]$Tri = '',
    [string]$GameDir = 'D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi',
    [string]$DataDir = 'D:\Arma_CWA\ARMA Cold War Assault [Classic]',
    [string]$Render = 'gl33',
    [int]$Retries = 0,
    [int]$TimeoutSeconds = 540
)
$ErrorActionPreference = 'Stop'
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
$engineRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../../..')).Path
if (-not $Tri) { $Tri = Join-Path $engineRoot 'engine/Trident/target/debug/tri.exe' }
if (-not (Test-Path -LiteralPath $Tri -PathType Leaf)) { throw 'Build Trident in this checkout or pass -Tri with the current runner path.' }
$argItems = @('test','--game-dir',$GameDir,'--data-dir',$DataDir,'--render',$Render,'--retries',"$Retries",'-j','1','--output-dir',$OutputDir,$TestPath)
$argLine = ($argItems | ForEach-Object { '"' + $_.Replace('"','\"') + '"' }) -join ' '
$runner = $null
try {
    $runner = Start-Process -FilePath $Tri -ArgumentList $argLine -WorkingDirectory $engineRoot -WindowStyle Hidden -PassThru -RedirectStandardOutput "$OutputDir\runner.stdout.log" -RedirectStandardError "$OutputDir\runner.stderr.log"
    $taskProcessHandle = $runner.Handle
    if (-not $runner.WaitForExit($TimeoutSeconds * 1000)) { throw "Acceptance runner exceeded $TimeoutSeconds seconds" }
    $runExit = $runner.ExitCode
    if ($null -eq $runExit) { throw 'Runner exited without an available exit code; inspect captured logs.' }
    "exit_code=$runExit" | Set-Content -LiteralPath "$OutputDir\exit.txt"
    Get-Content -LiteralPath "$OutputDir\runner.stdout.log" -Tail 20
    Get-Content -LiteralPath "$OutputDir\runner.stderr.log" -Tail 10
    exit $runExit
} finally {
    if ($runner) {
        Get-CimInstance Win32_Process | Where-Object { $_.ParentProcessId -eq $runner.Id -and $_.Name -in @('PoseidonGame.exe','PoseidonServer.exe') } | ForEach-Object { Stop-Process -Id $_.ProcessId -Force -ErrorAction SilentlyContinue }
        if (-not $runner.HasExited) { Stop-Process -Id $runner.Id -Force -ErrorAction SilentlyContinue }
    }
}
