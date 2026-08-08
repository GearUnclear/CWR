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

    STATE ON THIS MACHINE (checked 2026-08-08): @LoBo's own LoBoammo.pbo and
    LoBo_airammo.pbo were already repaired in place on 2026-07-16 and are now
    byte-identical to the shadows in addons/, so the shadows are inert - a
    reinstall of stock @LoBo makes them load-bearing again, which is why they
    stay. The fixture as a whole is NOT redundant either way: bin/config.cpp
    carries the CfgAddons preload roster and CfgGuerrillaFactions that several
    lobo-lane tests assert against. Do not delete this fixture.

    See also tools/lobo/fix-lobo-scope.ps1, which repairs a second, unrelated
    @LoBo content defect (LoBoWreck.pbo and LoBoPalObj.pbo omit the
    "#define public 2" header their sibling configs carry) with the same
    same-length in-place byte patch.

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
