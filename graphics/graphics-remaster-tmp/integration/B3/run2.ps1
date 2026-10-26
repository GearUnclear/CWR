# B3 lane game runs on base 7b7e3e6 (scratch copy D:\Arma_CWA_worktrees\gr-B3).
# Each run has its own data junction (RemasterUD-gr-B3*), so each cleanup stops
# only its own games.
#   before  the lane stills (b3v2: 8 night/day views + 3 dusk views) on the base
#           build 7b7e3e6 (a copy in the session scratchpad, dist-before-7b7e3e6)
#   new     the same stills on the B3 build (dist/gr-B3-rwdi)
#   test    any test path (relative to arma_CWR) on the B3 build: -Test <path>
#   smoke   smoke_abel on the B3 build
param([string]$Which, [string]$Tag = '', [string]$Test = '', [string]$GameDir = 'dist/gr-B3-rwdi')
$ErrorActionPreference = 'Continue'
$lane = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\B3'
$scratch = 'C:\Users\Dane\AppData\Local\Temp\claude\D--Arma-CWA\0cfe8c80-fc33-4f8e-a86e-52fa1175350d\scratchpad'
$rc = 'D:\Arma_CWA_worktrees\gr-B3\arma_CWR\tests\rendering\captures\run-captures.ps1'
$beforeDir = "$scratch\dist-before-7b7e3e6"
Set-Location 'D:\Arma_CWA_worktrees\gr-B3\arma_CWR'
$stamp = (Get-Date -Format 'yyyyMMdd-HHmmss') + $Tag
$tests = '../../graphics-remaster/tmp/integration/B3/tests'
# Captures land in the scratchpad (D: runs full); the evidence kept goes to the lane folder.
$out = "$scratch\b3lane\captures"
switch ($Which) {
    'before' { & powershell -NoProfile -File $rc -GameDir $beforeDir -DataJunction RemasterUD-gr-B3before -OutDir "$out\$stamp-before" -Tests "$tests/b3v2.test.sqf" }
    'new'    { & powershell -NoProfile -File $rc -GameDir $GameDir -DataJunction RemasterUD-gr-B3 -OutDir "$out\$stamp-new" -Tests "$tests/b3v3.test.sqf" }
    'smoke'  { & powershell -NoProfile -File $rc -GameDir $GameDir -DataJunction RemasterUD-gr-B3smoke -OutDir "$out\$stamp-smoke" -Tests 'tests/rendering/captures/smoke_abel.test.sqf' }
    'test'   { & powershell -NoProfile -File $rc -GameDir $GameDir -DataJunction RemasterUD-gr-B3 -OutDir "$out\$stamp-test" -Tests $Test }
}
"run exit: $LASTEXITCODE ($Which, $stamp)"
