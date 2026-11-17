# Copies the combat feature's script payload (features\combat\kj\combat) into the integrated script
# set (integration\kj\combat). The feature folder is only read. With kj\combat in the set,
# kj\start.sqs finds it, sets KJ_unkillable = false and starts kj\combat\start.sqs: he can be killed.
# Without it he is unkillable (the crush feature's heal loop).
#
#   powershell -NoProfile -File merge-combat.ps1            copy the payload in (again, after the feature changes)
#   powershell -NoProfile -File merge-combat.ps1 -Remove    take it out again (the set is then the unkillable one)
#
# The payload is copied unchanged: everything the integration needs from it goes through its own
# entry points and globals (KJ_hurt, KJ_woundStage, KJ_woundPace, KJ_dead, KJCB_fnScriptHit,
# KJCB_fnWound). The creature config values the feature measured (side, armor, armorStructural, zone
# factors) are in addon\gen_config.py.
param([switch]$Remove)
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$src = [IO.Path]::GetFullPath((Join-Path $here '..\..\features\combat\kj\combat'))
$dst = [IO.Path]::GetFullPath((Join-Path $here '..\kj\combat'))
if ($Remove) {
    if (Test-Path -LiteralPath $dst) { Remove-Item -LiteralPath $dst -Recurse -Force; Write-Host "removed $dst" }
    exit 0
}
if (-not (Test-Path -LiteralPath (Join-Path $src 'start.sqs'))) { throw "no combat payload at $src" }
if (Test-Path -LiteralPath $dst) { Remove-Item -LiteralPath $dst -Recurse -Force }
[void][IO.Directory]::CreateDirectory($dst)
Copy-Item -Path (Join-Path $src '*') -Destination $dst -Recurse -Force
$files = Get-ChildItem -LiteralPath $dst -File
$stamp = ($files | Sort-Object LastWriteTime | Select-Object -Last 1).LastWriteTime.ToString('s')
$note = "; kj\combat: the combat feature's payload, copied from features\combat\kj\combat by`r`n" +
    "; integration\tools\merge-combat.ps1 on $(Get-Date -Format s) (newest file of the feature: $stamp),`r`n" +
    "; with the one fix of tools\patch_combat_merge.py (the hit signal's bearing to himself).`r`n" +
    "; Do not edit this copy: change the feature, then run merge-combat.ps1 again.`r`n"
[IO.File]::WriteAllText((Join-Path $dst 'MERGED.sqs'), $note, (New-Object Text.UTF8Encoding($false)))
Write-Host "merged $($files.Count) files from $src (newest $stamp)"
# the one fix to the copy (see the patch's header); it does nothing once the feature has it
& python (Join-Path $here 'patch_combat_merge.py')
if ($LASTEXITCODE -ne 0) { throw "patch_combat_merge.py failed" }
