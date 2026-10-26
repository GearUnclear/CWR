# B11 lane game runs, each with its own data junction (RemasterUD-gr-B11*), so
# each cleanup stops only its own games. Runs from the scratch copy gr-B11.
#   before  the lane stills (Abel, Eden) on the base build (dist/gr-B11-rwdi-before, 193c735)
#   new     the lane stills (Abel, Eden) on the B11 build (dist/gr-B11-rwdi)
#   motion  the walk clip on the B11 build
#   smoke   smoke_abel on the B11 build
param([string]$Which, [string]$Tag = '')
$ErrorActionPreference = 'Continue'
$lane = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\B11\lane'
$rc = 'D:\Arma_CWA_worktrees\gr-B11\arma_CWR\tests\rendering\captures\run-captures.ps1'
Set-Location 'D:\Arma_CWA_worktrees\gr-B11\arma_CWR'
$stamp = (Get-Date -Format 'yyyyMMdd-HHmmss') + $Tag
$t = '../../graphics-remaster/tmp/integration/B11/lane/tests'
$stills = "$t/b11lane.test.sqf,$t/b11eden.test.sqf"
switch ($Which) {
    'before' { & powershell -NoProfile -File $rc -GameDir dist/gr-B11-rwdi-before -DataJunction RemasterUD-gr-B11before -OutDir "$lane\$stamp-before" -Tests $stills }
    'new'    { & powershell -NoProfile -File $rc -GameDir dist/gr-B11-rwdi -DataJunction RemasterUD-gr-B11 -OutDir "$lane\$stamp-new" -Tests $stills }
    'all'    { & powershell -NoProfile -File $rc -GameDir dist/gr-B11-rwdi -DataJunction RemasterUD-gr-B11 -OutDir "$lane\$stamp-all" -Tests "$stills,$t/b11offcheck.test.sqf,$t/video/b11motion.test.sqf" }
    'abel'   { & powershell -NoProfile -File $rc -GameDir dist/gr-B11-rwdi -DataJunction RemasterUD-gr-B11 -OutDir "$lane\$stamp-abel" -Tests "$t/b11lane.test.sqf" }
    'motion' { & powershell -NoProfile -File $rc -GameDir dist/gr-B11-rwdi -DataJunction RemasterUD-gr-B11 -OutDir "$lane\$stamp-motion" -Tests "$t/video/b11motion.test.sqf" }
    'smoke'  { & powershell -NoProfile -File $rc -GameDir dist/gr-B11-rwdi -DataJunction RemasterUD-gr-B11smoke -OutDir "$lane\$stamp-smoke" -Tests 'tests/rendering/captures/smoke_abel.test.sqf' }
}
"run exit: $LASTEXITCODE ($Which, $stamp)"
