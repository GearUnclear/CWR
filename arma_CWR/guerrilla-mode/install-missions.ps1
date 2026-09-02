<#
.SYNOPSIS
    Installs the Guerrilla Mode global faction library, the shared script core
    and the mission templates into the game data directory.

.DESCRIPTION
    Installs three things, in this order:

      1. the GLOBAL FACTION LIBRARY, guerrilla-mode/config ->
         <GameDir>\bin\guerrilla-factions.hpp, pulled in by
         <GameDir>\bin\config-extra.cpp
      2. the SHARED SCRIPT CORE, guerrilla-mode/core -> <GameDir>\gmcore\
      3. every guerrilla-mode/mission/<Prefix>.<World> template ->
         <GameDir>\Missions\

    WHY THE FACTION LIBRARY IS GLOBAL: the engine builds the faction table as
    the UNION of the global config's CfgGuerrillaFactions and the island
    template's own block (Game/Guerrilla/FactionSources.*, issue #54), the
    island winning on a class-name collision. So a roster that is not an
    island fact - the vanilla WEST/EAST/GUER order of battle is the same on
    every stock island - belongs in one place instead of being copied into
    every template. The engine merges <DataDir>\bin\config-extra.cpp into the
    global config LAST (ParseConfig, Asset/Addon/ConfigParsers.cpp), and the
    text preprocessor resolves an #include against the INCLUDING file's
    directory, which is why guerrilla-factions.hpp is installed beside it in
    bin\. config-extra.cpp is only CREATED when the package ships none: a
    future full-Remaster package is expected to ship its own (CfgLanguages and
    friends), so an existing file is only APPENDED to, never overwritten.

    WHY THE CORE IS SEPARATE: there is exactly ONE copy of the manager
    scripts in the repo (guerrilla-mode/core), not one per template. A
    mission's own init.sqs is a two-line bootstrap that runs
    `[] exec "\gmcore\init.sqs"`. The LEADING BACKSLASH is what makes that
    reach outside the mission folder: OpenScript (Game/Scripting/Scripts.cpp)
    strips it and resolves the rest against the game data root, checking the
    mounted pbo banks first (a gmcore.pbo in any addons\ dir would serve too)
    and then a loose file on disk. So the loose directory <GameDir>\gmcore\
    sitting beside <GameDir>\Missions\ needs no engine change. The core is
    installed BEFORE the templates: a template without it boots into a
    mission with no managers.

    WHY THE TEMPLATE LOCATION: the new-game flow (main menu GUERRILLA button ->
    IDD_GUERRILLA_NEW_GAME) launches the template by resolving the relative
    path "missions\Guerrilla.<World>" against the game working directory --
    either a PBO "missions\Guerrilla.<World>.pbo" (FilePathExists) or an
    unbanked directory "missions\Guerrilla.<World>\mission.sqm"
    (QIFStreamB::FileExist). See DisplayMain::OnChildDestroyed, case
    IDD_GUERRILLA_NEW_GAME, engine/Poseidon/UI/OptionsUIApp.cpp (~line 883).
    The game working directory is the data dir (GameDir), so the unbanked
    form lands at <GameDir>\Missions\Guerrilla.<World>\.

    WORLD GATE: a template is only installed when its world is actually
    loadable in GameDir. Installing a mission whose world is missing is a
    trap: the single-mission browser still lists it, and launching it
    silently keeps the CURRENT world, so the player spawns at the mission's
    coordinates on the WRONG island (typically in open sea) and drowns.

    A world counts as available when EITHER
      * <GameDir>\Worlds\<World>.wrp exists as a loose file, OR
      * some *.pbo under <GameDir>\AddOns, <GameDir>\Dta or a mod folder's
        addons\ dir carries an entry named <World>.wrp (any directory prefix,
        case-insensitive).
    The second case is why -IncludeWorld used to be mandatory for @LoBo's
    Sinai and Lebanon80 (their .wrp files live inside lost.pbo / LoBo_Leb.pbo)
    and is no longer: this script reads the pbo ENTRY TABLE - a few KB at the
    head of each file, never the 100+ MB body - and looks the world up there.
    Adding an island pack therefore needs no edit to this script and no
    name table anywhere. -IncludeWorld stays as a manual override for the
    cases detection cannot cover (a world arriving from a pbo this script
    does not scan, a package assembled after the install run).

    Idempotent: re-running mirrors the repo core and templates over any
    previous install (stale files inside <GameDir>\gmcore and inside each
    installed template are removed, and a previously installed template that
    is now world-gated out is REMOVED; unrelated missions are untouched).

.PARAMETER GameDir
    The Arma: Cold War Assault install (data) directory.
    Default: D:\Arma_CWA\ARMA Cold War Assault [Classic]

.PARAMETER ModDir
    Mod folders whose addons\ dir is scanned for worlds. Defaults to every
    "@*" directory sitting NEXT TO GameDir plus any "@*" inside it - the two
    places a mod is normally unpacked.

.PARAMETER IncludeWorld
    World name(s) to install even though detection found no <World>.wrp,
    loose or inside a pbo. The game must then be launched with whatever
    supplies that world or the mission is unloadable.

.EXAMPLE
    .\install-missions.ps1
    .\install-missions.ps1 -GameDir 'C:\Games\ArmaCWA'
    .\install-missions.ps1 -ModDir 'D:\mods\@LoBo','D:\mods\@Other'
    .\install-missions.ps1 -IncludeWorld Sinai,Lebanon80
#>
param(
    [string]$GameDir = 'D:\Arma_CWA\ARMA Cold War Assault [Classic]',
    [string[]]$ModDir = @(),
    [string[]]$IncludeWorld = @()
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $GameDir -PathType Container)) {
    throw "GameDir not found: $GameDir"
}

$missionRoot = Join-Path $PSScriptRoot 'mission'
if (-not (Test-Path -LiteralPath $missionRoot -PathType Container)) {
    throw "Template source not found: $missionRoot"
}

# ---- helpers --------------------------------------------------------------
# Every path API below is the literal .NET one: the default data dir is
# "...[Classic]", and the bracket pair is a character class to PowerShell's
# own path globbing, so Get-ChildItem -Path / New-Item -Path silently find
# nothing there. -LiteralPath and [System.IO.*] are the safe forms.

# The child directory of $Parent whose name equals $Name ignoring case, or
# $null. Windows itself is case-insensitive, but the NAME as shipped varies
# (bin vs BIN across CWA packages) and we have to echo the real one.
function Resolve-ChildDir {
    param([string]$Parent, [string]$Name)
    if (-not [System.IO.Directory]::Exists($Parent)) { return $null }
    foreach ($dir in [System.IO.Directory]::GetDirectories($Parent)) {
        if ([System.IO.Path]::GetFileName($dir) -ieq $Name) { return $dir }
    }
    return $null
}

function Resolve-ChildFile {
    param([string]$Parent, [string]$Name)
    if (-not [System.IO.Directory]::Exists($Parent)) { return $null }
    foreach ($file in [System.IO.Directory]::GetFiles($Parent)) {
        if ([System.IO.Path]::GetFileName($file) -ieq $Name) { return $file }
    }
    return $null
}

# One NUL-terminated string from the stream; $null at EOF or if it runs past
# $MaxLen (a malformed header, which we skip rather than trust).
function Read-PboString {
    param([System.IO.Stream]$Stream, [int]$MaxLen = 1024)
    $sb = New-Object System.Text.StringBuilder
    for ($i = 0; $i -lt $MaxLen; $i++) {
        $b = $Stream.ReadByte()
        if ($b -lt 0) { return $null }
        if ($b -eq 0) { return $sb.ToString() }
        [void]$sb.Append([char]$b)
    }
    return $null
}

function Read-PboUInt32 {
    param([System.IO.Stream]$Stream)
    $buf = New-Object byte[] 4
    if ($Stream.Read($buf, 0, 4) -ne 4) { return $null }
    return [System.BitConverter]::ToUInt32($buf, 0)
}

# The *.wrp entries a pbo's HEADER declares, as lowercase file names.
#
# PBO layout: a flat sequence of entries, each a NUL-terminated file name
# followed by five little-endian uint32 (packing method, original size,
# reserved, timestamp, data size); the table ends at an entry with an EMPTY
# name, and all file bodies follow it. A leading entry with an empty name and
# packing method 0x56657273 ('Vers') is the product header: NUL-terminated
# key/value strings up to an empty KEY, then the real entries begin. Only the
# head of the file is ever read - some of these pbos are 100+ MB.
function Get-PboWorldEntries {
    param([string]$Path)
    $found = New-Object System.Collections.Generic.List[string]
    $stream = $null
    try {
        $stream = New-Object System.IO.FileStream($Path, [System.IO.FileMode]::Open, [System.IO.FileAccess]::Read,
            [System.IO.FileShare]::ReadWrite, 65536)
        $first = $true
        # entry-count cap: a corrupt head must not turn into a whole-file walk
        for ($n = 0; $n -lt 65536; $n++) {
            $name = Read-PboString -Stream $stream
            if ($null -eq $name) { break }
            $packing = Read-PboUInt32 -Stream $stream
            if ($null -eq $packing) { break }
            for ($k = 0; $k -lt 4; $k++) {
                if ($null -eq (Read-PboUInt32 -Stream $stream)) { $name = $null; break }
            }
            if ($null -eq $name) { break }
            if ($name.Length -eq 0) {
                if ($first -and $packing -eq 0x56657273) {
                    while ($true) {
                        $key = Read-PboString -Stream $stream
                        if ($null -eq $key -or $key.Length -eq 0) { break }
                        if ($null -eq (Read-PboString -Stream $stream)) { break }
                    }
                    $first = $false
                    continue
                }
                break   # empty name = end of the entry table
            }
            $first = $false
            if ($name.ToLowerInvariant().EndsWith('.wrp')) {
                $found.Add(([System.IO.Path]::GetFileName($name)).ToLowerInvariant())
            }
        }
    } catch {
        # An unreadable or non-pbo file is not a reason to fail the install;
        # it just contributes no worlds.
        Write-Verbose ("pbo header unreadable, skipped: {0} ({1})" -f $Path, $_.Exception.Message)
    } finally {
        if ($stream) { $stream.Dispose() }
    }
    return , $found.ToArray()
}

# ---- 1. the global faction library -> <GameDir>\bin ------------------------
$configRoot = Join-Path $PSScriptRoot 'config'
$libSrc = Join-Path $configRoot 'guerrilla-factions.hpp'
$extraSrc = Join-Path $configRoot 'config-extra.cpp'
foreach ($src in @($libSrc, $extraSrc)) {
    if (-not (Test-Path -LiteralPath $src -PathType Leaf)) {
        throw "Faction library source not found: $src"
    }
}

# The bin dir must already exist. Creating one would be creating a config
# search path the package never had, which is a different (and unannounced)
# change to the install than dropping a file into an existing one.
$binDir = Resolve-ChildDir -Parent $GameDir -Name 'bin'
if (-not $binDir) {
    throw "No bin\ (or BIN\) directory in $GameDir - cannot install the global faction library"
}

$libDest = Join-Path $binDir 'guerrilla-factions.hpp'
$libExisting = Resolve-ChildFile -Parent $binDir -Name 'guerrilla-factions.hpp'
if ($libExisting) { $libDest = $libExisting }
$libSame = $false
if (Test-Path -LiteralPath $libDest -PathType Leaf) {
    $libSame = ([System.IO.File]::ReadAllText($libSrc) -ceq [System.IO.File]::ReadAllText($libDest))
}
if (-not $libSame) {
    [System.IO.File]::Copy($libSrc, $libDest, $true)
    Write-Output ("Installed: faction library -> {0}" -f $libDest)
} else {
    Write-Output ("Up to date: faction library -> {0}" -f $libDest)
}

# config-extra.cpp: create from the repo seed when the package ships none,
# otherwise append just the include line. Matching is on the include DIRECTIVE
# so a hand-edited file with its own comment around it still counts.
$includeLine = '#include "guerrilla-factions.hpp"'
$extraDest = Resolve-ChildFile -Parent $binDir -Name 'config-extra.cpp'
if (-not $extraDest) {
    $extraDest = Join-Path $binDir 'config-extra.cpp'
    [System.IO.File]::Copy($extraSrc, $extraDest, $false)
    Write-Output ("Created: {0} (with the faction-library include)" -f $extraDest)
} else {
    $extraText = [System.IO.File]::ReadAllText($extraDest)
    if ($extraText -match '(?m)^\s*#include\s*"guerrilla-factions\.hpp"') {
        Write-Output ("Up to date: {0} already includes the faction library" -f $extraDest)
    } else {
        $append = "`n// Uslu dur! Guerrilla Mode: the global faction library (CfgGuerrillaFactions),`n" +
                  "// installed beside this file by guerrilla-mode/install-missions.ps1.`n" +
                  $includeLine + "`n"
        [System.IO.File]::AppendAllText($extraDest, $append)
        Write-Output ("Appended: faction-library include -> {0}" -f $extraDest)
    }
}

# ---- 2. the shared script core -> <GameDir>\gmcore ------------------------
# Carries no world data, so no world gate applies, and every template's
# two-line init.sqs is dead without it.
$coreRoot = Join-Path $PSScriptRoot 'core'
if (-not (Test-Path -LiteralPath $coreRoot -PathType Container)) {
    throw "Script core source not found: $coreRoot"
}
$coreDest = Join-Path $GameDir 'gmcore'
$null = & robocopy $coreRoot $coreDest /MIR /NJH /NJS /NDL /NFL /NC /NS /NP
if ($LASTEXITCODE -ge 8) {
    throw "robocopy failed for the script core (exit $LASTEXITCODE)"
}
$coreVerb = if ($LASTEXITCODE -eq 0) { 'Up to date' } else { 'Installed' }
Write-Output ("{0}: script core -> {1}" -f $coreVerb, $coreDest)

# ---- 3. world discovery ---------------------------------------------------
# One index for the whole run: lowercase "<world>.wrp" -> where it was found.
$worldIndex = @{}

$worldsDir = Resolve-ChildDir -Parent $GameDir -Name 'Worlds'
if ($worldsDir) {
    foreach ($wrp in [System.IO.Directory]::GetFiles($worldsDir, '*.wrp')) {
        $key = ([System.IO.Path]::GetFileName($wrp)).ToLowerInvariant()
        if (-not $worldIndex.ContainsKey($key)) {
            $worldIndex[$key] = ("Worlds\{0}" -f [System.IO.Path]::GetFileName($wrp))
        }
    }
}

# Mod folders: the ones named on the command line, else every "@*" beside
# GameDir plus every "@*" inside it. GetDirectories' pattern is a .NET one,
# not a PowerShell glob, so the bracketed data-dir name is safe here.
$modDirs = @()
if ($ModDir.Count -gt 0) {
    $modDirs = $ModDir
} else {
    $parent = [System.IO.Path]::GetDirectoryName($GameDir.TrimEnd('\'))
    if ($parent -and [System.IO.Directory]::Exists($parent)) {
        $modDirs += [System.IO.Directory]::GetDirectories($parent, '@*')
    }
    $modDirs += [System.IO.Directory]::GetDirectories($GameDir, '@*')
}

$pboRoots = @()
foreach ($name in @('AddOns', 'Dta')) {
    $dir = Resolve-ChildDir -Parent $GameDir -Name $name
    if ($dir) { $pboRoots += $dir }
}
foreach ($mod in $modDirs) {
    $dir = Resolve-ChildDir -Parent $mod -Name 'addons'
    if ($dir) { $pboRoots += $dir }
}

$pboCount = 0
foreach ($root in $pboRoots) {
    foreach ($pbo in [System.IO.Directory]::GetFiles($root, '*.pbo')) {
        $pboCount++
        foreach ($wrp in (Get-PboWorldEntries -Path $pbo)) {
            if (-not $worldIndex.ContainsKey($wrp)) {
                $worldIndex[$wrp] = $pbo
            }
        }
    }
}
Write-Output ("Worlds: {0} found ({1} pbo header(s) scanned in {2} dir(s))" -f $worldIndex.Count, $pboCount,
    $pboRoots.Count)

# ---- 4. the mission templates -> <GameDir>\Missions -----------------------
$destRoot = Join-Path $GameDir 'Missions'
if (-not (Test-Path -LiteralPath $destRoot -PathType Container)) {
    # [System.IO.Directory]::CreateDirectory is fully literal: New-Item -Path
    # would treat the bracketed data-dir parent ([Classic]) as a glob and fail.
    [void][System.IO.Directory]::CreateDirectory($destRoot)
    Write-Output "Created: $destRoot"
}

# Every template below is script-free: none of them carries a scripts\
# directory any more, they all reach into <GameDir>\gmcore instead.
# Guerrilla.<World> = the playable campaign templates (two-line init.sqs, the
# whole core); Showcase.<World> = the issue #9 systems-showcase missions (same
# two-line init.sqs + a mission-local showcase\ overlay); Qrf.<World>,
# Market.<World> and Undercover.<World> = the reference slices, each with its
# own bootstrap init.sqs that execs only the ONE core policy script it exists
# to demonstrate (\gmcore\scripts\qrf.sqs / market.sqs / undercover.sqs, plus
# lib.sqs) and keeps its debug menu and HUD mission-local. Showcase.*,
# Undercover.*, Qrf.* and Market.* are surfaced as direct main-menu launch
# buttons (kReferenceMissions, UI/OptionsUIApp.cpp) that only appear when the
# mission is installed.
$templates = Get-ChildItem -LiteralPath $missionRoot -Directory |
    Where-Object { $_.Name -match '^(Guerrilla|Showcase|Undercover|Qrf|Market)\.' }
if (-not $templates) {
    throw "No Guerrilla.*/Showcase.*/Undercover.*/Qrf.*/Market.* templates under $missionRoot"
}

foreach ($tpl in $templates) {
    $dest = Join-Path $destRoot $tpl.Name

    # World gate (see .DESCRIPTION): template name is <Prefix>.<World>.
    $world = $tpl.Name.Substring($tpl.Name.IndexOf('.') + 1)
    $key = ("{0}.wrp" -f $world).ToLowerInvariant()
    $where = $null
    if ($worldIndex.ContainsKey($key)) {
        $where = $worldIndex[$key]
    } elseif ($IncludeWorld -contains $world) {
        $where = '-IncludeWorld override (not detected)'
    }
    if (-not $where) {
        if (Test-Path -LiteralPath $dest) {
            Remove-Item -LiteralPath $dest -Recurse -Force -Confirm:$false
            Write-Output ("Removed stale install: {0} (no '{1}.wrp' in {2}\Worlds or any scanned pbo)" -f $tpl.Name,
                $world, $GameDir)
        } else {
            Write-Output ("Skipped: {0} (no '{1}.wrp' in {2}\Worlds or any scanned pbo; -IncludeWorld {1} forces it)" -f
                $tpl.Name, $world, $GameDir)
        }
        continue
    }
    # /MIR mirrors the template (removes stale files from previous installs);
    # scoped to this template's own folder only.
    $null = & robocopy $tpl.FullName $dest /MIR /NJH /NJS /NDL /NFL /NC /NS /NP
    if ($LASTEXITCODE -ge 8) {
        throw "robocopy failed for $($tpl.Name) (exit $LASTEXITCODE)"
    }
    $verb = if ($LASTEXITCODE -eq 0) { 'Up to date' } else { 'Installed' }
    Write-Output ("{0}: {1} -> {2}  [world '{3}' from {4}]" -f $verb, $tpl.Name, $dest, $world, $where)
}

Write-Output "Done. Templates are picked up by the GUERRILLA new-game menu (missions\Guerrilla.<World>), load their managers from $coreDest and their global factions from $binDir."
# robocopy's informational exit codes (1 = files copied) must not leak as
# script failure.
exit 0
