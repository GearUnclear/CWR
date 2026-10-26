# A2 gate runs (plan section 8 items 4 to 7), each with its own data junction
# (RemasterUD-gr-A2*) so the runs can share the machine and each cleanup stops
# only its own games.
param([string]$Which = 'all')
$ErrorActionPreference = 'Continue'
$a2 = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\A2'
$rc = 'tests\rendering\captures\run-captures.ps1'
Set-Location 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR'
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$smoke = 'tests/rendering/captures/smoke_abel.test.sqf,tests/rendering/captures/smoke_eden.test.sqf,tests/rendering/captures/smoke_sinai_lobo.test.sqf'
switch ($Which) {
    'smoke' {
        & powershell -NoProfile -File $rc -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-A2smoke -OutDir "$a2\$stamp-smoke" -Tests $smoke
    }
    'ab' {
        & powershell -NoProfile -File $rc -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-A2 -OutDir "$a2\$stamp-ab-A2" -Tests 'tests/rendering/captures/features_ab.test.sqf'
    }
    'abprev' {
        & powershell -NoProfile -File $rc -GameDir dist/graphics-remaster-rwdi-A1 -DataJunction RemasterUD-gr-A2prev -OutDir "$a2\$stamp-ab-A1build" -Tests 'tests/rendering/captures/features_ab.test.sqf'
    }
    'set' {
        & powershell -NoProfile -File $rc -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-A2set -OutDir "$a2\$stamp-set"
    }
    'setprev' {
        & powershell -NoProfile -File $rc -GameDir dist/graphics-remaster-rwdi-A1 -DataJunction RemasterUD-gr-A2prev -OutDir "$a2\$stamp-set-A1build"
    }
    'timing' {
        & powershell -NoProfile -File 'tests\rendering\captures\run-timing.ps1' -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-A2timing -OutRoot $a2 -Stamp $stamp
    }
}
"run-captures exit: $LASTEXITCODE ($Which, $stamp)"
