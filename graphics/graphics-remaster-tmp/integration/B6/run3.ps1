# B6 review round game runs on the B6 build (dist/gr-B6-rwdi), junction
# RemasterUD-gr-B6, so the cleanup stops only this lane's games. Runs from the
# scratch copy gr-B6. -Tests: comma list of test file names under tests\ here;
# -Which smoke runs smoke_abel instead (junction RemasterUD-gr-B6smoke).
# Output goes to E:\gr-lanes\gr-B6\evidence (drive D is nearly full).
param([string]$Which = 'new', [string]$Tests = '', [string]$Tag = '', [string]$Override = '')
$ErrorActionPreference = 'Continue'
$out = 'E:\gr-lanes\gr-B6\evidence'
$rc = 'D:\Arma_CWA_worktrees\gr-B6\arma_CWR\tests\rendering\captures\run-captures.ps1'
Set-Location 'D:\Arma_CWA_worktrees\gr-B6\arma_CWR'
$stamp = (Get-Date -Format 'yyyyMMdd-HHmmss') + $Tag
$t = ($Tests -split ',' | ForEach-Object { "../../graphics-remaster/tmp/integration/B6/tests/$_" }) -join ','
$extra = @()
if ($Override) { $extra = @('-ShaderOverrideDir', $Override) }
switch ($Which) {
    'new'   { & powershell -NoProfile -File $rc -GameDir dist/gr-B6-rwdi -DataJunction RemasterUD-gr-B6 -OutDir "$out\$stamp-new" -Tests $t @extra }
    'smoke' { & powershell -NoProfile -File $rc -GameDir dist/gr-B6-rwdi -DataJunction RemasterUD-gr-B6smoke -OutDir "$out\$stamp-smoke" -Tests 'tests/rendering/captures/smoke_abel.test.sqf' }
}
"run exit: $LASTEXITCODE ($Which, $stamp)"
