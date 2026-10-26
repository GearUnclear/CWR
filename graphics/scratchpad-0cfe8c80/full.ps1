$root = 'D:\Arma_CWA_worktrees\lighting\arma_CWR\build\lighting-rwdi\tests\unit\engine\Poseidon'
$out = 'D:\Arma_CWA_worktrees\lighting\tmp\bugfix'
$runs = @(
    @{ exe = Join-Path $root 'PoseidonTests.exe'; log = 'integ2_full_poseidon.txt' },
    @{ exe = Join-Path $root 'PoseidonCoreTests.exe'; log = 'integ2_full_core.txt' },
    @{ exe = Join-Path $root 'Foundation\PoseidonFoundationTests.exe'; log = 'integ2_full_foundation.txt' }
)
foreach ($r in $runs) {
    Push-Location (Split-Path -Parent $r.exe)
    try {
        $o = & $r.exe 2>&1 | ForEach-Object { "$_" }
        $code = $LASTEXITCODE
    } finally { Pop-Location }
    $o | Out-File -FilePath (Join-Path $out $r.log) -Encoding utf8
    "== $($r.log) exit=$code"
    $o | Where-Object { $_ -match '^test cases:|^assertions:|All tests passed' } | Select-Object -Last 3
    for ($i = 0; $i -lt $o.Count; $i++) { if ($o[$i] -match 'FAILED:$') { "   $($o[$i])" } }
}
