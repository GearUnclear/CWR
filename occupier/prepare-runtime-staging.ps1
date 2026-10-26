$ErrorActionPreference = 'Stop'
$root = 'D:\Arma_CWA\tmp\runtime-portability-20260919'
if (Test-Path -LiteralPath $root) { throw 'Staging path already exists' }
$game = Join-Path $root 'Fresh [Classic]'
$lobo = Join-Path $root '@LoBo'
[void][IO.Directory]::CreateDirectory((Join-Path $game 'BIN'))
foreach ($name in @('AddOns','DTA','Worlds')) {
    Copy-Item -LiteralPath (Join-Path 'D:\Arma_CWA\ARMA Cold War Assault [Classic]' $name) -Destination (Join-Path $game $name) -Recurse
}
foreach ($name in @('CONFIG.BIN','RESOURCE.BIN','STRINGTABLE.CSV')) {
    Copy-Item -LiteralPath (Join-Path 'D:\Arma_CWA\ARMA Cold War Assault [Classic]\BIN' $name) -Destination (Join-Path $game "BIN/$name")
}
[void][IO.Directory]::CreateDirectory($lobo)
foreach ($name in @('addons','Bin','dta')) {
    Copy-Item -LiteralPath (Join-Path 'D:\Arma_CWA\@LoBo' $name) -Destination (Join-Path $lobo $name) -Recurse
}
# Remove only the copied/generated faction overlay, then restore known original
# PBOs in this isolated test copy so mod doctor exercises real repairs.
$copied = Join-Path $lobo 'Bin/config.cpp'
if ([IO.Path]::GetFullPath($copied).StartsWith($root + '\')) { Remove-Item -LiteralPath $copied }
foreach ($name in @('LoBoPalObj.pbo','LoBoWreck.pbo')) {
    Copy-Item -LiteralPath (Join-Path 'D:\Arma_CWA\@LoBo\_ud-orig' $name) -Destination (Join-Path $lobo "addons/$name") -Force
}
& 'D:\Arma_CWA\arma_CWR\setup-guerrilla.ps1' -GameDir $game -LoBoDir $lobo -Tools 'D:\Arma_CWA\arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe'
