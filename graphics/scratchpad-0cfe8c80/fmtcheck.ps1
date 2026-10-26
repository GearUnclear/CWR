param([string]$Root = 'D:\Arma_CWA_worktrees\gr-A6')
$cf = 'C:\Program Files\LLVM\bin\clang-format.exe'
Set-Location (Join-Path $Root 'arma_CWR')
$files = git -C $Root status --porcelain -- arma_CWR | ForEach-Object { $_.Substring(3) } | Where-Object { $_ -match '\.(cpp|hpp)$' }
foreach ($rel in $files) {
    $f = $rel -replace '^arma_CWR/', ''
    & $cf --dry-run --Werror $f 2>&1 | Out-Null
    $now = $LASTEXITCODE
    $base = 'new'
    $content = git -C $Root show "HEAD:$rel" 2>$null
    if ($LASTEXITCODE -eq 0) {
        $tmp = [IO.Path]::GetTempFileName()
        git -C $Root show "HEAD:$rel" | Out-File -FilePath $tmp -Encoding utf8
        Get-Content -Raw $tmp | & $cf --dry-run --Werror "--assume-filename=$f" 2>&1 | Out-Null
        $base = $LASTEXITCODE
        Remove-Item $tmp
    }
    "{0,-75} now={1} base={2}" -f $f, $now, $base
}
