<#
.SYNOPSIS
    Regenerates this fixture mod's patched @LoBo pbo shadows.

.DESCRIPTION
    @LoBo's LoBoammo.pbo and LoBo_airammo.pbo ship a malformed float literal
    ("0.0.1") in eleven CfgAmmo tracerColor[] arrays. This engine's config
    reader evaluates non-numeric tokens as script expressions AT PARSE TIME,
    and under --autotest any script error is a hard abort - so a pristine
    @LoBo cannot boot a test mission at all. bin/config.cpp cannot fix it
    (the deferred mod-config merge runs after the addon configs parse).

    The fix: same-named pbos in this mod's addons/ SHADOW @LoBo's copies on
    the mod path. "0.0.1" -> "0.001" is a same-length byte patch, so the pbo
    headers stay valid without repacking.

    The patched pbos are THIRD-PARTY content (APL-SA LoBo mod data) and are
    gitignored (see .gitignore here) - they must never be committed to this
    GPL repo. Run this script ONCE before first use of the lobo integration
    tests (guerrilla_sinai_swap, guerrilla_new_game_e2e), and rerun it if
    @LoBo is reinstalled/updated.

.PARAMETER LoBoDir
    The @LoBo mod folder. Default: D:\Arma_CWA\@LoBo
#>
param(
    [string]$LoBoDir = 'D:\Arma_CWA\@LoBo'
)
$ErrorActionPreference = 'Stop'
$dest = Join-Path $PSScriptRoot 'addons'
New-Item -ItemType Directory -Force $dest | Out-Null
foreach ($name in 'LoBoammo.pbo', 'LoBo_airammo.pbo') {
    $src = Join-Path (Join-Path $LoBoDir 'addons') $name
    if (-not (Test-Path -LiteralPath $src)) { throw "Not found: $src" }
    $bytes = [System.IO.File]::ReadAllBytes($src)
    $text = [System.Text.Encoding]::GetEncoding(28591).GetString($bytes)
    $n = ([regex]::Matches($text, [regex]::Escape('0.0.1'))).Count
    $patched = $text.Replace('0.0.1', '0.001')
    [System.IO.File]::WriteAllBytes((Join-Path $dest $name), [System.Text.Encoding]::GetEncoding(28591).GetBytes($patched))
    Write-Output ("{0}: {1} occurrence(s) patched" -f $name, $n)
}
Write-Output "Done. Mount order: --mod `"@LoBo;...\@lobofixup`" (fixup AFTER @LoBo)."
exit 0
