# B5 lane game runs, each with its own data junction (RemasterUD-gr-B5*), so
# each cleanup stops only its own games. Runs from the scratch copy gr-B5.
#   before  the lane stills on the base build (dist/gr-B5-rwdi-before, 5d7e7ae)
#   new     the lane stills on the B5 build (dist/gr-B5-rwdi)
#   smoke   smoke_abel on the B5 build
#   test    any test path (relative to arma_CWR) on the B5 build: -Test <path>
param([string]$Which, [string]$Tag = '', [string]$Test = '')
$ErrorActionPreference = 'Continue'
$lane = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\B5'
$rc = 'D:\Arma_CWA_worktrees\gr-B5\arma_CWR\tests\rendering\captures\run-captures.ps1'
Set-Location 'D:\Arma_CWA_worktrees\gr-B5\arma_CWR'
$stamp = (Get-Date -Format 'yyyyMMdd-HHmmss') + $Tag
$laneTest = '../../graphics-remaster/tmp/integration/B5/tests/b5lane.test.sqf'
# The base build has no sea sub-knobs: its test leaves the sweep shots out.
$baseTest = '../../graphics-remaster/tmp/integration/B5/tests/b5base.test.sqf'
switch ($Which) {
    'before' { & powershell -NoProfile -File $rc -GameDir dist/gr-B5-rwdi-before -DataJunction RemasterUD-gr-B5before -OutDir "$lane\$stamp-before" -Tests $baseTest }
    'newbase' { & powershell -NoProfile -File $rc -GameDir dist/gr-B5-rwdi -DataJunction RemasterUD-gr-B5 -OutDir "$lane\$stamp-newbase" -Tests $baseTest }
    'new'    { & powershell -NoProfile -File $rc -GameDir dist/gr-B5-rwdi -DataJunction RemasterUD-gr-B5 -OutDir "$lane\$stamp-new" -Tests $laneTest }
    'smoke'  { & powershell -NoProfile -File $rc -GameDir dist/gr-B5-rwdi -DataJunction RemasterUD-gr-B5smoke -OutDir "$lane\$stamp-smoke" -Tests 'tests/rendering/captures/smoke_abel.test.sqf' }
    'test'   { & powershell -NoProfile -File $rc -GameDir dist/gr-B5-rwdi -DataJunction RemasterUD-gr-B5 -OutDir "$lane\$stamp-test" -Tests $Test }
}
"run exit: $LASTEXITCODE ($Which, $stamp)"
