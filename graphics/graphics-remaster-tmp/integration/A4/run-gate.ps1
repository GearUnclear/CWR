# A4 gate runs (plan section 8 items 4 to 7), each with its own data junction
# (RemasterUD-gr-A4*) so the runs can share the machine and each cleanup stops
# only its own games. The A3 build (HEAD ced77ae) is a copy in
# dist/graphics-remaster-rwdi-A3.
param([string]$Which = 'all')
$ErrorActionPreference = 'Continue'
$a4 = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\integration\A4'
$rc = 'tests\rendering\captures\run-captures.ps1'
Set-Location 'D:\Arma_CWA_worktrees\graphics-remaster\arma_CWR'
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$smoke = 'tests/rendering/captures/smoke_abel.test.sqf,tests/rendering/captures/smoke_eden.test.sqf,tests/rendering/captures/smoke_sinai_lobo.test.sqf'
switch ($Which) {
    'smoke' {
        & powershell -NoProfile -File $rc -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-A4smoke -OutDir "$a4\$stamp-smoke" -Tests $smoke
    }
    'ab' {
        & powershell -NoProfile -File $rc -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-A4 -OutDir "$a4\$stamp-ab-A4" -Tests 'tests/rendering/captures/features_ab.test.sqf'
    }
    'abprev' {
        & powershell -NoProfile -File $rc -GameDir dist/graphics-remaster-rwdi-A3 -DataJunction RemasterUD-gr-A4prev -OutDir "$a4\$stamp-ab-A3build" -Tests 'tests/rendering/captures/features_ab.test.sqf'
    }
    'set' {
        & powershell -NoProfile -File $rc -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-A4set -OutDir "$a4\$stamp-set"
    }
    'setprev' {
        & powershell -NoProfile -File $rc -GameDir dist/graphics-remaster-rwdi-A3 -DataJunction RemasterUD-gr-A4prev -OutDir "$a4\$stamp-set-A3build"
    }
    'timing' {
        & powershell -NoProfile -File 'tests\rendering\captures\run-timing.ps1' -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-A4timing -OutRoot $a4 -Stamp $stamp
    }
    'perf' {
        # The A4 checks (perf/gen_perf.py): off and on in one game per view,
        # 3 interleaved rounds, with the GPU clock sampler.
        & powershell -NoProfile -File 'tests\rendering\captures\run-timing.ps1' -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-A4perf -OutRoot $a4 -Stamp "$stamp-perf" -Tests '../tmp/integration/A4/perf/perf_abel.test.sqf'
    }
    'quick' {
        & powershell -NoProfile -File 'tests\rendering\captures\run-timing.ps1' -GameDir dist/graphics-remaster-rwdi -DataJunction RemasterUD-gr-A4perf -OutRoot $a4 -Stamp "$stamp-quick" -Tests '../tmp/integration/A4/perf/perf_quick.test.sqf'
    }
    'perfprev' {
        & powershell -NoProfile -File 'tests\rendering\captures\run-timing.ps1' -GameDir dist/graphics-remaster-rwdi-A3 -DataJunction RemasterUD-gr-A4perfprev -OutRoot $a4 -Stamp "$stamp-perf-A3build" -Tests '../tmp/integration/A4/perf/perf_abel.test.sqf'
    }
}
"run-captures exit: $LASTEXITCODE ($Which, $stamp)"
