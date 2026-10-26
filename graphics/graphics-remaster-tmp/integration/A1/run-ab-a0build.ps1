# Gate item 5 reference: the feature A/B test on the previous step's build (A0,
# copied to dist/graphics-remaster-rwdi-A0 before the A1 build). Both shots of
# every view are plain "remastered" there (the switch keys are refused).
$ErrorActionPreference = 'Continue'
$a1 = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\A1'
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$out = Join-Path $a1 "$stamp-ab-A0build"
"slots before: " + ((& 'D:\Arma_CWA\arma_CWR\target\debug\tri.exe' slots) -join ' | ')
Set-Location 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR'
& powershell -NoProfile -File tests\rendering\captures\run-captures.ps1 -GameDir dist/graphics-remaster-rwdi-A0 -DataJunction RemasterUD-gr-A1prev -OutDir $out -Tests '../tmp/integration/A1/legacy/features_ab_legacy.test.sqf'
"run-captures exit: $LASTEXITCODE"
"out: $out"
