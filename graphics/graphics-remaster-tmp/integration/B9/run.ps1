# B9 lane game runs, each with its own data junction (RemasterUD-gr-B9*), so
# each cleanup stops only its own games. Runs from the scratch copy gr-B9.
#   before  the lane stills on the base build (dist/gr-B9-rwdi-before, 807ce4b)
#   new     the lane stills on the B9 build (dist/gr-B9-rwdi), with triCanopyCheck
#   smoke   smoke_abel on the B9 build
#   timing  the timing reading (run-timing.ps1: GPU clock sampler, summarize_timing.py)
#   test    any test path (relative to arma_CWR) on the B9 build: -Test <path>
param([string]$Which, [string]$Tag = '', [string]$Test = '')
$ErrorActionPreference = 'Continue'
$lane = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\B9'
$caps = 'D:\Arma_CWA_worktrees\gr-B9\arma_CWR\tests\rendering\captures'
$rc = Join-Path $caps 'run-captures.ps1'
$rt = Join-Path $caps 'run-timing.ps1'
Set-Location 'D:\Arma_CWA_worktrees\gr-B9\arma_CWR'
$stamp = (Get-Date -Format 'yyyyMMdd-HHmmss') + $Tag
$laneTest = '../../graphics-remaster/tmp/integration/B9/tests/b9lane.test.sqf'
$newTest = '../../graphics-remaster/tmp/integration/B9/tests/b9new.test.sqf'
$timingTest = '../../graphics-remaster/tmp/integration/B9/tests/b9timing.test.sqf'
switch ($Which) {
    'before' { & powershell -NoProfile -File $rc -GameDir dist/gr-B9-rwdi-before -DataJunction RemasterUD-gr-B9before -OutDir "$lane\$stamp-before" -Tests $laneTest }
    'new'    { & powershell -NoProfile -File $rc -GameDir dist/gr-B9-rwdi -DataJunction RemasterUD-gr-B9 -OutDir "$lane\$stamp-new" -Tests $newTest }
    'smoke'  { & powershell -NoProfile -File $rc -GameDir dist/gr-B9-rwdi -DataJunction RemasterUD-gr-B9smoke -OutDir "$lane\$stamp-smoke" -Tests 'tests/rendering/captures/smoke_abel.test.sqf' }
    'timing' { & powershell -NoProfile -File $rt -GameDir dist/gr-B9-rwdi -DataJunction RemasterUD-gr-B9timing -OutRoot $lane -Stamp $stamp -Tests $timingTest }
    'test'   { & powershell -NoProfile -File $rc -GameDir dist/gr-B9-rwdi -DataJunction RemasterUD-gr-B9 -OutDir "$lane\$stamp-test" -Tests $Test }
}
"run exit: $LASTEXITCODE ($Which, $stamp)"
