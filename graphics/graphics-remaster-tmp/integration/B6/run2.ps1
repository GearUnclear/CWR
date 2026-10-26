# B6 lane game runs, round 2, each with its own data junction, so each cleanup
# stops only its own games. Runs from the scratch copy gr-B6.
#   -Which base   a test on the base build (dist/gr-B6-rwdi-before, e91b3f3), junction RemasterUD-gr-B6before
#   -Which new    a test on the B6 build (dist/gr-B6-rwdi), junction RemasterUD-gr-B6
#   -Which smoke  smoke_abel on the B6 build, junction RemasterUD-gr-B6smoke
# -Test is the test file name under tests\ here (base and new). Output goes to
# E:\gr-lanes\gr-B6\evidence (linked as B6\files): drive D is nearly full.
param([string]$Which, [string]$Test = '', [string]$Tag = '')
$ErrorActionPreference = 'Continue'
$out = 'E:\gr-lanes\gr-B6\evidence'
$rc = 'D:\Arma_CWA_worktrees\gr-B6\arma_CWR\tests\rendering\captures\run-captures.ps1'
Set-Location 'D:\Arma_CWA_worktrees\gr-B6\arma_CWR'
$stamp = (Get-Date -Format 'yyyyMMdd-HHmmss') + $Tag
$t = "../../graphics-remaster/tmp/integration/B6/tests/$Test"
switch ($Which) {
    'base'  { & powershell -NoProfile -File $rc -GameDir dist/gr-B6-rwdi-before -DataJunction RemasterUD-gr-B6before -OutDir "$out\$stamp-base" -Tests $t }
    'new'   { & powershell -NoProfile -File $rc -GameDir dist/gr-B6-rwdi -DataJunction RemasterUD-gr-B6 -OutDir "$out\$stamp-new" -Tests $t }
    'smoke' { & powershell -NoProfile -File $rc -GameDir dist/gr-B6-rwdi -DataJunction RemasterUD-gr-B6smoke -OutDir "$out\$stamp-smoke" -Tests 'tests/rendering/captures/smoke_abel.test.sqf' }
}
"run exit: $LASTEXITCODE ($Which, $stamp)"
