# B7 lane game runs, each with its own data junction (RemasterUD-gr-B7*), so
# each cleanup stops only its own games. Runs from the scratch copy gr-B7.
#   before  the lane stills on the base build (E:\gr-lanes\gr-B7\dist-before, c36ee4b)
#   new     the lane test on the B7 build (dist/gr-B7-rwdi)
#   smoke   smoke_abel on the B7 build
#   test    any test path (relative to arma_CWR) on the B7 build: -Test <path>
# Output goes to E:\gr-lanes\gr-B7\evidence (drive D is nearly full).
param([string]$Which, [string]$Tag = '', [string]$Test = '', [string]$Lane = 'b7lane')
$ErrorActionPreference = 'Continue'
$out = 'E:\gr-lanes\gr-B7\evidence'
$rc = 'D:\Arma_CWA_worktrees\gr-B7\arma_CWR\tests\rendering\captures\run-captures.ps1'
Set-Location 'D:\Arma_CWA_worktrees\gr-B7\arma_CWR'
$stamp = (Get-Date -Format 'yyyyMMdd-HHmmss') + $Tag
$laneTest = "../../graphics-remaster/tmp/integration/B7/tests/$Lane.test.sqf"
switch ($Which) {
    'before' { & powershell -NoProfile -File $rc -GameDir 'E:\gr-lanes\gr-B7\dist-before' -DataJunction RemasterUD-gr-B7before -OutDir "$out\$stamp-before" -Tests $laneTest }
    'new'    { & powershell -NoProfile -File $rc -GameDir dist/gr-B7-rwdi -DataJunction RemasterUD-gr-B7 -OutDir "$out\$stamp-new" -Tests $laneTest }
    'smoke'  { & powershell -NoProfile -File $rc -GameDir dist/gr-B7-rwdi -DataJunction RemasterUD-gr-B7smoke -OutDir "$out\$stamp-smoke" -Tests 'tests/rendering/captures/smoke_abel.test.sqf' }
    'test'   { & powershell -NoProfile -File $rc -GameDir dist/gr-B7-rwdi -DataJunction RemasterUD-gr-B7 -OutDir "$out\$stamp-test" -Tests $Test }
}
"run exit: $LASTEXITCODE ($Which, $stamp)"
