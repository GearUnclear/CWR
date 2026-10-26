# look-fog lane game runs (scratch copy D:\Arma_CWA_worktrees\look-fog), each with
# its own data junction (RemasterUD-look-fog*), so each cleanup stops only its
# own games. Output lands under runs\ (a junction to E:\look-lanes\look-fog\runs).
#   -Tests  test paths relative to arma_CWR (comma list)
#   -Tag    a suffix for the output folder
#   -Override  a shader override dir (CWR_SHADER_OVERRIDE_DIR)
param([string]$Tests, [string]$Tag = '', [string]$Override = '', [string]$Junction = 'RemasterUD-look-fog')
$ErrorActionPreference = 'Continue'
$lane = 'D:\Arma_CWA_worktrees\graphics-remaster\tmp\look\fog'
$rc = 'D:\Arma_CWA_worktrees\look-fog\arma_CWR\tests\rendering\captures\run-captures.ps1'
Set-Location 'D:\Arma_CWA_worktrees\look-fog\arma_CWR'
$stamp = (Get-Date -Format 'yyyyMMdd-HHmmss') + $Tag
$args2 = @('-NoProfile', '-File', $rc, '-GameDir', 'dist/look-fog-rwdi', '-DataJunction', $Junction, '-OutDir', "$lane\runs\$stamp", '-Tests', $Tests)
if ($Override) { $args2 += @('-ShaderOverrideDir', $Override) }
& powershell @args2
"run exit: $LASTEXITCODE ($stamp)"
