<#
.SYNOPSIS
    Shared helpers for the mod-intake hooks (Add-ModFolder.ps1, Add-Island.ps1).

.DESCRIPTION
    Issue #54 step F1 needs two standalone, double-clickable PowerShell
    workflows and both need the same three things: a case-insensitive way to
    find a child dir/file inside a package whose casing varies (bin vs BIN),
    a way to list the *.wrp world entries a pbo's header declares without
    reading the (often 100+ MB) body, and the ud-mods.txt mod-list
    convention. Putting them here instead of copy-pasting keeps the two
    scripts from drifting the way tools/lobo's three repair scripts did
    before issue #54 D2 folded them into one tool.

    The pbo entry-table walk (Get-PboEntryNames) is the same header-only
    parse guerrilla-mode/install-missions.ps1's Get-PboWorldEntries uses,
    generalised to take an extension instead of being hardcoded to *.wrp -
    Add-ModFolder.ps1 step (f) needs it too, to list the worlds a freshly
    added mod ships. That script runs its world-gate logic at load time
    (top-level statements outside any function), so it cannot be dot-sourced
    for just the helper; this module carries an independent copy instead.

    PowerShell 5.1 compatible: no ternary/??/?., no class syntax.
#>

# One NUL-terminated string from the stream; $null at EOF or if it runs past
# $MaxLen (a malformed header, which the caller should treat as "no entries"
# rather than trust).
function Read-ModIntakePboString {
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

function Read-ModIntakePboUInt32 {
    param([System.IO.Stream]$Stream)
    $buf = New-Object byte[] 4
    if ($Stream.Read($buf, 0, 4) -ne 4) { return $null }
    return [System.BitConverter]::ToUInt32($buf, 0)
}

<#
.SYNOPSIS
    The entry names a pbo's header table declares, filtered by extension.

.DESCRIPTION
    PBO layout: a flat sequence of entries, each a NUL-terminated file name
    followed by five little-endian uint32 (packing method, original size,
    reserved, timestamp, data size); the table ends at an entry with an EMPTY
    name, and all file bodies follow it. A leading entry with an empty name
    and packing method 0x56657273 ('Vers') is the product header: NUL-
    terminated key/value strings up to an empty KEY, then the real entries
    begin. Only the head of the file is ever read - some pbos are 100+ MB.

    An unreadable or non-pbo file is not an error here: it contributes no
    entries, same as install-missions.ps1's Get-PboWorldEntries.

.PARAMETER Path
    The .pbo file to read.

.PARAMETER Extension
    Entries are returned only when their name ends with this (case-
    insensitive), e.g. '.wrp'. Default '.wrp'.

.OUTPUTS
    Lowercase file names (directory prefix stripped), possibly empty.
#>
function Get-PboEntryNames {
    param(
        [Parameter(Mandatory)][string]$Path,
        [string]$Extension = '.wrp'
    )
    $found = New-Object System.Collections.Generic.List[string]
    $stream = $null
    try {
        $stream = New-Object System.IO.FileStream($Path, [System.IO.FileMode]::Open, [System.IO.FileAccess]::Read,
            [System.IO.FileShare]::ReadWrite, 65536)
        $first = $true
        # entry-count cap: a corrupt head must not turn into a whole-file walk
        for ($n = 0; $n -lt 65536; $n++) {
            $name = Read-ModIntakePboString -Stream $stream
            if ($null -eq $name) { break }
            $packing = Read-ModIntakePboUInt32 -Stream $stream
            if ($null -eq $packing) { break }
            for ($k = 0; $k -lt 4; $k++) {
                if ($null -eq (Read-ModIntakePboUInt32 -Stream $stream)) { $name = $null; break }
            }
            if ($null -eq $name) { break }
            if ($name.Length -eq 0) {
                if ($first -and $packing -eq 0x56657273) {
                    while ($true) {
                        $key = Read-ModIntakePboString -Stream $stream
                        if ($null -eq $key -or $key.Length -eq 0) { break }
                        if ($null -eq (Read-ModIntakePboString -Stream $stream)) { break }
                    }
                    $first = $false
                    continue
                }
                break   # empty name = end of the entry table
            }
            $first = $false
            if ($name.ToLowerInvariant().EndsWith($Extension.ToLowerInvariant())) {
                $found.Add(([System.IO.Path]::GetFileName($name)).ToLowerInvariant())
            }
        }
    } catch {
        Write-Verbose ("pbo header unreadable, skipped: {0} ({1})" -f $Path, $_.Exception.Message)
    } finally {
        if ($stream) { $stream.Dispose() }
    }
    return , $found.ToArray()
}

<#
.SYNOPSIS
    The child directory of $Parent named $Name, ignoring case, or $null.

.DESCRIPTION
    Windows itself is case-insensitive, but the name AS SHIPPED varies
    (bin vs BIN, addons vs AddOns across CWA packages) and callers need the
    real one to build further paths or display it back to the player.
#>
function Resolve-ModIntakeChildDir {
    param([string]$Parent, [string]$Name)
    if (-not [System.IO.Directory]::Exists($Parent)) { return $null }
    foreach ($dir in [System.IO.Directory]::GetDirectories($Parent)) {
        if ([System.IO.Path]::GetFileName($dir) -ieq $Name) { return $dir }
    }
    return $null
}

function Resolve-ModIntakeChildFile {
    param([string]$Parent, [string]$Name)
    if (-not [System.IO.Directory]::Exists($Parent)) { return $null }
    foreach ($file in [System.IO.Directory]::GetFiles($Parent)) {
        if ([System.IO.Path]::GetFileName($file) -ieq $Name) { return $file }
    }
    return $null
}

<#
.SYNOPSIS
    Does $ModDir look like something worth running the intake hooks on?

.DESCRIPTION
    Two shapes count: an addons\ (any casing) directory holding at least one
    *.pbo, or a bin\ (any casing) directory holding a config.cpp - a
    bin-only overlay mod (fonts, stringtable tweaks) ships no pbo at all.

.OUTPUTS
    A PSCustomObject: Looks (bool), AddonsDir (string or $null),
    PboCount (int), ConfigCpp (string or $null), Reason (string, only
    meaningful when Looks is $false).
#>
function Test-ModFolderShape {
    param([Parameter(Mandatory)][string]$ModDir)

    if (-not (Test-Path -LiteralPath $ModDir -PathType Container)) {
        return [PSCustomObject]@{
            Looks = $false; AddonsDir = $null; PboCount = 0; ConfigCpp = $null
            Reason = "not a directory: $ModDir"
        }
    }

    $addonsDir = Resolve-ModIntakeChildDir -Parent $ModDir -Name 'addons'
    $pboCount = 0
    if ($addonsDir) {
        $pboCount = @([System.IO.Directory]::GetFiles($addonsDir, '*.pbo')).Count
    }

    $binDir = Resolve-ModIntakeChildDir -Parent $ModDir -Name 'bin'
    $configCpp = $null
    if ($binDir) { $configCpp = Resolve-ModIntakeChildFile -Parent $binDir -Name 'config.cpp' }

    if ($pboCount -gt 0 -or $configCpp) {
        return [PSCustomObject]@{
            Looks = $true; AddonsDir = $addonsDir; PboCount = $pboCount; ConfigCpp = $configCpp; Reason = $null
        }
    }
    return [PSCustomObject]@{
        Looks = $false; AddonsDir = $addonsDir; PboCount = 0; ConfigCpp = $null
        Reason = "no addons\*.pbo and no bin\config.cpp under $ModDir - doesn't look like a mod folder"
    }
}

<#
.SYNOPSIS
    Title-cases just the first character ("cain" -> "Cain").

.DESCRIPTION
    A CfgWorlds class name guessed from a lowercase .wrp entry name. Config
    class lookups in this engine are case-insensitive, and `guerrilla
    scaffold` prints the package's real CfgWorlds names on a miss, so a
    wrong guess here is a clear retry, not a silent dead end.
#>
function ConvertTo-ModIntakeWorldGuess {
    param([string]$Stem)
    if ([string]::IsNullOrEmpty($Stem)) { return $Stem }
    return $Stem.Substring(0, 1).ToUpperInvariant() + $Stem.Substring(1)
}

<#
.SYNOPSIS
    World-class guesses a single mod folder ships, from its pbo headers plus
    any loose Worlds\*.wrp.

.OUTPUTS
    PSCustomObjects: World (guessed CfgWorlds class), Source (pbo or file
    path), sorted by World, de-duplicated by World.
#>
function Get-ModWorldClasses {
    param([Parameter(Mandatory)][string]$ModDir)

    $rows = New-Object System.Collections.Generic.List[object]
    $seen = @{}

    $addonsDir = Resolve-ModIntakeChildDir -Parent $ModDir -Name 'addons'
    if ($addonsDir) {
        foreach ($pbo in [System.IO.Directory]::GetFiles($addonsDir, '*.pbo')) {
            foreach ($wrp in (Get-PboEntryNames -Path $pbo -Extension '.wrp')) {
                $stem = [System.IO.Path]::GetFileNameWithoutExtension($wrp)
                $world = ConvertTo-ModIntakeWorldGuess -Stem $stem
                if (-not $seen.ContainsKey($world.ToLowerInvariant())) {
                    $seen[$world.ToLowerInvariant()] = $true
                    $rows.Add([PSCustomObject]@{ World = $world; Source = $pbo })
                }
            }
        }
    }
    $worldsDir = Resolve-ModIntakeChildDir -Parent $ModDir -Name 'Worlds'
    if ($worldsDir) {
        foreach ($wrp in [System.IO.Directory]::GetFiles($worldsDir, '*.wrp')) {
            $stem = [System.IO.Path]::GetFileNameWithoutExtension($wrp)
            $world = ConvertTo-ModIntakeWorldGuess -Stem $stem
            if (-not $seen.ContainsKey($world.ToLowerInvariant())) {
                $seen[$world.ToLowerInvariant()] = $true
                $rows.Add([PSCustomObject]@{ World = $world; Source = $wrp })
            }
        }
    }
    return , ($rows.ToArray() | Sort-Object World)
}

<#
.SYNOPSIS
    World-class guesses offered by a game package plus a set of mod folders.

.DESCRIPTION
    Scans <GameDir>\Worlds\*.wrp (loose), every *.pbo under <GameDir>\AddOns
    and <GameDir>\Dta, and every *.pbo under each mod's addons\ dir. This is
    the "what island could I scaffold" picker for Add-Island.ps1 when
    -World is not given - the same header-only walk
    guerrilla-mode/install-missions.ps1 uses to decide whether a template's
    world is installable, run in reverse (list candidates instead of gating
    one).

.OUTPUTS
    PSCustomObjects: World, Source - sorted, de-duplicated by World.
#>
function Get-PackageWorldClasses {
    param(
        [Parameter(Mandatory)][string]$GameDir,
        [string[]]$ModDir = @()
    )

    $rows = New-Object System.Collections.Generic.List[object]
    $seen = @{}

    function AddRow($world, $source) {
        $key = $world.ToLowerInvariant()
        if (-not $seen.ContainsKey($key)) {
            $seen[$key] = $true
            $rows.Add([PSCustomObject]@{ World = $world; Source = $source })
        }
    }

    $worldsDir = Resolve-ModIntakeChildDir -Parent $GameDir -Name 'Worlds'
    if ($worldsDir) {
        foreach ($wrp in [System.IO.Directory]::GetFiles($worldsDir, '*.wrp')) {
            $stem = [System.IO.Path]::GetFileNameWithoutExtension($wrp)
            AddRow (ConvertTo-ModIntakeWorldGuess -Stem $stem) $wrp
        }
    }
    foreach ($name in @('AddOns', 'Dta')) {
        $dir = Resolve-ModIntakeChildDir -Parent $GameDir -Name $name
        if (-not $dir) { continue }
        foreach ($pbo in [System.IO.Directory]::GetFiles($dir, '*.pbo')) {
            foreach ($wrp in (Get-PboEntryNames -Path $pbo -Extension '.wrp')) {
                $stem = [System.IO.Path]::GetFileNameWithoutExtension($wrp)
                AddRow (ConvertTo-ModIntakeWorldGuess -Stem $stem) $pbo
            }
        }
    }
    foreach ($mod in $ModDir) {
        foreach ($row in (Get-ModWorldClasses -ModDir $mod)) {
            AddRow $row.World $row.Source
        }
    }
    return , ($rows.ToArray() | Sort-Object World)
}

<#
.SYNOPSIS
    Path to the ud-mods.txt mod-list file inside a game data directory.

.DESCRIPTION
    run-game.ps1/run-game-with-mods.ps1 take mod folders as a -Mod
    parameter, not a list file, and neither they nor install-missions.ps1
    read a persisted mod list, so there is no pre-existing convention to
    follow. ud-mods.txt is a new one: one absolute mod-folder path per
    line, written by Add-ModFolder.ps1's "mount" step, meant to be read by
    issue #29's future installer and by install-missions.ps1's -ModDir
    default (Add-Island.ps1 already defaults -ModDir to it).
#>
function Get-UdModsListPath {
    param([Parameter(Mandatory)][string]$GameDir)
    return Join-Path $GameDir 'ud-mods.txt'
}

<#
.SYNOPSIS
    The mod folder paths recorded in <GameDir>\ud-mods.txt, or @() if the
    file does not exist. Blank lines and lines starting with '#' are skipped.
#>
function Get-UdModsEntries {
    param([Parameter(Mandatory)][string]$GameDir)
    $path = Get-UdModsListPath -GameDir $GameDir
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { return @() }
    $lines = Get-Content -LiteralPath $path
    $out = @()
    foreach ($line in $lines) {
        $t = $line.Trim()
        if ($t -and -not $t.StartsWith('#')) { $out += $t }
    }
    return , $out
}

<#
.SYNOPSIS
    Idempotently records $ModDirPath (absolute) in <GameDir>\ud-mods.txt.

.OUTPUTS
    $true if the file was changed (entry added or file created), $false if
    the entry was already there.
#>
function Add-UdModsEntry {
    param(
        [Parameter(Mandatory)][string]$GameDir,
        [Parameter(Mandatory)][string]$ModDirPath
    )
    $abs = [System.IO.Path]::GetFullPath($ModDirPath)
    $path = Get-UdModsListPath -GameDir $GameDir
    $existing = Get-UdModsEntries -GameDir $GameDir
    foreach ($e in $existing) {
        if ($e -ieq $abs) { return $false }
    }
    # Append rather than rewrite: a hand-edited comment line must survive.
    $needsNewline = (Test-Path -LiteralPath $path -PathType Leaf) -and
        ([System.IO.File]::ReadAllText($path).Length -gt 0) -and
        (-not [System.IO.File]::ReadAllText($path).EndsWith("`n"))
    if ($needsNewline) { [System.IO.File]::AppendAllText($path, "`n") }
    [System.IO.File]::AppendAllText($path, "$abs`n")
    return $true
}

Export-ModuleMember -Function @(
    'Get-PboEntryNames',
    'Resolve-ModIntakeChildDir',
    'Resolve-ModIntakeChildFile',
    'Test-ModFolderShape',
    'ConvertTo-ModIntakeWorldGuess',
    'Get-ModWorldClasses',
    'Get-PackageWorldClasses',
    'Get-UdModsListPath',
    'Get-UdModsEntries',
    'Add-UdModsEntry'
)
