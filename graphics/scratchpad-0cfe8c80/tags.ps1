param([string]$Out, [string[]]$Extra = @())
$Extra = @($Extra | ForEach-Object { $_ -split ',' } | Where-Object { $_ })
$exe = 'D:\Arma_CWA_worktrees\lighting\arma_CWR\build\lighting-rwdi\tests\unit\engine\Poseidon\PoseidonTests.exe'
$dir = Split-Path -Parent $exe
$tags = $Extra + @('[remaster]', '[graphics]', '[gpu]', '[GL33]', '[Settings]', '[DebugGroup]', '[ShutdownOrder]',
    '[UBORestoreGuard]', '[StateOwnership]', '[StateCacheAudit]', '[BufferUsageAudit]', '[IboBindAudit]',
    '[Conventions]', '[Shaders]', '[paramFileExt]', '[io]', '[paramfile]', '[config]')
$lines = @()
Push-Location $dir
try {
    foreach ($t in $tags) {
        $o = & $exe $t 2>&1 | ForEach-Object { "$_" }
        $sum = $o | Where-Object { $_ -match 'All tests passed|test cases:|assertions:|FAILED:|No test cases matched' }
        $fails = @()
        for ($i = 0; $i -lt $o.Count; $i++) { if ($o[$i] -match 'FAILED:$') { $fails += $o[$i] } }
        $lines += "$t exit=$LASTEXITCODE : " + (($sum | Where-Object { $_ -notmatch 'FAILED:' }) -join ' | ')
        foreach ($f in $fails) { $lines += "   $f" }
    }
} finally { Pop-Location }
$lines | Out-File -FilePath $Out -Encoding utf8
$lines
