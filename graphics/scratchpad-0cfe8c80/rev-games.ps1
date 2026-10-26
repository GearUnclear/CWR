# Reviewer game runs for A2: smoke on the rebuilt A2 binary, then features_ab
# (to compare the repeatable views with the implementer's A1-build run).
$ErrorActionPreference = 'Continue'
$out = 'C:\Users\Dane\AppData\Local\Temp\claude\D--Arma-CWA\0cfe8c80-fc33-4f8e-a86e-52fa1175350d\scratchpad\games'
New-Item -ItemType Directory -Force $out | Out-Null
$rc = 'tests\rendering\captures\run-captures.ps1'
Set-Location 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR'
"slots before: " + ((& D:\Arma_CWA\arma_CWR\target\debug\tri.exe slots 2>&1) -join ' | ')
$smoke = 'tests/rendering/captures/smoke_abel.test.sqf,tests/rendering/captures/smoke_eden.test.sqf,tests/rendering/captures/smoke_sinai_lobo.test.sqf'
& powershell -NoProfile -File $rc -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-rev-A2 -OutDir "$out\smoke" -Tests $smoke
"smoke exit: $LASTEXITCODE"
& powershell -NoProfile -File $rc -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-rev-A2 -OutDir "$out\ab" -Tests 'tests/rendering/captures/features_ab.test.sqf'
"ab exit: $LASTEXITCODE"
"slots after: " + ((& D:\Arma_CWA\arma_CWR\target\debug\tri.exe slots 2>&1) -join ' | ')
