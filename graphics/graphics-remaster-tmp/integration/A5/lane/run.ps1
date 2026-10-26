# A5 lane game runs, each with its own data junction (RemasterUD-gr-A5*), so
# each cleanup stops only its own games. Runs from the scratch copy gr-A5.
#   before  the lane stills on the base build (dist/gr-A5-rwdi-before, ced77ae)
#   new     the lane stills on the A5 build (dist/gr-A5-rwdi)
#   motion  the strafe and pan clips on the A5 build
#   smoke   smoke_abel on the A5 build
#   ab      the full features_ab on the A5 build (not part of the light gate)
param([string]$Which, [string]$Tag = '')
$ErrorActionPreference = 'Continue'
$lane = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\A5\lane'
$rc = 'D:\Arma_CWA_worktrees\gr-A5\arma_CWR\tests\rendering\captures\run-captures.ps1'
Set-Location 'D:\Arma_CWA_worktrees\gr-A5\arma_CWR'
$stamp = (Get-Date -Format 'yyyyMMdd-HHmmss') + $Tag
$laneTest = '../../graphics-remaster/tmp/integration/A5/lane/tests/a5lane.test.sqf'
$motionTest = '../../graphics-remaster/tmp/integration/A5/lane/tests/video/a5motion.test.sqf'
switch ($Which) {
    'before' { & powershell -NoProfile -File $rc -GameDir dist/gr-A5-rwdi-before -DataJunction RemasterUD-gr-A5before -OutDir "$lane\$stamp-before" -Tests $laneTest }
    'new'    { & powershell -NoProfile -File $rc -GameDir dist/gr-A5-rwdi -DataJunction RemasterUD-gr-A5 -OutDir "$lane\$stamp-new" -Tests $laneTest }
    'motion' { & powershell -NoProfile -File $rc -GameDir dist/gr-A5-rwdi -DataJunction RemasterUD-gr-A5 -OutDir "$lane\$stamp-motion" -Tests $motionTest }
    'sweep'  { & powershell -NoProfile -File $rc -GameDir dist/gr-A5-rwdi -DataJunction RemasterUD-gr-A5 -OutDir "$lane\$stamp-sweep" -Tests '../../graphics-remaster/tmp/integration/A5/lane/tests/a5sweep.test.sqf' }
    'cost'   { & powershell -NoProfile -File $rc -GameDir dist/gr-A5-rwdi -DataJunction RemasterUD-gr-A5 -OutDir "$lane\$stamp-cost" -Tests '../../graphics-remaster/tmp/integration/A5/lane/tests/a5cost.test.sqf' }
    'sinai'  { & powershell -NoProfile -File $rc -GameDir dist/gr-A5-rwdi -DataJunction RemasterUD-gr-A5 -OutDir "$lane\$stamp-sinai" -Tests '../../graphics-remaster/tmp/integration/A5/lane/tests/a5sinai.test.sqf' }
    'smoke'  { & powershell -NoProfile -File $rc -GameDir dist/gr-A5-rwdi -DataJunction RemasterUD-gr-A5smoke -OutDir "$lane\$stamp-smoke" -Tests 'tests/rendering/captures/smoke_abel.test.sqf' }
}
"run exit: $LASTEXITCODE ($Which, $stamp)"
