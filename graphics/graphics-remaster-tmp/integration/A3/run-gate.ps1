# A3 gate runs (plan section 8 items 4 to 7), each with its own data junction
# (RemasterUD-gr-A3*) so the runs can share the machine and each cleanup stops
# only its own games. The A2 build (HEAD af5b999) is a copy in
# dist/graphics-remaster-rwdi-A2.
param([string]$Which = 'all')
$ErrorActionPreference = 'Continue'
$a3 = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\A3'
$rc = 'tests\rendering\captures\run-captures.ps1'
Set-Location 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR'
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$smoke = 'tests/rendering/captures/smoke_abel.test.sqf,tests/rendering/captures/smoke_eden.test.sqf,tests/rendering/captures/smoke_sinai_lobo.test.sqf'
switch ($Which) {
    'smoke' {
        & powershell -NoProfile -File $rc -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-A3smoke -OutDir "$a3\$stamp-smoke" -Tests $smoke
    }
    'ab' {
        & powershell -NoProfile -File $rc -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-A3 -OutDir "$a3\$stamp-ab-A3" -Tests 'tests/rendering/captures/features_ab.test.sqf'
    }
    'abprev' {
        & powershell -NoProfile -File $rc -GameDir dist/graphics-remaster-rwdi-A2 -DataJunction RemasterUD-gr-A3prev -OutDir "$a3\$stamp-ab-A2build" -Tests 'tests/rendering/captures/features_ab.test.sqf'
    }
    'set' {
        & powershell -NoProfile -File $rc -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-A3set -OutDir "$a3\$stamp-set"
    }
    'setprev' {
        & powershell -NoProfile -File $rc -GameDir dist/graphics-remaster-rwdi-A2 -DataJunction RemasterUD-gr-A3prev -OutDir "$a3\$stamp-set-A2build"
    }
    'timing' {
        & powershell -NoProfile -File 'tests\rendering\captures\run-timing.ps1' -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-A3timing -OutRoot $a3 -Stamp $stamp
    }
    'lindepth' {
        & powershell -NoProfile -File 'tests\rendering\captures\run-timing.ps1' -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-A3lindepth -OutRoot $a3 -Stamp "$stamp-lindepth" -Tests '../tmp/integration/A3/lindepth/lindepth_abel.test.sqf'
    }
}
"run-captures exit: $LASTEXITCODE ($Which, $stamp)"
