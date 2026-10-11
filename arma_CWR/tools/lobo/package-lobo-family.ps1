<#
.SYNOPSIS
    Builds a clean, offline package of the Lost Brothers (@LoBo) mod with the
    Uslu dur! repairs applied, for private family use.

.DESCRIPTION
    The assembled @LoBo on this machine is a working tree: it carries a nested
    git repo, the mod doctor's _ud-orig backups, installer backups, scripts
    and a Bin\config.cpp that only the Uslu dur! (UD) engine may load. This
    script turns it into something a relative can drop into a game folder:

        <OutDir>\stage\UD-LoBo-Family-<Version>\
          README-FIRST.txt   what it is, install and launch lines, verification
          SHA256SUMS.txt     sha256 of every other file (sha256sum -c format)
          @LoBo\             allowlisted copy of the mod, the 73 Easting pack
                             merged in, UD-CHANGES.txt describing the repairs
          @LoBo_Deps\        the third-party addons LoBo depends on, with
                             their readmes and licences
          @LoBo_UD\          bin\config.cpp (the Guerrilla faction table),
                             which must only ever be mounted on Uslu dur!
        <OutDir>\UD-LoBo-Family-<Version>.7z  (+ .sha256 sidecar)

    The 73 Easting release and the dependency set live on the server
    (ssh host alias <Server>, path <ServerPath>) and are fetched into
    <CacheDir> over ssh, verified against a sha256sum listing taken on the
    server, and reused on later runs when their hashes still match.

    Preflight runs before anything is written: the four repaired pbos must
    match their pinned hashes and must not be the stock (unrepaired) bytes,
    Bin\config.cpp must carry the GENERATED banner, and every top-level item
    of @LoBo must be either allowlisted, on the skip list, or (with
    -AllowUnknown) explicitly tolerated. Runtime junk (*.log, *.rpt, *.bak,
    Thumbs.db, crashdump*, error.txt, flashpoint.cfg) is skipped anywhere.

    Why @LoBo_UD is a separate modfolder: UD layers every mounted mod's
    bin\config onto the base config. The stock ColdWarAssault.exe and the
    official 3.05 Remastered PoseidonGame.exe make a mod's bin\config REPLACE
    the whole base config, which breaks the game. Keeping the file out of
    @LoBo makes the same @LoBo folder safe on all three engines.

    The script never modifies <LoBoDir>, the server, or anything outside
    <OutDir>. It deletes only its own staging folder under <OutDir>\stage and
    partial downloads under <CacheDir>.

    SHA256SUMS.txt and the .sha256 sidecar are written with LF line endings
    on purpose: GNU sha256sum -c treats a trailing CR as part of the file
    name. Every other generated .txt uses CRLF.

.PARAMETER LoBoDir
    The assembled @LoBo mod folder. Default: D:\Arma_CWA\@LoBo

.PARAMETER OutDir
    Where the staging folder, the cache and the archives go.
    Default: D:\LoBo-Family

.PARAMETER Version
    Package version string, used in folder and archive names.
    Default: today's date as yyyy-MM-dd.

.PARAMETER Server
    ssh host alias for the server holding the extras. Default: mega_nc

.PARAMETER ServerPath
    Base path of the @LoBo mirror on the server.
    Default: /root/mobile-dev/mod_@lobo

.PARAMETER CacheDir
    Local mirror of the fetched server files, laid out by server-relative
    path. Default: <OutDir>\_server-cache

.PARAMETER SkipFetch
    Do not contact the server; build from the cache, which must already hold
    everything the build needs.

.PARAMETER NoArchive
    Stop after staging and verification; write no .7z.

.PARAMETER IncludeSources
    Also build UD-LoBo-Sources-<Version>.7z with the original dependency
    downloads (verified against deps/SHA256SUMS), the server's manifest and
    scan log, the stock originals of the repaired pbos and the assembly notes.

.PARAMETER ExtraDir
    Optional folder mirroring the @LoBo layout (addons\, Missions\, docs\ ...)
    merged into the staged @LoBo with the same collision rules as the
    73 Easting files, for future finds. It may not carry a Bin folder.

.PARAMETER PlanOnly
    Run preflight, print the plan and exit without writing anything. Unless
    -SkipFetch is given this still asks the server for its file listing
    (read-only) so the fetch plan can be shown.

.PARAMETER SevenZip
    Path to 7z.exe. Default: C:\Program Files\7-Zip\7z.exe

.PARAMETER ExpectedRepairHashes
    Hashtable of @LoBo-relative path (forward slashes) to the SHA-256 the
    repaired file must have. Defaults to the four pins recorded 2026-10-11;
    a fixture test overrides it.

.PARAMETER AllowUnknown
    Warn and skip unknown top-level items in @LoBo (and ExtraDir) instead of
    failing.

.PARAMETER Force
    Overwrite an existing archive of the same Version.

.EXAMPLE
    .\package-lobo-family.ps1 -PlanOnly
    Preflight against the real @LoBo, list what the server would send, write nothing.

.EXAMPLE
    .\package-lobo-family.ps1 -IncludeSources
    Full build: fetch, stage, verify, UD-LoBo-Family-<date>.7z and UD-LoBo-Sources-<date>.7z.
#>
param(
    [string]$LoBoDir = 'D:\Arma_CWA\@LoBo',
    [string]$OutDir = 'D:\LoBo-Family',
    [string]$Version = (Get-Date).ToString('yyyy-MM-dd'),
    [string]$Server = 'mega_nc',
    [string]$ServerPath = '/root/mobile-dev/mod_@lobo',
    [string]$CacheDir = '',
    [switch]$SkipFetch,
    [switch]$NoArchive,
    [switch]$IncludeSources,
    [string]$ExtraDir = '',
    [switch]$PlanOnly,
    [string]$SevenZip = 'C:\Program Files\7-Zip\7z.exe',
    [hashtable]$ExpectedRepairHashes = @{
        'addons/LoBoWreck.pbo'    = '62B28E7CEBC005401EFAD3B43C94171910C972FD0B11330A8C3E0EE9364BEC68'
        'addons/LoBoPalObj.pbo'   = '68308A6A6C2BFC1B813CC8750CF2234E15B0A77E0E9EE4E55C47A19F63FB33C0'
        'addons/LoBoammo.pbo'     = '2C9B9C992E1ACB45D111D473416D088CAE019851300379DD29C5902EF0D0C69A'
        'addons/LoBo_airammo.pbo' = 'BA1C49E14678D5E9BACD1DF775C51BED31341BEDA94AE31684FF86B23D1FE53B'
    },
    [switch]$AllowUnknown,
    [switch]$Force
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------
$script:PackageName  = "UD-LoBo-Family-$Version"
$script:SourcesName  = "UD-LoBo-Sources-$Version"
$script:Today        = (Get-Date).ToString('yyyy-MM-dd')
$script:Banner       = '// GENERATED by arma_CWR/tools/lobo/install-lobo-factions.ps1'

# Stock (unrepaired) bytes of the two pbos the scope repair rewrote. These
# must never ship in addons, whatever -ExpectedRepairHashes says.
$script:StockHashes = @{
    'addons/LoBoWreck.pbo'  = '2149FEBC43FD378B7AA361D8AFE4D63738F11239688B5C7C46832715DAF1ABB2'
    'addons/LoBoPalObj.pbo' = 'F16A43BE9D538C50A61EA0E7DE8A98D27759F7639AE2FC5F8924E9E1CCE666A2'
}

# Top-level items of @LoBo. Anything not listed here or in SkipReasons is an
# error unless -AllowUnknown.
$script:AllowedDirs  = @('addons', 'Bin', 'dta', 'Missions', 'user missions', 'docs')
$script:AllowedFiles = @('README.txt', 'The_Lost_Brothers_Official_Readme2.doc')
$script:SkipReasons  = @{
    '.git'           = 'local git bookkeeping'
    '.gitattributes' = 'local git bookkeeping'
    '.gitignore'     = 'local git bookkeeping'
    'tools'          = 'build-machine scripts, tracked in arma_CWR/tools/lobo'
    '_ud-orig'       = 'stock originals of the repaired pbos (Sources archive only)'
    '.ud-backups'    = 'installer backups'
    '_scratch'       = 'local scratch space'
}
# Name segments that must not appear anywhere under the staged @LoBo.
$script:ForbiddenInStage = @('_ud-orig', '.git', '.gitattributes', '.gitignore', 'tools', '.ud-backups', '_scratch', 'config.cpp')
# Runtime junk the game writes into a mod folder; skipped anywhere.
$script:JunkPatterns = @('*.log', '*.rpt', '*.bak', 'Thumbs.db', 'crashdump*', 'error.txt', 'flashpoint.cfg')

# Server extras, as server-relative paths with forward slashes.
$script:EastingFiles = @(
    'addons/73eastings.pbo',
    'addons/Ob73East.pbo',
    'Missions/73EastingDemo.73eastings.pbo',
    'docs/73eastings/73eastings_provenance.txt'
)
$script:DepsFixedFiles = @(
    'deps/README.txt',
    'deps/SHA256SUMS',
    'deps/clamscan.log',
    'deps/@LoBo_Deps/POSSIBLY_UNNEEDED.txt'
)
$script:DepsAddonsDir  = 'deps/@LoBo_Deps/Addons'
$script:DepsReadmesDir = 'deps/readmes'
$script:ArchivesDir    = 'deps/archives'
$script:ManifestName   = '_remote-manifest.sha256'

# Rough size of the server extras when the cache is empty (free-space check).
$script:EstimatedDepsBytes     = 300MB
$script:EstimatedArchivesBytes = 750MB

# ---------------------------------------------------------------------------
# Output helpers
# ---------------------------------------------------------------------------
function Write-Step([string]$Text) { Write-Host ''; Write-Host "== $Text ==" -ForegroundColor Cyan }
function Write-Info([string]$Text) { Write-Host "   $Text" }
function Write-Skip([string]$Text) { Write-Host "   skip: $Text" -ForegroundColor DarkGray }
function Write-Warn([string]$Text) { Write-Warning $Text }

function Format-Bytes([long]$Bytes) {
    if ($Bytes -ge 1GB) { return ('{0:N0} bytes ({1:N2} GB)' -f $Bytes, ($Bytes / 1GB)) }
    if ($Bytes -ge 1MB) { return ('{0:N0} bytes ({1:N1} MB)' -f $Bytes, ($Bytes / 1MB)) }
    return ('{0:N0} bytes' -f $Bytes)
}

# ---------------------------------------------------------------------------
# File helpers
# ---------------------------------------------------------------------------
function Get-Sha256([string]$Path) {
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToUpperInvariant()
}

function Test-JunkName([string]$Name) {
    foreach ($pattern in $script:JunkPatterns) {
        if ($Name -like $pattern) { return $true }
    }
    return $false
}

function Test-ReparsePoint([string]$Path) {
    $attr = [IO.File]::GetAttributes($Path)
    return [bool]($attr -band [IO.FileAttributes]::ReparsePoint)
}

function Get-RelativeUnder([string]$Root, [string]$Full) {
    $r = [IO.Path]::GetFullPath($Root).TrimEnd('\', '/') + '\'
    $f = [IO.Path]::GetFullPath($Full)
    if (-not $f.StartsWith($r, [StringComparison]::OrdinalIgnoreCase)) {
        throw "Internal: $f is not under $r"
    }
    return $f.Substring($r.Length)
}

function Test-PathUnder([string]$Parent, [string]$Child) {
    $p = [IO.Path]::GetFullPath($Parent).TrimEnd('\', '/') + '\'
    $c = [IO.Path]::GetFullPath($Child).TrimEnd('\', '/') + '\'
    return $c.StartsWith($p, [StringComparison]::OrdinalIgnoreCase)
}

# Enumerates a tree. Returns Dirs (relative), Files (Full, Relative, Length)
# and Junk (relative paths skipped). Refuses to traverse links or junctions.
function Get-TreeEntries {
    param([Parameter(Mandatory)][string]$Root, [switch]$SkipJunk)
    $rootFull = [IO.Path]::GetFullPath($Root)
    $dirs  = New-Object System.Collections.Generic.List[string]
    $files = New-Object System.Collections.Generic.List[object]
    $junk  = New-Object System.Collections.Generic.List[string]
    $stack = New-Object System.Collections.Generic.Stack[string]
    $stack.Push($rootFull)
    while ($stack.Count -gt 0) {
        $dir = $stack.Pop()
        foreach ($sub in [IO.Directory]::GetDirectories($dir)) {
            if (Test-ReparsePoint $sub) { throw "Refusing to traverse a link/junction: $sub" }
            $dirs.Add((Get-RelativeUnder $rootFull $sub))
            $stack.Push($sub)
        }
        foreach ($file in [IO.Directory]::GetFiles($dir)) {
            if (Test-ReparsePoint $file) { throw "Refusing to copy a link: $file" }
            $relative = Get-RelativeUnder $rootFull $file
            if ($SkipJunk -and (Test-JunkName ([IO.Path]::GetFileName($file)))) {
                $junk.Add($relative)
                continue
            }
            $files.Add([pscustomobject]@{
                Full     = $file
                Relative = $relative
                Length   = (New-Object IO.FileInfo($file)).Length
            })
        }
    }
    return [pscustomobject]@{ Dirs = $dirs; Files = $files; Junk = $junk }
}

function Copy-FileTo([string]$Source, [string]$Dest) {
    [void][IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($Dest))
    [IO.File]::Copy($Source, $Dest, $false)
    # _ud-orig keeps its originals read-only; the family copy should not be.
    $attr = [IO.File]::GetAttributes($Dest)
    if ($attr -band [IO.FileAttributes]::ReadOnly) {
        [IO.File]::SetAttributes($Dest, ($attr -bxor [IO.FileAttributes]::ReadOnly))
    }
}

function Copy-TreeEntries([object]$Entries, [string]$SourceRoot, [string]$DestRoot) {
    [void][IO.Directory]::CreateDirectory($DestRoot)
    foreach ($d in $Entries.Dirs) { [void][IO.Directory]::CreateDirectory((Join-Path $DestRoot $d)) }
    foreach ($f in $Entries.Files) { Copy-FileTo $f.Full (Join-Path $DestRoot $f.Relative) }
}

# Merges one file into a staged tree. A file of the same name (any case)
# already there must be byte-identical (then the merge is skipped) or the
# build fails. Returns 'added' or 'identical'.
function Merge-FileInto {
    param([string]$SourceFile, [string]$DestRoot, [string]$Relative, [string]$Origin)
    $dest    = Join-Path $DestRoot $Relative
    $destDir = [IO.Path]::GetDirectoryName($dest)
    $leaf    = [IO.Path]::GetFileName($dest)
    $existing = $null
    if ([IO.Directory]::Exists($destDir)) {
        foreach ($f in [IO.Directory]::GetFiles($destDir)) {
            if ([IO.Path]::GetFileName($f) -ieq $leaf) { $existing = $f; break }
        }
    }
    if ($existing) {
        if ((Get-Sha256 $existing) -ieq (Get-Sha256 $SourceFile)) {
            Write-Skip "$Origin`: $Relative is already staged with identical bytes"
            return 'identical'
        }
        throw "Collision: $Origin file '$Relative' is already staged with different bytes ($existing)"
    }
    Copy-FileTo $SourceFile $dest
    return 'added'
}

# CRLF, UTF-8 without BOM, and the content must be plain ASCII with no
# em dashes and no ' -- ' used as a dash.
function Write-TextFileCrlf([string]$Path, [string[]]$Lines) {
    $text = ($Lines -join "`r`n") + "`r`n"
    foreach ($ch in $text.ToCharArray()) {
        if ([int]$ch -gt 127) { throw "Internal: non-ASCII character U+$(([int]$ch).ToString('X4')) in $Path" }
    }
    if ($text -match ' -- ') { throw "Internal: ' -- ' used as a dash in $Path" }
    [void][IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($Path))
    [IO.File]::WriteAllText($Path, $text, (New-Object System.Text.UTF8Encoding($false)))
}

# LF for the checksum files (see .DESCRIPTION).
function Write-TextFileLf([string]$Path, [string[]]$Lines) {
    $text = ($Lines -join "`n") + "`n"
    [void][IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($Path))
    [IO.File]::WriteAllText($Path, $text, (New-Object System.Text.UTF8Encoding($false)))
}

# Writes <Root>\SHA256SUMS.txt over every other file in the tree.
function Write-ChecksumFile([string]$Root) {
    $entries = Get-TreeEntries -Root $Root
    $lines = New-Object System.Collections.Generic.List[string]
    foreach ($f in ($entries.Files | Sort-Object -Property Relative)) {
        if ($f.Relative -ieq 'SHA256SUMS.txt') { continue }
        $lines.Add(((Get-Sha256 $f.Full).ToLowerInvariant()) + '  ' + $f.Relative.Replace('\', '/'))
    }
    Write-TextFileLf (Join-Path $Root 'SHA256SUMS.txt') $lines.ToArray()
    return $lines.Count
}

# Deletes a staging folder, and only a staging folder.
function Remove-StageFolder([string]$Path) {
    $stageBase = Join-Path $script:OutDirFull 'stage'
    if (-not (Test-PathUnder $stageBase $Path)) { throw "Refusing to delete outside $stageBase`: $Path" }
    if ([IO.Path]::GetFullPath($Path).TrimEnd('\') -ieq [IO.Path]::GetFullPath($stageBase).TrimEnd('\')) {
        throw "Refusing to delete the stage root itself: $Path"
    }
    if (-not (Test-Path -LiteralPath $Path)) { return }
    if (Test-ReparsePoint $Path) { throw "Refusing to delete a link/junction: $Path" }
    Write-Info "removing previous staging folder $Path"
    Remove-Item -LiteralPath $Path -Recurse -Force
}

# ---------------------------------------------------------------------------
# ssh helpers (Windows OpenSSH, key auth, BatchMode). Files are streamed
# through 'cat' and a raw byte copy so neither scp's sftp/legacy quoting nor
# PowerShell's text pipeline touches them.
# ---------------------------------------------------------------------------
function ConvertTo-WindowsCommandLine([string[]]$Arguments) {
    $parts = foreach ($a in $Arguments) {
        if ($a -ne '' -and $a -notmatch '[\s"]') { $a; continue }
        $sb = New-Object System.Text.StringBuilder
        [void]$sb.Append('"')
        $i = 0
        $n = $a.Length
        while ($i -lt $n) {
            $backslashes = 0
            while ($i -lt $n -and $a[$i] -eq '\') { $backslashes++; $i++ }
            if ($i -eq $n) {
                [void]$sb.Append('\', $backslashes * 2)
            } elseif ($a[$i] -eq '"') {
                [void]$sb.Append('\', $backslashes * 2 + 1)
                [void]$sb.Append('"')
                $i++
            } else {
                [void]$sb.Append('\', $backslashes)
                [void]$sb.Append($a[$i])
                $i++
            }
        }
        [void]$sb.Append('"')
        $sb.ToString()
    }
    return ($parts -join ' ')
}

function ConvertTo-ShellQuoted([string]$Text) {
    return "'" + $Text.Replace("'", "'\''") + "'"
}

function Get-SshExe {
    $inbox = Join-Path $env:SystemRoot 'System32\OpenSSH\ssh.exe'
    if (Test-Path -LiteralPath $inbox) { return $inbox }
    $cmd = Get-Command ssh.exe -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    throw 'ssh.exe not found (Windows OpenSSH client is required unless -SkipFetch)'
}

# Runs a remote command. Without -OutFile returns stdout as a UTF-8 string;
# with -OutFile streams stdout bytes to that file. stderr passes through.
function Invoke-Ssh {
    param([Parameter(Mandatory)][string]$RemoteCommand, [string]$OutFile = '')
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $script:SshExe
    $psi.Arguments = ConvertTo-WindowsCommandLine @('-o', 'BatchMode=yes', '-o', 'ConnectTimeout=30', '-n', $Server, $RemoteCommand)
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $false
    $psi.RedirectStandardInput = $false
    $p = [System.Diagnostics.Process]::Start($psi)
    try {
        $result = $null
        if ($OutFile -ne '') {
            $fs = [IO.File]::Open($OutFile, [IO.FileMode]::Create, [IO.FileAccess]::Write, [IO.FileShare]::None)
            try { $p.StandardOutput.BaseStream.CopyTo($fs) } finally { $fs.Dispose() }
        } else {
            $ms = New-Object IO.MemoryStream
            $p.StandardOutput.BaseStream.CopyTo($ms)
            $result = [Text.Encoding]::UTF8.GetString($ms.ToArray())
        }
        $p.WaitForExit()
        if ($p.ExitCode -ne 0) { throw "ssh $Server exited with code $($p.ExitCode) for: $RemoteCommand" }
        return $result
    } finally {
        $p.Dispose()
    }
}

# ---------------------------------------------------------------------------
# Server extras: roles, manifest, cache
# ---------------------------------------------------------------------------
function Get-ExtraRole([string]$Path) {
    if ($script:EastingFiles -ccontains $Path)   { return 'easting' }
    if ($script:DepsFixedFiles -ccontains $Path) { return 'depsfixed' }
    if ($Path -match ('^' + [regex]::Escape($script:DepsAddonsDir) + '/[^/]+\.pbo$')) { return 'depsaddon' }
    if ($Path -clike ($script:DepsReadmesDir + '/*')) { return 'depsreadme' }
    if ($Path -clike ($script:ArchivesDir + '/*'))    { return 'archive' }
    return ''
}

function Get-FetchRoots {
    $roots = @($script:EastingFiles) + @($script:DepsFixedFiles) + @($script:DepsAddonsDir, $script:DepsReadmesDir)
    if ($IncludeSources) { $roots += $script:ArchivesDir }
    return $roots
}

# One ssh call: sha256sum over every file under the fetch roots. Returns a
# list of objects (Hash upper, Path server-relative, Role).
function Get-RemoteManifest {
    $roots = Get-FetchRoots
    $quoted = @($roots | ForEach-Object { ConvertTo-ShellQuoted $_ }) -join ' '
    $cmd = 'cd ' + (ConvertTo-ShellQuoted $ServerPath) +
           ' && for f in ' + $quoted + '; do test -e "$f" || { echo "missing on server: $f" 1>&2; exit 3; }; done' +
           ' && find ' + $quoted + ' -type f -print0 | LC_ALL=C sort -z | xargs -0 -r sha256sum'
    Write-Info "ssh $Server`: sha256sum over $($roots.Count) roots under $ServerPath"
    $text = Invoke-Ssh -RemoteCommand $cmd
    $entries = New-Object System.Collections.Generic.List[object]
    $unknown = New-Object System.Collections.Generic.List[string]
    foreach ($raw in ($text -split "`n")) {
        $line = $raw.TrimEnd("`r")
        if ($line -eq '') { continue }
        if ($line -notmatch '^([0-9a-fA-F]{64})  (.+)$') { throw "Unparseable sha256sum line from server: $line" }
        $hash = $Matches[1].ToUpperInvariant()
        $path = $Matches[2]
        $role = Get-ExtraRole $path
        if ($role -eq '') { $unknown.Add($path); continue }
        $entries.Add([pscustomobject]@{ Hash = $hash; Path = $path; Role = $role })
    }
    if ($unknown.Count -gt 0) {
        $msg = "Unexpected files under the fetch roots on the server:`n   " + ($unknown -join "`n   ")
        if ($AllowUnknown) { Write-Warn ($msg + "`n   (ignored because -AllowUnknown)") } else { throw $msg }
    }
    foreach ($must in (@($script:EastingFiles) + @($script:DepsFixedFiles))) {
        if (-not ($entries | Where-Object { $_.Path -ceq $must })) { throw "Server is missing $must" }
    }
    return $entries
}

function Get-CachePath([string]$Relative) {
    return Join-Path $script:CacheDirFull ($Relative.Replace('/', '\'))
}

function Test-CachedMatch([object]$Entry) {
    $local = Get-CachePath $Entry.Path
    if (-not (Test-Path -LiteralPath $local)) { return $false }
    return ((Get-Sha256 $local) -ieq $Entry.Hash)
}

function Receive-RemoteFile([object]$Entry) {
    $local = Get-CachePath $Entry.Path
    $part  = $local + '.part'
    [void][IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($local))
    $remote = $ServerPath.TrimEnd('/') + '/' + $Entry.Path
    try {
        Invoke-Ssh -RemoteCommand ('cat ' + (ConvertTo-ShellQuoted $remote)) -OutFile $part
        $actual = Get-Sha256 $part
        if ($actual -ine $Entry.Hash) {
            throw "Hash mismatch after transfer for $($Entry.Path): server $($Entry.Hash), received $actual"
        }
        if (Test-Path -LiteralPath $local) { Remove-Item -LiteralPath $local -Force }
        [IO.File]::Move($part, $local)
    } finally {
        if (Test-Path -LiteralPath $part) { Remove-Item -LiteralPath $part -Force }
    }
    return (New-Object IO.FileInfo($local)).Length
}

# Builds the inventory of cache files the build will use, grouped by role.
# From a manifest (fetched now, or saved by an earlier fetch) or, with
# -SkipFetch and no saved manifest, from a directory scan.
function New-Inventory {
    return [pscustomobject]@{
        Easting     = New-Object System.Collections.Generic.List[object]
        DepsFixed   = New-Object System.Collections.Generic.List[object]
        DepsAddons  = New-Object System.Collections.Generic.List[object]
        DepsReadmes = New-Object System.Collections.Generic.List[object]
        Archives    = New-Object System.Collections.Generic.List[object]
    }
}

function Add-InventoryItem([object]$Inventory, [string]$Role, [string]$Relative) {
    $item = [pscustomobject]@{ Relative = $Relative; Full = (Get-CachePath $Relative) }
    switch ($Role) {
        'easting'    { $Inventory.Easting.Add($item) }
        'depsfixed'  { $Inventory.DepsFixed.Add($item) }
        'depsaddon'  { $Inventory.DepsAddons.Add($item) }
        'depsreadme' { $Inventory.DepsReadmes.Add($item) }
        'archive'    { $Inventory.Archives.Add($item) }
        default      { throw "Internal: unknown role $Role" }
    }
}

function Assert-InventoryComplete([object]$Inventory, [switch]$SkipLocalCheck) {
    # Note: @($genericList) throws 'Argument types do not match' on PS 5.1; use ToArray().
    $fixed = $Inventory.Easting.ToArray() + $Inventory.DepsFixed.ToArray()
    if (-not $SkipLocalCheck) {
        foreach ($item in $fixed) {
            if (-not (Test-Path -LiteralPath $item.Full)) { throw "Cache is missing $($item.Relative) ($($item.Full))" }
        }
    }
    foreach ($must in (@($script:EastingFiles) + @($script:DepsFixedFiles))) {
        $have = @($fixed | Where-Object { $_.Relative -ceq $must })
        if ($have.Count -eq 0) { throw "Cache inventory lacks $must" }
    }
    if ($Inventory.DepsAddons.Count -eq 0)  { throw "Cache has no pbo under $($script:DepsAddonsDir)" }
    if ($Inventory.DepsReadmes.Count -eq 0) { throw "Cache has no file under $($script:DepsReadmesDir)" }
    if ($IncludeSources -and $Inventory.Archives.Count -eq 0) { throw "Cache has no file under $($script:ArchivesDir) (needed for -IncludeSources)" }
}

# -NotFetchedYet: the manifest came from the server and nothing is cached
# yet (PlanOnly), so only the server-side completeness is checked.
function Get-InventoryFromManifest([object[]]$Entries, [switch]$NotFetchedYet) {
    $inv = New-Inventory
    foreach ($e in $Entries) {
        if ($e.Role -eq 'archive' -and -not $IncludeSources) { continue }
        Add-InventoryItem $inv $e.Role $e.Path
    }
    Assert-InventoryComplete $inv -SkipLocalCheck:$NotFetchedYet
    return $inv
}

function Read-SavedManifest {
    $path = Join-Path $script:CacheDirFull $script:ManifestName
    if (-not (Test-Path -LiteralPath $path)) { return $null }
    $entries = New-Object System.Collections.Generic.List[object]
    foreach ($raw in [IO.File]::ReadAllLines($path)) {
        $line = $raw.TrimEnd("`r")
        if ($line -eq '') { continue }
        if ($line -notmatch '^([0-9a-fA-F]{64})  (.+)$') { throw "Unparseable line in $path`: $line" }
        $role = Get-ExtraRole $Matches[2]
        if ($role -eq '') { continue }
        $entries.Add([pscustomobject]@{ Hash = $Matches[1].ToUpperInvariant(); Path = $Matches[2]; Role = $role })
    }
    return $entries.ToArray()
}

function Save-Manifest([object[]]$Entries) {
    $lines = @($Entries | Sort-Object -Property Path | ForEach-Object { $_.Hash.ToLowerInvariant() + '  ' + $_.Path })
    Write-TextFileLf (Join-Path $script:CacheDirFull $script:ManifestName) $lines
}

function Get-InventoryFromScan {
    $inv = New-Inventory
    foreach ($rel in $script:EastingFiles)   { Add-InventoryItem $inv 'easting' $rel }
    foreach ($rel in $script:DepsFixedFiles) { Add-InventoryItem $inv 'depsfixed' $rel }
    $addonsDir = Get-CachePath $script:DepsAddonsDir
    if (Test-Path -LiteralPath $addonsDir) {
        foreach ($f in [IO.Directory]::GetFiles($addonsDir)) {
            if ([IO.Path]::GetFileName($f) -like '*.pbo') {
                Add-InventoryItem $inv 'depsaddon' ($script:DepsAddonsDir + '/' + [IO.Path]::GetFileName($f))
            }
        }
    }
    $readmesDir = Get-CachePath $script:DepsReadmesDir
    if (Test-Path -LiteralPath $readmesDir) {
        foreach ($f in (Get-TreeEntries -Root $readmesDir).Files) {
            Add-InventoryItem $inv 'depsreadme' ($script:DepsReadmesDir + '/' + $f.Relative.Replace('\', '/'))
        }
    }
    if ($IncludeSources) {
        $archDir = Get-CachePath $script:ArchivesDir
        if (Test-Path -LiteralPath $archDir) {
            foreach ($f in (Get-TreeEntries -Root $archDir).Files) {
                Add-InventoryItem $inv 'archive' ($script:ArchivesDir + '/' + $f.Relative.Replace('\', '/'))
            }
        }
    }
    Assert-InventoryComplete $inv
    return $inv
}

function Get-InventoryBytes([object]$Inventory) {
    $total = [long]0
    foreach ($list in @($Inventory.Easting, $Inventory.DepsFixed, $Inventory.DepsAddons, $Inventory.DepsReadmes, $Inventory.Archives)) {
        foreach ($item in $list) {
            if (Test-Path -LiteralPath $item.Full) { $total += (New-Object IO.FileInfo($item.Full)).Length }
        }
    }
    return $total
}

# ---------------------------------------------------------------------------
# Source classification
# ---------------------------------------------------------------------------
# Classifies the top-level items of a @LoBo-shaped folder. Returns a list of
# objects (Name, Kind, Action, Reason, Full, Bin). Actions: copy, skip, bin
# (the source Bin folder, with its own sub-plan), unknown.
function Get-SourcePlan {
    param([Parameter(Mandatory)][string]$Root, [switch]$IsExtra)
    $plan = New-Object System.Collections.Generic.List[object]
    $label = if ($IsExtra) { 'ExtraDir' } else { '@LoBo' }
    foreach ($dir in [IO.Directory]::GetDirectories($Root)) {
        $name = [IO.Path]::GetFileName($dir)
        if (Test-ReparsePoint $dir) { throw "$label top-level item is a link/junction: $dir" }
        $item = [pscustomobject]@{ Name = $name; Kind = 'dir'; Action = 'unknown'; Reason = ''; Full = $dir; Bin = $null }
        if ($script:SkipReasons.ContainsKey($name)) {
            $item.Action = 'skip'; $item.Reason = $script:SkipReasons[$name]
        } elseif ($name -ieq 'Bin') {
            if ($IsExtra) { throw "ExtraDir may not carry a Bin folder: $dir" }
            $item.Action = 'bin'; $item.Bin = Get-BinPlan $dir
        } elseif ($script:AllowedDirs -icontains $name) {
            $item.Action = 'copy'
        } elseif ($script:AllowedFiles -icontains $name) {
            throw "$label item $name is a folder but a file was expected: $dir"
        }
        $plan.Add($item)
    }
    foreach ($file in [IO.Directory]::GetFiles($Root)) {
        $name = [IO.Path]::GetFileName($file)
        if (Test-ReparsePoint $file) { throw "$label top-level item is a link: $file" }
        $item = [pscustomobject]@{ Name = $name; Kind = 'file'; Action = 'unknown'; Reason = ''; Full = $file; Bin = $null }
        if ($script:SkipReasons.ContainsKey($name)) {
            $item.Action = 'skip'; $item.Reason = $script:SkipReasons[$name]
        } elseif (Test-JunkName $name) {
            $item.Action = 'skip'; $item.Reason = 'runtime junk'
        } elseif ($script:AllowedFiles -icontains $name) {
            $item.Action = 'copy'
        } elseif ($script:AllowedDirs -icontains $name -or $name -ieq 'Bin') {
            throw "$label item $name is a file but a folder was expected: $file"
        }
        $plan.Add($item)
    }
    $unknown = @($plan | Where-Object { $_.Action -eq 'unknown' } | ForEach-Object { $_.Name })
    if ($unknown.Count -gt 0) {
        $msg = "Unknown top-level item(s) in $label ($Root): " + ($unknown -join ', ')
        if ($AllowUnknown) {
            Write-Warn ($msg + ' (skipped because -AllowUnknown)')
            foreach ($item in $plan) { if ($item.Action -eq 'unknown') { $item.Action = 'skip'; $item.Reason = 'unknown item, -AllowUnknown' } }
        } else {
            throw ($msg + ' (allow it with -AllowUnknown, or add it to the allowlist)')
        }
    }
    return $plan.ToArray()
}

# Inside Bin: RESOURCE.bin and configBackup ship, config.cpp is routed to
# @LoBo_UD, junk is skipped, anything else (a config.bin!) fails outright.
function Get-BinPlan([string]$BinDir) {
    $plan = New-Object System.Collections.Generic.List[object]
    foreach ($dir in [IO.Directory]::GetDirectories($BinDir)) {
        $name = [IO.Path]::GetFileName($dir)
        if ($name -ieq 'configBackup') {
            $plan.Add([pscustomobject]@{ Name = $name; Kind = 'dir'; Action = 'copy'; Full = $dir })
        } else {
            throw "Unexpected folder in Bin: $dir"
        }
    }
    foreach ($file in [IO.Directory]::GetFiles($BinDir)) {
        $name = [IO.Path]::GetFileName($file)
        if ($name -ieq 'RESOURCE.bin') {
            $plan.Add([pscustomobject]@{ Name = $name; Kind = 'file'; Action = 'copy'; Full = $file })
        } elseif ($name -ieq 'config.cpp') {
            $plan.Add([pscustomobject]@{ Name = $name; Kind = 'file'; Action = 'route'; Full = $file })
        } elseif (Test-JunkName $name) {
            $plan.Add([pscustomobject]@{ Name = $name; Kind = 'file'; Action = 'skip'; Full = $file })
        } else {
            throw "Unexpected file in Bin: $file (a config.bin here would be loaded by every engine; merge or remove it first)"
        }
    }
    return $plan.ToArray()
}

# ---------------------------------------------------------------------------
# Generated text
# ---------------------------------------------------------------------------
function Get-RepairTableLines {
    $lines = New-Object System.Collections.Generic.List[string]
    foreach ($key in ($ExpectedRepairHashes.Keys | Sort-Object)) {
        $lines.Add('   ' + $key.Replace('/', '\'))
        if ($script:StockHashes.ContainsKey($key)) {
            $lines.Add('      stock (replaced): ' + $script:StockHashes[$key].ToLowerInvariant())
        } else {
            $lines.Add('      stock (replaced): not recorded before the repair')
        }
        $lines.Add('      shipped:          ' + ([string]$ExpectedRepairHashes[$key]).ToLowerInvariant())
    }
    return $lines.ToArray()
}

function Get-ReadmeFirstLines([int]$LoBoPboCount, [int]$DepsPboCount) {
    return @(
        "$($script:PackageName)",
        'The Lost Brothers (@LoBo) for Arma: Cold War Assault, family package',
        '====================================================================',
        '',
        'FAMILY USE ONLY. Please do not pass this package on or upload it',
        'anywhere. It bundles third-party content (the Lost Brothers mod and',
        'the addon packs it depends on), each under its own licence and credits.',
        'The original readmes and licences are kept under @LoBo\docs and',
        '@LoBo_Deps\readmes; please respect them.',
        '',
        'WHAT IS IN HERE',
        '---------------',
        '@LoBo\            The Lost Brothers mod itself, merged into one modfolder',
        "                  ($LoBoPboCount addon pbos, three islands, demo missions) with",
        '                  three small data repairs applied. See @LoBo\UD-CHANGES.txt.',
        '@LoBo_Deps\       The third-party addons LoBo depends on but never shipped',
        "                  ($DepsPboCount pbos), as a second modfolder. See @LoBo_Deps\README.txt.",
        '@LoBo_UD\         Uslu dur! only: the Guerrilla Mode faction table for the',
        '                  LoBo armies. Never mount it on the stock game or on the',
        '                  official Remastered exe (it would replace the whole game',
        '                  config and break the game). See @LoBo_UD\README.txt.',
        'SHA256SUMS.txt    Checksums of every file in this package.',
        'README-FIRST.txt  This file.',
        '',
        'INSTALL',
        '-------',
        '1. Copy the @LoBo and @LoBo_Deps folders (and @LoBo_UD if you play',
        '   Uslu dur!) into your game folder, next to the game exe and its',
        '   AddOns folder. The result looks like this:',
        '       <game folder>\ColdWarAssault.exe   (or PoseidonGame.exe)',
        '       <game folder>\AddOns\',
        '       <game folder>\@LoBo\addons\...',
        '       <game folder>\@LoBo_Deps\Addons\...',
        '   Keep the folder names exactly as they are.',
        '2. Start the game with the mod flag for your engine (next section).',
        '   The mod only loads when the flag is given, so do not just',
        '   double-click the exe: make a shortcut to it and append the flag to',
        '   the Target line, write a one-line .bat file, or (Steam) put the',
        '   flag in Properties > Launch Options.',
        '',
        'LAUNCH LINES',
        '------------',
        'Stock Arma: Cold War Assault 1.99 (ColdWarAssault.exe):',
        '    ColdWarAssault.exe -mod=@LoBo;@LoBo_Deps',
        '',
        'Official Remastered (Steam folder ...\ARMA Cold War Assault\Remastered,',
        'PoseidonGame.exe):',
        '    PoseidonGame.exe --mod "@LoBo;@LoBo_Deps"',
        '',
        'Uslu dur! (our build of the remaster engine, PoseidonGame.exe):',
        '    PoseidonGame.exe --mod "@LoBo;@LoBo_Deps;@LoBo_UD"',
        '',
        'Never mount @LoBo_UD on the stock exe or on the official Remastered exe.',
        '',
        'DEMO MISSIONS',
        '-------------',
        'The missions in @LoBo\Missions appear in Single Player > Missions',
        '(islands: Lebanon80, sinai, iran_iraq, 73eastings, Noe, Intro).',
        'The folders in @LoBo\user missions are unpacked editor missions. To',
        'use one, copy its folder into your profile Missions folder, which is',
        'Users\<your name>\Missions next to the stock exe, or',
        'Documents\ColdWarAssault\Users\<your name>\Missions for the remaster',
        'engines, then open it in the Mission Editor on the matching island.',
        '',
        'VERIFY THE FILES',
        '----------------',
        'SHA256SUMS.txt lists the SHA-256 of every file (Unix line endings, so',
        'sha256sum -c accepts it). From inside this folder:',
        '  Git Bash, Linux or macOS:   sha256sum -c SHA256SUMS.txt',
        '  Windows PowerShell:',
        '    Get-Content SHA256SUMS.txt | ForEach-Object { $h, $p = $_ -split ''  '', 2; if ((Get-FileHash -LiteralPath $p -Algorithm SHA256).Hash -ne $h) { "MISMATCH: $p" } }',
        '  No MISMATCH lines means everything arrived intact.',
        '',
        'CREDITS',
        '-------',
        'The Lost Brothers Mod Team (Miles Teg<GD>, Calm Terror, Uziyhau,',
        'Panzerfaust, Chris G., Noone and others) and the many addon authors',
        'credited in @LoBo\README.txt, @LoBo\docs and @LoBo_Deps\readmes.',
        "Modfolder assembly, repairs and this package: the Uslu dur! project, $($script:Today).",
        'Package built by arma_CWR/tools/lobo/package-lobo-family.ps1.'
    )
}

function Get-ChangesLines {
    $lines = New-Object System.Collections.Generic.List[string]
    $lines.AddRange([string[]]@(
        'UD-CHANGES.txt: what the Uslu dur! project changed in this @LoBo folder',
        '=======================================================================',
        "Package $($script:PackageName), written $($script:Today).",
        '',
        'ASSEMBLY',
        '--------',
        'README.txt in this folder describes how the eight Lost Brothers archives',
        '(LoBoAddonPack3, Arab Pack, Sinai 1.4, Iran/Iraq map, Jordanian/Syrian',
        'Pack, Lebanon 1.3 + Sinai 1.5, JAM2, JAM3_FIXED6) were merged into one',
        'modfolder in July 2026. On 2026-09-04 the Lost Brothers'' own 73 Easting',
        'release (73eastings.rar) was merged the same way:',
        '   addons\73eastings.pbo and addons\Ob73East.pbo',
        '   Missions\73EastingDemo.73eastings.pbo',
        '   docs\73eastings\73eastings_provenance.txt',
        'Everything else in this folder is the mod as released, apart from the',
        'three repairs and the one moved file below.',
        '',
        'REPAIRS (pure data fixes, valid on every engine)',
        '-----------------------------------------------',
        '1. Abstract wrecks and posters. addons\LoBoWreck.pbo and',
        '   addons\LoBoPalObj.pbo wrote "scope = public" without the #define',
        '   header that every other LoBo config carries, so the game read the',
        '   scope as 0 and refused to create 18 wreck and poster classes',
        '   ("type is abstract"). Eleven sites were rewritten in place to',
        '   "scope = 2".',
        '2. Buried M60 wrecks. LoBo_M60A1_wreck and LoBo_M60A1_wreck2 in',
        '   addons\LoBoWreck.pbo had their model origin above the mesh, so a',
        '   placed wreck sat under the ground. boundingCenter.Y was rewritten so',
        '   both seat on the surface.',
        '3. Malformed tracer colour. addons\LoBoammo.pbo (8 classes) and',
        '   addons\LoBo_airammo.pbo (3 classes) carried tracerColor 0.0.1, which',
        '   is not a number. Rewritten to 0.001.',
        '',
        '   SHA-256 of the repaired files, before and after:'
    ))
    $lines.AddRange([string[]](Get-RepairTableLines))
    $lines.AddRange([string[]]@(
        '',
        'MOVED OUT: Bin\config.cpp',
        '-------------------------',
        'On the build machine this folder also carries Bin\config.cpp, the',
        'Guerrilla Mode faction table for the eight LoBo war rosters. That file',
        'is for the Uslu dur! engine only. Uslu dur! layers each mounted mod''s',
        'bin\config onto the base game config; the stock ColdWarAssault.exe and',
        'the official Remastered PoseidonGame.exe instead REPLACE the whole base',
        'config with it, which breaks the game. The package therefore ships it',
        'as its own modfolder, @LoBo_UD\bin\config.cpp, to be mounted only on',
        'Uslu dur!. This folder ships Bin\RESOURCE.bin and',
        'Bin\configBackup\Config.bin exactly as stock.',
        '',
        'KNOWN QUIRKS (unrepaired, from the stock mod)',
        '---------------------------------------------',
        '- The sinai, iran_iraq and gaza worlds point their menu intro at a',
        '  lost_anims addon that was never released. Uslu dur! has an engine',
        '  fallback for it.',
        '- coc_diver.pbo: the classes CoC_Diverdes and CoC_DiverSaboteurj',
        '  reference a missing \seals\ model and crash the game if placed. Do',
        '  not use them in the editor.',
        '- About a dozen bundled prop addons use destrType names that are not',
        '  defined, so those props cannot be destroyed.',
        '- Three referenced addons were never publicly released',
        '  (vit_iraq_shilka, f3wx_markers, bde_weapons). The references are',
        '  harmless.',
        '',
        'Built by arma_CWR/tools/lobo/package-lobo-family.ps1.'
    ))
    return $lines.ToArray()
}

function Get-UdReadmeLines {
    return @(
        '@LoBo_UD: Uslu dur! faction table for the Lost Brothers mod',
        '===========================================================',
        'This modfolder holds one file, bin\config.cpp: the CfgGuerrillaFactions',
        'table that gives Guerrilla Mode its eight Lost Brothers war rosters',
        '(IDF, EgyptFrontier, EgyptArmy, Syria, Jordan, Hizballah, PLO, PLO_East).',
        '',
        'USLU DUR! ONLY. Mount it after @LoBo and @LoBo_Deps:',
        '    PoseidonGame.exe --mod "@LoBo;@LoBo_Deps;@LoBo_UD"',
        '',
        'Never mount it on the stock ColdWarAssault.exe or on the official',
        'Remastered PoseidonGame.exe. Those engines make a mod''s bin\config',
        'replace the entire base game config, so the game would start with no',
        'vehicles, weapons or menus. Uslu dur! layers the file on top of the',
        'base config instead, which is what it is written for.',
        '',
        'The file was generated by arma_CWR/tools/lobo/install-lobo-factions.ps1',
        'from the tracked source guerrilla-mode/config/lobo-factions.hpp and is',
        'copied here byte for byte from @LoBo\Bin\config.cpp on the build',
        'machine. It was moved out of @LoBo so that the same @LoBo folder is',
        'safe on every engine.',
        '',
        "Package $($script:PackageName), written $($script:Today)."
    )
}

function Get-SourcesNoteLines([int]$ArchiveCount, [int]$InstallerCount, [string[]]$StockNames) {
    $installers = ''
    if ($InstallerCount -eq 1) { $installers = ' (one of them is a self-extracting installer that was never run)' }
    elseif ($InstallerCount -gt 1) { $installers = " ($InstallerCount of them are self-extracting installers that were never run)" }
    $lines = New-Object System.Collections.Generic.List[string]
    $lines.AddRange([string[]]@(
        "$($script:SourcesName)",
        ('=' * $script:SourcesName.Length),
        "Companion to $($script:PackageName). Not needed to play; kept so the",
        'package can be rebuilt or audited.',
        '',
        "deps\archives\      the $ArchiveCount original downloads the dependency set was",
        "                    taken from, byte for byte$installers.",
        'deps\SHA256SUMS     checksums of those downloads as recorded on the',
        '                    server; every archive was verified against it',
        '                    before packing.',
        'deps\clamscan.log   antivirus scan log of the downloads.',
        'deps\README.txt     the dependency manifest with launch notes.',
        'stock-originals\    the @LoBo pbos exactly as released, before the',
        '                    repairs described in @LoBo\UD-CHANGES.txt',
        '                    (from @LoBo\_ud-orig on the build machine):'
    ))
    foreach ($n in $StockNames) { $lines.Add('                      ' + $n) }
    $lines.AddRange([string[]]@(
        'lobo-modfolder-assembly.txt',
        '                    the notes from assembling the @LoBo modfolder.',
        '',
        'FAMILY USE ONLY. Third-party content under each pack''s own licence.',
        "Written $($script:Today) by arma_CWR/tools/lobo/package-lobo-family.ps1."
    ))
    return $lines.ToArray()
}

# ---------------------------------------------------------------------------
# 7-Zip
# ---------------------------------------------------------------------------
function New-SevenZipArchive([string]$StageFolder, [string]$ArchivePath) {
    if (Test-Path -LiteralPath $ArchivePath) {
        if (-not $Force) { throw "Archive already exists: $ArchivePath (use -Force to overwrite)" }
        Write-Info "removing existing $ArchivePath (-Force)"
        Remove-Item -LiteralPath $ArchivePath -Force
    }
    $sidecar = $ArchivePath + '.sha256'
    if (Test-Path -LiteralPath $sidecar) { Remove-Item -LiteralPath $sidecar -Force }
    $parent = [IO.Path]::GetDirectoryName($StageFolder)
    $leaf   = [IO.Path]::GetFileName($StageFolder)
    Write-Info "7z a -t7z -mx=5 -mmt=on $ArchivePath $leaf  (cwd $parent)"
    Push-Location -LiteralPath $parent
    try {
        & $SevenZip a -t7z -mx=5 -mmt=on $ArchivePath $leaf
        if ($LASTEXITCODE -ne 0) { throw "7z a failed with exit code $LASTEXITCODE" }
    } finally {
        Pop-Location
    }
    Write-Info "7z t $ArchivePath"
    & $SevenZip t $ArchivePath
    if ($LASTEXITCODE -ne 0) { throw "7z t failed with exit code $LASTEXITCODE" }
    $hash = (Get-Sha256 $ArchivePath).ToLowerInvariant()
    Write-TextFileLf $sidecar @($hash + '  ' + [IO.Path]::GetFileName($ArchivePath))
    $size = (New-Object IO.FileInfo($ArchivePath)).Length
    Write-Info "archive: $ArchivePath ($(Format-Bytes $size))"
    Write-Info "sha256:  $hash  (sidecar $sidecar)"
}

# ---------------------------------------------------------------------------
# Preflight
# ---------------------------------------------------------------------------
Write-Step 'Preflight'

if ($Version -notmatch '^[A-Za-z0-9][A-Za-z0-9._-]*$') { throw "Version must be a plain token (letters, digits, . _ -): '$Version'" }
if (-not (Test-Path -LiteralPath $LoBoDir -PathType Container)) { throw "@LoBo folder not found: $LoBoDir" }
$script:LoBoDirFull  = [IO.Path]::GetFullPath($LoBoDir)
$script:OutDirFull   = [IO.Path]::GetFullPath($OutDir)
if ($CacheDir -eq '') { $CacheDir = Join-Path $script:OutDirFull '_server-cache' }
$script:CacheDirFull = [IO.Path]::GetFullPath($CacheDir)
$script:StageBase    = Join-Path $script:OutDirFull 'stage'
$script:StageRoot    = Join-Path $script:StageBase $script:PackageName
$script:SourcesRoot  = Join-Path $script:StageBase $script:SourcesName
$script:ArchivePath  = Join-Path $script:OutDirFull ($script:PackageName + '.7z')
$script:SourcesArchivePath = Join-Path $script:OutDirFull ($script:SourcesName + '.7z')
$script:AssemblyNotes = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot 'lobo-modfolder-assembly.txt'))

if (Test-PathUnder $script:LoBoDirFull $script:OutDirFull) { throw "OutDir must not be inside the @LoBo folder: $OutDir" }
if (Test-PathUnder $script:OutDirFull $script:LoBoDirFull) { throw "The @LoBo folder must not be inside OutDir: $LoBoDir" }
$script:ExtraDirFull = ''
if ($ExtraDir -ne '') {
    if (-not (Test-Path -LiteralPath $ExtraDir -PathType Container)) { throw "ExtraDir not found: $ExtraDir" }
    $script:ExtraDirFull = [IO.Path]::GetFullPath($ExtraDir)
    if (Test-PathUnder $script:StageBase $script:ExtraDirFull) { throw "ExtraDir must not be inside the staging area: $ExtraDir" }
}
if (-not $NoArchive -and -not (Test-Path -LiteralPath $SevenZip)) { throw "7z.exe not found: $SevenZip (install 7-Zip, pass -SevenZip, or use -NoArchive)" }
if ($IncludeSources -and -not (Test-Path -LiteralPath $script:AssemblyNotes)) { throw "assembly notes not found: $($script:AssemblyNotes)" }
if (-not $SkipFetch) { $script:SshExe = Get-SshExe } else { $script:SshExe = '' }

Write-Info "source @LoBo : $($script:LoBoDirFull)"
Write-Info "output folder: $($script:OutDirFull)"
Write-Info "version      : $Version"
Write-Info "cache        : $($script:CacheDirFull)"
if ($script:ExtraDirFull -ne '') { Write-Info "extra dir    : $($script:ExtraDirFull)" }

# 1. The repaired pbos: pinned hashes, and never the stock bytes.
Write-Info 'repaired pbos:'
foreach ($key in ($ExpectedRepairHashes.Keys | Sort-Object)) {
    $path = Join-Path $script:LoBoDirFull ($key.Replace('/', '\'))
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Repaired file missing: $path" }
    $actual = Get-Sha256 $path
    if ($script:StockHashes.ContainsKey($key) -and $actual -ieq $script:StockHashes[$key]) {
        throw "$key is the STOCK (unrepaired) file ($actual). Run the mod doctor (tools/lobo/fix-lobo-scope.ps1) first."
    }
    $expected = [string]$ExpectedRepairHashes[$key]
    if ($actual -ine $expected) { throw "$key hash $actual does not match the pinned repair hash $expected" }
    Write-Info "   ok  $key  $actual"
}

# 2. Bin\config.cpp with the GENERATED banner.
$script:BinDir = $null
foreach ($d in [IO.Directory]::GetDirectories($script:LoBoDirFull)) {
    if ([IO.Path]::GetFileName($d) -ieq 'Bin') { $script:BinDir = $d; break }
}
if (-not $script:BinDir) { throw "No Bin folder in $($script:LoBoDirFull)" }
$script:ConfigCpp = $null
foreach ($f in [IO.Directory]::GetFiles($script:BinDir)) {
    if ([IO.Path]::GetFileName($f) -ieq 'config.cpp') { $script:ConfigCpp = $f; break }
}
if (-not $script:ConfigCpp) { throw "Bin\config.cpp not found under $($script:BinDir); run tools/lobo/install-lobo-factions.ps1 first" }
$firstLine = ''
$reader = New-Object IO.StreamReader($script:ConfigCpp)
try { $firstLine = $reader.ReadLine() } finally { $reader.Dispose() }
if ($null -eq $firstLine -or -not $firstLine.StartsWith($script:Banner)) {
    throw "Bin\config.cpp does not start with the banner '$($script:Banner)'; refusing to package an unknown config (first line: '$firstLine')"
}
Write-Info "config.cpp   : $($script:ConfigCpp) (GENERATED banner ok) -> @LoBo_UD\bin\config.cpp"

# 3. Top-level items of @LoBo (and ExtraDir).
$script:SourcePlan = Get-SourcePlan -Root $script:LoBoDirFull
$script:ExtraPlan  = @()
if ($script:ExtraDirFull -ne '') { $script:ExtraPlan = Get-SourcePlan -Root $script:ExtraDirFull -IsExtra }

# Enumerate what will be copied (sizes, junk, source pbo count).
$script:SourceBytes = [long]0
$script:SourceFiles = 0
$script:JunkSkipped = New-Object System.Collections.Generic.List[string]
$script:SourceAddonPboNames = New-Object System.Collections.Generic.HashSet[string]([StringComparer]::OrdinalIgnoreCase)
Write-Info 'top-level items:'
foreach ($item in $script:SourcePlan) {
    switch ($item.Action) {
        'copy' {
            if ($item.Kind -eq 'dir') {
                $entries = Get-TreeEntries -Root $item.Full -SkipJunk
                $item | Add-Member -NotePropertyName Entries -NotePropertyValue $entries
                $bytes = [long]0
                foreach ($f in $entries.Files) { $bytes += $f.Length }
                foreach ($j in $entries.Junk) { $script:JunkSkipped.Add($item.Name + '\' + $j) }
                $script:SourceBytes += $bytes
                $script:SourceFiles += $entries.Files.Count
                $extra = ''
                if ($item.Name -ieq 'addons') {
                    foreach ($f in [IO.Directory]::GetFiles($item.Full)) {
                        $n = [IO.Path]::GetFileName($f)
                        if ($n -like '*.pbo' -and -not (Test-JunkName $n)) { [void]$script:SourceAddonPboNames.Add($n) }
                    }
                    $extra = ", $($script:SourceAddonPboNames.Count) pbo"
                }
                Write-Info ('   copy   {0,-40} {1} files, {2}{3}' -f ($item.Name + '\'), $entries.Files.Count, (Format-Bytes $bytes), $extra)
            } else {
                $len = (New-Object IO.FileInfo($item.Full)).Length
                $script:SourceBytes += $len
                $script:SourceFiles += 1
                Write-Info ('   copy   {0,-40} {1}' -f $item.Name, (Format-Bytes $len))
            }
        }
        'bin' {
            foreach ($b in $item.Bin) {
                switch ($b.Action) {
                    'copy' {
                        if ($b.Kind -eq 'dir') {
                            $entries = Get-TreeEntries -Root $b.Full -SkipJunk
                            $b | Add-Member -NotePropertyName Entries -NotePropertyValue $entries
                            $bytes = [long]0
                            foreach ($f in $entries.Files) { $bytes += $f.Length }
                            foreach ($j in $entries.Junk) { $script:JunkSkipped.Add($item.Name + '\' + $b.Name + '\' + $j) }
                            $script:SourceBytes += $bytes
                            $script:SourceFiles += $entries.Files.Count
                            Write-Info ('   copy   {0,-40} {1} files, {2}' -f ($item.Name + '\' + $b.Name + '\'), $entries.Files.Count, (Format-Bytes $bytes))
                        } else {
                            $len = (New-Object IO.FileInfo($b.Full)).Length
                            $script:SourceBytes += $len
                            $script:SourceFiles += 1
                            Write-Info ('   copy   {0,-40} {1}' -f ($item.Name + '\' + $b.Name), (Format-Bytes $len))
                        }
                    }
                    'route' { Write-Info ('   route  {0,-40} -> @LoBo_UD\bin\config.cpp' -f ($item.Name + '\' + $b.Name)) }
                    'skip'  { $script:JunkSkipped.Add($item.Name + '\' + $b.Name) }
                }
            }
        }
        'skip' { Write-Skip ('{0,-40} {1}' -f $item.Name, $item.Reason) }
    }
}
if ($script:SourceAddonPboNames.Count -eq 0) { throw 'No addons\*.pbo found in the source @LoBo' }
$script:ExtraAddonPboNew = 0
if ($script:ExtraPlan.Count -gt 0) {
    Write-Info 'ExtraDir items:'
    foreach ($item in $script:ExtraPlan) {
        if ($item.Action -eq 'copy') {
            if ($item.Kind -eq 'dir') {
                $entries = Get-TreeEntries -Root $item.Full -SkipJunk
                $item | Add-Member -NotePropertyName Entries -NotePropertyValue $entries
                foreach ($j in $entries.Junk) { $script:JunkSkipped.Add('ExtraDir\' + $item.Name + '\' + $j) }
                $bytes = [long]0
                foreach ($f in $entries.Files) { $bytes += $f.Length }
                $script:SourceBytes += $bytes
                if ($item.Name -ieq 'addons') {
                    foreach ($f in $entries.Files) {
                        if ($f.Relative -notlike '*\*' -and $f.Relative -like '*.pbo' -and -not $script:SourceAddonPboNames.Contains($f.Relative)) { $script:ExtraAddonPboNew++ }
                    }
                }
                Write-Info ('   merge  {0,-40} {1} files, {2}' -f ($item.Name + '\'), $entries.Files.Count, (Format-Bytes $bytes))
            } else {
                Write-Info ('   merge  {0,-40}' -f $item.Name)
            }
        } elseif ($item.Action -eq 'skip') {
            Write-Skip ('ExtraDir\{0,-31} {1}' -f $item.Name, $item.Reason)
        }
    }
}
if ($script:JunkSkipped.Count -gt 0) {
    Write-Info "runtime junk skipped ($($script:JunkSkipped.Count)):"
    foreach ($j in $script:JunkSkipped) { Write-Skip $j }
}
Write-Info "source total : $($script:SourceFiles) files, $(Format-Bytes $script:SourceBytes)"

# 4. Existing archives.
foreach ($a in @($script:ArchivePath) + $(if ($IncludeSources) { @($script:SourcesArchivePath) } else { @() })) {
    if (-not $NoArchive -and (Test-Path -LiteralPath $a)) {
        if ($Force) { Write-Warn "$a exists and will be overwritten (-Force)" }
        elseif ($PlanOnly) { Write-Warn "$a exists; a real run would refuse without -Force" }
        else { throw "Archive already exists: $a (use -Force to overwrite, or pick another -Version)" }
    }
}

# 5. Cache state and the server plan.
$script:Inventory = $null
$script:Manifest  = $null
if ($SkipFetch) {
    Write-Info 'server       : not contacted (-SkipFetch); verifying the cache'
    $saved = Read-SavedManifest
    if ($saved) {
        Write-Info "cache manifest: $(Join-Path $script:CacheDirFull $script:ManifestName) ($(@($saved).Count) entries)"
        foreach ($e in $saved) {
            if ($e.Role -eq 'archive' -and -not $IncludeSources) { continue }
            if (-not (Test-CachedMatch $e)) { throw "Cache entry missing or stale: $($e.Path) (rerun without -SkipFetch)" }
        }
        $script:Inventory = Get-InventoryFromManifest -Entries $saved
    } else {
        Write-Info 'cache manifest: none; using the cache contents as they are'
        $script:Inventory = Get-InventoryFromScan
    }
} else {
    $script:Manifest = @(Get-RemoteManifest)
    $toFetch = 0
    $cached = 0
    foreach ($e in $script:Manifest) {
        if ($e.Role -eq 'archive' -and -not $IncludeSources) { continue }
        if (Test-CachedMatch $e) { $cached++ } else { $toFetch++ }
    }
    Write-Info "server       : $($script:Manifest.Count) files listed; $cached already cached with matching hash, $toFetch to download"
    if ($PlanOnly) {
        foreach ($e in $script:Manifest) {
            if ($e.Role -eq 'archive' -and -not $IncludeSources) { continue }
            $state = if (Test-CachedMatch $e) { 'cached  ' } else { 'download' }
            Write-Info "   $state $($e.Path)"
        }
        $script:Inventory = Get-InventoryFromManifest -Entries $script:Manifest -NotFetchedYet
    }
}
if ($script:Inventory) {
    Write-Info ("extras       : {0} 73 Easting files, {1} dependency pbos, {2} readme files{3}" -f $script:Inventory.Easting.Count, $script:Inventory.DepsAddons.Count, $script:Inventory.DepsReadmes.Count, $(if ($IncludeSources) { ", $($script:Inventory.Archives.Count) archives" } else { '' }))
}

# 6. Free space, roughly: source and extras twice (stage + archive), plus the
#    cache for whatever is not downloaded yet.
$extrasBytes = [long]0
if ($script:Inventory) { $extrasBytes = Get-InventoryBytes $script:Inventory }
if ($extrasBytes -eq 0) { $extrasBytes = $script:EstimatedDepsBytes + $(if ($IncludeSources) { $script:EstimatedArchivesBytes } else { 0 }) }
$needed = 2 * ($script:SourceBytes + $extrasBytes) + $(if ($SkipFetch) { 0 } else { $extrasBytes })
$free = -1
try {
    $driveRoot = [IO.Path]::GetPathRoot($script:OutDirFull)
    $free = (New-Object IO.DriveInfo($driveRoot)).AvailableFreeSpace
} catch {
    Write-Warn "could not determine free space for $($script:OutDirFull): $($_.Exception.Message)"
}
if ($free -ge 0) {
    Write-Info "disk space   : need about $(Format-Bytes $needed), free $(Format-Bytes $free)"
    if ($free -lt $needed) { throw "Not enough free space on $driveRoot`: need about $(Format-Bytes $needed), have $(Format-Bytes $free)" }
}

# ---------------------------------------------------------------------------
# Plan
# ---------------------------------------------------------------------------
Write-Step 'Plan'
Write-Info "staging      : $($script:StageRoot)"
if ($NoArchive) { Write-Info 'archive      : none (-NoArchive)' } else { Write-Info "archive      : $($script:ArchivePath) (+ .sha256)" }
if ($IncludeSources) {
    Write-Info "sources      : $($script:SourcesRoot)"
    if (-not $NoArchive) { Write-Info "               $($script:SourcesArchivePath) (+ .sha256)" }
}
Write-Info "expected @LoBo\addons pbo count after staging: $($script:SourceAddonPboNames.Count) + $(@($script:EastingFiles | Where-Object { $_ -like 'addons/*.pbo' }).Count) (73 Easting) + $($script:ExtraAddonPboNew) (ExtraDir)"
if ($PlanOnly) {
    Write-Host ''
    Write-Host 'PlanOnly: nothing written.' -ForegroundColor Green
    exit 0
}

# ---------------------------------------------------------------------------
# Fetch
# ---------------------------------------------------------------------------
if (-not $SkipFetch) {
    Write-Step "Fetch from $Server"
    [void][IO.Directory]::CreateDirectory($script:CacheDirFull)
    $wanted = @($script:Manifest | Where-Object { $_.Role -ne 'archive' -or $IncludeSources })
    $i = 0
    $downloaded = 0
    $reused = 0
    $bytes = [long]0
    foreach ($e in $wanted) {
        $i++
        if (Test-CachedMatch $e) {
            $reused++
            Write-Info "[$i/$($wanted.Count)] cached   $($e.Path)"
            continue
        }
        $len = Receive-RemoteFile $e
        $downloaded++
        $bytes += $len
        Write-Info "[$i/$($wanted.Count)] fetched  $($e.Path) ($(Format-Bytes $len))"
    }
    Save-Manifest -Entries $script:Manifest
    Write-Info "downloaded $downloaded files ($(Format-Bytes $bytes)), reused $reused; manifest saved to $(Join-Path $script:CacheDirFull $script:ManifestName)"
    $script:Inventory = Get-InventoryFromManifest -Entries $script:Manifest
}

# ---------------------------------------------------------------------------
# Stage
# ---------------------------------------------------------------------------
Write-Step 'Stage'
[void][IO.Directory]::CreateDirectory($script:StageBase)
Remove-StageFolder $script:StageRoot
[void][IO.Directory]::CreateDirectory($script:StageRoot)
$stageLoBo = Join-Path $script:StageRoot '@LoBo'
$stageDeps = Join-Path $script:StageRoot '@LoBo_Deps'
$stageUd   = Join-Path $script:StageRoot '@LoBo_UD'
[void][IO.Directory]::CreateDirectory($stageLoBo)

# Allowlisted @LoBo content.
foreach ($item in $script:SourcePlan) {
    switch ($item.Action) {
        'copy' {
            if ($item.Kind -eq 'dir') {
                Copy-TreeEntries $item.Entries $item.Full (Join-Path $stageLoBo $item.Name)
                Write-Info "copied $($item.Name)\ ($($item.Entries.Files.Count) files)"
            } else {
                Copy-FileTo $item.Full (Join-Path $stageLoBo $item.Name)
                Write-Info "copied $($item.Name)"
            }
        }
        'bin' {
            $stageBin = Join-Path $stageLoBo $item.Name
            [void][IO.Directory]::CreateDirectory($stageBin)
            foreach ($b in $item.Bin) {
                switch ($b.Action) {
                    'copy' {
                        if ($b.Kind -eq 'dir') { Copy-TreeEntries $b.Entries $b.Full (Join-Path $stageBin $b.Name) }
                        else { Copy-FileTo $b.Full (Join-Path $stageBin $b.Name) }
                        Write-Info "copied $($item.Name)\$($b.Name)"
                    }
                    'route' {
                        Copy-FileTo $b.Full (Join-Path (Join-Path $stageUd 'bin') 'config.cpp')
                        Write-Info "routed $($item.Name)\$($b.Name) -> @LoBo_UD\bin\config.cpp"
                    }
                }
            }
        }
    }
}

# 73 Easting, merged into @LoBo.
$eastingAdded = 0
foreach ($e in $script:Inventory.Easting) {
    if ((Merge-FileInto -SourceFile $e.Full -DestRoot $stageLoBo -Relative ($e.Relative.Replace('/', '\')) -Origin '73 Easting') -eq 'added') { $eastingAdded++ }
}
Write-Info "merged 73 Easting: $eastingAdded files"

# ExtraDir, merged into @LoBo.
if ($script:ExtraPlan.Count -gt 0) {
    $extraAdded = 0
    foreach ($item in $script:ExtraPlan) {
        if ($item.Action -ne 'copy') { continue }
        if ($item.Kind -eq 'dir') {
            foreach ($d in $item.Entries.Dirs) { [void][IO.Directory]::CreateDirectory((Join-Path (Join-Path $stageLoBo $item.Name) $d)) }
            foreach ($f in $item.Entries.Files) {
                if ((Merge-FileInto -SourceFile $f.Full -DestRoot $stageLoBo -Relative ($item.Name + '\' + $f.Relative) -Origin 'ExtraDir') -eq 'added') { $extraAdded++ }
            }
        } else {
            if ((Merge-FileInto -SourceFile $item.Full -DestRoot $stageLoBo -Relative $item.Name -Origin 'ExtraDir') -eq 'added') { $extraAdded++ }
        }
    }
    Write-Info "merged ExtraDir: $extraAdded files"
}

# @LoBo_Deps.
$stagedAddonNames = New-Object System.Collections.Generic.HashSet[string]([StringComparer]::OrdinalIgnoreCase)
$stagedAddonsDir = Join-Path $stageLoBo 'addons'
foreach ($d in [IO.Directory]::GetDirectories($stageLoBo)) {
    if ([IO.Path]::GetFileName($d) -ieq 'addons') { $stagedAddonsDir = $d; break }
}
foreach ($f in [IO.Directory]::GetFiles($stagedAddonsDir)) { [void]$stagedAddonNames.Add([IO.Path]::GetFileName($f)) }
$depsAddonsOut = Join-Path $stageDeps 'Addons'
[void][IO.Directory]::CreateDirectory($depsAddonsOut)
foreach ($p in $script:Inventory.DepsAddons) {
    $name = [IO.Path]::GetFileName($p.Full)
    if ($stagedAddonNames.Contains($name)) { throw "@LoBo_Deps pbo '$name' collides (case-insensitively) with a staged @LoBo\addons file; the game would load one at random" }
    Copy-FileTo $p.Full (Join-Path $depsAddonsOut $name)
}
Write-Info "staged @LoBo_Deps\Addons: $($script:Inventory.DepsAddons.Count) pbos"
foreach ($p in $script:Inventory.DepsFixed) {
    switch ($p.Relative) {
        'deps/@LoBo_Deps/POSSIBLY_UNNEEDED.txt' { Copy-FileTo $p.Full (Join-Path $stageDeps 'POSSIBLY_UNNEEDED.txt') }
        'deps/README.txt'                        { Copy-FileTo $p.Full (Join-Path $stageDeps 'README.txt') }
        default { }   # SHA256SUMS and clamscan.log belong to the Sources archive
    }
}
$readmeCount = 0
foreach ($p in $script:Inventory.DepsReadmes) {
    $rel = $p.Relative.Substring($script:DepsReadmesDir.Length + 1).Replace('/', '\')
    if (Test-JunkName ([IO.Path]::GetFileName($rel))) { Write-Skip "@LoBo_Deps\readmes\$rel (runtime junk)"; continue }
    Copy-FileTo $p.Full (Join-Path (Join-Path $stageDeps 'readmes') $rel)
    $readmeCount++
}
Write-Info "staged @LoBo_Deps: README.txt, POSSIBLY_UNNEEDED.txt, readmes\ ($readmeCount files)"

# @LoBo_UD.
if (-not (Test-Path -LiteralPath (Join-Path (Join-Path $stageUd 'bin') 'config.cpp'))) { throw 'Internal: @LoBo_UD\bin\config.cpp was not routed' }
Write-TextFileCrlf (Join-Path $stageUd 'README.txt') (Get-UdReadmeLines)
Write-Info 'staged @LoBo_UD: bin\config.cpp, README.txt'

# Notes.
$eastingAddonCount = @($script:EastingFiles | Where-Object { $_ -like 'addons/*.pbo' }).Count
Write-TextFileCrlf (Join-Path $stageLoBo 'UD-CHANGES.txt') (Get-ChangesLines)
$stagedPboCount = @([IO.Directory]::GetFiles($stagedAddonsDir) | Where-Object { [IO.Path]::GetFileName($_) -like '*.pbo' }).Count
Write-TextFileCrlf (Join-Path $script:StageRoot 'README-FIRST.txt') (Get-ReadmeFirstLines -LoBoPboCount $stagedPboCount -DepsPboCount $script:Inventory.DepsAddons.Count)
Write-Info 'wrote README-FIRST.txt, @LoBo\UD-CHANGES.txt, @LoBo_UD\README.txt'

# ---------------------------------------------------------------------------
# Verify
# ---------------------------------------------------------------------------
Write-Step 'Verify'
foreach ($key in ($ExpectedRepairHashes.Keys | Sort-Object)) {
    $path = Join-Path $stageLoBo ($key.Replace('/', '\'))
    $actual = Get-Sha256 $path
    if ($actual -ine [string]$ExpectedRepairHashes[$key]) { throw "Staged $key hash $actual does not match the pin" }
    if ($script:StockHashes.ContainsKey($key) -and $actual -ieq $script:StockHashes[$key]) { throw "Staged $key is the stock file" }
}
Write-Info "repaired pbos re-hashed in staging: $($ExpectedRepairHashes.Count) ok"

$stagedLoBo = Get-TreeEntries -Root $stageLoBo
foreach ($d in $stagedLoBo.Dirs) {
    foreach ($seg in $d.Split('\')) {
        if ($script:ForbiddenInStage -icontains $seg) { throw "Forbidden folder in staged @LoBo: $d" }
    }
}
foreach ($f in $stagedLoBo.Files) {
    foreach ($seg in $f.Relative.Split('\')) {
        if ($script:ForbiddenInStage -icontains $seg) { throw "Forbidden file in staged @LoBo: $($f.Relative)" }
    }
    if (Test-JunkName ([IO.Path]::GetFileName($f.Full))) { throw "Runtime junk in staged @LoBo: $($f.Relative)" }
}
Write-Info "no _ud-orig, .git, tools, .ud-backups, config.cpp or runtime junk under staged @LoBo ($($stagedLoBo.Files.Count) files checked)"

$expectedPbo = $script:SourceAddonPboNames.Count + $eastingAddonCount + $script:ExtraAddonPboNew
if ($stagedPboCount -ne $expectedPbo) { throw "Staged @LoBo\addons holds $stagedPboCount pbos, expected $expectedPbo (source $($script:SourceAddonPboNames.Count) + 73 Easting $eastingAddonCount + ExtraDir $($script:ExtraAddonPboNew))" }
Write-Info "@LoBo\addons pbo count: $stagedPboCount = $($script:SourceAddonPboNames.Count) + $eastingAddonCount + $($script:ExtraAddonPboNew)"

$sumCount = Write-ChecksumFile $script:StageRoot
Write-Info "SHA256SUMS.txt: $sumCount entries"

Write-Host ''
Write-Host ('   {0,-18} {1,8} {2,28}' -f 'folder', 'files', 'bytes')
$totalFiles = 0
$totalBytes = [long]0
foreach ($d in ([IO.Directory]::GetDirectories($script:StageRoot) | Sort-Object)) {
    $t = Get-TreeEntries -Root $d
    $b = [long]0
    foreach ($f in $t.Files) { $b += $f.Length }
    $totalFiles += $t.Files.Count
    $totalBytes += $b
    Write-Host ('   {0,-18} {1,8} {2,28}' -f ([IO.Path]::GetFileName($d) + '\'), $t.Files.Count, (Format-Bytes $b))
}
$rootFiles = @([IO.Directory]::GetFiles($script:StageRoot))
$rb = [long]0
foreach ($f in $rootFiles) { $rb += (New-Object IO.FileInfo($f)).Length }
$totalFiles += $rootFiles.Count
$totalBytes += $rb
Write-Host ('   {0,-18} {1,8} {2,28}' -f '(root files)', $rootFiles.Count, (Format-Bytes $rb))
Write-Host ('   {0,-18} {1,8} {2,28}' -f 'TOTAL', $totalFiles, (Format-Bytes $totalBytes))

# ---------------------------------------------------------------------------
# Archive
# ---------------------------------------------------------------------------
if ($NoArchive) {
    Write-Step 'Archive'
    Write-Info 'skipped (-NoArchive)'
} else {
    Write-Step 'Archive'
    New-SevenZipArchive -StageFolder $script:StageRoot -ArchivePath $script:ArchivePath
}

# ---------------------------------------------------------------------------
# Sources (optional)
# ---------------------------------------------------------------------------
if ($IncludeSources) {
    Write-Step 'Sources'
    # Verify the archives against the server's own SHA256SUMS (basenames,
    # relative to deps/archives) before anything is copied.
    $sumsFile = Get-CachePath 'deps/SHA256SUMS'
    $expectedSums = @{}
    foreach ($raw in [IO.File]::ReadAllLines($sumsFile)) {
        $line = $raw.TrimEnd("`r")
        if ($line -eq '') { continue }
        if ($line -notmatch '^([0-9a-fA-F]{64}) [ *](.+)$') { throw "Unparseable line in deps/SHA256SUMS: $line" }
        $expectedSums[[IO.Path]::GetFileName($Matches[2].Replace('/', '\'))] = $Matches[1].ToUpperInvariant()
    }
    $seen = New-Object System.Collections.Generic.HashSet[string]
    foreach ($a in $script:Inventory.Archives) {
        $name = [IO.Path]::GetFileName($a.Full)
        if (-not $expectedSums.ContainsKey($name)) { throw "deps/SHA256SUMS has no entry for archive $name" }
        $actual = Get-Sha256 $a.Full
        if ($actual -ine $expectedSums[$name]) { throw "Archive $name hash $actual does not match deps/SHA256SUMS ($($expectedSums[$name]))" }
        [void]$seen.Add($name)
    }
    foreach ($name in $expectedSums.Keys) {
        if (-not $seen.Contains($name)) { throw "deps/SHA256SUMS lists $name but the cache has no such archive" }
    }
    Write-Info "archives verified against deps/SHA256SUMS: $($seen.Count) ok"

    Remove-StageFolder $script:SourcesRoot
    [void][IO.Directory]::CreateDirectory($script:SourcesRoot)
    foreach ($a in $script:Inventory.Archives) {
        Copy-FileTo $a.Full (Join-Path $script:SourcesRoot ($a.Relative.Replace('/', '\')))
    }
    foreach ($p in $script:Inventory.DepsFixed) {
        if ($p.Relative -eq 'deps/@LoBo_Deps/POSSIBLY_UNNEEDED.txt') { continue }
        Copy-FileTo $p.Full (Join-Path $script:SourcesRoot ($p.Relative.Replace('/', '\')))
    }
    $origDir = $null
    foreach ($d in [IO.Directory]::GetDirectories($script:LoBoDirFull)) {
        if ([IO.Path]::GetFileName($d) -ieq '_ud-orig') { $origDir = $d; break }
    }
    if (-not $origDir) { throw "No _ud-orig folder under $($script:LoBoDirFull); the stock originals are required for -IncludeSources" }
    $stockOut = Join-Path $script:SourcesRoot 'stock-originals'
    $stockNames = New-Object System.Collections.Generic.List[string]
    $stockNote = New-Object System.Collections.Generic.List[string]
    $stockNote.AddRange([string[]]@(
        'stock-originals: the @LoBo pbos exactly as the Lost Brothers released them',
        '==========================================================================',
        'These are the UNREPAIRED files that the mod doctor backed up to',
        '@LoBo\_ud-orig before rewriting the copies in @LoBo\addons. They are',
        'kept for provenance only. Do not put them into a game folder: they carry',
        'the abstract-class and buried-wreck defects described in',
        '@LoBo\UD-CHANGES.txt of the family package.',
        ''
    ))
    foreach ($f in (Get-TreeEntries -Root $origDir).Files) {
        Copy-FileTo $f.Full (Join-Path $stockOut $f.Relative)
        $stockNames.Add($f.Relative)
        $stockNote.Add(('   {0}  {1}' -f (Get-Sha256 $f.Full).ToLowerInvariant(), $f.Relative.Replace('\', '/')))
    }
    if ($stockNames.Count -eq 0) { throw "$origDir is empty" }
    Write-TextFileCrlf (Join-Path $stockOut 'NOTE.txt') $stockNote.ToArray()
    Copy-FileTo $script:AssemblyNotes (Join-Path $script:SourcesRoot 'lobo-modfolder-assembly.txt')
    $installerCount = @($script:Inventory.Archives | Where-Object { [IO.Path]::GetFileName($_.Full) -like '*.exe' }).Count
    Write-TextFileCrlf (Join-Path $script:SourcesRoot 'NOTE.txt') (Get-SourcesNoteLines -ArchiveCount $script:Inventory.Archives.Count -InstallerCount $installerCount -StockNames $stockNames.ToArray())
    $srcSums = Write-ChecksumFile $script:SourcesRoot
    Write-Info "staged $($script:SourcesRoot): $($script:Inventory.Archives.Count) archives, $($stockNames.Count) stock originals, SHA256SUMS.txt ($srcSums entries)"
    if (-not $NoArchive) {
        New-SevenZipArchive -StageFolder $script:SourcesRoot -ArchivePath $script:SourcesArchivePath
    }
}

Write-Host ''
Write-Host "Done: $($script:PackageName)" -ForegroundColor Green
exit 0
