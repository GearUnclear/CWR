<#
.SYNOPSIS
    LoBo Reloaded library builder: a clean, offline package of the Lost
    Brothers (@LoBo) mod with every Uslu dur! repair applied, for private use
    on the official Remastered exe and on Uslu dur! (and the legacy 1.99 exe).

.DESCRIPTION
    The assembled @LoBo on this machine is a working tree: it carries a nested
    git repo, the mod doctor's _ud-orig backups, the content repair's
    .ud-backups, build-machine scripts and a Bin\config.cpp that only the
    Uslu dur! (UD) engine may load. It also carries the content repair in its
    UD profile, whose wreck-fire scripts use a command the legacy 1.99 engine
    does not have. This script turns it into something a relative can drop
    into a game folder:

        <OutDir>\stage\LoBo-Reloaded-<Version>\
          README-FIRST.txt   what it is, install and launch lines, verification
          SHA256SUMS.txt     sha256 of every other file (sha256sum -c format)
          @LoBo\             allowlisted copy of the mod, the 73 Easting pack
                             merged in, the content repair switched to its
                             Classic profile, UD-CHANGES.txt describing it all
          @LoBo_Deps\        the third-party addons LoBo depends on, with
                             their readmes and licences
          @LoBo_Extras\      only with -ExtrasDir: optional extra addon packs,
                             copied verbatim from an already-built folder.
                             Mounted FIRST so core always wins a name clash.
          @LoBo_UD\          Uslu dur! only: bin\config.cpp (the Guerrilla
                             faction table) plus addons\ holding the UD
                             profile versions of exactly the pbos the Classic
                             switch rewrote. Mounted LAST so its pbos win.
        <OutDir>\LoBo-Reloaded-<Version>.7z   (+ .sha256 sidecar)
        <OutDir>\reports\<Version>\           tool output and build-report.txt
                                              (never packaged)
        <OutDir>\_work\<Version>\             the classic apply's backup batch
                                              and the composite check tree

    With -LibraryDir the finished build is also filed into the LoBo Reloaded
    library:
        <LibraryDir>\modfolders\<Version>\    the staged package folder, verbatim
                                              (modfolders, README-FIRST.txt,
                                              SHA256SUMS.txt), re-verified
        <LibraryDir>\dist\                    the .7z archive(s) and .sha256 sidecars
        <LibraryDir>\docs\build-<Version>.txt the build report
    An existing version folder, archive or build report is never overwritten
    without -Force, and nothing else in the library is touched.

    Two repair sets are already applied in place in the source @LoBo and are
    verified, never re-applied, there:
      A. the mod doctor (PoseidonTools mod doctor --fix): undefined scope
         keyword in LoBoWreck/LoBoPalObj, buried M60A1 wreck origins,
         malformed tracerColor floats. Checked with a report-only run that
         must say "Nothing to do".
         The dependency set and the extras are not repaired at the source:
         the doctor runs with --fix on their STAGED copies only (never on
         the cache, <ExtrasDir> or the library), because the official
         Remastered exe reads a malformed float such as IRAR_EE9's tracerColor
         0.0.1 as a script expression ("Script error at '0.0|#|.1'", fatal
         under --autotest). Its _ud-orig backups leave the package for
         <OutDir>\_work\<Version>\doctor-orig, what it patched goes into
         UD-CHANGES.txt and the build report, and before SHA256SUMS.txt is
         written a report-only run must say "Nothing to do" for every
         staged modfolder.
      B. the issue-69 content repair (repair_lobo.py), applied with
         --compatibility ud. Checked with "--compatibility ud --check".
    The staged copy of @LoBo is then switched to the Classic profile with
    "repair_lobo.py --compatibility classic --apply" (the source is never the
    apply target; the script asserts the path before every apply). Every pbo
    whose bytes changed is copied from the source, byte for byte, into
    @LoBo_UD\addons so Uslu dur! gets the full profile back.

    The source addons are hashed before and after the preflight UD check, so
    the bytes that passed it are pinned. The staged copy, the @LoBo_UD overlay
    and the final package are all checked against those preflight hashes (a
    source that changes under the build fails it), every repair_lobo.py run
    must report the same repair_version and rule sha256 as that preflight
    check, and a composite of the staged Classic addons with the overlay laid
    over them (what Uslu dur! will load, built from hard links under
    <OutDir>\_work) must pass "--compatibility ud --check" as well.

    Why the split: the engine walks the --mod list from the LAST entry
    backwards and skips a pbo name it has already loaded, so the last-listed
    modfolder wins a name clash. UD also layers every mounted mod's
    bin\config onto the base config, while the official 3.05 Remastered
    PoseidonGame.exe and the legacy ColdWarAssault.exe make a mod's bin\config
    REPLACE the base config, which breaks the game. So config.cpp and the
    UD-profile pbos live in @LoBo_UD, which is mounted last on UD and never
    on the other two engines. For the same reason @LoBo_Extras is mounted
    FIRST, and the build refuses an extras folder that carries a Bin folder
    or any pbo file name or CfgPatches class name that @LoBo, @LoBo_Deps or
    @LoBo_UD already carry (checked with lobo_cfgpatches.py next to this
    script, which reads each addon's config the way the engine does). Going
    first only settles pbo-name clashes: an extras config usually merges
    AFTER core and would overwrite any class it shares, so after staging
    "build-lobo-extras.py --check" also checks every extras config class
    against the staged @LoBo, @LoBo_Deps and @LoBo_UD and the stock game
    config (-StockGameDir), and refuses a changed value, a rebase or a config
    the engine parser cannot read in full.

    UD-CHANGES.txt lists every applied content repair batch from the backup
    manifests under <LoBoDir>\.ud-backups\issue-69\*\manifest.json, and the
    KNOWN ISSUES lists of README-FIRST.txt and UD-CHANGES.txt come from
    -KnownIssuesFile when it is given.

    The 73 Easting release and the dependency set live on the server
    (ssh host alias <Server>, path <ServerPath>). The server's copy of the mod
    proper is OLDER than the local one, so only those extras are fetched,
    into <CacheDir>, verified against one sha256sum listing taken on the
    server, and reused on later runs when their hashes still match.

    The script never modifies <LoBoDir>, <ExtraDir>, <ExtrasDir>, the server,
    or anything outside <OutDir> and the three library targets above. It
    deletes only its own folders under <OutDir>\stage and <OutDir>\_work,
    partial downloads under <CacheDir>, and (with -LibraryDir) its own
    modfolders\<Version> folder and that folder's .partial/.old siblings.
    Relative path parameters resolve against the PowerShell location
    (Set-Location), and <CacheDir> may not lie on, under or above <LoBoDir>,
    <ExtraDir> or <ExtrasDir>, nor under <OutDir>\stage or <OutDir>\_work.

    Generated .txt files are plain ASCII with CRLF line endings, except
    SHA256SUMS.txt and the .sha256 sidecar, which use LF on purpose: GNU
    sha256sum -c treats a trailing CR as part of the file name.

.PARAMETER LoBoDir
    The assembled @LoBo mod folder. Default: D:\Arma_CWA\@LoBo

.PARAMETER OutDir
    Where the staging folder, the cache, the reports and the archives go.
    Default: D:\LoBo-Family
    Must lie outside -LibraryDir, and outside the @LoBo folder, -ExtraDir,
    -ExtrasDir and -CacheDir: "mod doctor --fix" refuses a staged modfolder
    on, under or above any build input (Assert-DoctorFixTarget), and that
    refusal comes only after @LoBo is staged, so an OutDir such as
    <LibraryDir>\_work\... wastes the run. The 2026-10-12 library build used
    an OutDir under D:\Arma_CWA\tmp.

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
    path. Default: <OutDir>\_server-cache. Refused when it is on, under or
    above the @LoBo folder or ExtraDir, or under <OutDir>\stage or
    <OutDir>\_work.

.PARAMETER Tools
    PoseidonTools executable (mod doctor, the pbo reader repair_lobo.py and
    lobo_cfgpatches.py use for compressed members, and config debin).
    Default: arma_CWR\dist\x64-win-rwdi\PoseidonTools.exe of the checkout
    this script is in; when that checkout is a linked git worktree without a
    build, the same path in the main checkout.

.PARAMETER RepairTool
    Path to repair_lobo.py. Default: repair_lobo.py next to this script
    (arma_CWR\tools\lobo), in a linked worktree as in the main checkout; if
    it is missing the script stops and asks for -RepairTool.

.PARAMETER Python
    Python 3 interpreter for repair_lobo.py. Default: python (from PATH).

.PARAMETER SkipFetch
    Do not contact the server; build from the cache, which must already hold
    everything the build needs.

.PARAMETER NoArchive
    Stop after staging and verification; write no .7z.

.PARAMETER IncludeSources
    Also build LoBo-Reloaded-Sources-<Version>.7z with the original dependency
    downloads (verified against deps/SHA256SUMS), the server's scan log and
    manifest, the stock originals from @LoBo\_ud-orig, the pre-repair
    originals from @LoBo\.ud-backups\issue-69 and the assembly notes. The
    preflight checks that _ud-orig and an issue-69 batch exist; the
    deps/SHA256SUMS cross-check runs in preflight with -SkipFetch and right
    after the fetch otherwise, in both cases before anything is staged.

.PARAMETER ExtraDir
    Optional folder mirroring the @LoBo layout (addons\, Missions\, docs\ ...)
    merged into the staged @LoBo before the Classic switch, with the same
    collision rules as the 73 Easting files. It may not carry a Bin folder.
    (Not to be confused with -ExtrasDir, which adds a separate modfolder.)

.PARAMETER ExtrasDir
    Optional, already-built @LoBo_Extras folder (any folder name), included
    as a fourth modfolder, @LoBo_Extras, copied verbatim (then patched by
    "mod doctor --fix" where the doctor finds a defect) and covered by
    SHA256SUMS.txt. Refused when it carries a top-level Bin folder, links,
    runtime junk or git/backup folders, or when any of its pbo file names
    (any subfolder) or CfgPatches class names matches one in @LoBo,
    @LoBo_Deps or @LoBo_UD, or when, after staging, one of its config classes
    would change a class those folders or the stock game define (see
    -StockGameDir). Its files are hashed in preflight and must not change
    during the build. Its docs\BUILD-MANIFEST.json (written by
    build-lobo-extras.py) is shown in the plan and the build report; a
    folder without one, or built without its own class check, gets a
    warning.

.PARAMETER StockGameDir
    Game folder whose BIN\CONFIG.BIN and AddOns are the stock config, read by
    the @LoBo_Extras class check (its access locks decide which stub rebases
    the engine refuses). Used only with -ExtrasDir, and then required.
    Default: D:\SteamLibrary\steamapps\common\ARMA Cold War Assault\Remastered

.PARAMETER LibraryDir
    Existing LoBo Reloaded library root (for example D:\LoBo-Reloaded).
    After a successful build the package is filed into
    modfolders\<Version>\, dist\ and docs\build-<Version>.txt there (see
    .DESCRIPTION). Needs the archive, so it cannot be combined with
    -NoArchive. Existing targets are refused without -Force. -OutDir must
    not lie inside it (see -OutDir).

.PARAMETER KnownIssuesFile
    Plain ASCII text file holding the KNOWN ISSUES list for README-FIRST.txt
    and @LoBo\UD-CHANGES.txt, inserted line by line (lines starting with #
    are comments; no em dashes, no ' -- '). When the parameter is not given,
    or the file is absent (a warning), the built-in list is used.

.PARAMETER PlanOnly
    Run every preflight check (all read-only), print the plan and exit
    without writing anything. Unless -SkipFetch is given this still asks the
    server for its file listing (read-only) so the fetch plan can be shown.
    With -ExtrasDir the CfgPatches cross-check runs here on the source side
    (debin uses a temporary file in the system temp folder, removed again);
    the @LoBo_Extras class check needs the staged folders, so it runs only
    in a real build.

.PARAMETER SevenZip
    Path to 7z.exe. Default: C:\Program Files\7-Zip\7z.exe

.PARAMETER AllowUnknown
    Warn and skip unknown top-level items in @LoBo (and ExtraDir) instead of
    failing.

.PARAMETER Force
    Overwrite an existing archive of the same Version, and with -LibraryDir
    the library's modfolders\<Version> folder, archives and build report.

.EXAMPLE
    .\package-lobo-family.ps1 -PlanOnly
    Preflight against the real @LoBo, list what the server would send, write nothing.

.EXAMPLE
    .\package-lobo-family.ps1 -IncludeSources
    Full build: fetch, stage, Classic switch, verify, LoBo-Reloaded-<date>.7z and LoBo-Reloaded-Sources-<date>.7z.

.EXAMPLE
    .\package-lobo-family.ps1 -SkipFetch -OutDir D:\LoBo-Family -ExtrasDir D:\LoBo-Reloaded\modfolders\@LoBo_Extras -LibraryDir D:\LoBo-Reloaded -KnownIssuesFile D:\LoBo-Reloaded\docs\known-issues.txt
    Build from the cache with the optional extras (built by build-lobo-extras.py)
    and file the result into the library as modfolders\<date>, dist\ and docs\.
    The OutDir must stay outside the library (see -OutDir).

.NOTES
    Windows PowerShell 5.1 compatible. Dot-sourcing the script (". .\package-lobo-family.ps1")
    only defines its functions and constants and returns before the preflight, so
    a test harness can exercise single functions such as Assert-ApplyTarget.
#>
param(
    [string]$LoBoDir = 'D:\Arma_CWA\@LoBo',
    [string]$OutDir = 'D:\LoBo-Family',
    [string]$Version = (Get-Date).ToString('yyyy-MM-dd'),
    [string]$Server = 'mega_nc',
    [string]$ServerPath = '/root/mobile-dev/mod_@lobo',
    [string]$CacheDir = '',
    [string]$Tools = '',
    [string]$RepairTool = '',
    [string]$Python = 'python',
    [switch]$SkipFetch,
    [switch]$NoArchive,
    [switch]$IncludeSources,
    [string]$ExtraDir = '',
    [string]$ExtrasDir = '',
    [string]$LibraryDir = '',
    [string]$KnownIssuesFile = '',
    [string]$StockGameDir = '',
    [switch]$PlanOnly,
    [string]$SevenZip = 'C:\Program Files\7-Zip\7z.exe',
    [switch]$AllowUnknown,
    [switch]$Force
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------
$script:ProductName  = 'LoBo Reloaded'
$script:PackageName  = "LoBo-Reloaded-$Version"
$script:SourcesName  = "LoBo-Reloaded-Sources-$Version"
$script:Today        = (Get-Date).ToString('yyyy-MM-dd')
$script:Banner       = '// GENERATED by arma_CWR/tools/lobo/install-lobo-factions.ps1'
$script:DoctorOkPattern = '(?m)^Nothing to do: no known defect found in (\d+) pbo\(s\)'
$script:ExtrasName   = '@LoBo_Extras'
# What "mod doctor --fix" patched in the staged @LoBo_Deps and @LoBo_Extras
# (Invoke-StagedDoctorFix records), and the final report-only gate results.
$script:DoctorFixes  = New-Object System.Collections.Generic.List[object]
$script:DoctorGate   = @()
$script:ExtrasDoctorHashes = @{}
$script:CfgPatchesHelper = Join-Path $PSScriptRoot 'lobo_cfgpatches.py'
$script:ExtrasBuilder = Join-Path $PSScriptRoot 'build-lobo-extras.py'
# The stock game config the @LoBo_Extras class check reads (-StockGameDir).
$script:DefaultStockGameDir = 'D:\SteamLibrary\steamapps\common\ARMA Cold War Assault\Remastered'

# The built-in KNOWN ISSUES list of README-FIRST.txt, used when no
# -KnownIssuesFile is given or the file is absent. HasShilka: the package
# ships Vit_Iraq_Shilka.pbo in @LoBo_Extras.
function Get-DefaultKnownIssues([bool]$HasShilka = $false) {
    $lines = @(
        '- Runtime testing of the content repair (repair set B in',
        '  UD-CHANGES.txt) is partial: see the lobo tracking repo for what was',
        '  checked in game. Multiplayer, saves and long range AI fire are',
        '  untested.',
        '- The official Remastered exe itself has not been run with this',
        '  package; it is expected to load it, but that is unverified.',
        '- On the legacy 1.99 exe and on the official Remastered exe the',
        '  burning wreck scripts run in a limited form: they only reach the',
        '  player''s and the wreck''s own groups, not every nearby soldier.',
        '  Uslu dur! with @LoBo_UD gets the full version.',
        '- The sinai, iran_iraq and gaza islands point their menu intro at an',
        '  addon (lost_anims) that the mod team never released. Uslu dur! works',
        '  around it; the legacy 1.99 exe and the official Remastered exe may',
        '  complain about the missing addon when those islands load their',
        '  intro. Harmless.',
        '- Two referenced addons (f3wx_markers, bde_weapons) were never',
        '  publicly released. The references are harmless. A third one,',
        '  vit_iraq_shilka, was recovered in 2026; LoBo does not need it.'
    )
    if ($HasShilka) { $lines += '  It ships in @LoBo_Extras as the optional "Shilka Iraq" vehicle.' }
    return $lines
}
# Folder or file names that must not appear anywhere in an -ExtrasDir.
$script:ForbiddenInExtras = @('.git', '.gitattributes', '.gitignore', '.ud-backups', '_ud-orig', '_scratch')

# Stock (unrepaired) bytes of the two pbos the scope repair rewrote. These
# must never ship, in any folder of the package.
$script:StockHashes = @{
    'LoBoWreck.pbo'  = '2149FEBC43FD378B7AA361D8AFE4D63738F11239688B5C7C46832715DAF1ABB2'
    'LoBoPalObj.pbo' = 'F16A43BE9D538C50A61EA0E7DE8A98D27759F7639AE2FC5F8924E9E1CCE666A2'
}


# The archives the UD -> Classic profile switch is expected to rewrite (16
# wreck-fire members in these 14 pbos). An archive rewritten that is not in
# this list is fatal (the overlay would carry something this script was not
# written for); an expected archive left unchanged is a warning.
$script:ExpectedClassicSwitch = @(
    'LoBo_airammo.pbo', 'LoBo_BTR60.pbo', 'LoBo_FAB.pbo', 'LoBo_Zelda.pbo', 'LoBo_ZU23.pbo',
    'LoBoCen.pbo', 'LoBoEgAPC.pbo', 'LoBoF15.pbo', 'LoBoF16.pbo', 'LoBoM60.pbo',
    'LoBoMerkava2.pbo', 'LoBoMerkava4.pbo', 'LoBoSA6.pbo', 'LoBoTer.pbo'
)

# Top-level items of @LoBo. Anything not listed here or in SkipReasons is an
# error unless -AllowUnknown.
$script:AllowedDirs  = @('addons', 'Bin', 'dta', 'Missions', 'user missions', 'docs')
$script:AllowedFiles = @('README.txt', 'The_Lost_Brothers_Official_Readme2.doc')
$script:SkipReasons  = @{
    '.git'           = 'local git bookkeeping'
    '.gitattributes' = 'local git bookkeeping'
    '.gitignore'     = 'local git bookkeeping'
    'tools'          = 'build-machine scripts, tracked in arma_CWR/tools/lobo'
    '_ud-orig'       = 'stock originals of the doctor-repaired pbos (Sources archive only)'
    '.ud-backups'    = 'repair and installer backups (issue-69 batch goes to the Sources archive only)'
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
function Write-ToolOutput([string]$Text, [string]$Prefix = '      | ') {
    foreach ($raw in ($Text -split "`n")) {
        $line = $raw.TrimEnd("`r")
        if ($line -ne '') { Write-Host ($Prefix + $line) -ForegroundColor DarkGray }
    }
}

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

# Absolute, normalised form of a path parameter. A relative path resolves
# against the PowerShell location (what Set-Location changed), not against
# the process working directory, which Windows PowerShell 5.1 leaves where
# the process started; [IO.Path]::GetFullPath alone would use the latter.
function Resolve-FullPath([string]$Path) {
    if ($Path -eq '') { throw 'Internal: empty path' }
    $resolved = $null
    try { $resolved = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($Path) } catch { $resolved = $null }
    if ($null -eq $resolved -or $resolved -eq '' -or $resolved -match '::' -or -not [IO.Path]::IsPathRooted($resolved)) {
        $base = (Get-Location -PSProvider FileSystem).ProviderPath
        $resolved = Join-Path $base $Path
    }
    return [IO.Path]::GetFullPath($resolved)
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

# True when Child is Parent itself or anything below it.
function Test-PathUnder([string]$Parent, [string]$Child) {
    $p = [IO.Path]::GetFullPath($Parent).TrimEnd('\', '/') + '\'
    $c = [IO.Path]::GetFullPath($Child).TrimEnd('\', '/') + '\'
    return $c.StartsWith($p, [StringComparison]::OrdinalIgnoreCase)
}

# Case-insensitive lookup of a direct child (file or folder) by name.
function Find-ChildPath([string]$Dir, [string]$Name, [switch]$Directory) {
    if (-not [IO.Directory]::Exists($Dir)) { return $null }
    $items = if ($Directory) { [IO.Directory]::GetDirectories($Dir) } else { [IO.Directory]::GetFiles($Dir) }
    foreach ($i in $items) {
        if ([IO.Path]::GetFileName($i) -ieq $Name) { return $i }
    }
    return $null
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
    # Many stock pbos are read-only (they came off a CD image) and _ud-orig
    # keeps its originals read-only. The staged copy must be writable: the
    # Classic switch replaces files in place, and the family copy should not
    # inherit the flag either.
    $attr = [IO.File]::GetAttributes($Dest)
    if ($attr -band [IO.FileAttributes]::ReadOnly) {
        [IO.File]::SetAttributes($Dest, ($attr -bxor [IO.FileAttributes]::ReadOnly))
    }
}

# A hard link to Target at Link (no bytes copied, same volume, NTFS), or a
# plain copy when the file system refuses. Returns 'link' or 'copy'.
function New-HardLinkOrCopy([string]$Target, [string]$Link) {
    [void][IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($Link))
    try {
        [void](New-Item -ItemType HardLink -Path $Link -Value $Target -ErrorAction Stop)
        return 'link'
    } catch {
        if (Test-Path -LiteralPath $Link) { Remove-Item -LiteralPath $Link -Force }
        Copy-FileTo $Target $Link
        return 'copy'
    }
}

function Copy-TreeEntries([object]$Entries, [string]$SourceRoot, [string]$DestRoot) {
    [void][IO.Directory]::CreateDirectory($DestRoot)
    foreach ($d in $Entries.Dirs) { [void][IO.Directory]::CreateDirectory((Join-Path $DestRoot $d)) }
    foreach ($f in $Entries.Files) { Copy-FileTo $f.Full (Join-Path $DestRoot $f.Relative) }
}

# Replaces every non-ASCII character with '?', for values (paths, strings
# from manifests) that go into the ASCII-only generated text files.
function ConvertTo-AsciiText([string]$Text) {
    if ($null -eq $Text) { return '' }
    $sb = New-Object System.Text.StringBuilder
    foreach ($ch in $Text.ToCharArray()) {
        if ([int]$ch -gt 127) { [void]$sb.Append('?') } else { [void]$sb.Append($ch) }
    }
    return $sb.ToString()
}

# ---------------------------------------------------------------------------
# Default tool locations
# ---------------------------------------------------------------------------
# The root of the MAIN checkout of the git repository that contains
# StartDir: StartDir's own checkout when that is the main one (".git" is a
# folder), or the checkout a linked worktree belongs to (".git" is a file
# "gitdir: <main>\.git\worktrees\<name>", whose commondir names <main>\.git).
# $null when StartDir is not in a git checkout or the layout is unexpected.
function Get-MainCheckoutRoot([string]$StartDir) {
    $dir = [IO.Path]::GetFullPath($StartDir).TrimEnd('\', '/')
    while ($dir -ne '') {
        $dotGit = Join-Path $dir '.git'
        if ([IO.Directory]::Exists($dotGit)) { return $dir }
        if ([IO.File]::Exists($dotGit)) {
            $gitDir = $null
            foreach ($line in [IO.File]::ReadAllLines($dotGit)) {
                if ($line -match '^gitdir:\s*(.+?)\s*$') { $gitDir = $Matches[1]; break }
            }
            if (-not $gitDir) { return $null }
            if (-not [IO.Path]::IsPathRooted($gitDir)) { $gitDir = Join-Path $dir $gitDir }
            $common = $gitDir
            $commonFile = Join-Path $gitDir 'commondir'
            if ([IO.File]::Exists($commonFile)) {
                $c = ([IO.File]::ReadAllText($commonFile)).Trim()
                if ($c -ne '') { $common = $(if ([IO.Path]::IsPathRooted($c)) { $c } else { Join-Path $gitDir $c }) }
            }
            $common = [IO.Path]::GetFullPath($common).TrimEnd('\', '/')
            if ([IO.Path]::GetFileName($common) -ine '.git') { return $null }
            return [IO.Path]::GetDirectoryName($common)
        }
        $parent = [IO.Path]::GetDirectoryName($dir)
        if ($null -eq $parent -or $parent -eq $dir) { break }
        $dir = $parent.TrimEnd('\', '/')
    }
    return $null
}

# repair_lobo.py lives next to this script (arma_CWR\tools\lobo) on main, so
# the copy beside the running script is the one that matches it, in a linked
# worktree as in the main checkout.
function Get-DefaultRepairToolPath([string]$ScriptDir) {
    return [IO.Path]::GetFullPath((Join-Path $ScriptDir 'repair_lobo.py'))
}

# PoseidonTools of this checkout's build; a linked worktree usually has no
# build of its own, so fall back to the main checkout's. Returns the first
# candidate when neither exists, so the error message names it.
function Get-DefaultToolsPath([string]$ScriptDir) {
    $rel = 'dist\x64-win-rwdi\PoseidonTools.exe'
    $own = [IO.Path]::GetFullPath((Join-Path (Join-Path $ScriptDir '..\..') $rel))
    if ([IO.File]::Exists($own)) { return $own }
    $main = Get-MainCheckoutRoot $ScriptDir
    if ($main) {
        $other = [IO.Path]::GetFullPath((Join-Path (Join-Path $main 'arma_CWR') $rel))
        if ([IO.File]::Exists($other)) { return $other }
    }
    return $own
}

# Merges one file into a staged tree. A file of the same name (any case)
# already there must be byte-identical (then the merge is skipped) or the
# build fails. Returns 'added' or 'identical'.
function Merge-FileInto {
    param([string]$SourceFile, [string]$DestRoot, [string]$Relative, [string]$Origin)
    $dest    = Join-Path $DestRoot $Relative
    $destDir = [IO.Path]::GetDirectoryName($dest)
    $leaf    = [IO.Path]::GetFileName($dest)
    $existing = Find-ChildPath $destDir $leaf
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
# em dashes (non-ASCII) and no ' -- ' used as a dash.
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

# Raw tool output saved under reports\: line endings normalised to CRLF,
# content otherwise untouched.
function Write-ReportText([string]$Path, [string]$Text) {
    $lines = @(($Text -replace "`r`n", "`n") -split "`n")
    [void][IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($Path))
    [IO.File]::WriteAllText($Path, (($lines -join "`r`n").TrimEnd() + "`r`n"), (New-Object System.Text.UTF8Encoding($false)))
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

# Deletes a folder the script owns: something strictly below <OutDir>\stage
# or <OutDir>\_work. Anything else is refused.
function Remove-OwnedFolder([string]$Path) {
    $owned = $false
    foreach ($base in @($script:StageBase, $script:WorkBase)) {
        if ((Test-PathUnder $base $Path) -and
            ([IO.Path]::GetFullPath($Path).TrimEnd('\') -ine [IO.Path]::GetFullPath($base).TrimEnd('\'))) { $owned = $true }
    }
    if (-not $owned) { throw "Refusing to delete outside $($script:StageBase) and $($script:WorkBase)`: $Path" }
    if (-not (Test-Path -LiteralPath $Path)) { return }
    if (Test-ReparsePoint $Path) { throw "Refusing to delete a link/junction: $Path" }
    Write-Info "removing $Path"
    Remove-Item -LiteralPath $Path -Recurse -Force
}

# Hashes of the top-level *.pbo files of an addons folder: name -> SHA-256
# (PowerShell hashtable keys compare case-insensitively).
function Get-AddonPboHashes([string]$AddonsDir) {
    $map = @{}
    foreach ($f in [IO.Directory]::GetFiles($AddonsDir)) {
        $n = [IO.Path]::GetFileName($f)
        if ($n -like '*.pbo' -and -not (Test-JunkName $n)) { $map[$n] = Get-Sha256 $f }
    }
    return $map
}

# ---------------------------------------------------------------------------
# Native processes
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

# Runs a native program with stdout and stderr captured. Returns ExitCode,
# StdOut, StdErr. Nothing is thrown on a non-zero exit; callers decide.
function Invoke-Capture {
    param([Parameter(Mandatory)][string]$FilePath, [string[]]$Arguments = @())
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $FilePath
    $psi.Arguments = ConvertTo-WindowsCommandLine $Arguments
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.RedirectStandardInput = $false
    $psi.CreateNoWindow = $true
    $p = [System.Diagnostics.Process]::Start($psi)
    try {
        $errTask = $p.StandardError.ReadToEndAsync()
        $out = $p.StandardOutput.ReadToEnd()
        $p.WaitForExit()
        $err = $errTask.Result
        return [pscustomobject]@{ ExitCode = $p.ExitCode; StdOut = $out; StdErr = $err }
    } finally {
        $p.Dispose()
    }
}

function Get-JsonField([object]$Object, [string]$Name) {
    if ($null -eq $Object) { return $null }
    $prop = $Object.PSObject.Properties[$Name]
    if ($null -eq $prop) { return $null }
    return $prop.Value
}

# Report-only mod doctor run. Succeeds only on exit 0 with the "Nothing to
# do" line. Exit 1 means defects were found (run the doctor --fix on the
# source first), exit 2 means I/O errors. Returns the scanned pbo count.
function Invoke-ModDoctor {
    param([Parameter(Mandatory)][string]$ModDir, [Parameter(Mandatory)][string]$Label, [string]$SaveTo = '')
    Write-Info "mod doctor (report only) on $Label`: $ModDir"
    $r = Invoke-Capture -FilePath $script:ToolsFull -Arguments @('mod', 'doctor', $ModDir)
    Write-ToolOutput $r.StdOut
    Write-ToolOutput $r.StdErr '      ! '
    $script:LastDoctorText = "exit code $($r.ExitCode)`n--- stdout ---`n" + $r.StdOut + "`n--- stderr ---`n" + $r.StdErr
    if ($SaveTo -ne '') { Write-ReportText $SaveTo $script:LastDoctorText }
    if ($r.ExitCode -ne 0) {
        $hint = if ($r.ExitCode -eq 1) { 'defects found; this means the mod doctor repair (set A) is missing or incomplete on the source' } else { 'I/O error' }
        throw "mod doctor on $Label exited $($r.ExitCode) ($hint)"
    }
    if ($r.StdOut -notmatch $script:DoctorOkPattern) {
        throw "mod doctor on $Label exited 0 but did not print the 'Nothing to do: no known defect found' line"
    }
    $count = [int]$Matches[1]
    Write-Info "mod doctor: nothing to do in $count pbo(s) ($Label)"
    return $count
}

# Parses the stdout of "mod doctor --fix" into one record per pbo with at
# least one patchable finding: Pbo, Findings (the defect lines, e.g.
# "MALFORMED_FLOAT  config.cpp:55  '0.0.1' -> '0.000'", children such as the
# UNDEFINED_SCOPE_KEYWORD sites of a MISSING_DEFINE_HEADER prefixed with
# "  ") and Sites (from the "patched N site(s) in place" line, 0 when it
# patched nothing). NOT_PATCHABLE lines (compressed or binarised configs the
# doctor cannot read) are notes, not findings, and are left out.
function ConvertFrom-DoctorFixOutput([string]$Text) {
    $records = New-Object System.Collections.Generic.List[object]
    $current = $null
    foreach ($raw in ($Text -split "`n")) {
        $line = $raw.TrimEnd("`r")
        if ($line -match '^(\S.*\.pbo):$') {
            $current = [pscustomobject]@{ Pbo = $Matches[1]; Findings = (New-Object System.Collections.Generic.List[string]); Sites = 0 }
            $records.Add($current)
            continue
        }
        if ($null -eq $current) { continue }
        if ($line -cmatch '^    ([A-Z][A-Z_]+)  (.+)$') {
            if ($Matches[1] -cne 'NOT_PATCHABLE') { $current.Findings.Add($Matches[1] + '  ' + $Matches[2].Trim()) }
            continue
        }
        if ($line -cmatch '^        ([A-Z][A-Z_]+)  (.+)$') {
            if ($Matches[1] -cne 'NOT_PATCHABLE') { $current.Findings.Add('  ' + $Matches[1] + '  ' + $Matches[2].Trim()) }
            continue
        }
        if ($line -match '^    patched (\d+) site\(s\) in place') { $current.Sites = [int]$Matches[1] }
    }
    return @($records | Where-Object { $_.Findings.Count -gt 0 })
}

# "mod doctor --fix" rewrites pbos in place, so it may only ever run on a
# modfolder of this build's staged package: strictly inside <StageRoot>, and
# never on, under or above the source @LoBo, ExtraDir, ExtrasDir, the cache
# or the library (whose files the staged copies came from).
function Assert-DoctorFixTarget([string]$Target) {
    $t = [IO.Path]::GetFullPath($Target).TrimEnd('\')
    if (-not (Test-PathUnder $script:StageRoot $t) -or ($t -ieq [IO.Path]::GetFullPath($script:StageRoot).TrimEnd('\'))) {
        throw "REFUSING to run mod doctor --fix outside the staged package ($($script:StageRoot)): $t"
    }
    foreach ($guard in @($script:LoBoDirFull, $script:ExtraDirFull, $script:ExtrasDirFull, $script:CacheDirFull, $script:LibraryDirFull)) {
        if ($null -eq $guard -or $guard -eq '') { continue }
        if ((Test-PathUnder $guard $t) -or (Test-PathUnder $t $guard)) {
            throw "REFUSING to run mod doctor --fix on or above a build input ($guard): $t"
        }
    }
    if (-not [IO.Directory]::Exists($t)) { throw "mod doctor --fix target does not exist: $t" }
}

# Runs "mod doctor --fix" on one STAGED modfolder (Assert-DoctorFixTarget),
# so the cache and the library sources keep their original bytes. The doctor
# keeps each original in <modfolder>\_ud-orig; that folder leaves the package
# at once, for <WorkRoot>\doctor-orig\<modfolder name>, after every backup was
# checked against the bytes staged before the fix. Exit 1 after --fix only
# means entries the doctor cannot read were noted; whether anything is left
# to do is the report-only gate's call (Assert-StagedDoctorClean). Returns
# one record per patched pbo: Folder, Pbo, Findings, Sites, OrigHash, NewHash.
function Invoke-StagedDoctorFix {
    param([Parameter(Mandatory)][string]$ModDir, [Parameter(Mandatory)][string]$Label)
    $mod = [IO.Path]::GetFullPath($ModDir).TrimEnd('\')
    Assert-DoctorFixTarget $mod
    $addons = Find-ChildPath $mod 'addons' -Directory
    if (-not $addons) { Write-Skip "mod doctor --fix: staged $Label has no addons folder"; return @() }
    if (Find-ChildPath $mod '_ud-orig' -Directory) { throw "Staged $Label already holds a _ud-orig folder before the mod doctor ran: $mod" }
    $before = Get-AddonPboHashes $addons
    Write-Info "mod doctor --fix on staged $Label`: $mod"
    $r = Invoke-Capture -FilePath $script:ToolsFull -Arguments @('mod', 'doctor', $mod, '--fix')
    Write-ToolOutput $r.StdOut
    Write-ToolOutput $r.StdErr '      ! '
    Write-ReportText (Join-Path $script:ReportDir ('doctor-fix-' + $Label.TrimStart('@') + '.txt')) ("exit code $($r.ExitCode)`n--- stdout ---`n" + $r.StdOut + "`n--- stderr ---`n" + $r.StdErr)
    if ($r.ExitCode -ne 0 -and $r.ExitCode -ne 1) { throw "mod doctor --fix on staged $Label exited $($r.ExitCode) (I/O error); see the doctor-fix report" }
    $after = Get-AddonPboHashes $addons
    $parsed = @(ConvertFrom-DoctorFixOutput $r.StdOut)

    # The pbos whose bytes changed must be exactly the ones the doctor says it patched.
    foreach ($n in $after.Keys) { if (-not $before.ContainsKey($n)) { throw "mod doctor --fix created $Label\addons\$n" } }
    foreach ($n in $before.Keys) { if (-not $after.ContainsKey($n)) { throw "mod doctor --fix removed $Label\addons\$n" } }
    $changed = @($before.Keys | Where-Object { $before[$_] -ine $after[$_] } | Sort-Object)
    foreach ($p in $parsed) {
        if ($p.Sites -le 0) { throw "mod doctor reported $($p.Findings.Count) finding(s) in $Label\addons\$($p.Pbo) but patched nothing" }
        if (-not $before.ContainsKey($p.Pbo)) { throw "mod doctor reported $($p.Pbo), which is not a top-level pbo of $Label\addons" }
        if ($before[$p.Pbo] -ieq $after[$p.Pbo]) { throw "mod doctor says it patched $Label\addons\$($p.Pbo), but its bytes did not change" }
    }
    foreach ($n in $changed) {
        if (-not ($parsed | Where-Object { $_.Pbo -ieq $n })) { throw "mod doctor --fix changed $Label\addons\$n without reporting a patch for it" }
    }

    # Backups: one per patched pbo, holding the bytes staged before the fix,
    # moved out of the package.
    $orig = Find-ChildPath $mod '_ud-orig' -Directory
    if ($parsed.Count -eq 0) {
        if ($orig) { throw "mod doctor --fix patched nothing in $Label but left a _ud-orig folder: $orig" }
        Write-Info "mod doctor --fix: nothing patched in staged $Label"
        return @()
    }
    if (-not $orig) { throw "mod doctor --fix patched $($parsed.Count) pbo(s) in $Label but kept no _ud-orig backups" }
    $origFiles = (Get-TreeEntries -Root $orig).Files
    if ($origFiles.Count -ne $parsed.Count) { throw "$Label\_ud-orig holds $($origFiles.Count) file(s), the doctor patched $($parsed.Count) pbo(s)" }
    foreach ($f in $origFiles) {
        $leaf = [IO.Path]::GetFileName($f.Relative)
        if (-not $before.ContainsKey($leaf) -or (Get-Sha256 $f.Full) -ine $before[$leaf]) { throw "$Label\_ud-orig\$($f.Relative) is not the pbo staged before the fix" }
    }
    $origOut = Join-Path (Join-Path $script:WorkRoot 'doctor-orig') $Label
    [void][IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($origOut))
    Remove-OwnedFolder $origOut
    Move-Item -LiteralPath $orig -Destination $origOut
    if (Find-ChildPath $mod '_ud-orig' -Directory) { throw "Internal: _ud-orig still present under the staged $Label" }
    Write-Info "moved the doctor's $($origFiles.Count) original(s) of staged $Label to $origOut"

    $records = foreach ($p in ($parsed | Sort-Object -Property Pbo)) {
        $name = ($before.Keys | Where-Object { $_ -ieq $p.Pbo } | Select-Object -First 1)
        Write-Info ('   patched {0}\{1}: {2} site(s)' -f $Label, (Get-RelativeUnder $mod (Join-Path $addons $name)), $p.Sites)
        [pscustomobject]@{
            Folder   = $Label
            Pbo      = $name
            Relative = (Get-RelativeUnder $mod (Join-Path $addons $name))
            Findings = $p.Findings.ToArray()
            Sites    = $p.Sites
            OrigHash = $before[$name]
            NewHash  = $after[$name]
        }
    }
    return @($records)
}

# The report-only gate after every fix: the doctor must print "Nothing to
# do" for every staged modfolder that has an addons folder. Returns one
# record per modfolder (Folder, Pbos), saved to reports\doctor-gate-*.txt.
function Assert-StagedDoctorClean([object[]]$Folders) {
    $results = New-Object System.Collections.Generic.List[object]
    foreach ($pair in $Folders) {
        $label = [string]$pair[0]
        $dir = [string]$pair[1]
        if (-not [IO.Directory]::Exists($dir)) { continue }
        if (-not (Find-ChildPath $dir 'addons' -Directory)) { Write-Skip "mod doctor gate: staged $label has no addons folder"; continue }
        if (Find-ChildPath $dir '_ud-orig' -Directory) { throw "Staged $label still holds a _ud-orig folder" }
        $count = Invoke-ModDoctor -ModDir $dir -Label "staged $label" -SaveTo (Join-Path $script:ReportDir ('doctor-gate-' + $label.TrimStart('@') + '.txt'))
        $results.Add([pscustomobject]@{ Folder = $label; Pbos = $count })
    }
    return $results.ToArray()
}

# The source @LoBo (and ExtraDir) must never be the target of --apply, and
# an apply target must lie inside the staging area.
function Assert-ApplyTarget([string]$Target) {
    $t = [IO.Path]::GetFullPath($Target)
    $src = $script:LoBoDirFull
    if ((Test-PathUnder $src $t) -or (Test-PathUnder $t $src)) {
        throw "REFUSING to run repair_lobo.py --apply on or above the source @LoBo ($src): $t"
    }
    if ($script:ExtraDirFull -ne '' -and ((Test-PathUnder $script:ExtraDirFull $t) -or (Test-PathUnder $t $script:ExtraDirFull))) {
        throw "REFUSING to run repair_lobo.py --apply on or above ExtraDir ($($script:ExtraDirFull)): $t"
    }
    if (-not (Test-PathUnder $script:StageBase $t) -or ($t.TrimEnd('\') -ieq $script:StageBase.TrimEnd('\'))) {
        throw "REFUSING to run repair_lobo.py --apply outside the staging area ($($script:StageBase)): $t"
    }
    if (-not [IO.Directory]::Exists($t)) { throw "apply target does not exist: $t" }
}

# Runs repair_lobo.py. Returns ExitCode, Result (the parsed JSON summary it
# prints, or $null), StdOut, StdErr. --check exits 1 when it would change
# data, 2 on incompatible input or I/O failure.
function Invoke-RepairTool {
    param(
        [Parameter(Mandatory)][string]$ModDir,
        [Parameter(Mandatory)][ValidateSet('ud', 'classic')][string]$Compatibility,
        [Parameter(Mandatory)][ValidateSet('check', 'apply')][string]$Mode,
        [string]$Report = '',
        [string]$Label = ''
    )
    $modFull = [IO.Path]::GetFullPath($ModDir)
    if ($Mode -eq 'apply') { Assert-ApplyTarget $modFull }
    $cmdArgs = @($script:RepairToolFull, '--mod', $modFull, '--compatibility', $Compatibility, "--$Mode", '--tools', $script:ToolsFull)
    if ($Report -ne '') {
        [void][IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($Report))
        $cmdArgs += @('--report', $Report)
    }
    Write-Info "repair_lobo.py --compatibility $Compatibility --$Mode on $Label`: $modFull"
    $r = Invoke-Capture -FilePath $script:PythonExe -Arguments $cmdArgs
    Write-ToolOutput $r.StdOut
    Write-ToolOutput $r.StdErr '      ! '
    $result = $null
    $trimmed = $r.StdOut.Trim()
    if ($trimmed.StartsWith('{')) {
        try { $result = $trimmed | ConvertFrom-Json } catch { $result = $null }
    }
    return [pscustomobject]@{ ExitCode = $r.ExitCode; Result = $result; StdOut = $r.StdOut; StdErr = $r.StdErr }
}

function Get-RepairIdentity([object]$Result, [string]$Label) {
    if ($null -eq $Result) { throw "repair_lobo.py printed no JSON summary ($Label)" }
    $ver = [string](Get-JsonField $Result 'repair_version')
    $sha = [string](Get-JsonField $Result 'rule_sha256')
    if ($ver -eq '' -or $sha -eq '') { throw "repair_lobo.py summary lacks repair_version/rule_sha256 ($Label)" }
    return [pscustomobject]@{ Version = $ver; RuleSha256 = $sha.ToLowerInvariant() }
}

# Records the identity the preflight UD check ran with. (Whether it matches
# the newest repair batch applied on the source is checked, as a warning,
# once the backup manifests are read; see step 5b.)
function Register-RepairIdentity([object]$Result, [string]$Label) {
    $id = Get-RepairIdentity $Result $Label
    $script:RepairVersion = $id.Version
    $script:RuleSha256 = $id.RuleSha256
}

# Every later repair_lobo.py run must report exactly the identity the
# preflight check registered: the rule file is another checkout's live
# working tree and must not change under the build.
function Assert-RepairIdentity([object]$Result, [string]$Label) {
    $id = Get-RepairIdentity $Result $Label
    if ($id.Version -ne $script:RepairVersion -or $id.RuleSha256 -ine $script:RuleSha256) {
        throw "repair_lobo.py identity changed during the build ($Label reports repair_version $($id.Version), rule_sha256 $($id.RuleSha256); the preflight check ran with $($script:RepairVersion), $($script:RuleSha256)). The repair tool was modified while this script ran; rerun."
    }
}

# Checks hashtables of pbo hashes: every name in Names must be in both maps
# with the same hash. Throws naming the first difference.
function Assert-HashesMatch([hashtable]$Expected, [hashtable]$Actual, [string[]]$Names, [string]$What) {
    foreach ($n in $Names) {
        if (-not $Expected.ContainsKey($n)) { throw "$What`: no preflight hash recorded for $n" }
        if (-not $Actual.ContainsKey($n)) { throw "$What`: $n is missing" }
        if ($Expected[$n] -ine $Actual[$n]) { throw "$What`: $n has hash $($Actual[$n]), preflight recorded $($Expected[$n]); the source @LoBo changed while this script ran. Rerun." }
    }
}

# ---------------------------------------------------------------------------
# ssh helpers (Windows OpenSSH, key auth, BatchMode). Files are streamed
# through 'cat' and a raw byte copy so neither scp's sftp/legacy quoting nor
# PowerShell's text pipeline touches them.
# ---------------------------------------------------------------------------
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
        # Invoke-Ssh returns $null in file mode, and a returned $null still lands in
        # this function's output stream; discard it so the caller gets one length.
        [void](Invoke-Ssh -RemoteCommand ('cat ' + (ConvertTo-ShellQuoted $remote)) -OutFile $part)
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

# Cross-checks every cached dependency archive against the server's own
# deps/SHA256SUMS (basenames, relative to deps/archives): every archive must
# be listed with a matching hash and every listed name must be cached.
# Read-only. Returns the number of archives verified.
function Test-ArchivesAgainstSums([object]$Inventory) {
    $sumsFile = Get-CachePath 'deps/SHA256SUMS'
    if (-not (Test-Path -LiteralPath $sumsFile)) { throw "Cache is missing deps/SHA256SUMS ($sumsFile)" }
    $expectedSums = @{}
    foreach ($raw in [IO.File]::ReadAllLines($sumsFile)) {
        $line = $raw.TrimEnd("`r")
        if ($line -eq '') { continue }
        if ($line -notmatch '^([0-9a-fA-F]{64}) [ *](.+)$') { throw "Unparseable line in deps/SHA256SUMS: $line" }
        $expectedSums[[IO.Path]::GetFileName($Matches[2].Replace('/', '\'))] = $Matches[1].ToUpperInvariant()
    }
    $seen = New-Object System.Collections.Generic.HashSet[string]([StringComparer]::OrdinalIgnoreCase)
    foreach ($a in $Inventory.Archives) {
        $name = [IO.Path]::GetFileName($a.Full)
        if (-not (Test-Path -LiteralPath $a.Full)) { throw "Cache is missing archive $($a.Relative)" }
        if (-not $expectedSums.ContainsKey($name)) { throw "deps/SHA256SUMS has no entry for archive $name" }
        $actual = Get-Sha256 $a.Full
        if ($actual -ine $expectedSums[$name]) { throw "Archive $name hash $actual does not match deps/SHA256SUMS ($($expectedSums[$name]))" }
        [void]$seen.Add($name)
    }
    foreach ($name in $expectedSums.Keys) {
        if (-not $seen.Contains($name)) { throw "deps/SHA256SUMS lists $name but the cache has no such archive" }
    }
    Write-Info "archives verified against deps/SHA256SUMS: $($seen.Count) ok"
    return $seen.Count
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
# @LoBo_Extras (-ExtrasDir)
# ---------------------------------------------------------------------------
# Validates an already-built extras folder and pins its bytes. Returns Root,
# Files (Full, Relative, Length, Hash), Dirs, AddonPbos (top-level *.pbo of a
# top-level addons folder), AllPbos (every *.pbo, any folder) and Bytes.
# Refuses a top-level Bin folder (the engine would load its config), links,
# runtime junk, git or backup folders, and an empty folder.
function Get-ExtrasPlan([string]$Root) {
    foreach ($d in [IO.Directory]::GetDirectories($Root)) {
        if ([IO.Path]::GetFileName($d) -ieq 'Bin') {
            throw "ExtrasDir may not carry a Bin folder: $d (a modfolder's bin\config replaces the whole game config on the official Remastered exe)"
        }
    }
    $entries = Get-TreeEntries -Root $Root
    foreach ($rel in (@($entries.Dirs.ToArray()) + @($entries.Files.ToArray() | ForEach-Object { $_.Relative }))) {
        foreach ($seg in $rel.Split('\')) {
            if ($script:ForbiddenInExtras -icontains $seg) { throw "ExtrasDir carries a forbidden item ($seg): $rel" }
        }
    }
    $files = New-Object System.Collections.Generic.List[object]
    $addonPbos = New-Object System.Collections.Generic.List[object]
    $allPbos = New-Object System.Collections.Generic.List[object]
    $bytes = [long]0
    foreach ($f in ($entries.Files | Sort-Object -Property Relative)) {
        $leaf = [IO.Path]::GetFileName($f.Relative)
        if (Test-JunkName $leaf) { throw "ExtrasDir carries runtime junk: $($f.Relative) (remove it from the extras build)" }
        $item = [pscustomobject]@{ Full = $f.Full; Relative = $f.Relative; Length = $f.Length; Hash = (Get-Sha256 $f.Full) }
        $files.Add($item)
        $bytes += $f.Length
        if ($leaf -like '*.pbo') {
            $allPbos.Add($item)
            $parts = $f.Relative.Split('\')
            if ($parts.Count -eq 2 -and $parts[0] -ieq 'addons') { $addonPbos.Add($item) }
            elseif ($parts.Count -eq 1) { Write-Warn "ExtrasDir pbo at the top level is never loaded by the engine: $($f.Relative)" }
        }
    }
    if ($files.Count -eq 0) { throw "ExtrasDir is empty: $Root" }
    if ($addonPbos.Count -eq 0) { Write-Warn "ExtrasDir has no addons\*.pbo: $Root" }
    return [pscustomobject]@{
        Root = $Root; Files = $files.ToArray(); Dirs = $entries.Dirs.ToArray()
        AddonPbos = $addonPbos.ToArray(); AllPbos = $allPbos.ToArray(); Bytes = $bytes
    }
}

# What build-lobo-extras.py recorded about an extras folder it built
# (docs\BUILD-MANIFEST.json): Found, Tool, BuildVersion, Checked (its own
# class check ran), Against (folder name, pbo count, digest), StockGame,
# Warnings (count), Problem (why the record cannot be used, or ''). Shown in
# the plan and the build report; the authoritative class check is the one
# this script runs on the staged folders.
function Get-ExtrasManifest([string]$Root) {
    $info = [pscustomobject]@{ Found = $false; Tool = ''; BuildVersion = ''; Checked = $false; Against = @(); StockGame = ''; Warnings = 0; Problem = '' }
    $docs = Find-ChildPath $Root 'docs' -Directory
    $path = if ($docs) { Find-ChildPath $docs 'BUILD-MANIFEST.json' } else { $null }
    if (-not $path) { $info.Problem = 'no docs\BUILD-MANIFEST.json, so it was not built by build-lobo-extras.py'; return $info }
    $info.Found = $true
    $m = $null
    try { $m = [IO.File]::ReadAllText($path) | ConvertFrom-Json } catch { $m = $null }
    if ($null -eq $m) { $info.Problem = 'docs\BUILD-MANIFEST.json does not parse'; return $info }
    $info.Tool = [string](Get-JsonField $m 'tool')
    $info.BuildVersion = [string](Get-JsonField $m 'build_version')
    if ($info.Tool -ne 'build-lobo-extras') { $info.Problem = "docs\BUILD-MANIFEST.json is not from build-lobo-extras.py (tool '$($info.Tool)')"; return $info }
    $clash = Get-JsonField $m 'clash_check'
    $info.Checked = [bool](Get-JsonField $clash 'checked')
    $info.StockGame = [string](Get-JsonField $clash 'stock_game')
    $info.Warnings = @(Get-JsonField $clash 'warnings' | Where-Object { $null -ne $_ }).Count
    $folders = @(Get-JsonField $clash 'against_folders' | Where-Object { $null -ne $_ })
    if ($folders.Count -gt 0) {
        $info.Against = @($folders | ForEach-Object { '{0} ({1} pbos, digest {2})' -f (Get-JsonField $_ 'name'), (Get-JsonField $_ 'pbos'), (Get-JsonField $_ 'sha256') })
    } else {
        $info.Against = @(Get-JsonField $clash 'against' | Where-Object { $null -ne $_ } | ForEach-Object { [string]$_ })
    }
    if (-not $info.Checked -or $info.Against.Count -eq 0) { $info.Problem = 'it was built without its class-clash check (no --against folders)' }
    return $info
}

# The class-clash check (build-lobo-extras.py --check) of the staged extras
# against the other staged modfolders and the stock game config: an extras
# config usually merges after core, so it must not change a core class, not
# only avoid its pbo and CfgPatches names. Throws on any clash or on an
# extras config the engine parser cannot read in full. Returns the report.
function Invoke-ExtrasClassCheck([string]$ExtrasRoot, [string[]]$Against, [string]$ReportPath, [string]$WorkDir) {
    if (Test-Path -LiteralPath $ReportPath) { Remove-Item -LiteralPath $ReportPath -Force }
    $cmdArgs = @('-I', $script:ExtrasBuilderFull, '--check', $ExtrasRoot, '--tools', $script:ToolsFull,
                 '--stock-game', $script:StockGameDirFull, '--work', $WorkDir, '--report', $ReportPath)
    foreach ($a in $Against) { $cmdArgs += @('--against', $a) }
    Write-Info "build-lobo-extras.py --check (report $ReportPath)"
    $r = Invoke-Capture -FilePath $script:PythonExe -Arguments $cmdArgs
    $report = $null
    if (Test-Path -LiteralPath $ReportPath) { try { $report = [IO.File]::ReadAllText($ReportPath) | ConvertFrom-Json } catch { $report = $null } }
    if (($r.ExitCode -ne 0 -and $r.ExitCode -ne 1) -or $null -eq $report) {
        Write-ToolOutput $r.StdErr '      ! '
        throw "build-lobo-extras.py --check could not check the staged $($script:ExtrasName) (exit $($r.ExitCode))"
    }
    $errors = @(Get-JsonField $report 'errors' | Where-Object { $null -ne $_ } | ForEach-Object { [string]$_ })
    if ($r.ExitCode -ne 0 -or $errors.Count -gt 0) {
        throw ("$($script:ExtrasName) class check failed; an extras config would change a class that @LoBo, @LoBo_Deps, @LoBo_UD or the stock game define, or could not be read in full:`n   " + ($errors -join "`n   "))
    }
    return $report
}

# Fails when any extras pbo file name (case-insensitive, any folder) is
# already used by another modfolder. Others: name -> where (string).
function Assert-NoPboNameClash([object[]]$ExtrasPbos, [hashtable]$Others, [string]$When) {
    $clashes = New-Object System.Collections.Generic.List[string]
    foreach ($p in $ExtrasPbos) {
        $n = [IO.Path]::GetFileName($p.Relative)
        if ($Others.ContainsKey($n)) { $clashes.Add("$($script:ExtrasName)\$($p.Relative) vs $($Others[$n])") }
    }
    if ($clashes.Count -gt 0) {
        throw ("ExtrasDir pbo file name clash ($When); the engine keeps only one pbo of each name, so an extra must not reuse one:`n   " + ($clashes -join "`n   "))
    }
}

# Runs lobo_cfgpatches.py over pbo files (chunked to keep the command line
# short). Returns a hashtable full path -> pscustomobject (Config,
# CfgPatches, Error).
function Get-CfgPatchesIndex([string[]]$Paths) {
    $map = @{}
    $chunk = New-Object System.Collections.Generic.List[string]
    $len = 0
    $all = @($Paths)
    for ($i = 0; $i -le $all.Count; $i++) {
        $flush = ($i -eq $all.Count) -or ($len + $all[$i].Length -gt 20000)
        if ($flush -and $chunk.Count -gt 0) {
            $cmdArgs = @('-I', $script:CfgPatchesHelperFull, '--tools', $script:ToolsFull) + $chunk.ToArray()
            $r = Invoke-Capture -FilePath $script:PythonExe -Arguments $cmdArgs
            if ($r.ExitCode -ne 0 -and $r.ExitCode -ne 3) {
                Write-ToolOutput $r.StdErr '      ! '
                throw "lobo_cfgpatches.py exited $($r.ExitCode)"
            }
            $parsed = $null
            try { $parsed = $r.StdOut | ConvertFrom-Json } catch { $parsed = $null }
            if ($null -eq $parsed -or $null -eq (Get-JsonField $parsed 'pbos')) { throw "lobo_cfgpatches.py printed no JSON result (exit $($r.ExitCode))" }
            foreach ($p in @($parsed.pbos)) {
                $map[[string]$p.path] = [pscustomobject]@{
                    Config = [string](Get-JsonField $p 'config')
                    CfgPatches = @(@(Get-JsonField $p 'cfgpatches') | Where-Object { $null -ne $_ } | ForEach-Object { [string]$_ })
                    Error = [string](Get-JsonField $p 'error')
                }
            }
            $chunk.Clear()
            $len = 0
        }
        if ($i -lt $all.Count) { $chunk.Add($all[$i]); $len += $all[$i].Length + 3 }
    }
    foreach ($p in $all) { if (-not $map.ContainsKey($p)) { throw "lobo_cfgpatches.py returned no entry for $p" } }
    return $map
}

# The CfgPatches cross-check. Groups: list of @{ Label; Paths } for the
# other modfolders; Extras: the extras addon pbo paths. Every pbo must be
# readable; an extras CfgPatches name must not be declared by any other pbo
# (other extras included). Returns the extras index (path -> names).
function Assert-NoCfgPatchesClash([object[]]$Groups, [string[]]$ExtrasPaths, [string]$ExtrasLabel, [string]$When) {
    $allPaths = New-Object System.Collections.Generic.List[string]
    foreach ($g in $Groups) { foreach ($p in $g.Paths) { $allPaths.Add($p) } }
    foreach ($p in $ExtrasPaths) { $allPaths.Add($p) }
    $index = Get-CfgPatchesIndex $allPaths.ToArray()
    $errors = New-Object System.Collections.Generic.List[string]
    foreach ($p in $allPaths) { if ($index[$p].Error -ne '') { $errors.Add("$p`: $($index[$p].Error)") } }
    if ($errors.Count -gt 0) { throw ("Cannot read the CfgPatches of every pbo ($When), so the extras cannot be checked:`n   " + ($errors -join "`n   ")) }
    $declared = @{}   # lower-case CfgPatches name -> list of 'label\pbo'
    $otherCount = 0
    foreach ($g in $Groups) {
        foreach ($p in $g.Paths) {
            $otherCount++
            foreach ($n in $index[$p].CfgPatches) {
                $k = $n.ToLowerInvariant()
                if (-not $declared.ContainsKey($k)) { $declared[$k] = New-Object System.Collections.Generic.List[string] }
                $declared[$k].Add("$($g.Label)\$([IO.Path]::GetFileName($p))")
            }
        }
    }
    $clashes = New-Object System.Collections.Generic.List[string]
    $extrasSeen = @{}
    $extrasNames = 0
    foreach ($p in $ExtrasPaths) {
        $where = "$ExtrasLabel\$([IO.Path]::GetFileName($p))"
        foreach ($n in $index[$p].CfgPatches) {
            $extrasNames++
            $k = $n.ToLowerInvariant()
            if ($declared.ContainsKey($k)) { $clashes.Add("CfgPatches $n in $where is also declared by " + (($declared[$k] | Select-Object -Unique) -join ', ')) }
            if ($extrasSeen.ContainsKey($k)) { $clashes.Add("CfgPatches $n is declared by both $($extrasSeen[$k]) and $where") } else { $extrasSeen[$k] = $where }
        }
    }
    if ($clashes.Count -gt 0) {
        throw ("ExtrasDir CfgPatches clash ($When); a clashing addon would merge into or replace another one's config:`n   " + ($clashes -join "`n   "))
    }
    Write-Info "CfgPatches: $extrasNames name(s) in $($ExtrasPaths.Count) extras pbo(s), none declared by the $otherCount other pbo(s) ($When)"
    return $index
}

# ---------------------------------------------------------------------------
# Known issues (-KnownIssuesFile) and the content repair history
# ---------------------------------------------------------------------------
# Lines of the known-issues file: '#' comment lines dropped, leading and
# trailing blank lines trimmed. Must be plain ASCII with no ' -- ' dash.
function Read-KnownIssuesFile([string]$Path) {
    $raw = [IO.File]::ReadAllBytes($Path)
    $start = 0
    if ($raw.Length -ge 3 -and $raw[0] -eq 0xEF -and $raw[1] -eq 0xBB -and $raw[2] -eq 0xBF) { $start = 3 }
    for ($i = $start; $i -lt $raw.Length; $i++) {
        if ($raw[$i] -gt 127) { throw "KnownIssuesFile $Path is not plain ASCII (byte 0x$($raw[$i].ToString('X2')) at offset $i; em dashes and typographic quotes are not allowed)" }
    }
    $text = [Text.Encoding]::ASCII.GetString($raw, $start, $raw.Length - $start)
    $lines = New-Object System.Collections.Generic.List[string]
    $n = 0
    foreach ($l in ($text -replace "`r`n", "`n" -split "`n")) {
        $n++
        if ($l -match '^\s*#') { continue }
        $line = $l.TrimEnd()
        if ($line -match ' -- ' -or $line -match '^-- ' -or $line -match ' --$') { throw "KnownIssuesFile $Path line $n uses ' -- ' as a dash" }
        if ($line -match "`t") { throw "KnownIssuesFile $Path line $n contains a tab; use spaces" }
        $lines.Add($line)
    }
    while ($lines.Count -gt 0 -and $lines[0] -eq '') { $lines.RemoveAt(0) }
    while ($lines.Count -gt 0 -and $lines[$lines.Count - 1] -eq '') { $lines.RemoveAt($lines.Count - 1) }
    if ($lines.Count -eq 0) { throw "KnownIssuesFile $Path holds no issue lines" }
    return $lines.ToArray()
}

# The content repair batches recorded under <LoBoDir>\.ud-backups\issue-69:
# one object per batch folder (Batch, Manifest, State, RepairVersion,
# RuleSha256, Compatibility, Created, ArchiveCount, MemberCount, Archives),
# oldest first. A manifest that does not parse, or an applied one without
# its identity and lists, is fatal; a batch without manifest.json is
# reported with State 'no-manifest'.
function Get-RepairBatches([string]$LoBoRoot) {
    $out = New-Object System.Collections.Generic.List[object]
    $backups = Find-ChildPath $LoBoRoot '.ud-backups' -Directory
    if (-not $backups) { return $out.ToArray() }
    $issue = Find-ChildPath $backups 'issue-69' -Directory
    if (-not $issue) { return $out.ToArray() }
    foreach ($batch in ([IO.Directory]::GetDirectories($issue) | Sort-Object)) {
        $name = [IO.Path]::GetFileName($batch)
        $manifestPath = Find-ChildPath $batch 'manifest.json'
        $item = [pscustomobject]@{ Batch = $name; Manifest = $manifestPath; State = 'no-manifest'; RepairVersion = ''; RuleSha256 = ''
                                   Compatibility = ''; Created = ''; ArchiveCount = 0; MemberCount = 0; Archives = @() }
        if ($manifestPath) {
            $m = $null
            try { $m = [IO.File]::ReadAllText($manifestPath) | ConvertFrom-Json } catch { throw "Cannot parse the repair backup manifest $manifestPath`: $($_.Exception.Message)" }
            $item.State = ConvertTo-AsciiText ([string](Get-JsonField $m 'state'))
            $item.RepairVersion = ConvertTo-AsciiText ([string](Get-JsonField $m 'repair_version'))
            $item.RuleSha256 = (ConvertTo-AsciiText ([string](Get-JsonField $m 'rule_sha256'))).ToLowerInvariant()
            $item.Compatibility = ConvertTo-AsciiText ([string](Get-JsonField $m 'compatibility'))
            $item.Created = ConvertTo-AsciiText ([string](Get-JsonField $m 'created_utc'))
            $archives = Get-JsonField $m 'archives'
            $members = Get-JsonField $m 'members'
            if ($item.State -eq 'applied') {
                if ($item.RepairVersion -eq '' -or $item.RuleSha256 -eq '' -or $null -eq $archives -or $null -eq $members) {
                    throw "Repair backup manifest $manifestPath is marked applied but lacks repair_version, rule_sha256, archives or members"
                }
            }
            if ($null -ne $archives) {
                $item.Archives = @(@($archives) | ForEach-Object { ConvertTo-AsciiText ([string](Get-JsonField $_ 'file')) })
                $item.ArchiveCount = $item.Archives.Count
            }
            if ($null -ne $members) { $item.MemberCount = @($members).Count }
        }
        $out.Add($item)
    }
    return $out.ToArray()
}

# ---------------------------------------------------------------------------
# LoBo Reloaded library (-LibraryDir)
# ---------------------------------------------------------------------------
# Verifies a package folder against its own SHA256SUMS.txt: every listed
# file present with the listed hash, and no file that is not listed.
# Returns the number of files verified.
function Test-PackageFolder([string]$Root) {
    $sumsPath = Join-Path $Root 'SHA256SUMS.txt'
    if (-not (Test-Path -LiteralPath $sumsPath)) { throw "No SHA256SUMS.txt in $Root" }
    $listed = @{}
    foreach ($raw in [IO.File]::ReadAllLines($sumsPath)) {
        if ($raw -eq '') { continue }
        if ($raw -notmatch '^([0-9a-f]{64})  (.+)$') { throw "Unparseable line in $sumsPath`: $raw" }
        $listed[$Matches[2].Replace('/', '\')] = $Matches[1]
    }
    $entries = Get-TreeEntries -Root $Root
    $count = 0
    foreach ($f in $entries.Files) {
        if ($f.Relative -ieq 'SHA256SUMS.txt') { continue }
        if (-not $listed.ContainsKey($f.Relative)) { throw "$($f.Full) is not listed in $sumsPath" }
        $h = (Get-Sha256 $f.Full).ToLowerInvariant()
        if ($h -ne $listed[$f.Relative]) { throw "$($f.Full) has sha256 $h, SHA256SUMS.txt lists $($listed[$f.Relative])" }
        $count++
    }
    if ($count -ne $listed.Count) { throw "$Root holds $count of the $($listed.Count) files SHA256SUMS.txt lists" }
    return $count
}

# Deletes one of this build's own library folders: exactly
# modfolders\<Version>, or its .partial/.old sibling. Anything else is refused.
function Remove-LibraryVersionFolder([string]$Path) {
    $full = [IO.Path]::GetFullPath($Path).TrimEnd('\')
    $ok = $false
    foreach ($own in @($script:LibVersionDir, $script:LibPartialDir, $script:LibOldDir)) {
        if ($full -ieq ([IO.Path]::GetFullPath($own).TrimEnd('\'))) { $ok = $true }
    }
    if (-not $ok) { throw "Refusing to delete in the library anything but this version's own folders: $Path" }
    if (-not (Test-Path -LiteralPath $full)) { return }
    if (Test-ReparsePoint $full) { throw "Refusing to delete a link/junction: $full" }
    Write-Info "removing $full"
    Remove-Item -LiteralPath $full -Recurse -Force
}

# The library files this build would write (archives, sidecars, report)
# that already exist. Returns full paths.
function Get-LibraryConflicts {
    $hits = New-Object System.Collections.Generic.List[string]
    if (Test-Path -LiteralPath $script:LibVersionDir) { $hits.Add($script:LibVersionDir) }
    foreach ($a in $script:LibArchivePaths) {
        foreach ($p in @($a, ($a + '.sha256'))) { if (Test-Path -LiteralPath $p) { $hits.Add($p) } }
    }
    if (Test-Path -LiteralPath $script:LibReportPath) { $hits.Add($script:LibReportPath) }
    return $hits.ToArray()
}

# Copies one file into the library through a temporary name next to it and
# verifies the bytes before it takes the final name.
function Publish-LibraryFile([string]$Source, [string]$Dest) {
    if ((Test-Path -LiteralPath $Dest) -and -not $Force) { throw "Library file already exists: $Dest (use -Force to overwrite)" }
    $tmp = $Dest + '.partial'
    if (Test-Path -LiteralPath $tmp) { Remove-Item -LiteralPath $tmp -Force }
    Copy-FileTo $Source $tmp
    if ((Get-Sha256 $tmp) -ine (Get-Sha256 $Source)) { Remove-Item -LiteralPath $tmp -Force; throw "Library copy of $Source is not byte-identical" }
    if (Test-Path -LiteralPath $Dest) { Remove-Item -LiteralPath $Dest -Force }
    [IO.File]::Move($tmp, $Dest)
    Write-Info "library: $Dest"
}

# ---------------------------------------------------------------------------
# Generated text
# ---------------------------------------------------------------------------
# The world names the packed demo missions target, from the staged
# Missions\<name>.<world>.pbo files (distinct, case-insensitive, sorted).
function Get-MissionWorlds([string]$MissionsDir) {
    $worlds = New-Object System.Collections.Generic.List[string]
    if (-not [IO.Directory]::Exists($MissionsDir)) { return $worlds.ToArray() }
    foreach ($f in [IO.Directory]::GetFiles($MissionsDir)) {
        $n = [IO.Path]::GetFileName($f)
        if ($n -notlike '*.pbo') { continue }
        $stem = [IO.Path]::GetFileNameWithoutExtension($n)
        $dot = $stem.LastIndexOf('.')
        if ($dot -lt 1 -or $dot -ge $stem.Length - 1) { continue }
        $w = $stem.Substring($dot + 1)
        $dup = $false
        foreach ($x in $worlds) { if ($x -ieq $w) { $dup = $true } }
        if (-not $dup) { $worlds.Add($w) }
    }
    return @($worlds.ToArray() | Sort-Object)
}

function Get-ReadmeFirstLines {
    param(
        [int]$LoBoPboCount, [int]$DepsPboCount, [int]$UdPboCount, [string[]]$MissionWorlds,
        [object]$Extras = $null, [string[]]$KnownIssues = @(), [object[]]$DoctorFixes = @()
    )
    $worldList = if ($MissionWorlds.Count -gt 0) { $MissionWorlds -join ', ' } else { 'none packed' }
    $hasExtras = ($null -ne $Extras)
    $lead = if ($hasExtras) { '@LoBo_Extras;' } else { '' }
    $title = "$($script:ProductName) $Version"
    $sub = 'The Lost Brothers (@LoBo) for Arma: Cold War Assault, rebuilt and repaired'
    $lines = New-Object System.Collections.Generic.List[string]
    $lines.AddRange([string[]]@(
        $title,
        $sub,
        ('=' * $sub.Length),
        "Package $($script:PackageName).",
        '',
        'FAMILY USE ONLY. Please do not pass this package on or upload it',
        'anywhere. It bundles third-party content (the Lost Brothers mod and',
        'the addon packs it depends on), each under its own licence and credits.',
        $(if ($hasExtras) { 'The original readmes and licences are kept under @LoBo\docs,' } else { 'The original readmes and licences are kept under @LoBo\docs and' }),
        $(if ($hasExtras) { '@LoBo_Deps\readmes and @LoBo_Extras; please respect them.' } else { '@LoBo_Deps\readmes; please respect them.' }),
        '',
        'WHAT IS IN HERE',
        '---------------',
        '@LoBo\            The Lost Brothers mod itself, merged into one modfolder',
        "                  ($LoBoPboCount addon pbos, five islands: Sinai, Lebanon, Gaza,",
        '                  Iran/Iraq and 73 Easting, plus demo missions), with the',
        '                  Uslu dur! data repairs applied in their Classic-safe form.',
        '                  Built to load on the official Remastered exe and on',
        '                  Uslu dur! (see KNOWN ISSUES below and',
        '                  @LoBo\UD-CHANGES.txt).',
        '                  NOTE: the launch line and the "no further addons are',
        '                  required" statement inside @LoBo\README.txt predate the',
        '                  @LoBo_Deps split and are superseded by this file. Use',
        '                  only the LAUNCH LINES below.',
        '@LoBo_Deps\       The third-party addons LoBo depends on but never shipped',
        "                  ($DepsPboCount pbos), as a second modfolder. See @LoBo_Deps\README.txt."
    ))
    $depsFixes = @($DoctorFixes | Where-Object { $_.Folder -eq '@LoBo_Deps' })
    if ($depsFixes.Count -gt 0) {
        $lines.AddRange([string[]]@(
            "                  Mod doctor byte repairs in $($depsFixes.Count) of them, listed in",
            '                  @LoBo\UD-CHANGES.txt.'
        ))
    }
    if ($hasExtras) {
        $lines.AddRange([string[]]@(
            '@LoBo_Extras\     OPTIONAL: extra addon packs recovered alongside LoBo',
            "                  ($($Extras.AddonPbos.Count) addon pbos, $($Extras.MissionPbos.Count) missions), with their readmes.",
            '                  Nothing in @LoBo or @LoBo_Deps needs them; leave the',
            '                  folder out of the launch line to play without them.'
        ))
        $extrasFixes = @($DoctorFixes | Where-Object { $_.Folder -eq $script:ExtrasName })
        if ($extrasFixes.Count -gt 0) {
            $lines.AddRange([string[]]@(
                "                  Mod doctor byte repairs in $($extrasFixes.Count) of them, listed in",
                '                  @LoBo\UD-CHANGES.txt.'
            ))
        }
    }
    $lines.AddRange([string[]]@(
        '@LoBo_UD\         Uslu dur! ONLY: the Guerrilla Mode faction table plus',
        "                  $UdPboCount addon pbos in their full-strength (UD profile) form.",
        '                  It must be the LAST modfolder on the command line and',
        '                  must never be mounted on the official Remastered exe',
        '                  or the legacy 1.99 exe. See @LoBo_UD\README.txt.',
        'SHA256SUMS.txt    Checksums of every file in this package.',
        'README-FIRST.txt  This file.',
        '',
        'INSTALL',
        '-------'
    ))
    if ($hasExtras) {
        $lines.AddRange([string[]]@(
            '1. If your game folder already has an @LoBo, @LoBo_Deps, @LoBo_Extras',
            '   or @LoBo_UD folder (an earlier copy of the mod), delete it or move it',
            '   away first. Do not copy over it: Explorer would merge the folders',
            '   and keep stale files, including a Bin\config.cpp that breaks the',
            '   official Remastered engine.',
            '2. Copy the @LoBo and @LoBo_Deps folders (plus @LoBo_Extras if you',
            '   want the optional extras, and @LoBo_UD if you play Uslu dur!) into',
            '   your game folder, next to the game exe and its AddOns folder. The',
            '   result looks like this:',
            '       <game folder>\PoseidonGame.exe',
            '       <game folder>\AddOns\',
            '       <game folder>\@LoBo_Extras\addons\...  (optional)'
        ))
    } else {
        $lines.AddRange([string[]]@(
            '1. If your game folder already has an @LoBo, @LoBo_Deps or @LoBo_UD',
            '   folder (an earlier copy of the mod), delete it or move it away',
            '   first. Do not copy over it: Explorer would merge the folders and',
            '   keep stale files, including a Bin\config.cpp that breaks the',
            '   official Remastered engine.',
            '2. Copy the @LoBo and @LoBo_Deps folders (and @LoBo_UD if you play',
            '   Uslu dur!) into your game folder, next to the game exe and its',
            '   AddOns folder. The result looks like this:',
            '       <game folder>\PoseidonGame.exe',
            '       <game folder>\AddOns\'
        ))
    }
    $lines.AddRange([string[]]@(
        '       <game folder>\@LoBo\addons\...',
        '       <game folder>\@LoBo_Deps\Addons\...',
        '       <game folder>\@LoBo_UD\addons\...      (Uslu dur! only)',
        '   Keep the folder names exactly as they are.',
        '3. Start the game with the mod flag for your engine (next section).',
        '   The mod only loads when the flag is given, so do not just',
        '   double-click the exe: make a shortcut to it and append the flag to',
        '   the Target line, write a one-line .bat file, or (Steam) put the',
        '   flag in Properties > Launch Options.',
        '',
        'LAUNCH LINES',
        '------------',
        'Official Remastered (Steam folder ...\ARMA Cold War Assault\Remastered,',
        'PoseidonGame.exe):',
        "    PoseidonGame.exe --mod `"$($lead)@LoBo;@LoBo_Deps`"",
        '',
        'Uslu dur! (our build of the remaster engine, PoseidonGame.exe):',
        "    PoseidonGame.exe --mod `"$($lead)@LoBo;@LoBo_Deps;@LoBo_UD`"",
        '',
        'Legacy: Arma: Cold War Assault 1.99 (ColdWarAssault.exe):',
        "    ColdWarAssault.exe -mod=$($lead)@LoBo;@LoBo_Deps",
        ''
    ))
    if ($hasExtras) {
        $lines.AddRange([string[]]@(
            '@LoBo_Extras is optional and can be left out. To play without the',
            'extras, drop "@LoBo_Extras;" from the line, for example:',
            '    PoseidonGame.exe --mod "@LoBo;@LoBo_Deps"',
            ''
        ))
    }
    $lines.AddRange([string[]]@(
        'Why this order: the engine walks the list from the LAST entry',
        'backwards and keeps the first-loaded copy of every pbo name, so when',
        'two modfolders carry a pbo of the same name, the last-listed one wins.',
        '@LoBo_UD must stay LAST so that its Uslu dur! versions win.'
    ))
    if ($hasExtras) {
        $lines.AddRange([string[]]@(
            '@LoBo_Extras goes FIRST so that the core @LoBo and @LoBo_Deps files',
            'always win a pbo-name clash with an extra. The build checked that no',
            'extras pbo name, CfgPatches name or config class conflicts with',
            '@LoBo, @LoBo_Deps or @LoBo_UD, but keep the order anyway.'
        ))
    }
    $lines.AddRange([string[]]@(
        'Never mount @LoBo_UD on the official Remastered exe or on the legacy',
        '1.99 exe.',
        'These lines replace the "-mod=@LoBo" line in @LoBo\README.txt.',
        '',
        'DEMO MISSIONS',
        '-------------',
        'The missions in @LoBo\Missions appear in Single Player > Missions',
        "(filter by island: $worldList)."
    ))
    if ($hasExtras -and $Extras.MissionPbos.Count -gt 0) {
        $extraWorlds = if ($Extras.MissionWorlds.Count -gt 0) { $Extras.MissionWorlds -join ', ' } else { 'unknown' }
        $lines.Add("The $($Extras.MissionPbos.Count) missions in @LoBo_Extras\Missions appear there too while")
        $lines.Add("@LoBo_Extras is mounted (filter by island: $extraWorlds).")
    }
    $lines.AddRange([string[]]@(
        'The folders in @LoBo\user missions are unpacked editor missions. To',
        'use one, copy its folder into the editor missions folder of your',
        'engine, then open it in the Mission Editor on the matching island:',
        '  official Remastered  Documents\Cold War Assault\missions',
        '  and Uslu dur!:       (the folder name has spaces and there is no',
        '                       Users\<name> level; the engine creates it on',
        '                       its first start)',
        '  legacy 1.99 exe:     <game folder>\Users\<your name>\Missions',
        '',
        'MULTIPLAYER',
        '-----------',
        'Everyone in a game must run identical content: the same package',
        'version and the same modfolders. Mixing this package with an older',
        '@LoBo, or a UD player with a player on another engine, will desync',
        'or crash.'
    ))
    if ($hasExtras) { $lines.Add('Either everyone mounts @LoBo_Extras or nobody does.') }
    $lines.AddRange([string[]]@(
        '',
        'KNOWN ISSUES',
        '------------',
        'Please read these before reporting a problem; they are known and are',
        'not signs of a broken install. Details in @LoBo\UD-CHANGES.txt.'
    ))
    $lines.AddRange([string[]]$KnownIssues)
    $lines.AddRange([string[]]@(
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
        'Panzerfaust, Chris G., Noone and others; LOBO ownership is now under',
        'Null_X) and the many addon authors credited in @LoBo\README.txt,',
        '@LoBo\docs and @LoBo_Deps\readmes.'
    ))
    if ($hasExtras) { $lines.Add('The authors of the optional packs are credited in the readmes in @LoBo_Extras.') }
    $lines.AddRange([string[]]@(
        "Modfolder assembly, repairs and this package: the Uslu dur! project, $($script:Today).",
        "Package built by arma_CWR/tools/lobo/package-lobo-family.ps1 ($($script:ProductName) library builder)."
    ))
    return $lines.ToArray()
}

# The UD-CHANGES.txt section on what "mod doctor --fix" patched in the staged
# @LoBo_Deps and @LoBo_Extras (Invoke-StagedDoctorFix records).
function Get-DoctorFixLines([object[]]$DoctorFixes, [bool]$HasExtras = $false) {
    $folders = if ($HasExtras) { "@LoBo_Deps and $($script:ExtrasName)" } else { '@LoBo_Deps' }
    $lines = New-Object System.Collections.Generic.List[string]
    $lines.AddRange([string[]]@(
        'REPAIR SET A IN THE OTHER MODFOLDERS (at packaging)',
        '---------------------------------------------------',
        'The same mod doctor ran with --fix on the packaged copies of',
        "$folders; the downloaded files on the build machine",
        'keep their original bytes. A report-only doctor run then found nothing',
        'left to do in any modfolder of this package (configs stored compressed',
        'or binarised cannot be read by the doctor and were not scanned). This',
        'matters most on the official Remastered exe: it reads a malformed',
        'float such as 0.0.1 as a script expression and logs "Script error at',
        '''0.0|#|.1''", which its test mode treats as fatal.'
    ))
    if (@($DoctorFixes).Count -eq 0) {
        $lines.Add('The doctor found nothing to patch there.')
        return $lines.ToArray()
    }
    $lines.Add('Patched, with each archive''s sha256 before and after:')
    foreach ($f in $DoctorFixes) {
        $lines.Add(('   {0}\{1}, {2} site(s)' -f $f.Folder, (ConvertTo-AsciiText $f.Relative), $f.Sites))
        foreach ($finding in $f.Findings) { $lines.Add('      ' + (ConvertTo-AsciiText $finding)) }
        $lines.Add('      was ' + $f.OrigHash.ToLowerInvariant())
        $lines.Add('      now ' + $f.NewHash.ToLowerInvariant())
    }
    return $lines.ToArray()
}

function Get-ChangesLines {
    param(
        [object[]]$Overlay, [string[]]$ExtraMerged, [hashtable]$StagedHashes,
        [object[]]$Batches = @(), [string[]]$KnownIssues = @(),
        [object[]]$DoctorFixes = @(), [bool]$HasExtras = $false
    )
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
        '   docs\73eastings\73eastings_provenance.txt'
    ))
    if ($ExtraMerged.Count -gt 0) {
        $lines.Add('Files merged from a local extras folder when this package was built:')
        foreach ($e in $ExtraMerged) { $lines.Add('   ' + $e) }
    }
    $lines.AddRange([string[]]@(
        'README.txt is shipped unedited. Its HOW TO LAUNCH line (-mod=@LoBo) and',
        'its REQUIRED GAME STATE claim that no further addons are needed predate',
        'the @LoBo_Deps split and no longer hold: the third-party dependencies',
        'now live in the @LoBo_Deps modfolder, and the only valid launch lines',
        'are the ones in README-FIRST.txt at the top of the package.',
        'Everything else in this folder is the mod as released, apart from the',
        'two repair sets below, the profile split and the one moved file.',
        '',
        'REPAIR SET A: mod doctor (byte patches, valid on every engine)',
        '--------------------------------------------------------------',
        'Applied with "PoseidonTools mod doctor --fix" (engine source:',
        'engine/Poseidon/Asset/Addon/ModDoctor.cpp). Each patch keeps the file',
        'length, so the pbo header tables never move.',
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
        '3. Malformed tracer colour. addons\LoBoammo.pbo and',
        '   addons\LoBo_airammo.pbo carried tracerColor 0.0.1, which is not a',
        '   number. Rewritten to 0.001.',
        '   The stock (unrepaired) files, which must never be used:',
        "      LoBoWreck.pbo   $($script:StockHashes['LoBoWreck.pbo'].ToLowerInvariant())",
        "      LoBoPalObj.pbo  $($script:StockHashes['LoBoPalObj.pbo'].ToLowerInvariant())",
        '   The files shipped here:',
        "      LoBoWreck.pbo   $(([string]$StagedHashes['LoBoWreck.pbo']).ToLowerInvariant())",
        "      LoBoPalObj.pbo  $(([string]$StagedHashes['LoBoPalObj.pbo']).ToLowerInvariant())",
        ''
    ))
    $lines.AddRange([string[]](Get-DoctorFixLines -DoctorFixes $DoctorFixes -HasExtras $HasExtras))
    $lines.AddRange([string[]]@(
        '',
        'REPAIR SET B: content repair (repair_lobo.py)',
        '---------------------------------------------',
        'A reproducible, offline repair of the mod''s configs, scripts and',
        'models. When this package was built, repair_lobo.py',
        "repair_version $($script:RepairVersion), rule sha256",
        "$($script:RuleSha256),",
        'reported nothing left to change on the source in its UD profile.',
        'Applied on the build machine, oldest first (from the backup manifests',
        'under @LoBo\.ud-backups\issue-69 there, which are not shipped):'
    ))
    $applied = @($Batches | Where-Object { $_.State -eq 'applied' })
    $distinct = @{}
    foreach ($b in $applied) {
        $lines.Add(('   repair_version {0}, {1} profile, batch {2}' -f $b.RepairVersion, $(if ($b.Compatibility -ne '') { $b.Compatibility.ToUpperInvariant() } else { 'unknown' }), $b.Batch))
        $lines.Add(('      {0} archives, {1} members, rule sha256' -f $b.ArchiveCount, $b.MemberCount))
        $lines.Add(('      {0}' -f $b.RuleSha256))
        foreach ($a in $b.Archives) { if ($a -ne '') { $distinct[$a] = $true } }
    }
    if ($applied.Count -eq 0) {
        $lines.Add('   (no applied batch is recorded on the build machine)')
    } else {
        $lines.Add("   $($applied.Count) applied batch(es), $($distinct.Count) distinct archives rewritten in all.")
    }
    $lines.AddRange([string[]]@(
        'The rules cover, among others (the full list is CONTENT-REPAIRS.md',
        'next to repair_lobo.py in arma_CWR/tools/lobo of the Uslu dur!',
        'repository):',
        '   - the missing M833 (LoBo_105Sabot2) projectile class',
        '   - machine-gun AI engagement profiles (JAM-derived, MG and rifle use',
        '     of 5.56 separated, suppressed Negev magazine corrected)',
        '   - unsupported tank armor bindings (M1 and Merkava hull, turret and',
        '     track selections now bind to real model selections)',
        '   - aircraft labels (MiG-21, MiG-25, F-5)',
        '   - the Micro Uzi magazine reference',
        '   - the landing-craft numbering scripts',
        '   - the wreck-fire scripts (undefined function, invalid glow state)',
        '   - the CoC divers (both missing outfits now use the bundled swimmer',
        '     model, so placing them no longer crashes the game)',
        '   - splash damage on ordinary MG and AP rounds (removed)',
        '   - the 60 mm and vehicle-bomb blast plateaus (bounded)',
        '   - the Mk82 family (GBU-12 shares the Mk82 warhead values)',
        '   - missile launch helpers (deleted after their last read)',
        '   - display text (Assault, Missile, Gauge, 122 mm strings)',
        'Class names, magazine capacities, velocities and unrelated values are',
        'unchanged. Values are a documented gameplay baseline, not measured',
        'ballistics.',
        '',
        'TWO PROFILES, ONE SPLIT',
        '-----------------------',
        'The wreck-fire repair exists in two profiles. The UD profile uses the',
        'engine command nearestObjects to find every nearby infantry unit; the',
        'legacy 1.99 engine does not have that command, so it must never run',
        'there. The Classic profile uses older commands and finds only the',
        'player''s and the wreck''s groups: a deliberately limited fallback, not',
        'full parity.',
        '',
        'This @LoBo folder carries the CLASSIC profile (applied to this copy with',
        '"repair_lobo.py --compatibility classic"), so it is safe on all three',
        'engines. The @LoBo_UD\addons folder carries the UD-profile versions of',
        'exactly the archives that differ between the profiles. On Uslu dur!,',
        '@LoBo_UD is listed last and its copies win. SHA-256 of each twin:',
        '   archive            profile  sha256',
        '                      classic  = the file in this folder (@LoBo\addons)',
        '                      UD       = the file in @LoBo_UD\addons'
    ))
    foreach ($o in $Overlay) {
        $lines.Add(('   {0,-18} classic  {1}' -f $o.Name, $o.ClassicHash.ToLowerInvariant()))
        $lines.Add(('   {0,-18} UD       {1}' -f '', $o.UdHash.ToLowerInvariant()))
    }
    if ($Overlay.Count -eq 0) { $lines.Add('   (none: the profile switch changed no archive)') }
    $lines.AddRange([string[]]@(
        '',
        'MOVED OUT: Bin\config.cpp',
        '-------------------------',
        'On the build machine this folder also carries Bin\config.cpp, the',
        'Guerrilla Mode faction table for the eight LoBo war rosters. That file',
        'is for the Uslu dur! engine only. Uslu dur! layers each mounted mod''s',
        'bin\config onto the base game config; the official Remastered',
        'PoseidonGame.exe and the legacy ColdWarAssault.exe instead REPLACE the',
        'whole base config with it, which breaks the game. The package therefore',
        'ships it as @LoBo_UD\bin\config.cpp, to be mounted only on Uslu dur!.',
        'This folder ships Bin\RESOURCE.bin and Bin\configBackup\Config.bin',
        'exactly as stock.',
        ''
    ))
    if ($KnownIssues.Count -gt 0) {
        $lines.AddRange([string[]]@(
            'KNOWN ISSUES (the same list as in README-FIRST.txt)',
            '---------------------------------------------------'
        ))
        $lines.AddRange([string[]]$KnownIssues)
        $lines.AddRange([string[]]@(
            '',
            "Built by arma_CWR/tools/lobo/package-lobo-family.ps1 on $($script:Today), package $($script:PackageName)."
        ))
        return $lines.ToArray()
    }
    $lines.AddRange([string[]]@(
        'CAVEATS',
        '-------',
        '- The Classic wreck-fire profile is a limited fallback: it finds the',
        '  player''s and the wreck''s groups, not every nearby infantry unit.',
        '- The official Remastered engine''s runtime was not verified with either',
        '  profile.',
        '- Runtime testing of the set B work is partial (see the lobo tracking',
        '  repo). Long range AI fire, animated hit locations, saves and',
        '  multiplayer still need a play test.',
        '- In multiplayer everyone must run identical content.',
        '',
        'KNOWN QUIRKS (unrepaired, from the stock mod)',
        '---------------------------------------------',
        '- The sinai, iran_iraq and gaza worlds point their menu intro at a',
        '  lost_anims addon that was never released. Uslu dur! has an engine',
        '  fallback for it.',
        '- Two referenced addons were never publicly released (f3wx_markers,',
        '  bde_weapons). The references are harmless. A third one,',
        '  vit_iraq_shilka, was recovered in 2026; LoBo does not need it.',
        '',
        "Built by arma_CWR/tools/lobo/package-lobo-family.ps1 on $($script:Today), package $($script:PackageName)."
    ))
    return $lines.ToArray()
}

function Get-UdReadmeLines([object[]]$Overlay, [bool]$HasExtras = $false) {
    $lines = New-Object System.Collections.Generic.List[string]
    $lines.AddRange([string[]]@(
        '@LoBo_UD: Uslu dur! only overlay for the Lost Brothers mod',
        '==========================================================',
        'USLU DUR! ONLY, and it must be the LAST modfolder on the command line:'
    ))
    if ($HasExtras) {
        $lines.AddRange([string[]]@(
            '    PoseidonGame.exe --mod "@LoBo_Extras;@LoBo;@LoBo_Deps;@LoBo_UD"',
            '(@LoBo_Extras is optional: without it, --mod "@LoBo;@LoBo_Deps;@LoBo_UD")'
        ))
    } else {
        $lines.Add('    PoseidonGame.exe --mod "@LoBo;@LoBo_Deps;@LoBo_UD"')
    }
    $lines.AddRange([string[]]@(
        '',
        'Never mount it on the official Remastered PoseidonGame.exe or on the',
        'legacy ColdWarAssault.exe, for two reasons. Those engines make a',
        'mod''s bin\config replace the entire base game config, so the game',
        'would start with no vehicles, weapons or menus. And the addons here',
        'use the nearestObjects script command, which the legacy 1.99 engine',
        'does not have.',
        '',
        'WHAT IS IN HERE',
        '---------------',
        'bin\config.cpp   The CfgGuerrillaFactions table that gives Guerrilla',
        '                 Mode its eight Lost Brothers war rosters (IDF,',
        '                 EgyptFrontier, EgyptArmy, Syria, Jordan, Hizballah,',
        '                 PLO, PLO_East). Generated by',
        '                 arma_CWR/tools/lobo/install-lobo-factions.ps1 from',
        '                 guerrilla-mode/config/lobo-factions.hpp and copied',
        '                 here byte for byte from @LoBo\Bin\config.cpp on the',
        '                 build machine.',
        "addons\          $($Overlay.Count) pbos: the UD-profile versions of the archives",
        '                 whose wreck-fire scripts differ between the Classic',
        '                 and UD profiles of the content repair (see',
        '                 @LoBo\UD-CHANGES.txt). They carry the same names as',
        '                 their Classic-profile twins in @LoBo\addons. Uslu dur!',
        '                 walks the --mod list from the last entry backwards and',
        '                 skips a pbo name it has already loaded, so with',
        '                 @LoBo_UD last these copies are the ones that load.'
    ))
    foreach ($o in $Overlay) { $lines.Add(('   {0,-18} {1}' -f $o.Name, $o.UdHash.ToLowerInvariant())) }
    $lines.AddRange([string[]]@(
        '',
        "Package $($script:PackageName), written $($script:Today)."
    ))
    return $lines.ToArray()
}

function Get-SourcesNoteLines([int]$ArchiveCount, [int]$InstallerCount, [string[]]$StockNames, [string[]]$BatchNames) {
    $installers = ''
    if ($InstallerCount -eq 1) { $installers = ' (one of them is a self-extracting installer that was never run)' }
    elseif ($InstallerCount -gt 1) { $installers = " ($InstallerCount of them are self-extracting installers that were never run)" }
    $lines = New-Object System.Collections.Generic.List[string]
    $lines.AddRange([string[]]@(
        "$($script:SourcesName)",
        ('=' * $script:SourcesName.Length),
        "Companion to $($script:PackageName). Not needed to play; kept so the",
        'package can be rebuilt or audited. NOTHING in here belongs in a game',
        'folder: every pbo below is an unrepaired or pre-repair original.',
        '',
        "deps\archives\      the $ArchiveCount original downloads the dependency set was",
        "                    taken from, byte for byte$installers.",
        'deps\SHA256SUMS     checksums of those downloads as recorded on the',
        '                    server; every archive was verified against it',
        '                    before packing.',
        'deps\clamscan.log   antivirus scan log of the downloads.',
        'deps\README.txt     the dependency manifest with launch notes.',
        'stock-originals\    UNREPAIRED: the @LoBo pbos exactly as released, before',
        '                    repair set A (mod doctor) described in',
        '                    @LoBo\UD-CHANGES.txt of the main package (from',
        '                    @LoBo\_ud-orig on the build machine):'
    ))
    foreach ($n in $StockNames) { $lines.Add('                      ' + $n) }
    $lines.AddRange([string[]]@(
        'pre-repair-originals\issue-69\<batch>\',
        '                    PRE-REPAIR: the archives repair set B (repair_lobo.py,',
        '                    UD profile) replaced on the build machine, as they',
        '                    were just before that apply (set A already in them),',
        '                    with the batch manifest.json (from',
        '                    @LoBo\.ud-backups\issue-69 on the build machine):'
    ))
    foreach ($n in $BatchNames) { $lines.Add('                      ' + $n) }
    $lines.AddRange([string[]]@(
        'lobo-modfolder-assembly.txt',
        '                    the notes from assembling the @LoBo modfolder.',
        '',
        'FAMILY USE ONLY. Third-party content under each pack''s own licence.',
        "Written $($script:Today) by arma_CWR/tools/lobo/package-lobo-family.ps1."
    ))
    return $lines.ToArray()
}

# The build report (reports\<Version>\build-report.txt, and with
# -LibraryDir docs\build-<Version>.txt): inputs, hashes, repair identities,
# counts. Reads the script-scope state of a finished build.
function Get-BuildReportLines {
    param([object[]]$FolderStats, [int]$SumCount, [object[]]$Archives)
    $title = "$($script:ProductName) build report: $($script:PackageName)"
    $lines = New-Object System.Collections.Generic.List[string]
    $now = Get-Date
    $lines.AddRange([string[]]@(
        $title,
        ('=' * $title.Length),
        "Built $($now.ToString('yyyy-MM-dd HH:mm:ss')) local, $($now.ToUniversalTime().ToString('yyyy-MM-ddTHH:mm:ssZ')) UTC.",
        'Builder: arma_CWR/tools/lobo/package-lobo-family.ps1',
        "Builder commit: $(ConvertTo-AsciiText $script:BuilderCommit)",
        "PowerShell $($PSVersionTable.PSVersion), python $(ConvertTo-AsciiText $script:PythonExe)",
        '',
        'INPUTS',
        '------',
        "@LoBo source    $(ConvertTo-AsciiText $script:LoBoDirFull)",
        "                $($script:SourceAddonPbos.Count) addon pbos, $($script:SourceFiles) files copied, $($script:SourceBytes) bytes",
        "cache           $(ConvertTo-AsciiText $script:CacheDirFull)",
        "                $(if ($SkipFetch) { 'server not contacted (-SkipFetch)' } else { ConvertTo-AsciiText ('fetched from ' + $Server + ':' + $ServerPath) })",
        "ExtraDir        $(if ($script:ExtraDirFull -ne '') { ConvertTo-AsciiText $script:ExtraDirFull } else { 'none' })",
        "ExtrasDir       $(if ($null -ne $script:Extras) { ConvertTo-AsciiText $script:ExtrasDirFull } else { 'none' })"
    ))
    if ($null -ne $script:Extras) {
        $lines.Add("                $($script:Extras.Files.Count) files, $($script:Extras.AddonPbos.Count) addon pbos, $($script:Extras.MissionPbos.Count) missions, $($script:Extras.Bytes) bytes")
        $m = $script:ExtrasManifest
        if ($m.Found -and $m.Tool -eq 'build-lobo-extras') {
            $lines.Add("                built by build-lobo-extras.py $(ConvertTo-AsciiText $m.BuildVersion)")
            if ($m.Checked) {
                $lines.Add("                its class check ran against$(if ($m.StockGame -ne '') { ' the stock game and' } else { '' }):")
                foreach ($a in $m.Against) { $lines.Add("                  $(ConvertTo-AsciiText $a)") }
            } else {
                $lines.Add('                its class check did not run (no --against)')
            }
        } else {
            $lines.Add("                $(ConvertTo-AsciiText $m.Problem)")
        }
        if ($null -ne $script:ExtrasClassCheck) {
            $lines.Add(("                class check at packaging: no clash against {0} configs of the staged" -f (Get-JsonField $script:ExtrasClassCheck 'indexed_configs')))
            $lines.Add(("                modfolders and the stock game; {0} reported overlap(s), listed in" -f @(Get-JsonField $script:ExtrasClassCheck 'warnings' | Where-Object { $null -ne $_ }).Count))
            $lines.Add("                reports\$Version\extras-class-check.json")
            $lines.Add("                stock game: $(ConvertTo-AsciiText $script:StockGameDirFull)")
        }
    }
    $lines.AddRange([string[]]@(
        "known issues    $(if ($script:KnownIssuesSource -ne '') { ConvertTo-AsciiText $script:KnownIssuesSource } else { 'built-in list' })",
        "Bin\config.cpp  sha256 $((Get-Sha256 $script:ConfigCpp).ToLowerInvariant())",
        "PoseidonTools   $(ConvertTo-AsciiText $script:ToolsFull)",
        "                sha256 $((Get-Sha256 $script:ToolsFull).ToLowerInvariant())",
        "repair tool     $(ConvertTo-AsciiText $script:RepairToolFull)",
        "                sha256 $((Get-Sha256 $script:RepairToolFull).ToLowerInvariant())",
        '',
        'REPAIR IDENTITIES',
        '-----------------',
        "mod doctor (report only): nothing to do in $($script:DoctorSourceCount) source pbos and $($script:DoctorStagedCount) staged pbos",
        "mod doctor --fix on the staged @LoBo_Deps$(if ($null -ne $script:Extras) { ' and @LoBo_Extras' }) (the cache and ExtrasDir"
    ))
    $lines.Add("   keep their bytes; originals moved to _work\$Version\doctor-orig):")
    foreach ($f in $script:DoctorFixes) {
        $lines.Add(('   {0}\{1}  {2} site(s)' -f $f.Folder, (ConvertTo-AsciiText $f.Relative), $f.Sites))
        foreach ($finding in $f.Findings) { $lines.Add('      ' + (ConvertTo-AsciiText $finding)) }
        $lines.Add('      before ' + $f.OrigHash.ToLowerInvariant())
        $lines.Add('      after  ' + $f.NewHash.ToLowerInvariant())
    }
    if ($script:DoctorFixes.Count -eq 0) { $lines.Add('   nothing patched') }
    $lines.Add('mod doctor gate (report only, before SHA256SUMS.txt): nothing to do in')
    $lines.Add('   ' + ((@($script:DoctorGate) | ForEach-Object { "$($_.Folder) $($_.Pbos) pbos" }) -join ', '))
    $lines.AddRange([string[]]@(
        "repair_lobo.py at build time: repair_version $($script:RepairVersion)",
        "   rule sha256 $($script:RuleSha256)",
        "   UD check on the source, classic apply + check on the staged copy and",
        "   UD check on the composite all ran with this identity.",
        "classic switch: $($script:ClassicChangedMembers) members in $($script:Overlay.Count) archives rewritten",
        'applied content repair batches on the source (oldest first):'
    ))
    foreach ($b in $script:RepairBatches) {
        $lines.Add(('   {0}  {1}  state {2}  profile {3}  {4} archives  {5} members' -f $b.Batch, $(if ($b.RepairVersion -ne '') { $b.RepairVersion } else { '?' }), $b.State, $(if ($b.Compatibility -ne '') { $b.Compatibility } else { '?' }), $b.ArchiveCount, $b.MemberCount))
        if ($b.RuleSha256 -ne '') { $lines.Add("      rule sha256 $($b.RuleSha256)") }
    }
    if (@($script:RepairBatches).Count -eq 0) { $lines.Add('   (none recorded)') }
    $lines.Add('profile switch overlay (@LoBo_UD\addons):')
    foreach ($o in $script:Overlay) {
        $lines.Add(('   {0,-18} classic {1}' -f $o.Name, $o.ClassicHash.ToLowerInvariant()))
        $lines.Add(('   {0,-18} UD      {1}' -f '', $o.UdHash.ToLowerInvariant()))
    }
    $lines.AddRange([string[]]@(
        '',
        'COUNTS',
        '------',
        ('   {0,-18} {1,8} {2,8} {3,16}' -f 'folder', 'files', 'pbos', 'bytes')
    ))
    foreach ($s in $FolderStats) { $lines.Add(('   {0,-18} {1,8} {2,8} {3,16}' -f $s.Name, $s.Files, $s.Pbos, $s.Bytes)) }
    $lines.Add("SHA256SUMS.txt: $SumCount entries, sha256 $((Get-Sha256 (Join-Path $script:StageRoot 'SHA256SUMS.txt')).ToLowerInvariant())")
    $lines.AddRange([string[]]@(
        '',
        'OUTPUTS',
        '-------',
        "staged folder   $(ConvertTo-AsciiText $script:StageRoot)"
    ))
    foreach ($a in $Archives) {
        $lines.Add("archive         $(ConvertTo-AsciiText $a)")
        $lines.Add("                sha256 $((Get-Sha256 $a).ToLowerInvariant()), $((New-Object IO.FileInfo($a)).Length) bytes")
    }
    if (@($Archives).Count -eq 0) { $lines.Add('archive         none (-NoArchive)') }
    if ($script:LibraryDirFull -ne '') {
        $lines.AddRange([string[]]@(
            "library         $(ConvertTo-AsciiText $script:LibraryDirFull)",
            "                modfolders\$Version, dist\, docs\build-$Version.txt"
        ))
    } else {
        $lines.Add('library         not filed (no -LibraryDir)')
    }
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
        & $script:SevenZipFull a -t7z -mx=5 -mmt=on $ArchivePath $leaf
        if ($LASTEXITCODE -ne 0) { throw "7z a failed with exit code $LASTEXITCODE" }
    } finally {
        Pop-Location
    }
    Write-Info "7z t $ArchivePath"
    & $script:SevenZipFull t $ArchivePath
    if ($LASTEXITCODE -ne 0) { throw "7z t failed with exit code $LASTEXITCODE" }
    $hash = (Get-Sha256 $ArchivePath).ToLowerInvariant()
    Write-TextFileLf $sidecar @($hash + '  ' + [IO.Path]::GetFileName($ArchivePath))
    $size = (New-Object IO.FileInfo($ArchivePath)).Length
    Write-Info "archive: $ArchivePath ($(Format-Bytes $size))"
    Write-Info "sha256:  $hash  (sidecar $sidecar)"
}

# Dot-sourced for tests: functions and constants only, no preflight.
if ($MyInvocation.InvocationName -eq '.') { return }

# ---------------------------------------------------------------------------
# Preflight (read-only)
# ---------------------------------------------------------------------------
Write-Step 'Preflight'

if ($Version -notmatch '^[A-Za-z0-9][A-Za-z0-9._-]*$') { throw "Version must be a plain token (letters, digits, . _ -): '$Version'" }
# Every path parameter is resolved against the PowerShell location first
# (Resolve-FullPath); the resolved form is what every check and write uses.
$script:LoBoDirFull  = Resolve-FullPath $LoBoDir
if (-not (Test-Path -LiteralPath $script:LoBoDirFull -PathType Container)) { throw "@LoBo folder not found: $LoBoDir ($($script:LoBoDirFull))" }
$script:OutDirFull   = Resolve-FullPath $OutDir
if ($CacheDir -eq '') { $CacheDir = Join-Path $script:OutDirFull '_server-cache' }
$script:CacheDirFull = Resolve-FullPath $CacheDir
$script:StageBase    = Join-Path $script:OutDirFull 'stage'
$script:WorkBase     = Join-Path $script:OutDirFull '_work'
$script:WorkRoot     = Join-Path $script:WorkBase $Version
$script:ReportDir    = Join-Path (Join-Path $script:OutDirFull 'reports') $Version
$script:StageRoot    = Join-Path $script:StageBase $script:PackageName
$script:SourcesRoot  = Join-Path $script:StageBase $script:SourcesName
$script:ArchivePath  = Join-Path $script:OutDirFull ($script:PackageName + '.7z')
$script:SourcesArchivePath = Join-Path $script:OutDirFull ($script:SourcesName + '.7z')
$script:AssemblyNotes = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot 'lobo-modfolder-assembly.txt'))

if (Test-PathUnder $script:LoBoDirFull $script:OutDirFull) { throw "OutDir must not be inside the @LoBo folder: $OutDir" }
if (Test-PathUnder $script:OutDirFull $script:LoBoDirFull) { throw "The @LoBo folder must not be inside OutDir: $LoBoDir" }
$script:ExtraDirFull = ''
if ($ExtraDir -ne '') {
    $script:ExtraDirFull = Resolve-FullPath $ExtraDir
    if (-not (Test-Path -LiteralPath $script:ExtraDirFull -PathType Container)) { throw "ExtraDir not found: $ExtraDir ($($script:ExtraDirFull))" }
    if (Test-PathUnder $script:OutDirFull $script:ExtraDirFull) { throw "ExtraDir must not be inside OutDir: $ExtraDir" }
    if (Test-PathUnder $script:LoBoDirFull $script:ExtraDirFull) { throw "ExtraDir must not be inside the @LoBo folder: $ExtraDir" }
    if (Test-PathUnder $script:ExtraDirFull $script:LoBoDirFull) { throw "ExtraDir must not contain the @LoBo folder: $ExtraDir" }
}
# ExtrasDir: an already-built modfolder, copied verbatim. It may not overlap
# a source, OutDir or ExtraDir.
$script:ExtrasDirFull = ''
if ($ExtrasDir -ne '') {
    $script:ExtrasDirFull = Resolve-FullPath $ExtrasDir
    if (-not (Test-Path -LiteralPath $script:ExtrasDirFull -PathType Container)) { throw "ExtrasDir not found: $ExtrasDir ($($script:ExtrasDirFull))" }
    if (Test-ReparsePoint $script:ExtrasDirFull) { throw "ExtrasDir is a link/junction: $($script:ExtrasDirFull)" }
    foreach ($pair in @(@('OutDir', $script:OutDirFull), @('the @LoBo folder', $script:LoBoDirFull), @('ExtraDir', $script:ExtraDirFull))) {
        if ($pair[1] -eq '') { continue }
        if ((Test-PathUnder $pair[1] $script:ExtrasDirFull) -or (Test-PathUnder $script:ExtrasDirFull $pair[1])) {
            throw "ExtrasDir must not be on, under or above $($pair[0]): $ExtrasDir ($($script:ExtrasDirFull))"
        }
    }
}
# The fetch writes into CacheDir (mirrored tree, .part files, the manifest)
# and replaces stale files there, so it may not point at a source folder or
# at anything this script wipes.
foreach ($pair in @(@('the @LoBo folder', $script:LoBoDirFull), @('ExtraDir', $script:ExtraDirFull), @('ExtrasDir', $script:ExtrasDirFull))) {
    if ($pair[1] -eq '') { continue }
    if ((Test-PathUnder $pair[1] $script:CacheDirFull) -or (Test-PathUnder $script:CacheDirFull $pair[1])) {
        throw "CacheDir must not be on, under or above $($pair[0]): $CacheDir ($($script:CacheDirFull))"
    }
}
foreach ($owned in @($script:StageBase, $script:WorkBase)) {
    if (Test-PathUnder $owned $script:CacheDirFull) { throw "CacheDir must not be under $owned (that tree is wiped by the build): $CacheDir" }
}

# LibraryDir: only three targets in it are ever written (this version's
# modfolders folder, dist\, docs\build-<Version>.txt); none of them may
# overlap an input or a folder this script wipes.
$script:LibraryDirFull = ''
$script:LibVersionDir = ''
$script:LibPartialDir = ''
$script:LibOldDir = ''
$script:LibDistDir = ''
$script:LibDocsDir = ''
$script:LibReportPath = ''
$script:LibArchivePaths = @()
if ($LibraryDir -ne '') {
    if ($NoArchive) { throw '-LibraryDir files the .7z archive into the library, so it cannot be combined with -NoArchive' }
    $script:LibraryDirFull = Resolve-FullPath $LibraryDir
    if (-not (Test-Path -LiteralPath $script:LibraryDirFull -PathType Container)) { throw "LibraryDir not found: $LibraryDir ($($script:LibraryDirFull)); create the library root first" }
    if (Test-ReparsePoint $script:LibraryDirFull) { throw "LibraryDir is a link/junction: $($script:LibraryDirFull)" }
    $modRoot = Join-Path $script:LibraryDirFull 'modfolders'
    $script:LibVersionDir = Join-Path $modRoot $Version
    $script:LibPartialDir = Join-Path $modRoot ".$Version.partial"
    $script:LibOldDir     = Join-Path $modRoot ".$Version.old"
    $script:LibDistDir    = Join-Path $script:LibraryDirFull 'dist'
    $script:LibDocsDir    = Join-Path $script:LibraryDirFull 'docs'
    $script:LibReportPath = Join-Path $script:LibDocsDir "build-$Version.txt"
    $script:LibArchivePaths = @(Join-Path $script:LibDistDir ($script:PackageName + '.7z'))
    if ($IncludeSources) { $script:LibArchivePaths += (Join-Path $script:LibDistDir ($script:SourcesName + '.7z')) }
    foreach ($sub in @($modRoot, $script:LibVersionDir, $script:LibDistDir, $script:LibDocsDir)) {
        if ((Test-Path -LiteralPath $sub) -and (Test-ReparsePoint $sub)) { throw "Library folder is a link/junction: $sub" }
    }
    $targets = @($script:LibVersionDir, $script:LibPartialDir, $script:LibOldDir, $script:LibDistDir, $script:LibDocsDir)
    $inputs = @(@('the @LoBo folder', $script:LoBoDirFull), @('ExtraDir', $script:ExtraDirFull), @('ExtrasDir', $script:ExtrasDirFull),
                @('CacheDir', $script:CacheDirFull), @("OutDir\stage", $script:StageBase), @("OutDir\_work", $script:WorkBase))
    foreach ($t in $targets) {
        foreach ($pair in $inputs) {
            if ($pair[1] -eq '') { continue }
            if ((Test-PathUnder $pair[1] $t) -or (Test-PathUnder $t $pair[1])) {
                throw "LibraryDir target $t must not be on, under or above $($pair[0]) ($($pair[1]))"
            }
        }
    }
    if ([IO.Path]::GetFullPath($script:LibDistDir).TrimEnd('\') -ieq $script:OutDirFull.TrimEnd('\')) {
        throw "LibraryDir\dist is OutDir itself ($($script:OutDirFull)); the archives would be copied onto themselves"
    }
}

# KnownIssuesFile: the KNOWN ISSUES list, or the built-in one.
$script:KnownIssuesSource = ''
$script:KnownIssuesFromFile = @()
if ($KnownIssuesFile -ne '') {
    $kiFull = Resolve-FullPath $KnownIssuesFile
    if (Test-Path -LiteralPath $kiFull -PathType Leaf) {
        $script:KnownIssuesFromFile = @(Read-KnownIssuesFile $kiFull)
        $script:KnownIssuesSource = "$kiFull (sha256 $((Get-Sha256 $kiFull).ToLowerInvariant()))"
    } else {
        Write-Warn "KnownIssuesFile not found: $KnownIssuesFile ($kiFull); using the built-in known-issues list"
    }
}

# Tools: PoseidonTools, python, repair_lobo.py, lobo_cfgpatches.py, 7z, ssh.
if ($Tools -eq '') { $Tools = Get-DefaultToolsPath $PSScriptRoot }
$script:ToolsFull = Resolve-FullPath $Tools
if (-not (Test-Path -LiteralPath $script:ToolsFull -PathType Leaf)) { throw "PoseidonTools not found: $Tools ($($script:ToolsFull)) (build it, or pass -Tools)" }
$pyCmd = Get-Command $Python -ErrorAction SilentlyContinue
if (-not $pyCmd) { throw "Python interpreter not found: '$Python' (pass -Python)" }
$script:PythonExe = if ($pyCmd.PSObject.Properties['Source'] -and $pyCmd.Source) { $pyCmd.Source } else { $Python }
if ($RepairTool -eq '') {
    $candidate = Get-DefaultRepairToolPath $PSScriptRoot
    if (-not (Test-Path -LiteralPath $candidate -PathType Leaf)) {
        throw "repair_lobo.py not found at its default location $candidate; pass -RepairTool <path to repair_lobo.py>"
    }
    $RepairTool = $candidate
}
$script:RepairToolFull = Resolve-FullPath $RepairTool
if (-not (Test-Path -LiteralPath $script:RepairToolFull -PathType Leaf)) { throw "repair tool not found: $RepairTool ($($script:RepairToolFull)) (pass -RepairTool)" }
$script:CfgPatchesHelperFull = [IO.Path]::GetFullPath($script:CfgPatchesHelper)
if ($script:ExtrasDirFull -ne '' -and -not (Test-Path -LiteralPath $script:CfgPatchesHelperFull -PathType Leaf)) { throw "lobo_cfgpatches.py not found next to this script: $($script:CfgPatchesHelperFull)" }
$script:ExtrasBuilderFull = [IO.Path]::GetFullPath($script:ExtrasBuilder)
$script:StockGameDirFull = ''
if ($script:ExtrasDirFull -ne '') {
    if (-not (Test-Path -LiteralPath $script:ExtrasBuilderFull -PathType Leaf)) { throw "build-lobo-extras.py not found next to this script: $($script:ExtrasBuilderFull)" }
    # The extras class check needs the stock config: without its access locks,
    # stubs that the stock config refuses to rebase would read as clashes.
    $stockDir = if ($StockGameDir -ne '') { $StockGameDir } else { $script:DefaultStockGameDir }
    $script:StockGameDirFull = Resolve-FullPath $stockDir
    $binDir = Find-ChildPath $script:StockGameDirFull 'BIN' -Directory
    if (-not $binDir -or -not (Find-ChildPath $binDir 'CONFIG.BIN') -or -not (Find-ChildPath $script:StockGameDirFull 'AddOns' -Directory)) {
        $how = if ($StockGameDir -ne '') { 'StockGameDir' } else { "the default stock game folder (pass -StockGameDir)" }
        throw "The @LoBo_Extras class check needs a stock game folder with BIN\CONFIG.BIN and AddOns; not found in $how`: $($script:StockGameDirFull)"
    }
} elseif ($StockGameDir -ne '') {
    Write-Warn '-StockGameDir is only used with -ExtrasDir; ignored'
}
$script:SevenZipFull = Resolve-FullPath $SevenZip
if (-not $NoArchive -and -not (Test-Path -LiteralPath $script:SevenZipFull -PathType Leaf)) { throw "7z.exe not found: $SevenZip ($($script:SevenZipFull)) (install 7-Zip, pass -SevenZip, or use -NoArchive)" }
if ($IncludeSources -and -not (Test-Path -LiteralPath $script:AssemblyNotes)) { throw "assembly notes not found: $($script:AssemblyNotes)" }
if (-not $SkipFetch) { $script:SshExe = Get-SshExe } else { $script:SshExe = '' }

# The builder's own commit, for the build report (best effort).
$script:BuilderCommit = 'unknown (git not available)'
$gitCmd = Get-Command git -ErrorAction SilentlyContinue
if ($gitCmd) {
    try {
        $rev = Invoke-Capture -FilePath $gitCmd.Source -Arguments @('-C', $PSScriptRoot, 'rev-parse', 'HEAD')
        if ($rev.ExitCode -eq 0 -and $rev.StdOut.Trim() -ne '') {
            $script:BuilderCommit = $rev.StdOut.Trim()
            $dirty = Invoke-Capture -FilePath $gitCmd.Source -Arguments @('-C', $PSScriptRoot, 'status', '--porcelain', '--', '.')
            if ($dirty.ExitCode -eq 0 -and $dirty.StdOut.Trim() -ne '') { $script:BuilderCommit += ' (with uncommitted changes in arma_CWR/tools/lobo)' }
        }
    } catch { $script:BuilderCommit = 'unknown (git failed)' }
}

Write-Info "source @LoBo : $($script:LoBoDirFull)"
Write-Info "output folder: $($script:OutDirFull)"
Write-Info "version      : $Version"
Write-Info "package      : $($script:PackageName)"
Write-Info "cache        : $($script:CacheDirFull)"
Write-Info "reports      : $($script:ReportDir)"
if ($script:ExtraDirFull -ne '') { Write-Info "extra dir    : $($script:ExtraDirFull)" }
if ($script:ExtrasDirFull -ne '') { Write-Info "extras dir   : $($script:ExtrasDirFull) -> $($script:ExtrasName)" }
if ($script:StockGameDirFull -ne '') { Write-Info "stock game   : $($script:StockGameDirFull) (for the $($script:ExtrasName) class check)" }
if ($script:LibraryDirFull -ne '') { Write-Info "library      : $($script:LibraryDirFull)" }
if ($script:KnownIssuesSource -ne '') { Write-Info "known issues : $($script:KnownIssuesSource), $($script:KnownIssuesFromFile.Count) lines" } else { Write-Info 'known issues : built-in list' }
Write-Info "tools        : $($script:ToolsFull)"
Write-Info "python       : $($script:PythonExe)"
Write-Info "repair tool  : $($script:RepairToolFull)"
Write-Info "builder      : $($script:BuilderCommit)"
if (-not $NoArchive) { Write-Info "7-Zip        : $($script:SevenZipFull)" }

# 1. Top-level items of @LoBo (and ExtraDir): cheap, fails fast on a layout
#    surprise before the slower content checks.
# (@() keeps a one-item plan an array; PowerShell unrolls a single-element return.)
$script:SourcePlan = @(Get-SourcePlan -Root $script:LoBoDirFull)
$script:ExtraPlan  = @()
if ($script:ExtraDirFull -ne '') { $script:ExtraPlan = @(Get-SourcePlan -Root $script:ExtraDirFull -IsExtra) }
# ExtrasDir layout, and its bytes pinned (every file hashed now; the staged
# copy and the source at the end of the build must match these hashes).
$script:Extras = $null
$script:ExtrasManifest = $null
$script:ExtrasClassCheck = $null
if ($script:ExtrasDirFull -ne '') {
    $script:Extras = Get-ExtrasPlan $script:ExtrasDirFull
    $missionPbos = @($script:Extras.AllPbos | Where-Object { $_.Relative -match '^(?i)missions\\[^\\]+\.pbo$' })
    $script:Extras | Add-Member -NotePropertyName MissionPbos -NotePropertyValue $missionPbos
    $script:Extras | Add-Member -NotePropertyName MissionWorlds -NotePropertyValue @(Get-MissionWorlds (Join-Path $script:ExtrasDirFull 'Missions'))
    Write-Info ('@LoBo_Extras : {0} files, {1} addon pbos, {2} missions, {3}; no Bin folder, links, junk or git/backup folders' -f $script:Extras.Files.Count, $script:Extras.AddonPbos.Count, $script:Extras.MissionPbos.Count, (Format-Bytes $script:Extras.Bytes))
    $script:ExtrasManifest = Get-ExtrasManifest $script:ExtrasDirFull
    if ($script:ExtrasManifest.Found -and $script:ExtrasManifest.Tool -eq 'build-lobo-extras') {
        Write-Info ("@LoBo_Extras : built by build-lobo-extras.py {0}; its own class check: {1}" -f $script:ExtrasManifest.BuildVersion, $(if ($script:ExtrasManifest.Checked) { 'against ' + ($script:ExtrasManifest.Against -join ', ') + $(if ($script:ExtrasManifest.StockGame -ne '') { ' and the stock game' } else { '' }) } else { 'not run' }))
    }
    if ($script:ExtrasManifest.Problem -ne '') {
        Write-Warn "@LoBo_Extras: $($script:ExtrasManifest.Problem). The class check after staging still covers it; rebuild it with build-lobo-extras.py --against the staged modfolders to catch a clash before packaging."
    }
}
# The KNOWN ISSUES list: the file's, or the built-in one.
$hasShilka = ($null -ne $script:Extras) -and (@($script:Extras.AddonPbos | Where-Object { [IO.Path]::GetFileName($_.Relative) -ieq 'Vit_Iraq_Shilka.pbo' }).Count -gt 0)
$script:KnownIssueLines = @(if ($script:KnownIssuesFromFile.Count -gt 0) { $script:KnownIssuesFromFile } else { Get-DefaultKnownIssues -HasShilka $hasShilka })

$script:SourceAddonsDir = $null
$script:BinItem = $null
foreach ($item in $script:SourcePlan) {
    if ($item.Action -eq 'copy' -and $item.Kind -eq 'dir' -and $item.Name -ieq 'addons') { $script:SourceAddonsDir = $item.Full }
    if ($item.Action -eq 'bin') { $script:BinItem = $item }
}
if (-not $script:SourceAddonsDir) { throw "No addons folder in $($script:LoBoDirFull)" }
if (-not $script:BinItem) { throw "No Bin folder in $($script:LoBoDirFull)" }

# 2. The two doctor-repaired pbos must exist and must not be the stock bytes.
Write-Info 'doctor-repaired pbos:'
foreach ($name in ($script:StockHashes.Keys | Sort-Object)) {
    $path = Find-ChildPath $script:SourceAddonsDir $name
    if (-not $path) { throw "Repaired file missing: addons\$name" }
    $actual = Get-Sha256 $path
    if ($actual -ieq $script:StockHashes[$name]) {
        throw "addons\$name is the STOCK (unrepaired) file ($actual). Run 'PoseidonTools mod doctor <LoBoDir> --fix' (tools/lobo/fix-lobo-scope.ps1) first."
    }
    Write-Info "   ok  addons\$name  $actual (not stock)"
}

# 3. Bin\config.cpp with the GENERATED banner.
$script:ConfigCpp = $null
foreach ($b in $script:BinItem.Bin) { if ($b.Action -eq 'route') { $script:ConfigCpp = $b.Full } }
if (-not $script:ConfigCpp) { throw "Bin\config.cpp not found under $($script:BinItem.Full); run tools/lobo/install-lobo-factions.ps1 first" }
$firstLine = ''
$reader = New-Object IO.StreamReader($script:ConfigCpp)
try { $firstLine = $reader.ReadLine() } finally { $reader.Dispose() }
if ($null -eq $firstLine -or -not $firstLine.StartsWith($script:Banner)) {
    throw "Bin\config.cpp does not start with the banner '$($script:Banner)'; refusing to package an unknown config (first line: '$firstLine')"
}
Write-Info "config.cpp   : $($script:ConfigCpp) (GENERATED banner ok) -> @LoBo_UD\bin\config.cpp"

# 4. Repair set A is complete: the doctor has nothing left to do.
$script:LastDoctorText = ''
$doctorSourceCount = Invoke-ModDoctor -ModDir $script:LoBoDirFull -Label 'source @LoBo'
$script:DoctorSourceCount = $doctorSourceCount
$script:DoctorSourceText = $script:LastDoctorText

# 5. Repair set B is complete in the UD profile: --check exits 0 and would
#    change nothing. The source addons are hashed before and after the
#    check: those hashes pin the exact bytes that passed, and everything
#    staged or overlaid later is compared against them.
$script:PreflightHashes = Get-AddonPboHashes $script:SourceAddonsDir
Write-Info "hashed $($script:PreflightHashes.Count) source pbos before the UD check"
$udCheck = Invoke-RepairTool -ModDir $script:LoBoDirFull -Compatibility ud -Mode check -Label 'source @LoBo'
if ($udCheck.ExitCode -ne 0) {
    $hint = if ($udCheck.ExitCode -eq 1) { 'it would still change members: the issue-69 repair is not applied in the UD profile on the source' } else { 'incompatible input or I/O failure' }
    throw "repair_lobo.py --compatibility ud --check on the source exited $($udCheck.ExitCode) ($hint)"
}
Register-RepairIdentity $udCheck.Result 'source UD check'
$udChanged = Get-JsonField $udCheck.Result 'changed_members'
if ($null -ne $udChanged -and [int]$udChanged -ne 0) { throw "repair_lobo.py --check exited 0 but reports changed_members $udChanged on the source" }
$afterCheck = Get-AddonPboHashes $script:SourceAddonsDir
if ($afterCheck.Count -ne $script:PreflightHashes.Count) { throw "The source addons folder changed while the UD check ran ($($script:PreflightHashes.Count) pbos before, $($afterCheck.Count) after). Rerun." }
Assert-HashesMatch $script:PreflightHashes $afterCheck @($afterCheck.Keys) 'source addons after the UD check'
Write-Info "content repair: UD profile verified on the source (repair_version $($script:RepairVersion), rule sha256 $($script:RuleSha256)); source pbo hashes unchanged across the check"

# 5b. The content repair history for UD-CHANGES.txt: every backup batch
#     manifest under .ud-backups\issue-69 (read-only).
$script:RepairBatches = @(Get-RepairBatches $script:LoBoDirFull)
$appliedBatches = @($script:RepairBatches | Where-Object { $_.State -eq 'applied' })
foreach ($b in $script:RepairBatches) {
    $line = '{0}: state {1}, repair_version {2}, profile {3}, {4} archives, {5} members' -f $b.Batch, $b.State, $(if ($b.RepairVersion -ne '') { $b.RepairVersion } else { '?' }), $(if ($b.Compatibility -ne '') { $b.Compatibility } else { '?' }), $b.ArchiveCount, $b.MemberCount
    if ($b.State -eq 'applied') { Write-Info "repair batch : $line" }
    else { Write-Warn "content repair batch not applied, left out of UD-CHANGES.txt: $line" }
}
if ($appliedBatches.Count -eq 0) { Write-Warn "no applied content repair batch under $($script:LoBoDirFull)\.ud-backups\issue-69; UD-CHANGES.txt will say so" }
else {
    # Not an error: a newer rule set that changes nothing on this mod passes
    # the check without an apply. Worth a look all the same.
    $newest = $appliedBatches[$appliedBatches.Count - 1]
    if ($newest.RepairVersion -ne $script:RepairVersion -or $newest.RuleSha256 -ine $script:RuleSha256) {
        Write-Warn "repair_lobo.py is repair_version $($script:RepairVersion) (rule sha256 $($script:RuleSha256)), but the newest batch applied on the source is $($newest.RepairVersion) (rule sha256 $($newest.RuleSha256)); the check passed, so the newer rules change nothing here"
    }
}

# 6. Enumerate what will be copied (sizes, junk, source pbo names).
$script:SourceBytes = [long]0
$script:SourceFiles = 0
$script:JunkSkipped = New-Object System.Collections.Generic.List[string]
$script:SourceAddonPbos = @{}       # name -> full path (case-insensitive keys)
$script:ClassicSwitchBytes = [long]0
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
                        if ($n -like '*.pbo' -and -not (Test-JunkName $n)) {
                            $script:SourceAddonPbos[$n] = $f
                            if ($script:ExpectedClassicSwitch -icontains $n) { $script:ClassicSwitchBytes += (New-Object IO.FileInfo($f)).Length }
                        }
                    }
                    $extra = ", $($script:SourceAddonPbos.Count) pbo"
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
if ($script:SourceAddonPbos.Count -eq 0) { throw 'No addons\*.pbo found in the source @LoBo' }
if ($script:SourceAddonPbos.Count -ne $script:PreflightHashes.Count) { throw "The source addons folder changed while preflight ran ($($script:PreflightHashes.Count) pbos hashed, $($script:SourceAddonPbos.Count) listed). Rerun." }
foreach ($n in $script:SourceAddonPbos.Keys) { if (-not $script:PreflightHashes.ContainsKey($n)) { throw "The source addons folder changed while preflight ran: $n appeared after the hashes were taken. Rerun." } }
if ($script:SourceAddonPbos.Count -ne $doctorSourceCount) {
    Write-Warn "the mod doctor scanned $doctorSourceCount pbo(s) but the source addons folder holds $($script:SourceAddonPbos.Count)"
}
foreach ($n in $script:ExpectedClassicSwitch) {
    if (-not $script:SourceAddonPbos.ContainsKey($n)) { Write-Warn "expected profile-switch archive $n is not in the source addons folder" }
}

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
                        if ($f.Relative -notlike '*\*' -and $f.Relative -like '*.pbo' -and -not $script:SourceAddonPbos.ContainsKey($f.Relative)) {
                            $eastingName = $false
                            foreach ($e in $script:EastingFiles) { if ($e -ieq ('addons/' + $f.Relative)) { $eastingName = $true } }
                            if (-not $eastingName) { $script:ExtraAddonPboNew++ }
                        }
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

# 6b. -IncludeSources prerequisites that live in the source @LoBo. Checked
#     here so a missing folder fails before the build, not after the 7z.
$script:OrigDir = $null
$script:OrigFiles = @()
$script:IssueDir = $null
$script:IssueBatches = @()
if ($IncludeSources) {
    $script:OrigDir = Find-ChildPath $script:LoBoDirFull '_ud-orig' -Directory
    if (-not $script:OrigDir) { throw "No _ud-orig folder under $($script:LoBoDirFull); the stock originals are required for -IncludeSources" }
    # (.ToArray(): @($genericList) throws 'Argument types do not match' on PS 5.1.)
    $script:OrigFiles = (Get-TreeEntries -Root $script:OrigDir -SkipJunk).Files.ToArray()
    if ($script:OrigFiles.Count -eq 0) { throw "$($script:OrigDir) is empty; the stock originals are required for -IncludeSources" }
    $backupsDir = Find-ChildPath $script:LoBoDirFull '.ud-backups' -Directory
    if ($backupsDir) { $script:IssueDir = Find-ChildPath $backupsDir 'issue-69' -Directory }
    if (-not $script:IssueDir) { throw "No .ud-backups\issue-69 folder under $($script:LoBoDirFull); the pre-repair originals are required for -IncludeSources" }
    $script:IssueBatches = @([IO.Directory]::GetDirectories($script:IssueDir) | Sort-Object)
    if ($script:IssueBatches.Count -eq 0) { throw "$($script:IssueDir) holds no backup batch; the pre-repair originals are required for -IncludeSources" }
    Write-Info "sources      : _ud-orig ($($script:OrigFiles.Count) files), .ud-backups\issue-69 ($($script:IssueBatches.Count) batch(es)), assembly notes present"
}

# 7. Existing archives.
foreach ($a in @($script:ArchivePath) + $(if ($IncludeSources) { @($script:SourcesArchivePath) } else { @() })) {
    if (-not $NoArchive -and (Test-Path -LiteralPath $a)) {
        if ($Force) { Write-Warn "$a exists and will be overwritten (-Force)" }
        elseif ($PlanOnly) { Write-Warn "$a exists; a real run would refuse without -Force" }
        else { throw "Archive already exists: $a (use -Force to overwrite, or pick another -Version)" }
    }
}
# 7b. Existing library targets (-LibraryDir).
if ($script:LibraryDirFull -ne '') {
    $libHits = @(Get-LibraryConflicts)
    foreach ($h in $libHits) {
        if ($Force) { Write-Warn "library: $h exists and will be replaced (-Force)" }
        elseif ($PlanOnly) { Write-Warn "library: $h exists; a real run would refuse without -Force" }
        else { throw "Library target already exists: $h (use -Force to replace this version's library entry, or pick another -Version)" }
    }
    foreach ($leftover in @($script:LibPartialDir, $script:LibOldDir)) {
        if (Test-Path -LiteralPath $leftover) { Write-Warn "library: leftover $leftover from an interrupted run will be removed" }
    }
    $staleSources = Join-Path $script:LibDistDir ($script:SourcesName + '.7z')
    if (-not $IncludeSources -and (Test-Path -LiteralPath $staleSources)) {
        Write-Warn "library: $staleSources is from an earlier build of this version and is left as it is (this build has no -IncludeSources)"
    }
}

# 8. Cache state and the server plan.
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
# 8b. The dependency archives against deps/SHA256SUMS: now when the cache is
#     complete (-SkipFetch), otherwise right after the fetch.
if ($IncludeSources) {
    if ($SkipFetch) { [void](Test-ArchivesAgainstSums $script:Inventory) }
    else { Write-Info 'archives     : deps/SHA256SUMS cross-check runs right after the fetch, before staging' }
}

# 8c. @LoBo_Extras against everything else: pbo file names now (from the
#     plan), the extras' own CfgPatches now (readable, no duplicates), and the
#     full CfgPatches cross-check here only for -PlanOnly (on the source side);
#     a real build runs it on the staged folders after staging.
if ($null -ne $script:Extras) {
    $otherPbos = @{}
    foreach ($item in $script:SourcePlan) {
        if ($item.Action -eq 'copy' -and $item.Kind -eq 'dir') {
            foreach ($f in $item.Entries.Files) {
                $leaf = [IO.Path]::GetFileName($f.Relative)
                if ($leaf -like '*.pbo') { $otherPbos[$leaf] = "@LoBo\$($item.Name)\$($f.Relative)" }
            }
        }
    }
    foreach ($e in $script:EastingFiles) {
        if ($e -like '*.pbo') { $otherPbos[[IO.Path]::GetFileName($e)] = '@LoBo\' + $e.Replace('/', '\') + ' (73 Easting)' }
    }
    foreach ($item in $script:ExtraPlan) {
        if ($item.Action -eq 'copy' -and $item.Kind -eq 'dir') {
            foreach ($f in $item.Entries.Files) {
                $leaf = [IO.Path]::GetFileName($f.Relative)
                if ($leaf -like '*.pbo') { $otherPbos[$leaf] = "@LoBo\$($item.Name)\$($f.Relative) (ExtraDir)" }
            }
        }
    }
    $depsNames = @()
    if ($script:Inventory) { $depsNames = @($script:Inventory.DepsAddons.ToArray() | ForEach-Object { [IO.Path]::GetFileName($_.Full) }) }
    elseif ($script:Manifest) { $depsNames = @($script:Manifest | Where-Object { $_.Role -eq 'depsaddon' } | ForEach-Object { $_.Path.Substring($_.Path.LastIndexOf('/') + 1) }) }
    foreach ($n in $depsNames) { $otherPbos[$n] = "@LoBo_Deps\Addons\$n" }
    Assert-NoPboNameClash $script:Extras.AllPbos $otherPbos 'preflight, against the planned @LoBo, @LoBo_Deps and @LoBo_UD'
    Write-Info "@LoBo_Extras : $($script:Extras.AllPbos.Count) pbo file names, none used by the $($otherPbos.Count) planned @LoBo/@LoBo_Deps pbos"

    $extrasAddonPaths = @($script:Extras.AddonPbos | ForEach-Object { $_.Full })
    if ($PlanOnly) {
        $groups = New-Object System.Collections.Generic.List[object]
        $groups.Add(@{ Label = '@LoBo\addons'; Paths = @($script:SourceAddonPbos.Values | ForEach-Object { [string]$_ } | Sort-Object) })
        $eastPaths = @()
        if ($script:Inventory) { $eastPaths = @($script:Inventory.Easting.ToArray() | Where-Object { $_.Relative -like 'addons/*.pbo' -and (Test-Path -LiteralPath $_.Full) } | ForEach-Object { $_.Full }) }
        $groups.Add(@{ Label = '@LoBo\addons (73 Easting)'; Paths = $eastPaths })
        $extraDirPaths = @()
        foreach ($item in $script:ExtraPlan) {
            if ($item.Action -eq 'copy' -and $item.Kind -eq 'dir' -and $item.Name -ieq 'addons') {
                $extraDirPaths = @($item.Entries.Files.ToArray() | Where-Object { $_.Relative -notlike '*\*' -and $_.Relative -like '*.pbo' } | ForEach-Object { $_.Full })
            }
        }
        $groups.Add(@{ Label = '@LoBo\addons (ExtraDir)'; Paths = $extraDirPaths })
        $depPaths = @()
        $depMissing = 0
        if ($script:Inventory) {
            foreach ($d in $script:Inventory.DepsAddons) { if (Test-Path -LiteralPath $d.Full) { $depPaths += $d.Full } else { $depMissing++ } }
        }
        $groups.Add(@{ Label = '@LoBo_Deps\Addons'; Paths = $depPaths })
        if ($depMissing -gt 0) { Write-Warn "$depMissing dependency pbo(s) are not cached yet; a real run checks them against the extras after the fetch" }
        [void](Assert-NoCfgPatchesClash $groups.ToArray() $extrasAddonPaths '@LoBo_Extras\addons' 'preflight, source side')
    } else {
        [void](Assert-NoCfgPatchesClash @() $extrasAddonPaths '@LoBo_Extras\addons' 'preflight, extras only')
        Write-Info '@LoBo_Extras : the CfgPatches cross-check against the staged folders runs after staging'
    }
}

# 9. Free space, roughly: source and extras twice (stage + archive), the
#    cache for whatever is not downloaded yet, and the profile-switch set
#    twice more (the @LoBo_UD overlay and the classic apply's backup batch).
#    With -LibraryDir the library copy needs the stage and the archive once
#    more, on the library's drive.
$extrasBytes = [long]0
if ($script:Inventory) { $extrasBytes = Get-InventoryBytes $script:Inventory }
if ($extrasBytes -eq 0) { $extrasBytes = $script:EstimatedDepsBytes + $(if ($IncludeSources) { $script:EstimatedArchivesBytes } else { 0 }) }
$extrasModBytes = [long]0
if ($null -ne $script:Extras) { $extrasModBytes = $script:Extras.Bytes }
$needed = 2 * ($script:SourceBytes + $extrasBytes + $extrasModBytes) + $(if ($SkipFetch) { 0 } else { $extrasBytes }) + 2 * $script:ClassicSwitchBytes
$libNeeded = [long]0
if ($script:LibraryDirFull -ne '') { $libNeeded = 2 * ($script:SourceBytes + $extrasBytes + $extrasModBytes) }
$spaceChecks = @(, @($script:OutDirFull, $needed))
if ($libNeeded -gt 0) {
    if ([IO.Path]::GetPathRoot($script:LibraryDirFull) -ieq [IO.Path]::GetPathRoot($script:OutDirFull)) { $spaceChecks = @(, @($script:OutDirFull, ($needed + $libNeeded))) }
    else { $spaceChecks += , @($script:LibraryDirFull, $libNeeded) }
}
foreach ($sc in $spaceChecks) {
    $free = -1
    $driveRoot = ''
    try {
        $driveRoot = [IO.Path]::GetPathRoot($sc[0])
        $free = (New-Object IO.DriveInfo($driveRoot)).AvailableFreeSpace
    } catch {
        Write-Warn "could not determine free space for $($sc[0]): $($_.Exception.Message)"
    }
    if ($free -ge 0) {
        Write-Info "disk space   : $driveRoot need about $(Format-Bytes $sc[1]), free $(Format-Bytes $free)"
        if ($free -lt $sc[1]) { throw "Not enough free space on $driveRoot`: need about $(Format-Bytes $sc[1]), have $(Format-Bytes $free)" }
    }
}

# ---------------------------------------------------------------------------
# Plan
# ---------------------------------------------------------------------------
Write-Step 'Plan'
$eastingAddonNew = 0
foreach ($e in $script:EastingFiles) {
    if ($e -like 'addons/*.pbo' -and -not $script:SourceAddonPbos.ContainsKey([IO.Path]::GetFileName($e))) { $eastingAddonNew++ }
}
Write-Info "staging      : $($script:StageRoot)"
Write-Info "classic apply: repair_lobo.py --compatibility classic --apply on the STAGED @LoBo only (source is never a target)"
Write-Info "overlay      : changed pbos (expected $($script:ExpectedClassicSwitch.Count): $($script:ExpectedClassicSwitch -join ', ')) copied from the source into @LoBo_UD\addons, checked against the preflight hashes"
Write-Info "backups      : classic apply batch moved to $(Join-Path $script:WorkRoot 'classic-apply-backups')"
Write-Info "composite    : staged Classic addons + overlay (hard links under $(Join-Path $script:WorkRoot 'ud-composite')) must pass --compatibility ud --check"
Write-Info "mod doctor   : --fix on the STAGED @LoBo_Deps$(if ($null -ne $script:Extras) { ' and @LoBo_Extras' }) only, originals moved to $(Join-Path $script:WorkRoot 'doctor-orig'); then a report-only run must say 'Nothing to do' for every staged modfolder before SHA256SUMS.txt"
if ($NoArchive) { Write-Info 'archive      : none (-NoArchive)' } else { Write-Info "archive      : $($script:ArchivePath) (+ .sha256)" }
if ($IncludeSources) {
    Write-Info "sources      : $($script:SourcesRoot)"
    if (-not $NoArchive) { Write-Info "               $($script:SourcesArchivePath) (+ .sha256)" }
}
Write-Info "expected @LoBo\addons pbo count after staging: $($script:SourceAddonPbos.Count) + $eastingAddonNew (73 Easting) + $($script:ExtraAddonPboNew) (ExtraDir)"
if ($null -ne $script:Extras) {
    Write-Info "@LoBo_Extras : $($script:Extras.Files.Count) files copied verbatim to $(Join-Path $script:StageRoot $script:ExtrasName), then pbo names, CfgPatches and config classes checked against the staged @LoBo, @LoBo_Deps and @LoBo_UD (classes with the stock game config, by build-lobo-extras.py --check)"
} else {
    Write-Info '@LoBo_Extras : none (no -ExtrasDir)'
}
Write-Info ("launch lines : official Remastered and Uslu dur! first, legacy 1.99 last; order {0}@LoBo;@LoBo_Deps[;@LoBo_UD]" -f $(if ($null -ne $script:Extras) { '@LoBo_Extras;' } else { '' }))
Write-Info "report       : $(Join-Path $script:ReportDir 'build-report.txt')"
if ($script:LibraryDirFull -ne '') {
    Write-Info "library      : $($script:LibVersionDir)"
    foreach ($a in $script:LibArchivePaths) { Write-Info "               $a (+ .sha256)" }
    Write-Info "               $($script:LibReportPath)"
}
if ($PlanOnly) {
    Write-Host ''
    Write-Host 'PlanOnly: nothing written.' -ForegroundColor Green
    exit 0
}

# ---------------------------------------------------------------------------
# Reports from preflight (first writes; preflight has passed)
# ---------------------------------------------------------------------------
[void][IO.Directory]::CreateDirectory($script:ReportDir)
Write-ReportText (Join-Path $script:ReportDir 'ud-check-source.json') $udCheck.StdOut
Write-ReportText (Join-Path $script:ReportDir 'doctor-source.txt') $script:DoctorSourceText

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
    if ($IncludeSources) { [void](Test-ArchivesAgainstSums $script:Inventory) }
}

# ---------------------------------------------------------------------------
# Stage
# ---------------------------------------------------------------------------
Write-Step 'Stage'
[void][IO.Directory]::CreateDirectory($script:StageBase)
[void][IO.Directory]::CreateDirectory($script:WorkBase)
Remove-OwnedFolder $script:StageRoot
[void][IO.Directory]::CreateDirectory($script:StageRoot)
$stageLoBo = Join-Path $script:StageRoot '@LoBo'
$stageDeps = Join-Path $script:StageRoot '@LoBo_Deps'
$stageUd   = Join-Path $script:StageRoot '@LoBo_UD'
[void][IO.Directory]::CreateDirectory($stageLoBo)
[void][IO.Directory]::CreateDirectory($stageUd)

# Allowlisted @LoBo content (config.cpp is routed later, after the Classic
# switch, so nothing but the mod proper is under the staged @LoBo while
# repair_lobo.py runs on it).
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
                if ($b.Action -ne 'copy') { continue }
                if ($b.Kind -eq 'dir') { Copy-TreeEntries $b.Entries $b.Full (Join-Path $stageBin $b.Name) }
                else { Copy-FileTo $b.Full (Join-Path $stageBin $b.Name) }
                Write-Info "copied $($item.Name)\$($b.Name)"
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

# ExtraDir, merged into @LoBo before the Classic switch so its pbos are
# checked and switched like everything else.
$script:ExtraAddedNames = New-Object System.Collections.Generic.List[string]
if ($script:ExtraPlan.Count -gt 0) {
    foreach ($item in $script:ExtraPlan) {
        if ($item.Action -ne 'copy') { continue }
        if ($item.Kind -eq 'dir') {
            foreach ($d in $item.Entries.Dirs) { [void][IO.Directory]::CreateDirectory((Join-Path (Join-Path $stageLoBo $item.Name) $d)) }
            foreach ($f in $item.Entries.Files) {
                $rel = $item.Name + '\' + $f.Relative
                if ((Merge-FileInto -SourceFile $f.Full -DestRoot $stageLoBo -Relative $rel -Origin 'ExtraDir') -eq 'added') { $script:ExtraAddedNames.Add($rel) }
            }
        } else {
            if ((Merge-FileInto -SourceFile $item.Full -DestRoot $stageLoBo -Relative $item.Name -Origin 'ExtraDir') -eq 'added') { $script:ExtraAddedNames.Add($item.Name) }
        }
    }
    Write-Info "merged ExtraDir: $($script:ExtraAddedNames.Count) files"
}

$stagedAddonsDir = Find-ChildPath $stageLoBo 'addons' -Directory
if (-not $stagedAddonsDir) { throw 'Internal: staged @LoBo has no addons folder' }

# ---------------------------------------------------------------------------
# Classic switch on the staged copy
# ---------------------------------------------------------------------------
Write-Step 'Classic profile switch (staged @LoBo only)'
$hashesBefore = Get-AddonPboHashes $stagedAddonsDir
Write-Info "hashed $($hashesBefore.Count) staged pbos before the switch"
# The staged copy of every source pbo must be the bytes the preflight UD
# check passed; 73 Easting and ExtraDir pbos have no preflight entry.
Assert-HashesMatch $script:PreflightHashes $hashesBefore @($script:SourceAddonPbos.Keys) 'staged @LoBo\addons before the switch'
Write-Info "staged copies of all $($script:SourceAddonPbos.Count) source pbos match their preflight hashes"

$classicApply = Invoke-RepairTool -ModDir $stageLoBo -Compatibility classic -Mode apply -Report (Join-Path $script:ReportDir 'classic-apply.json') -Label 'staged @LoBo'
if ($classicApply.ExitCode -ne 0) { throw "repair_lobo.py --compatibility classic --apply on the staged @LoBo exited $($classicApply.ExitCode)" }
if ($null -eq $classicApply.Result) { throw 'repair_lobo.py --apply printed no JSON summary' }
Assert-RepairIdentity $classicApply.Result 'classic apply'
$script:ClassicChangedMembers = Get-JsonField $classicApply.Result 'changed_members'
if ($null -eq $script:ClassicChangedMembers) { $script:ClassicChangedMembers = '?' }
$classicCheck = Invoke-RepairTool -ModDir $stageLoBo -Compatibility classic -Mode check -Report (Join-Path $script:ReportDir 'classic-check.json') -Label 'staged @LoBo'
if ($classicCheck.ExitCode -ne 0) {
    $hint = if ($classicCheck.ExitCode -eq 1) { 'the apply did not reach a fixed point' } else { 'incompatible input or I/O failure' }
    throw "repair_lobo.py --compatibility classic --check on the staged @LoBo exited $($classicCheck.ExitCode) ($hint)"
}
Assert-RepairIdentity $classicCheck.Result 'classic check'

$hashesAfter = Get-AddonPboHashes $stagedAddonsDir
foreach ($n in $hashesAfter.Keys) { if (-not $hashesBefore.ContainsKey($n)) { throw "The Classic switch ADDED a pbo: $n" } }
foreach ($n in $hashesBefore.Keys) { if (-not $hashesAfter.ContainsKey($n)) { throw "The Classic switch REMOVED a pbo: $n" } }
$changedNames = New-Object System.Collections.Generic.List[string]
foreach ($n in ($hashesBefore.Keys | Sort-Object)) {
    if ($hashesBefore[$n] -ine $hashesAfter[$n]) { $changedNames.Add($n) }
}
Write-Info "pbos rewritten by the switch: $($changedNames.Count)"

# The tool's own list of rewritten archives must agree with the hashes.
$reported = @()
$reportedRaw = Get-JsonField $classicApply.Result 'archives'
if ($null -ne $reportedRaw) { $reported = @($reportedRaw | ForEach-Object { [string]$_ }) }
$changedStems = @($changedNames | ForEach-Object { [IO.Path]::GetFileNameWithoutExtension($_) })
foreach ($s in $reported) { if (-not ($changedStems -icontains $s)) { throw "repair_lobo.py reports archive '$s' rewritten but its hash did not change" } }
foreach ($s in $changedStems) { if (-not ($reported -icontains $s)) { throw "Staged pbo '$s' changed but repair_lobo.py did not report it" } }

$expectedSet = $script:ExpectedClassicSwitch
$missingExp = @($expectedSet | Where-Object { -not ($changedNames -icontains $_) })
$extraExp   = @($changedNames | Where-Object { -not ($expectedSet -icontains $_) })
if ($extraExp.Count -gt 0) {
    throw "The profile switch rewrote archive(s) this script was not written for: $($extraExp -join ', '). Expected only: $($expectedSet -join ', '). Review the repair tool's changes and update ExpectedClassicSwitch before packaging."
}
if ($missingExp.Count -gt 0) {
    Write-Warn "the profile switch left $($missingExp.Count) of the $($expectedSet.Count) expected archives unchanged: $($missingExp -join ', ')"
}

# @LoBo_UD\addons: the UD-profile bytes come from the SOURCE, byte-identical,
# and must still be the bytes the preflight UD check passed.
$udAddonsDir = Join-Path $stageUd 'addons'
$script:Overlay = New-Object System.Collections.Generic.List[object]
if ($changedNames.Count -gt 0) {
    [void][IO.Directory]::CreateDirectory($udAddonsDir)
    foreach ($n in $changedNames) {
        if (-not $script:SourceAddonPbos.ContainsKey($n)) {
            throw "Staged pbo $n was rewritten by the Classic switch but has no counterpart in the source addons folder, so no UD-profile copy exists for @LoBo_UD"
        }
        $srcFile = [string]$script:SourceAddonPbos[$n]
        $srcName = [IO.Path]::GetFileName($srcFile)
        $srcHash = Get-Sha256 $srcFile
        if ($srcHash -ine $script:PreflightHashes[$n]) {
            throw "Source $srcName ($srcHash) no longer matches its preflight hash ($($script:PreflightHashes[$n])): the source @LoBo changed while this script ran, so its bytes cannot be trusted for @LoBo_UD. Rerun."
        }
        $dest = Join-Path $udAddonsDir $srcName
        Copy-FileTo $srcFile $dest
        $destHash = Get-Sha256 $dest
        if ($destHash -ine $srcHash) { throw "Overlay copy of $srcName is not byte-identical to the source ($destHash vs $srcHash)" }
        $script:Overlay.Add([pscustomobject]@{ Name = $srcName; UdHash = $srcHash; ClassicHash = $hashesAfter[$n] })
        Write-Info ('   overlay {0,-18} UD {1}  classic {2}' -f $srcName, $srcHash.ToLowerInvariant(), $hashesAfter[$n].ToLowerInvariant())
    }
}

# The classic apply's backup batch leaves the package and goes under _work.
$stagedBackups = Find-ChildPath $stageLoBo '.ud-backups' -Directory
$backupsOut = Join-Path $script:WorkRoot 'classic-apply-backups'
if ($stagedBackups) {
    [void][IO.Directory]::CreateDirectory($script:WorkRoot)
    Remove-OwnedFolder $backupsOut
    Move-Item -LiteralPath $stagedBackups -Destination $backupsOut
    Write-Info "moved the classic apply backups to $backupsOut"
} elseif ($changedNames.Count -gt 0) {
    throw 'The Classic switch rewrote pbos but left no .ud-backups batch under the staged @LoBo'
} else {
    Write-Warn 'the Classic switch changed nothing, so there is no backup batch'
}
if (Find-ChildPath $stageLoBo '.ud-backups' -Directory) { throw 'Internal: .ud-backups still present under the staged @LoBo' }
foreach ($d in [IO.Directory]::GetDirectories($stageLoBo)) {
    if ([IO.Path]::GetFileName($d) -like 'lobo-repair-*') { throw "repair_lobo.py left a temp folder behind: $d" }
}

# Repair set A must still hold on the switched copy.
$script:DoctorStagedCount = Invoke-ModDoctor -ModDir $stageLoBo -Label 'staged @LoBo' -SaveTo (Join-Path $script:ReportDir 'doctor-staged.txt')

# Composite check: what Uslu dur! will load is the staged Classic addons with
# the @LoBo_UD overlay laid over them. Built under _work from hard links (no
# bytes copied; a copy only where the file system refuses links) and it must
# pass the UD check exactly like the source did. This does not depend on the
# hash bookkeeping above.
Write-Step 'Composite UD check (staged @LoBo + @LoBo_UD overlay)'
$compositeRoot = Join-Path $script:WorkRoot 'ud-composite'
$compositeAddons = Join-Path $compositeRoot 'addons'
Remove-OwnedFolder $compositeRoot
[void][IO.Directory]::CreateDirectory($compositeAddons)
$linkCount = 0
$copyCount = 0
foreach ($f in [IO.Directory]::GetFiles($stagedAddonsDir)) {
    $n = [IO.Path]::GetFileName($f)
    if ($n -notlike '*.pbo') { continue }
    $from = $f
    if ($script:Overlay | Where-Object { $_.Name -ieq $n }) { $from = Join-Path $udAddonsDir $n }
    $how = New-HardLinkOrCopy $from (Join-Path $compositeAddons $n)
    if ($how -eq 'link') { $linkCount++ } else { $copyCount++ }
}
Write-Info "composite: $linkCount hard links, $copyCount copies under $compositeAddons ($($script:Overlay.Count) overlay pbos laid over the Classic set)"
$compositeCheck = Invoke-RepairTool -ModDir $compositeRoot -Compatibility ud -Mode check -Report (Join-Path $script:ReportDir 'ud-check-composite.json') -Label 'composite'
if ($compositeCheck.ExitCode -ne 0) {
    $hint = if ($compositeCheck.ExitCode -eq 1) { 'the staged Classic addons plus the @LoBo_UD overlay do not amount to the UD profile' } else { 'incompatible input or I/O failure' }
    throw "repair_lobo.py --compatibility ud --check on the composite exited $($compositeCheck.ExitCode) ($hint)"
}
Assert-RepairIdentity $compositeCheck.Result 'composite UD check'
$compChanged = Get-JsonField $compositeCheck.Result 'changed_members'
if ($null -ne $compChanged -and [int]$compChanged -ne 0) { throw "repair_lobo.py --check exited 0 but reports changed_members $compChanged on the composite" }
if (Find-ChildPath $compositeRoot '.ud-backups' -Directory) { throw 'repair_lobo.py --check wrote a backup batch into the composite; it must not write' }
# A check must not have touched the linked files.
$compHashes = Get-AddonPboHashes $compositeAddons
foreach ($n in $compHashes.Keys) {
    $want = $hashesAfter[$n]
    if ($script:Overlay | Where-Object { $_.Name -ieq $n }) { $want = $script:PreflightHashes[$n] }
    if ($compHashes[$n] -ine $want) { throw "Composite pbo $n changed during the UD check ($($compHashes[$n]) vs $want)" }
}
Write-Info 'composite passes the UD check with no changes; removing the composite tree'
Remove-OwnedFolder $compositeRoot

# ---------------------------------------------------------------------------
# @LoBo_Deps, @LoBo_UD\bin, notes
# ---------------------------------------------------------------------------
Write-Step 'Stage @LoBo_Deps and @LoBo_UD'
$stagedAddonNames = New-Object System.Collections.Generic.HashSet[string]([StringComparer]::OrdinalIgnoreCase)
foreach ($f in [IO.Directory]::GetFiles($stagedAddonsDir)) { [void]$stagedAddonNames.Add([IO.Path]::GetFileName($f)) }
if (Test-Path -LiteralPath $udAddonsDir) {
    foreach ($f in [IO.Directory]::GetFiles($udAddonsDir)) { [void]$stagedAddonNames.Add([IO.Path]::GetFileName($f)) }
}
$depsAddonsOut = Join-Path $stageDeps 'Addons'
[void][IO.Directory]::CreateDirectory($depsAddonsOut)
foreach ($p in $script:Inventory.DepsAddons) {
    $name = [IO.Path]::GetFileName($p.Full)
    if ($stagedAddonNames.Contains($name)) { throw "@LoBo_Deps pbo '$name' collides (case-insensitively) with a staged @LoBo\addons or @LoBo_UD\addons file; the engine would silently drop one of them" }
    Copy-FileTo $p.Full (Join-Path $depsAddonsOut $name)
}
Write-Info "staged @LoBo_Deps\Addons: $($script:Inventory.DepsAddons.Count) pbos"
# Repair set A on the dependencies: the official Remastered exe does not
# coerce a malformed float such as IRAR_EE9's tracerColor 0.0.1, it hands it
# to the script evaluator ("Script error at '0.0|#|.1'", fatal under
# --autotest). Only the staged copies are patched; the cache keeps the
# downloaded bytes. Runs before anything below reads the staged @LoBo_Deps.
foreach ($rec in @(Invoke-StagedDoctorFix -ModDir $stageDeps -Label '@LoBo_Deps')) { $script:DoctorFixes.Add($rec) }
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

# @LoBo_UD\bin\config.cpp, byte copy.
$udConfig = Join-Path (Join-Path $stageUd 'bin') 'config.cpp'
Copy-FileTo $script:ConfigCpp $udConfig
if ((Get-Sha256 $udConfig) -ine (Get-Sha256 $script:ConfigCpp)) { throw 'Internal: @LoBo_UD\bin\config.cpp is not byte-identical to the source' }
Write-TextFileCrlf (Join-Path $stageUd 'README.txt') (Get-UdReadmeLines -Overlay $script:Overlay.ToArray() -HasExtras ($null -ne $script:Extras))
Write-Info "staged @LoBo_UD: bin\config.cpp, addons\ ($($script:Overlay.Count) pbos), README.txt"

# ---------------------------------------------------------------------------
# @LoBo_Extras (-ExtrasDir): verbatim copy, then the cross-checks against
# the staged @LoBo, @LoBo_Deps and @LoBo_UD.
# ---------------------------------------------------------------------------
$stageExtras = Join-Path $script:StageRoot $script:ExtrasName
if ($null -ne $script:Extras) {
    Write-Step "Stage $($script:ExtrasName)"
    [void][IO.Directory]::CreateDirectory($stageExtras)
    foreach ($d in $script:Extras.Dirs) { [void][IO.Directory]::CreateDirectory((Join-Path $stageExtras $d)) }
    foreach ($f in $script:Extras.Files) {
        $dest = Join-Path $stageExtras $f.Relative
        Copy-FileTo $f.Full $dest
        $h = Get-Sha256 $dest
        if ($h -ine $f.Hash) {
            throw "ExtrasDir file $($f.Relative) is not the file preflight hashed (staged copy $h, preflight $($f.Hash)); ExtrasDir changed while this script ran. Rerun."
        }
    }
    Write-Info "copied $($script:Extras.Files.Count) files verbatim; every staged copy matches its preflight hash"

    # Repair set A on the staged extras (the ExtrasDir keeps its bytes). A pbo
    # it patches is the one file allowed to differ from its preflight hash in
    # Verify, and only by exactly these bytes.
    $script:ExtrasDoctorHashes = @{}
    foreach ($rec in @(Invoke-StagedDoctorFix -ModDir $stageExtras -Label $script:ExtrasName)) {
        $script:DoctorFixes.Add($rec)
        $script:ExtrasDoctorHashes[$rec.Relative] = $rec.NewHash
    }

    # pbo file names, any folder, against the three staged modfolders.
    $otherPbos = @{}
    foreach ($pair in @(@('@LoBo', $stageLoBo), @('@LoBo_Deps', $stageDeps), @('@LoBo_UD', $stageUd))) {
        foreach ($f in (Get-TreeEntries -Root $pair[1]).Files) {
            $leaf = [IO.Path]::GetFileName($f.Relative)
            if ($leaf -like '*.pbo') { $otherPbos[$leaf] = "$($pair[0])\$($f.Relative)" }
        }
    }
    Assert-NoPboNameClash $script:Extras.AllPbos $otherPbos 'staged folders'
    Write-Info "$($script:Extras.AllPbos.Count) pbo file names, none used by the $($otherPbos.Count) staged @LoBo/@LoBo_Deps/@LoBo_UD pbos"

    # CfgPatches of every addon pbo of the four staged modfolders.
    $groups = New-Object System.Collections.Generic.List[object]
    $groups.Add(@{ Label = '@LoBo\addons'; Paths = @([IO.Directory]::GetFiles($stagedAddonsDir) | Where-Object { [IO.Path]::GetFileName($_) -like '*.pbo' } | Sort-Object) })
    $groups.Add(@{ Label = '@LoBo_Deps\Addons'; Paths = @([IO.Directory]::GetFiles($depsAddonsOut) | Where-Object { [IO.Path]::GetFileName($_) -like '*.pbo' } | Sort-Object) })
    if (Test-Path -LiteralPath $udAddonsDir) {
        $groups.Add(@{ Label = '@LoBo_UD\addons'; Paths = @([IO.Directory]::GetFiles($udAddonsDir) | Where-Object { [IO.Path]::GetFileName($_) -like '*.pbo' } | Sort-Object) })
    }
    $stagedExtrasAddons = @($script:Extras.AddonPbos | ForEach-Object { Join-Path $stageExtras $_.Relative })
    [void](Assert-NoCfgPatchesClash $groups.ToArray() $stagedExtrasAddons '@LoBo_Extras\addons' 'staged folders')

    # Config classes, against the staged modfolders and the stock game config.
    $against = @($stageLoBo, $stageDeps)
    if (Test-Path -LiteralPath $udAddonsDir) { $against += $stageUd }
    $checkWork = Join-Path $script:WorkRoot 'extras-class-check'
    try {
        $script:ExtrasClassCheck = Invoke-ExtrasClassCheck -ExtrasRoot $stageExtras -Against $against -ReportPath (Join-Path $script:ReportDir 'extras-class-check.json') -WorkDir $checkWork
    } finally {
        if (Test-Path -LiteralPath $checkWork) { Remove-Item -LiteralPath $checkWork -Recurse -Force }
    }
    $ccWarnings = @(Get-JsonField $script:ExtrasClassCheck 'warnings' | Where-Object { $null -ne $_ })
    $ccDefects = @(Get-JsonField $script:ExtrasClassCheck 'config_defects' | Where-Object { $null -ne $_ })
    Write-Info ("class check: {0} extras pbo(s) against {1} config(s) of the staged modfolders and the stock game, no class clash; {2} reported overlap(s) in extras-class-check.json" -f (Get-JsonField $script:ExtrasClassCheck 'pbos'), (Get-JsonField $script:ExtrasClassCheck 'indexed_configs'), $ccWarnings.Count)
    foreach ($d in $ccDefects) { Write-Warn "@LoBo_Extras config defect (advice for the extras build): $d" }
    # (The mod doctor already ran on the staged extras with --fix, above; the
    # report-only gate in Verify covers them with the other modfolders.)
}

# Notes.
Write-TextFileCrlf (Join-Path $stageLoBo 'UD-CHANGES.txt') (Get-ChangesLines -Overlay $script:Overlay.ToArray() -ExtraMerged $script:ExtraAddedNames.ToArray() -StagedHashes $hashesAfter -Batches $script:RepairBatches -KnownIssues $script:KnownIssuesFromFile -DoctorFixes $script:DoctorFixes.ToArray() -HasExtras ($null -ne $script:Extras))
$stagedPboCount = $hashesAfter.Count
$missionWorlds = @(Get-MissionWorlds (Join-Path $stageLoBo 'Missions'))
Write-TextFileCrlf (Join-Path $script:StageRoot 'README-FIRST.txt') (Get-ReadmeFirstLines -LoBoPboCount $stagedPboCount -DepsPboCount $script:Inventory.DepsAddons.Count -UdPboCount $script:Overlay.Count -MissionWorlds $missionWorlds -Extras $script:Extras -KnownIssues $script:KnownIssueLines -DoctorFixes $script:DoctorFixes.ToArray())
Write-Info "wrote README-FIRST.txt (mission worlds: $($missionWorlds -join ', ')), @LoBo\UD-CHANGES.txt, @LoBo_UD\README.txt"

# ---------------------------------------------------------------------------
# Verify
# ---------------------------------------------------------------------------
Write-Step 'Verify'
$stagedLoBo = Get-TreeEntries -Root $stageLoBo
foreach ($d in $stagedLoBo.Dirs) {
    foreach ($seg in $d.Split('\')) {
        if ($script:ForbiddenInStage -icontains $seg -or $seg -like 'lobo-repair-*') { throw "Forbidden folder in staged @LoBo: $d" }
    }
}
foreach ($f in $stagedLoBo.Files) {
    foreach ($seg in $f.Relative.Split('\')) {
        if ($script:ForbiddenInStage -icontains $seg) { throw "Forbidden file in staged @LoBo: $($f.Relative)" }
    }
    if (Test-JunkName ([IO.Path]::GetFileName($f.Full))) { throw "Runtime junk in staged @LoBo: $($f.Relative)" }
}
Write-Info "no _ud-orig, .git, tools, .ud-backups, config.cpp or runtime junk under staged @LoBo ($($stagedLoBo.Files.Count) files checked)"

$expectedPbo = $script:SourceAddonPbos.Count + $eastingAddonNew + $script:ExtraAddonPboNew
$finalHashes = Get-AddonPboHashes $stagedAddonsDir
if ($finalHashes.Count -ne $expectedPbo) { throw "Staged @LoBo\addons holds $($finalHashes.Count) pbos, expected $expectedPbo (source $($script:SourceAddonPbos.Count) + 73 Easting $eastingAddonNew + ExtraDir $($script:ExtraAddonPboNew))" }
Write-Info "@LoBo\addons pbo count: $($finalHashes.Count) = $($script:SourceAddonPbos.Count) + $eastingAddonNew + $($script:ExtraAddonPboNew)"
foreach ($n in $finalHashes.Keys) {
    if ($finalHashes[$n] -ine $hashesAfter[$n]) { throw "Staged pbo $n changed after the Classic switch" }
}

# Every @LoBo_UD pbo must equal the source pbo of the same name as the
# preflight UD check saw it (the preflight hash) and as it is now (a source
# that changed under the build is a failure even if the package is right).
$udHashes = @{}
if (Test-Path -LiteralPath $udAddonsDir) { $udHashes = Get-AddonPboHashes $udAddonsDir }
if ($udHashes.Count -ne $script:Overlay.Count) { throw "Internal: @LoBo_UD\addons holds $($udHashes.Count) pbos, overlay list has $($script:Overlay.Count)" }
foreach ($n in $udHashes.Keys) {
    if (-not $script:SourceAddonPbos.ContainsKey($n)) { throw "Internal: @LoBo_UD\addons\$n has no source counterpart" }
    if ($udHashes[$n] -ine $script:PreflightHashes[$n]) { throw "@LoBo_UD\addons\$n ($($udHashes[$n])) is not the source pbo the preflight UD check passed ($($script:PreflightHashes[$n]))" }
    $srcHash = Get-Sha256 ([string]$script:SourceAddonPbos[$n])
    if ($udHashes[$n] -ine $srcHash) { throw "@LoBo_UD\addons\$n ($($udHashes[$n])) is not byte-identical to the source as it is now ($srcHash); the source @LoBo changed while this script ran. Rerun." }
    if (-not $finalHashes.ContainsKey($n)) { throw "@LoBo_UD\addons\$n has no Classic twin in the staged @LoBo\addons" }
    if ($finalHashes[$n] -ieq $udHashes[$n]) { throw "@LoBo_UD\addons\$n is identical to its staged @LoBo twin; the overlay would be pointless" }
}
Write-Info "@LoBo_UD\addons: $($udHashes.Count) pbos byte-identical to the source (preflight hash and current bytes), each differing from its Classic twin"
# Unchanged source pbos in the staged @LoBo must still be the preflight bytes.
$unchangedNames = @($script:SourceAddonPbos.Keys | Where-Object { -not $udHashes.ContainsKey($_) })
Assert-HashesMatch $script:PreflightHashes $finalHashes $unchangedNames 'staged @LoBo\addons (pbos the switch left alone)'
Write-Info "$($unchangedNames.Count) pbos the switch left alone match their preflight hashes"

foreach ($name in $script:StockHashes.Keys) {
    foreach ($map in @($finalHashes, $udHashes)) {
        if ($map.ContainsKey($name) -and $map[$name] -ieq $script:StockHashes[$name]) { throw "Staged $name is the STOCK (unrepaired) file" }
    }
}
Write-Info 'stock LoBoWreck/LoBoPalObj bytes: absent from the package'

# @LoBo_Extras: exactly the preflight files with the preflight bytes (or,
# for a pbo the mod doctor patched, exactly the patched bytes), the
# ExtrasDir itself unchanged, and no Bin folder.
if ($null -ne $script:Extras) {
    $stagedExtras = Get-TreeEntries -Root $stageExtras
    if ($stagedExtras.Files.Count -ne $script:Extras.Files.Count) { throw "Staged $($script:ExtrasName) holds $($stagedExtras.Files.Count) files, ExtrasDir had $($script:Extras.Files.Count)" }
    $want = @{}
    foreach ($f in $script:Extras.Files) { $want[$f.Relative] = $f.Hash }
    foreach ($k in $script:ExtrasDoctorHashes.Keys) {
        if (-not $want.ContainsKey($k)) { throw "Internal: the mod doctor patched $($script:ExtrasName)\$k, which the ExtrasDir does not have" }
        $want[$k] = $script:ExtrasDoctorHashes[$k]
    }
    foreach ($f in $stagedExtras.Files) {
        if (-not $want.ContainsKey($f.Relative)) { throw "Staged $($script:ExtrasName) has a file ExtrasDir did not: $($f.Relative)" }
        if ((Get-Sha256 $f.Full) -ine $want[$f.Relative]) { throw "Staged $($script:ExtrasName)\$($f.Relative) changed after it was copied (and patched)" }
    }
    foreach ($f in $script:Extras.Files) {
        if (-not (Test-Path -LiteralPath $f.Full)) { throw "ExtrasDir file $($f.Relative) disappeared while this script ran. Rerun." }
        if ((Get-Sha256 $f.Full) -ine $f.Hash) { throw "ExtrasDir file $($f.Relative) changed while this script ran (it no longer matches its preflight hash). Rerun." }
    }
    if (Find-ChildPath $stageExtras 'Bin' -Directory) { throw "Internal: staged $($script:ExtrasName) has a Bin folder" }
    $patchedExtras = $script:ExtrasDoctorHashes.Count
    Write-Info "$($script:ExtrasName): $($stagedExtras.Files.Count) files, $($stagedExtras.Files.Count - $patchedExtras) byte-identical to the ExtrasDir (preflight hash and current bytes) and $patchedExtras patched by the mod doctor, no Bin folder"
}
$allowedTop = @('@LoBo', '@LoBo_Deps', '@LoBo_UD')
if ($null -ne $script:Extras) { $allowedTop += $script:ExtrasName }
foreach ($d in [IO.Directory]::GetDirectories($script:StageRoot)) {
    if (-not ($allowedTop -icontains [IO.Path]::GetFileName($d))) { throw "Internal: unexpected folder in the staged package: $d" }
}

# No mod doctor backups anywhere in the package (Invoke-StagedDoctorFix moved
# them to <WorkRoot>\doctor-orig), then the report-only gate: the doctor must
# have nothing left to do in any staged modfolder. Both run before the
# checksums are written.
foreach ($d in [IO.Directory]::GetDirectories($script:StageRoot)) {
    $t = Get-TreeEntries -Root $d
    foreach ($rel in (@($t.Dirs) + @($t.Files | ForEach-Object { $_.Relative }))) {
        if ($rel.Split('\') -icontains '_ud-orig') { throw "mod doctor backup in the staged package: $([IO.Path]::GetFileName($d))\$rel" }
    }
}
$gateFolders = New-Object System.Collections.Generic.List[object]
$gateFolders.Add(@('@LoBo', $stageLoBo))
$gateFolders.Add(@('@LoBo_Deps', $stageDeps))
if ($null -ne $script:Extras) { $gateFolders.Add(@($script:ExtrasName, $stageExtras)) }
$gateFolders.Add(@('@LoBo_UD', $stageUd))
$script:DoctorGate = @(Assert-StagedDoctorClean $gateFolders.ToArray())
Write-Info ('mod doctor gate: nothing to do in ' + (($script:DoctorGate | ForEach-Object { "$($_.Folder) ($($_.Pbos) pbos)" }) -join ', '))

$sumCount = Write-ChecksumFile $script:StageRoot
Write-Info "SHA256SUMS.txt: $sumCount entries"

Write-Host ''
Write-Host ('   {0,-18} {1,8} {2,28}' -f 'folder', 'files', 'bytes')
$totalFiles = 0
$totalBytes = [long]0
$totalPbos = 0
$folderStats = New-Object System.Collections.Generic.List[object]
foreach ($d in ([IO.Directory]::GetDirectories($script:StageRoot) | Sort-Object)) {
    $t = Get-TreeEntries -Root $d
    $b = [long]0
    $pbos = 0
    foreach ($f in $t.Files) { $b += $f.Length; if ($f.Relative -like '*.pbo') { $pbos++ } }
    $totalFiles += $t.Files.Count
    $totalBytes += $b
    $totalPbos += $pbos
    $folderStats.Add([pscustomobject]@{ Name = ([IO.Path]::GetFileName($d) + '\'); Files = $t.Files.Count; Pbos = $pbos; Bytes = $b })
    Write-Host ('   {0,-18} {1,8} {2,28}' -f ([IO.Path]::GetFileName($d) + '\'), $t.Files.Count, (Format-Bytes $b))
}
$rootFiles = @([IO.Directory]::GetFiles($script:StageRoot))
$rb = [long]0
foreach ($f in $rootFiles) { $rb += (New-Object IO.FileInfo($f)).Length }
$totalFiles += $rootFiles.Count
$totalBytes += $rb
$folderStats.Add([pscustomobject]@{ Name = '(root files)'; Files = $rootFiles.Count; Pbos = 0; Bytes = $rb })
$folderStats.Add([pscustomobject]@{ Name = 'TOTAL'; Files = $totalFiles; Pbos = $totalPbos; Bytes = $totalBytes })
Write-Host ('   {0,-18} {1,8} {2,28}' -f '(root files)', $rootFiles.Count, (Format-Bytes $rb))
Write-Host ('   {0,-18} {1,8} {2,28}' -f 'TOTAL', $totalFiles, (Format-Bytes $totalBytes))

# ---------------------------------------------------------------------------
# Archive
# ---------------------------------------------------------------------------
Write-Step 'Archive'
if ($NoArchive) {
    Write-Info 'skipped (-NoArchive)'
} else {
    New-SevenZipArchive -StageFolder $script:StageRoot -ArchivePath $script:ArchivePath
}

# ---------------------------------------------------------------------------
# Sources (optional)
# ---------------------------------------------------------------------------
if ($IncludeSources) {
    Write-Step 'Sources'
    # The archives were verified against deps/SHA256SUMS before staging
    # (preflight with -SkipFetch, else right after the fetch); _ud-orig and
    # the issue-69 batches were located in preflight.
    Remove-OwnedFolder $script:SourcesRoot
    [void][IO.Directory]::CreateDirectory($script:SourcesRoot)
    foreach ($a in $script:Inventory.Archives) {
        Copy-FileTo $a.Full (Join-Path $script:SourcesRoot ($a.Relative.Replace('/', '\')))
    }
    foreach ($p in $script:Inventory.DepsFixed) {
        if ($p.Relative -eq 'deps/@LoBo_Deps/POSSIBLY_UNNEEDED.txt') { continue }
        Copy-FileTo $p.Full (Join-Path $script:SourcesRoot ($p.Relative.Replace('/', '\')))
    }

    # Stock originals (pre set A) from _ud-orig.
    $origDir = $script:OrigDir
    $stockOut = Join-Path $script:SourcesRoot 'stock-originals'
    $stockNames = New-Object System.Collections.Generic.List[string]
    $stockNote = New-Object System.Collections.Generic.List[string]
    $stockNote.AddRange([string[]]@(
        'stock-originals: the @LoBo pbos exactly as the Lost Brothers released them',
        '==========================================================================',
        'UNREPAIRED. These are the files the mod doctor backed up to',
        '@LoBo\_ud-orig before rewriting the copies in @LoBo\addons (repair set A',
        'in @LoBo\UD-CHANGES.txt of the main package). They are kept for',
        'provenance only. Do not put them into a game folder: they carry the',
        'abstract-class and buried-wreck defects.',
        ''
    ))
    foreach ($f in (Get-TreeEntries -Root $origDir -SkipJunk).Files) {
        Copy-FileTo $f.Full (Join-Path $stockOut $f.Relative)
        $stockNames.Add($f.Relative)
        $stockNote.Add(('   {0}  {1}' -f (Get-Sha256 $f.Full).ToLowerInvariant(), $f.Relative.Replace('\', '/')))
    }
    if ($stockNames.Count -eq 0) { throw "$origDir is empty" }
    Write-TextFileCrlf (Join-Path $stockOut 'NOTE.txt') $stockNote.ToArray()

    # Pre-repair originals (pre set B, set A already in) from .ud-backups\issue-69.
    $issueDir = $script:IssueDir
    $batchOut = Join-Path (Join-Path $script:SourcesRoot 'pre-repair-originals') 'issue-69'
    $batchNames = New-Object System.Collections.Generic.List[string]
    $batchNote = New-Object System.Collections.Generic.List[string]
    $batchNote.AddRange([string[]]@(
        'pre-repair-originals\issue-69: what the content repair replaced',
        '================================================================',
        'PRE-REPAIR. Each batch folder holds the archives that repair_lobo.py',
        '(repair set B in @LoBo\UD-CHANGES.txt of the main package, UD',
        'profile) replaced on the build machine, exactly as they were just',
        'before that apply: repair set A (mod doctor) is already in them, set B',
        'is not. manifest.json records repair_version, rule sha256, profile and',
        'the before/after hashes. Do not put these pbos into a game folder.',
        ''
    ))
    $batchFiles = 0
    foreach ($batch in ([IO.Directory]::GetDirectories($issueDir) | Sort-Object)) {
        $batchName = [IO.Path]::GetFileName($batch)
        # Only batches whose manifest says applied hold real pre-repair
        # originals; a rolled-back batch duplicates the next one's backups.
        $batchManifest = Join-Path $batch 'manifest.json'
        if (-not (Test-Path -LiteralPath $batchManifest)) { throw "$batch has no manifest.json" }
        $batchState = (Get-Content -LiteralPath $batchManifest -Raw -Encoding UTF8 | ConvertFrom-Json).state
        if ($batchState -ne 'applied') {
            Write-Info "sources: skipping backup batch $batchName (state '$batchState', not applied)"
            continue
        }
        $batchNames.Add($batchName)
        $batchNote.Add($batchName)
        foreach ($f in (Get-TreeEntries -Root $batch -SkipJunk).Files) {
            Copy-FileTo $f.Full (Join-Path (Join-Path $batchOut $batchName) $f.Relative)
            $batchNote.Add(('   {0}  {1}' -f (Get-Sha256 $f.Full).ToLowerInvariant(), $f.Relative.Replace('\', '/')))
            $batchFiles++
        }
    }
    if ($batchNames.Count -eq 0) { throw "$issueDir holds no backup batch" }
    Write-TextFileCrlf (Join-Path $batchOut 'NOTE.txt') $batchNote.ToArray()

    Copy-FileTo $script:AssemblyNotes (Join-Path $script:SourcesRoot 'lobo-modfolder-assembly.txt')
    $installerCount = @($script:Inventory.Archives | Where-Object { [IO.Path]::GetFileName($_.Full) -like '*.exe' }).Count
    Write-TextFileCrlf (Join-Path $script:SourcesRoot 'NOTE.txt') (Get-SourcesNoteLines -ArchiveCount $script:Inventory.Archives.Count -InstallerCount $installerCount -StockNames $stockNames.ToArray() -BatchNames $batchNames.ToArray())
    $srcSums = Write-ChecksumFile $script:SourcesRoot
    Write-Info "staged $($script:SourcesRoot): $($script:Inventory.Archives.Count) archives, $($stockNames.Count) stock originals, $batchFiles pre-repair files in $($batchNames.Count) batch(es), SHA256SUMS.txt ($srcSums entries)"
    if (-not $NoArchive) {
        New-SevenZipArchive -StageFolder $script:SourcesRoot -ArchivePath $script:SourcesArchivePath
    }
}

# ---------------------------------------------------------------------------
# Build report
# ---------------------------------------------------------------------------
Write-Step 'Build report'
$builtArchives = @()
if (-not $NoArchive) {
    $builtArchives += $script:ArchivePath
    if ($IncludeSources) { $builtArchives += $script:SourcesArchivePath }
}
$reportPath = Join-Path $script:ReportDir 'build-report.txt'
Write-TextFileCrlf $reportPath (Get-BuildReportLines -FolderStats $folderStats.ToArray() -SumCount $sumCount -Archives $builtArchives)
Write-Info "wrote $reportPath"

# ---------------------------------------------------------------------------
# Library (-LibraryDir)
# ---------------------------------------------------------------------------
if ($script:LibraryDirFull -ne '') {
    Write-Step "Library $($script:LibraryDirFull)"
    # Nothing may have appeared since preflight.
    $libHits = @(Get-LibraryConflicts)
    if ($libHits.Count -gt 0 -and -not $Force) { throw ("Library target(s) appeared during the build: " + ($libHits -join ', ') + ' (use -Force to replace them)') }
    [void][IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($script:LibVersionDir))
    Remove-LibraryVersionFolder $script:LibPartialDir
    Remove-LibraryVersionFolder $script:LibOldDir
    # The staged package folder, verbatim, under a temporary name first.
    $stageEntries = Get-TreeEntries -Root $script:StageRoot
    Copy-TreeEntries $stageEntries $script:StageRoot $script:LibPartialDir
    $libSums = Join-Path $script:LibPartialDir 'SHA256SUMS.txt'
    if ((Get-Sha256 $libSums) -ine (Get-Sha256 (Join-Path $script:StageRoot 'SHA256SUMS.txt'))) { throw "Internal: the library copy of SHA256SUMS.txt differs from the staged one" }
    $verified = Test-PackageFolder $script:LibPartialDir
    if ($verified -ne $sumCount) { throw "Library copy verified $verified files, SHA256SUMS.txt has $sumCount entries" }
    if (Test-Path -LiteralPath $script:LibVersionDir) {
        Move-Item -LiteralPath $script:LibVersionDir -Destination $script:LibOldDir
    }
    Move-Item -LiteralPath $script:LibPartialDir -Destination $script:LibVersionDir
    Remove-LibraryVersionFolder $script:LibOldDir
    Write-Info "library: $($script:LibVersionDir) ($verified files verified against its SHA256SUMS.txt)"
    [void][IO.Directory]::CreateDirectory($script:LibDistDir)
    foreach ($a in $builtArchives) {
        $leaf = [IO.Path]::GetFileName($a)
        Publish-LibraryFile $a (Join-Path $script:LibDistDir $leaf)
        Publish-LibraryFile ($a + '.sha256') (Join-Path $script:LibDistDir ($leaf + '.sha256'))
    }
    [void][IO.Directory]::CreateDirectory($script:LibDocsDir)
    Publish-LibraryFile $reportPath $script:LibReportPath
}

Write-Host ''
Write-Host "Done: $($script:PackageName)" -ForegroundColor Green
exit 0
