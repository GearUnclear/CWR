# B6 lane game runs, each with its own data junction (RemasterUD-gr-B6*), so
# each cleanup stops only its own games. Runs from the scratch copy gr-B6.
#   before  the lane stills on the base build (dist/gr-B6-rwdi-before, e91b3f3)
#   new     the lane test on the B6 build (dist/gr-B6-rwdi)
#   smoke   smoke_abel on the B6 build
#   test    any test path (relative to arma_CWR) on the B6 build: -Test <path>
# Output goes to E:\gr-lanes\gr-B6\evidence (linked as B6\files): drive D was
# full when the lane started.
param([string]$Which, [string]$Tag = '', [string]$Test = '')
$ErrorActionPreference = 'Continue'
$out = 'E:\gr-lanes\gr-B6\evidence'
$rc = 'D:\Arma_CWA_worktrees\gr-B6\arma_CWR\tests\rendering\captures\run-captures.ps1'
Set-Location 'D:\Arma_CWA_worktrees\gr-B6\arma_CWR'
$stamp = (Get-Date -Format 'yyyyMMdd-HHmmss') + $Tag
$laneTest = '../../graphics-remaster/tmp/integration/B6/tests/b6lane.test.sqf'
$baseTest = '../../graphics-remaster/tmp/integration/B6/tests/b6base.test.sqf'
switch ($Which) {
    'before' { & powershell -NoProfile -File $rc -GameDir dist/gr-B6-rwdi-before -DataJunction RemasterUD-gr-B6before -OutDir "$out\$stamp-before" -Tests $baseTest }
    'new'    { & powershell -NoProfile -File $rc -GameDir dist/gr-B6-rwdi -DataJunction RemasterUD-gr-B6 -OutDir "$out\$stamp-new" -Tests $laneTest }
    'smoke'  { & powershell -NoProfile -File $rc -GameDir dist/gr-B6-rwdi -DataJunction RemasterUD-gr-B6smoke -OutDir "$out\$stamp-smoke" -Tests 'tests/rendering/captures/smoke_abel.test.sqf' }
    'test'   { & powershell -NoProfile -File $rc -GameDir dist/gr-B6-rwdi -DataJunction RemasterUD-gr-B6 -OutDir "$out\$stamp-test" -Tests $Test }
}
"run exit: $LASTEXITCODE ($Which, $stamp)"
