# B8 lane game runs, each with its own data junction (RemasterUD-gr-B8*), so
# each cleanup stops only its own games. Runs from the scratch copy gr-B8.
#   before  the lane stills on the base build (dist/gr-B8-rwdi-before, 807ce4b)
#   new     the lane stills on the B8 build (dist/gr-B8-rwdi)
#   smoke   smoke_abel on the B8 build
#   test    any test path (relative to arma_CWR) on the B8 build: -Test <path>
param([string]$Which, [string]$Tag = '', [string]$Test = '')
$ErrorActionPreference = 'Continue'
$lane = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\B8'
$rc = 'D:\Arma_CWA_worktrees\gr-B8\arma_CWR\tests\rendering\captures\run-captures.ps1'
Set-Location 'D:\Arma_CWA_worktrees\gr-B8\arma_CWR'
$stamp = (Get-Date -Format 'yyyyMMdd-HHmmss') + $Tag
$laneTest = '../../graphics-remaster/tmp/integration/B8/tests/b8lane.test.sqf'
switch ($Which) {
    # D: ran out of space: the base build's copy lives in the session scratchpad on C:.
    'before' { & powershell -NoProfile -File $rc -GameDir C:\Users\Dane\AppData\Local\Temp\claude\D--Arma-CWA\0cfe8c80-fc33-4f8e-a86e-52fa1175350d\scratchpad\gr-B8-rwdi-before -DataJunction RemasterUD-gr-B8before -OutDir "$lane\$stamp-before" -Tests $laneTest }
    'new'    { & powershell -NoProfile -File $rc -GameDir dist/gr-B8-rwdi -DataJunction RemasterUD-gr-B8 -OutDir "$lane\$stamp-new" -Tests $laneTest }
    'smoke'  { & powershell -NoProfile -File $rc -GameDir dist/gr-B8-rwdi -DataJunction RemasterUD-gr-B8smoke -OutDir "$lane\$stamp-smoke" -Tests 'tests/rendering/captures/smoke_abel.test.sqf' }
    'test'   { & powershell -NoProfile -File $rc -GameDir dist/gr-B8-rwdi -DataJunction RemasterUD-gr-B8 -OutDir "$lane\$stamp-test" -Tests $Test }
}
"run exit: $LASTEXITCODE ($Which, $stamp)"
