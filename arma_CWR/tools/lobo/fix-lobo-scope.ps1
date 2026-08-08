<#
.SYNOPSIS
    Repairs @LoBo addon configs that use a scope keyword the file never defines.

.DESCRIPTION
    OFP-era configs write `scope = public;` and rely on a private preprocessor
    header at the top of the same file:

        #define private   0
        #define protected 1
        #define public    2

    Ninety-odd @LoBo addons carry that header. Two do not, and in those two the
    bare identifier resolves to nothing, so the config reader stores the string
    "public", CfgVehicles reads scope 0, and every class in the file becomes
    non-createable. `createVehicle` then logs

        Cannot create '<class>': type is abstract

    and hands back objNull. The models and textures are finished; they were
    unreachable behind a missing five-line header.

        LoBoWreck.pbo   10 classes  (t54/t55/M60A1/Sho't/BTR-60/Ural wrecks)
        LoBoPalObj.pbo   1 class    (LoBo_Poster_01, base of 7 posters + the
                                     Islamic University of Gaza facade)

    HOW THE PATCH IS APPLIED. config.cpp is stored uncompressed inside the pbo,
    and `scope = 2;` is shorter than every spelling of `scope = public;`, so the
    value is rewritten IN PLACE and padded back to the original byte length with
    spaces. Nothing in the pbo header table moves, so no repack and no offset
    recalculation is involved. Same technique as
    tests/fixtures/mods-lobo/@lobofixup/gen-patched-pbos.ps1 used for the
    malformed "0.0.1" tracer floats.

    The script is idempotent (a second run finds nothing to do) and it discovers
    its own targets: it reads every addon's config.cpp, and only patches a file
    that uses a scope keyword WITHOUT defining it. Adding a new @LoBo pbo with
    the same defect needs no edit here.

    @LoBo is gitignored as third-party game data, so the repaired pbos live only
    on this machine. THIS SCRIPT is the tracked artifact - rerun it after
    reinstalling or updating @LoBo.

.PARAMETER LoBoDir
    The @LoBo mod folder. Default: D:\Arma_CWA\@LoBo

.PARAMETER Tools
    PoseidonTools executable, used to read config.cpp out of each pbo.

.PARAMETER WhatIf
    Report what would change and write nothing.
#>
param(
    [string]$LoBoDir = 'D:\Arma_CWA\@LoBo',
    [string]$Tools = (Join-Path $PSScriptRoot '..\..\dist\x64-win-rwdi\PoseidonTools.exe'),
    [switch]$WhatIf
)
$ErrorActionPreference = 'Stop'

$addons = Join-Path $LoBoDir 'addons'
if (-not (Test-Path -LiteralPath $addons)) { throw "Not found: $addons" }
if (-not (Test-Path -LiteralPath $Tools)) { throw "PoseidonTools not found: $Tools (build it, or pass -Tools)" }

# scope keyword -> the value the missing #define would have given it
$keywords = @{ 'private' = 0; 'protected' = 1; 'public' = 2 }
# config entries whose value is read as a number, i.e. where a bare keyword breaks
$entries = 'scope|scopeWeapon|scopeMagazine|scopeCurator|access'

$latin1 = [System.Text.Encoding]::GetEncoding(28591)
$backupDir = Join-Path $LoBoDir '_ud-orig'
$patchedFiles = 0
$patchedSites = 0

foreach ($pbo in (Get-ChildItem -LiteralPath $addons -Filter '*.pbo' | Sort-Object Name)) {
    # Not every addon ships a config.cpp; a missing one writes to stderr, which
    # Windows PowerShell turns into a terminating error under -ErrorAction Stop.
    $cfg = ''
    try {
        $ErrorActionPreference = 'Continue'
        $cfg = (& $Tools pbo show $pbo.FullName config.cpp 2>$null) -join "`n"
    } finally {
        $ErrorActionPreference = 'Stop'
    }
    if (-not $cfg) { continue }

    # Which keywords does this file use as a value but never define?
    $undefined = @()
    foreach ($kw in $keywords.Keys) {
        $usesIt = $cfg -match "(?im)^\s*($entries)\s*=\s*$kw\s*;"
        $definesIt = $cfg -match "(?im)^\s*#\s*define\s+$kw\b"
        if ($usesIt -and -not $definesIt) { $undefined += $kw }
    }
    if ($undefined.Count -eq 0) { continue }

    $bytes = [System.IO.File]::ReadAllBytes($pbo.FullName)
    $text = $latin1.GetString($bytes)
    $before = $text.Length
    $sites = 0

    foreach ($kw in $undefined) {
        $value = $keywords[$kw]
        $rx = [regex]"(?m)^([ \t]*)($entries)[ \t]*=[ \t]*$kw[ \t]*;"
        $sites += $rx.Matches($text).Count
        $text = $rx.Replace($text, {
                param($m)
                $repl = "$($m.Groups[1].Value)$($m.Groups[2].Value) = $value;"
                if ($repl.Length -gt $m.Value.Length) {
                    throw "Replacement longer than original in $($pbo.Name): '$($m.Value)'"
                }
                $repl.PadRight($m.Value.Length)
            })
    }

    if ($text.Length -ne $before) { throw "Length changed while patching $($pbo.Name) ($before -> $($text.Length))" }
    if ($sites -eq 0) { continue }

    Write-Output ("{0}: {1} site(s) [{2}]" -f $pbo.Name, $sites, ($undefined -join ', '))
    if (-not $WhatIf) {
        New-Item -ItemType Directory -Force $backupDir | Out-Null
        $backup = Join-Path $backupDir $pbo.Name
        if (-not (Test-Path -LiteralPath $backup)) {
            Copy-Item -LiteralPath $pbo.FullName -Destination $backup
            Write-Output ("    backed up original -> {0}" -f $backup)
        }
        # @LoBo ships some pbos read-only (they came off a 2006 CD image).
        if ($pbo.IsReadOnly) { Set-ItemProperty -LiteralPath $pbo.FullName -Name IsReadOnly -Value $false }
        [System.IO.File]::WriteAllBytes($pbo.FullName, $latin1.GetBytes($text))
    }
    $patchedFiles++
    $patchedSites += $sites
}

if ($patchedFiles -eq 0) {
    Write-Output 'Nothing to do: every @LoBo addon that uses a scope keyword also defines it.'
} elseif ($WhatIf) {
    Write-Output ("Would patch {0} site(s) in {1} pbo(s). Rerun without -WhatIf to apply." -f $patchedSites, $patchedFiles)
} else {
    Write-Output ("Patched {0} site(s) in {1} pbo(s). Originals kept in {2}." -f $patchedSites, $patchedFiles, $backupDir)
}
exit 0
