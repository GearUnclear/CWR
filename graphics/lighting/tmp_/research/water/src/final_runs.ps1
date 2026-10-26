# Final shoot: three sequential passes of stock, A, B, C (one game at a time from
# this study), so the GPU timings can be taken as the median of three.
$s = 'D:\Arma_CWA_worktrees\lighting\tmp\research\water\src\run_variant.ps1'
foreach ($pass in 1, 2, 3) {
    foreach ($id in 'stock', 'A', 'B', 'C') {
        "=== pass $pass $id $(Get-Date -Format HH:mm:ss)"
        & powershell -NoProfile -ExecutionPolicy Bypass -File $s -Id $id -Junction RemasterUD-water -Tag "final$pass" | Select-Object -Last 2
    }
}
"=== done $(Get-Date -Format HH:mm:ss)"
