# Builds the MODEL FINISHING round's own mount, <git root>\tmp\@kj_models, from its own creature table
# (addon\models\creatures.models.json) with the integration's build script and its three finishing-round
# parameters. The integration's mount (tmp\@kj_dedushka), its creatures.json and its script set are not
# touched: the timing tables go into this folder's copy of the script set (models-tests\kj\anim).
#
#   powershell -NoProfile -File build-km.ps1 [-Creatures a,d]
#
# Before it: each creature's model build folder must be finished and staged
#   (kaiju-notes\finishing\models\finish.sh, clips.sh, addon\models\stage_build.py).
param([string[]]$Creatures = @())
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$gitRoot = [IO.Path]::GetFullPath((Join-Path $here '..\..\..\..\..'))
$build = [IO.Path]::GetFullPath((Join-Path $here '..\addon\build-kj-dedushka.ps1'))
$argv = @('-NoProfile', '-File', $build,
    '-Mount', (Join-Path $gitRoot 'tmp\@kj_models'),
    '-CreaturesFile', (Join-Path $here '..\addon\models\creatures.models.json'),
    '-KjAnimDir', (Join-Path $here 'kj\anim'),
    '-OutDir', 'D:\Arma_CWA_worktrees\kaiju-notes\finishing\models\build\mount')
if ($Creatures.Count -gt 0) { $argv += @('-Creatures', ($Creatures -join ',')) }
& powershell @argv
exit $LASTEXITCODE
