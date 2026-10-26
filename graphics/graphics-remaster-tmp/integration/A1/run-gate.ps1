# A1 gate runs (plan section 8 items 4 to 6, and the video skeleton), each with
# its own data junction so the runs can share the machine and each cleanup
# stops only its own games.
param([string]$Which = 'all')
$ErrorActionPreference = 'Continue'
$a1 = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\A1'
$rc = 'tests\rendering\captures\run-captures.ps1'
Set-Location 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR'
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
switch ($Which) {
    'smoke' {
        & powershell -NoProfile -File $rc -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-A1smoke -OutDir "$a1\$stamp-smoke" -Tests 'tests/rendering/captures/smoke_abel.test.sqf,tests/rendering/captures/smoke_eden.test.sqf,tests/rendering/captures/smoke_sinai_lobo.test.sqf'
    }
    'ab' {
        & powershell -NoProfile -File $rc -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-A1 -OutDir "$a1\$stamp-ab-A1" -Tests 'tests/rendering/captures/features_ab.test.sqf'
    }
    'set' {
        & powershell -NoProfile -File $rc -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-A1set -OutDir "$a1\$stamp-set"
    }
    'abprev' {
        & powershell -NoProfile -File $rc -GameDir dist/graphics-remaster-rwdi-A0 -DataJunction RemasterUD-gr-A1prev -OutDir "$a1\$stamp-ab-A0build-fixedclock" -Tests '../tmp/integration/A1/legacy/features_ab_legacy.test.sqf'
    }
    'overlay' {
        & powershell -NoProfile -File $rc -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-A1video -OutDir "$a1\$stamp-overlay" -Tests '../tmp/integration/A1/legacy/overlay_check.test.sqf'
    }
    'video' {
        & powershell -NoProfile -File $rc -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-A1video -OutDir "$a1\$stamp-video" -Tests '../tmp/integration/A1/legacy/overlay_check.test.sqf,tests/rendering/captures/video/paused_fire_check.test.sqf,tests/rendering/captures/video/perf_sunrise_ridge.test.sqf,tests/rendering/captures/video/temporal_forest_edge.test.sqf,tests/rendering/captures/video/shade_lowsun_fp_orbit.test.sqf,tests/rendering/captures/video/atmosphere_dawn_lapse.test.sqf,tests/rendering/captures/video/combat_ffh.test.sqf'
    }
}
"run-captures exit: $LASTEXITCODE ($Which, $stamp)"
