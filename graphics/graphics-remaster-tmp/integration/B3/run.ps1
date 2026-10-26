# B3 lane game runs, each with its own data junction (RemasterUD-gr-B3*), so
# each cleanup stops only its own games. Runs from the scratch copy gr-B3.
#   before  the lane stills on the base build (706b9a3; moved off the full D:
#           drive into the session scratchpad after the BEFORE run)
#   new     the lane stills on the B3 build (dist/gr-B3-rwdi)
#   each    the lane stills plus each switch alone on the B3 build
#   dev     the tuning shots (strengths, the moon's shadows in debug view 1)
#   smoke   smoke_abel on the B3 build
#   test    any test path (relative to arma_CWR) on the B3 build: -Test <path>
param([string]$Which, [string]$Tag = '', [string]$Test = '', [string]$GameDir = 'dist/gr-B3-rwdi')
$ErrorActionPreference = 'Continue'
$lane = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\B3'
$rc = 'D:\Arma_CWA_worktrees\gr-B3\arma_CWR\tests\rendering\captures\run-captures.ps1'
$beforeDir = 'C:\Users\Dane\AppData\Local\Temp\claude\D--Arma-CWA\0cfe8c80-fc33-4f8e-a86e-52fa1175350d\scratchpad\dist-before'
Set-Location 'D:\Arma_CWA_worktrees\gr-B3\arma_CWR'
$stamp = (Get-Date -Format 'yyyyMMdd-HHmmss') + $Tag
$tests = '../../graphics-remaster/tmp/integration/B3/tests'
switch ($Which) {
    'before' { & powershell -NoProfile -File $rc -GameDir $beforeDir -DataJunction RemasterUD-gr-B3before -OutDir "$lane\captures\$stamp-before" -Tests "$tests/b3lane.test.sqf" }
    'new'    { & powershell -NoProfile -File $rc -GameDir $GameDir -DataJunction RemasterUD-gr-B3 -OutDir "$lane\captures\$stamp-new" -Tests "$tests/b3lane.test.sqf" }
    'each'   { & powershell -NoProfile -File $rc -GameDir $GameDir -DataJunction RemasterUD-gr-B3 -OutDir "$lane\captures\$stamp-each" -Tests "$tests/b3each.test.sqf" }
    'dev'    { & powershell -NoProfile -File $rc -GameDir $GameDir -DataJunction RemasterUD-gr-B3 -OutDir "$lane\captures\$stamp-dev" -Tests "$tests/b3dev.test.sqf" }
    'smoke'  { & powershell -NoProfile -File $rc -GameDir $GameDir -DataJunction RemasterUD-gr-B3smoke -OutDir "$lane\captures\$stamp-smoke" -Tests 'tests/rendering/captures/smoke_abel.test.sqf' }
    'test'   { & powershell -NoProfile -File $rc -GameDir $GameDir -DataJunction RemasterUD-gr-B3 -OutDir "$lane\captures\$stamp-test" -Tests $Test }
}
"run exit: $LASTEXITCODE ($Which, $stamp)"
