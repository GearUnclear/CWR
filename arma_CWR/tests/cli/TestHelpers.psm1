#requires -Version 5.1
# TestHelpers.psm1 - shared helpers for the Pester smoke suite (tests/smoke/*.tests.ps1).
#
# Written against the call sites in tests/smoke/*.tests.ps1 (issue #49 part 2: this
# module was referenced by every smoke test's `Import-Module` but was never committed).
# Compatible with both Windows PowerShell 5.1 and PowerShell 7+ - no `??`, no ternary
# `?:`, no `&&`/`||` statement chaining, ASCII only.
#
# Conventions used throughout:
#   - "RepoRoot" means the arma_CWR engine checkout (the directory that holds
#     CMakePresets.json / .trident.env / dist/), not the outer D:\Arma_CWA git root.
#   - Functions that look for something optional (a missing exe, a missing data dir)
#     return $null instead of throwing, so callers can build a Skip reason string
#     ($script:Skip = if (-not $script:gameExe) { "PoseidonGame not found" } ... ).
#   - Functions that drive a live process/harness throw on timeout/failure, since the
#     calling `It` block is expected to fail loudly in that case.

Set-StrictMode -Version Latest

function Get-RepoRoot {
    <#
    .SYNOPSIS
        Walks up from a starting path to the arma_CWR engine root.
    .DESCRIPTION
        Climbs parent directories starting at -StartPath (or the directory containing
        it, if StartPath is itself a file) until it finds a directory containing
        CMakePresets.json - the marker file for the arma_CWR CMake project root.
        Every tests/smoke/*.tests.ps1 file calls this as `Get-RepoRoot $PSScriptRoot`,
        so in practice StartPath is always .../arma_CWR/tests/smoke and the walk is
        exactly two levels.
    .PARAMETER StartPath
        Directory (or file) to start the upward search from.
    .OUTPUTS
        System.String - absolute path to the arma_CWR root.
    .NOTES
        Throws if no CMakePresets.json is found before reaching the filesystem root -
        that indicates a broken checkout, not a soft "skip" condition, so unlike most
        other helpers here this one does NOT return $null.
    #>
    param(
        [Parameter(Mandatory, Position = 0)]
        [string]$StartPath
    )

    $dir = $StartPath
    if (Test-Path -LiteralPath $dir -PathType Leaf) {
        $dir = Split-Path -Parent $dir
    }
    $dir = (Resolve-Path -LiteralPath $dir).ProviderPath

    while ($true) {
        $marker = Join-Path $dir 'CMakePresets.json'
        if (Test-Path -LiteralPath $marker -PathType Leaf) {
            return $dir
        }
        $parent = Split-Path -Parent $dir
        if ([string]::IsNullOrEmpty($parent) -or $parent -eq $dir) {
            throw "Get-RepoRoot: no CMakePresets.json found walking up from '$StartPath'"
        }
        $dir = $parent
    }
}

function Get-GameDataDir {
    <#
    .SYNOPSIS
        Resolves the OFPR_DATA_DIR game-data directory for smoke/integration tests.
    .DESCRIPTION
        Resolution order (first candidate that actually exists on disk wins), mirroring
        how `tri` resolves the same setting per tests/README.md:
          1. OFPR_DATA_DIR= line in <RepoRoot>/.trident.env (KEY=VALUE, quotes optional -
             both '...' and "..." are stripped; see .trident.env.example).
          2. $env:OFPR_DATA_DIR, if set.
          3. <RepoRoot>/packages/Demo (the recommended local layout for a Demo checkout).
        Returns $null if none of the candidates resolve to an existing directory - the
        smoke tests treat that as a Skip condition, not an error.
    .PARAMETER RepoRoot
        Path returned by Get-RepoRoot.
    .OUTPUTS
        System.String or $null.
    #>
    param(
        [Parameter(Mandatory)]
        [string]$RepoRoot
    )

    $candidates = New-Object System.Collections.Generic.List[string]

    $envFile = Join-Path $RepoRoot '.trident.env'
    if (Test-Path -LiteralPath $envFile -PathType Leaf) {
        foreach ($line in (Get-Content -LiteralPath $envFile)) {
            $trimmed = $line.Trim()
            if ($trimmed -eq '' -or $trimmed.StartsWith('#')) { continue }
            if ($trimmed -match '^OFPR_DATA_DIR\s*=\s*(.*)$') {
                $val = $matches[1].Trim()
                if ($val.Length -ge 2) {
                    $first = $val.Substring(0, 1)
                    $last = $val.Substring($val.Length - 1, 1)
                    if (($first -eq "'" -and $last -eq "'") -or ($first -eq '"' -and $last -eq '"')) {
                        $val = $val.Substring(1, $val.Length - 2)
                    }
                }
                if ($val -ne '') { $candidates.Add($val) }
            }
        }
    }

    if ($env:OFPR_DATA_DIR) { $candidates.Add($env:OFPR_DATA_DIR) }

    $candidates.Add((Join-Path $RepoRoot 'packages/Demo'))

    foreach ($candidate in $candidates) {
        if (Test-Path -LiteralPath $candidate -PathType Container) {
            return (Resolve-Path -LiteralPath $candidate).ProviderPath
        }
    }

    return $null
}

function Get-DistPresetName {
    <#
    .SYNOPSIS
        Internal: reproduces the DIST_PRESET_NAME derivation from the root
        CMakeLists.txt (~lines 166-192) in PowerShell.
    .DESCRIPTION
        win-x64-clang-rwdi -> x64-win-rwdi (arch-platform-suffix, "clang" dropped);
        win-x64-clang -> x64-win (no suffix); steamrt4-x64-clang-rel -> x64-steamrt4-rel.
        Presets that don't match either shape pass through unchanged, matching the
        CMake `elseif` chain leaving DIST_PRESET_NAME as the raw preset name.
    .PARAMETER Preset
        CMake configure preset name, e.g. "win-x64-clang-rwdi".
    .OUTPUTS
        System.String
    #>
    param(
        [Parameter(Mandatory)]
        [string]$Preset
    )

    if ($Preset -match '^(win|macos|linux)-([^-]+)(-(.*))?$') {
        $platform = $matches[1]
        $arch = $matches[2]
        $suffix = $matches[4]
        if ($suffix -match '^clang-(.+)$') {
            $suffix = $matches[1]
        } elseif ($suffix -eq 'clang') {
            $suffix = ''
        }
        if ($suffix) { return "$arch-$platform-$suffix" }
        return "$arch-$platform"
    }

    if ($Preset -match '^steamrt4-([^-]+)-clang(-(.*))?$') {
        $arch = $matches[1]
        $suffix = $matches[3]
        if ($suffix) { return "$arch-steamrt4-$suffix" }
        return "$arch-steamrt4"
    }

    return $Preset
}

function Get-GameExe {
    <#
    .SYNOPSIS
        Resolves the path to a built binary under dist/<dist-name>/.
    .DESCRIPTION
        Derives the dist directory name from -Preset the same way the root
        CMakeLists.txt does (see Get-DistPresetName), then looks for
        <RepoRoot>/dist/<dist-name>/<Name>.exe. Returns $null (not an error) when the
        binary is not present, so callers build a Skip reason from it - e.g.
        `$script:Skip = if (-not $script:gameExe) { "PoseidonGame not found" } ...`.
    .PARAMETER RepoRoot
        Path returned by Get-RepoRoot.
    .PARAMETER Preset
        CMake configure preset name, e.g. "win-x64-clang-rwdi".
    .PARAMETER Name
        Binary base name without extension, e.g. "PoseidonGame" or "PoseidonServer".
    .OUTPUTS
        System.String or $null.
    #>
    param(
        [Parameter(Mandatory)]
        [string]$RepoRoot,
        [Parameter(Mandatory)]
        [string]$Preset,
        [Parameter(Mandatory)]
        [string]$Name
    )

    $distName = Get-DistPresetName -Preset $Preset
    $ext = '.exe'
    if ($env:OS -ne 'Windows_NT') { $ext = '' }
    $exePath = Join-Path $RepoRoot "dist/$distName/$Name$ext"
    if (Test-Path -LiteralPath $exePath -PathType Leaf) {
        return (Resolve-Path -LiteralPath $exePath).ProviderPath
    }
    return $null
}

function Get-TriExe {
    <#
    .SYNOPSIS
        Resolves the path to the Trident (`tri`) integration-test CLI.
    .DESCRIPTION
        `tri` is a Rust/Cargo binary, not staged into dist/<preset>/ by CMake, so this
        does not use Get-DistPresetName. Looks in order: the Cargo debug build
        (engine/Trident/target/debug/), the Cargo release build
        (engine/Trident/target/release/), then `tri` on PATH. -Preset is accepted for
        call-site symmetry with Get-GameExe (every smoke test resolves gameExe and
        triExe the same way) but does not affect which tri binary is picked - there is
        only ever one Trident build per checkout.
    .PARAMETER RepoRoot
        Path returned by Get-RepoRoot.
    .PARAMETER Preset
        Accepted for symmetry with Get-GameExe; unused.
    .OUTPUTS
        System.String or $null.
    #>
    param(
        [Parameter(Mandatory)]
        [string]$RepoRoot,
        [string]$Preset
    )

    $ext = '.exe'
    if ($env:OS -ne 'Windows_NT') { $ext = '' }

    $candidates = @(
        (Join-Path $RepoRoot "engine/Trident/target/debug/tri$ext"),
        (Join-Path $RepoRoot "engine/Trident/target/release/tri$ext")
    )
    foreach ($candidate in $candidates) {
        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            return (Resolve-Path -LiteralPath $candidate).ProviderPath
        }
    }

    $onPath = Get-Command -Name 'tri' -CommandType Application -ErrorAction SilentlyContinue |
              Select-Object -First 1
    if ($onPath) { return $onPath.Source }

    return $null
}

function Test-HasAddons {
    <#
    .SYNOPSIS
        Reports whether a game-data directory has an AddOns/Addons folder.
    .DESCRIPTION
        Checks for either casing (`AddOns` - the Classic/1.99 spelling - or `Addons`)
        since call sites treat this as "does the data dir look real", not a strict
        path check. Used to build Skip reasons like "dist/Game/Addons not found".
    .PARAMETER Dir
        Game-data directory, typically from Get-GameDataDir.
    .OUTPUTS
        System.Boolean
    #>
    param(
        [Parameter(Mandatory, Position = 0)]
        [string]$Dir
    )

    if (-not $Dir) { return $false }
    return (Test-Path -LiteralPath (Join-Path $Dir 'AddOns')) -or
           (Test-Path -LiteralPath (Join-Path $Dir 'Addons'))
}

function Invoke-GuiExe {
    <#
    .SYNOPSIS
        Runs a GUI (or server) exe, merging stdout+stderr, and returns the output lines.
    .DESCRIPTION
        Every smoke test that boots PoseidonGame in --check mode does so through this
        wrapper: `Invoke-GuiExe $script:gameExe -C $script:gameDir --window --check
        --log-format jsonl --render dummy`. It exists (rather than calling `&` inline)
        so the merge-stderr-into-stdout + array-of-lines shape is centralized and to
        give the pattern a name distinct from the direct `& $script:serverExe ... 2>&1`
        calls boot_logs.tests.ps1 makes for the server exe (which has no window to
        manage).
        $LASTEXITCODE is left set by the underlying native invocation exactly as `&`
        would leave it, since PowerShell does not scope $LASTEXITCODE to the function
        call - every caller reads it immediately after calling this function.
    .PARAMETER Exe
        Path to the executable to run.
    .PARAMETER ExeArgs
        Remaining arguments, passed through verbatim (ValueFromRemainingArguments, so
        callers do not need to build an array themselves).
    .OUTPUTS
        System.Object[] - one entry per output line (stdout and stderr interleaved).
    #>
    param(
        [Parameter(Mandatory, Position = 0)]
        [string]$Exe,
        [Parameter(ValueFromRemainingArguments = $true)]
        [string[]]$ExeArgs
    )

    if (-not $ExeArgs) { $ExeArgs = @() }
    & $Exe @ExeArgs 2>&1
}

function Get-JsonlLogs {
    <#
    .SYNOPSIS
        Parses --log-format jsonl output into an array of log-entry objects.
    .DESCRIPTION
        Each engine JSONL line is a flat object with (at least) ts/app/level/cat/msg
        fields (see Logging.cpp). Lines that are not valid JSON (crash-handler text
        written straight to stderr, stray console noise) are silently skipped rather
        than throwing, so a boot that logs cleanly plus one unrelated stray line does
        not fail every test that reads its logs.
    .PARAMETER Output
        The line array returned by Invoke-GuiExe (or an equivalent `& exe ... 2>&1`
        capture).
    .OUTPUTS
        System.Object[] - parsed JSONL entries, in original order.
    #>
    param(
        [Parameter(Position = 0)]
        [AllowNull()]
        [AllowEmptyCollection()]
        [object[]]$Output
    )

    $result = New-Object System.Collections.Generic.List[object]
    if (-not $Output) { return $result.ToArray() }

    foreach ($line in $Output) {
        $text = [string]$line
        if ([string]::IsNullOrWhiteSpace($text)) { continue }
        if (-not $text.TrimStart().StartsWith('{')) { continue }
        try {
            $obj = $text | ConvertFrom-Json -ErrorAction Stop
            $result.Add($obj)
        } catch {
            # Not a JSONL log line (crash text, stray console output) - skip it.
        }
    }
    return $result.ToArray()
}

function Find-LogMessage {
    <#
    .SYNOPSIS
        Finds the first JSONL log entry whose msg contains a substring.
    .DESCRIPTION
        Plain substring match (not regex/-like globbing) against each entry's `.msg`
        field, so callers can pass log text containing regex metacharacters (`:`, `=`,
        `[`, `)`) verbatim, e.g. Find-LogMessage $logs 'user_dir:' or
        Find-LogMessage $logs 'autodetected preset='. When -Category is given, entries
        are also filtered on `.cat -eq $Category` (used for e.g.
        Find-LogMessage $logs '...' -Category 'Config').
        Returns the first match, or $null if none - callers assert on it with both
        `Should -Not -BeNullOrEmpty` (existence) and by reading `.msg` off the single
        returned object, so this intentionally returns one entry, not a collection.
    .PARAMETER Logs
        Array of parsed log entries from Get-JsonlLogs.
    .PARAMETER Pattern
        Substring to search for within each entry's msg field.
    .PARAMETER Category
        Optional `cat` field to additionally filter on.
    .OUTPUTS
        A single log-entry object, or $null.
    #>
    param(
        [Parameter(Position = 0)]
        [AllowNull()]
        [AllowEmptyCollection()]
        [object[]]$Logs,
        [Parameter(Mandatory, Position = 1)]
        [string]$Pattern,
        [string]$Category
    )

    if (-not $Logs) { return $null }

    foreach ($entry in $Logs) {
        if (-not $entry) { continue }
        $msg = $null
        if ($entry.PSObject.Properties.Match('msg').Count -gt 0) { $msg = [string]$entry.msg }
        if ([string]::IsNullOrEmpty($msg)) { continue }
        if ($msg.IndexOf($Pattern, [System.StringComparison]::Ordinal) -lt 0) { continue }
        if ($Category) {
            $cat = $null
            if ($entry.PSObject.Properties.Match('cat').Count -gt 0) { $cat = [string]$entry.cat }
            if ($cat -ne $Category) { continue }
        }
        return $entry
    }
    return $null
}

function New-EphemeralGamePaths {
    <#
    .SYNOPSIS
        Creates a throwaway POSEIDON_USER_DIR/CACHE_DIR/TEMP_DIR set for one test.
    .DESCRIPTION
        Every cfg-persistence and boot-dance smoke test wraps its body in
        `$eph = New-EphemeralGamePaths; try { ... } finally { Remove-EphemeralGamePaths
        $eph }` so the engine reads/writes files under a fresh, empty directory instead
        of the developer's real profile. Creates BaseDir/user, BaseDir/cache,
        BaseDir/temp under a new folder in the OS temp directory, and points
        $env:POSEIDON_USER_DIR / POSEIDON_CACHE_DIR / POSEIDON_TEMP_DIR at them (the
        exact env var names GamePaths::ResolveUserDir et al. read - see
        engine/Poseidon/Foundation/Common/GamePaths.cpp). The previous values of those
        three env vars are captured so Remove-EphemeralGamePaths can restore them.
        editor_mission_folder.tests.ps1 additionally reads `$eph.BaseDir` to build a
        POSEIDON_USER_CONTENT_DIR of its own alongside these three - that env var is
        deliberately NOT managed here (it has its own resolution precedence over
        POSEIDON_USER_DIR; see GamePaths::ResolveUserContentDir) so tests that don't
        care about it never set it.
    .OUTPUTS
        PSCustomObject with BaseDir, UserDir, CacheDir, TempDir.
    #>
    param()

    $base = Join-Path ([System.IO.Path]::GetTempPath()) ('ud-smoke-' + [System.Guid]::NewGuid().ToString('N').Substring(0, 12))
    $userDir = Join-Path $base 'user'
    $cacheDir = Join-Path $base 'cache'
    $tempDir = Join-Path $base 'temp'

    New-Item -ItemType Directory -Path $base -Force | Out-Null
    New-Item -ItemType Directory -Path $userDir -Force | Out-Null
    New-Item -ItemType Directory -Path $cacheDir -Force | Out-Null
    New-Item -ItemType Directory -Path $tempDir -Force | Out-Null

    $eph = [PSCustomObject]@{
        BaseDir       = $base
        UserDir       = $userDir
        CacheDir      = $cacheDir
        TempDir       = $tempDir
        PrevUserDir   = $env:POSEIDON_USER_DIR
        PrevCacheDir  = $env:POSEIDON_CACHE_DIR
        PrevTempDir   = $env:POSEIDON_TEMP_DIR
    }

    $env:POSEIDON_USER_DIR = $userDir
    $env:POSEIDON_CACHE_DIR = $cacheDir
    $env:POSEIDON_TEMP_DIR = $tempDir

    return $eph
}

function Remove-EphemeralGamePaths {
    <#
    .SYNOPSIS
        Undoes New-EphemeralGamePaths: restores env vars, deletes the temp tree.
    .DESCRIPTION
        Restores POSEIDON_USER_DIR/CACHE_DIR/TEMP_DIR to whatever they were before the
        matching New-EphemeralGamePaths call (removing the env var entirely if it was
        unset before), then best-effort deletes -Eph.BaseDir. Deletion failures are
        swallowed (a locked file from a not-fully-exited game process should not fail
        the test that already got its assertions in) - this is the "delete the per-run
        temp paths the tests create" behavior the smoke tests rely on in their `finally`
        blocks.
    .PARAMETER Eph
        The object returned by New-EphemeralGamePaths.
    #>
    param(
        [Parameter(Mandatory, Position = 0)]
        [object]$Eph
    )

    if (-not $Eph) { return }

    if ($Eph.PSObject.Properties.Match('PrevUserDir').Count -gt 0) {
        if ($null -eq $Eph.PrevUserDir) { Remove-Item Env:POSEIDON_USER_DIR -ErrorAction SilentlyContinue }
        else { $env:POSEIDON_USER_DIR = $Eph.PrevUserDir }
    }
    if ($Eph.PSObject.Properties.Match('PrevCacheDir').Count -gt 0) {
        if ($null -eq $Eph.PrevCacheDir) { Remove-Item Env:POSEIDON_CACHE_DIR -ErrorAction SilentlyContinue }
        else { $env:POSEIDON_CACHE_DIR = $Eph.PrevCacheDir }
    }
    if ($Eph.PSObject.Properties.Match('PrevTempDir').Count -gt 0) {
        if ($null -eq $Eph.PrevTempDir) { Remove-Item Env:POSEIDON_TEMP_DIR -ErrorAction SilentlyContinue }
        else { $env:POSEIDON_TEMP_DIR = $Eph.PrevTempDir }
    }

    if ($Eph.PSObject.Properties.Match('BaseDir').Count -gt 0 -and $Eph.BaseDir) {
        Remove-Item -LiteralPath $Eph.BaseDir -Recurse -Force -ErrorAction SilentlyContinue
    }
}

function Read-CfgFile {
    <#
    .SYNOPSIS
        Reads a `key=value;` engine ParamFile-style cfg into a hashtable.
    .DESCRIPTION
        Generic replacement for the ad hoc `Read-AudioCfg` functions some smoke tests
        define locally (audio_config.tests.ps1, audio_volume_persist.tests.ps1) - those
        two keep their own copies since they predate this module's Read-CfgFile, but
        difficulty_persist / display_apply_persist / display_config /
        graphics_apply_persist / graphics_config all call `Read-CfgFile` with no local
        definition, so it must live here.
        Parses `key=value;` lines (trailing `;` optional), strips one layer of
        surrounding double quotes from the value, and returns a hashtable keyed by the
        raw key text. Keys may contain `[]` (e.g. `diffFriendlyTag[]` in
        difficulty.cfg's array fields), so the key pattern is intentionally wider than
        `\w+`. Returns $null if the file does not exist, matching the local
        Read-AudioCfg convention the other tests already use.
    .PARAMETER Path
        Path to the cfg file.
    .OUTPUTS
        System.Collections.Hashtable or $null.
    #>
    param(
        [Parameter(Mandatory, Position = 0)]
        [string]$Path
    )

    if (-not (Test-Path -LiteralPath $Path)) { return $null }

    $kv = @{}
    $text = Get-Content -LiteralPath $Path -Raw
    if ($null -eq $text) { return $kv }

    foreach ($line in ($text -split "`r?`n")) {
        if ($line -match '^\s*([A-Za-z0-9_\[\]]+)\s*=\s*(.*?);?\s*$') {
            $key = $matches[1]
            $val = $matches[2].Trim()
            if ($val -match '^"(.*)"$') { $val = $matches[1] }
            $kv[$key] = $val
        }
    }
    return $kv
}

function Read-HarnessMessage {
    <#
    .SYNOPSIS
        Internal: reads and JSON-decodes one line from a Trident harness connection.
    .DESCRIPTION
        Not exported - a private building block for Invoke-HarnessCommand and
        Wait-HarnessEvent, mirroring the private `Read-HarnessMessage` that
        display_disabled_rows.tests.ps1 defines locally (script:-scoped) for the same
        purpose. Kept internal because nothing outside this module needs to read a raw
        harness line without also either matching it against an `ok` response or an
        `event` name.
    #>
    param(
        [Parameter(Mandatory)]
        [hashtable]$Connection,
        [int]$TimeoutMs = 10000
    )

    $deadline = [DateTime]::UtcNow.AddMilliseconds($TimeoutMs)
    while ([DateTime]::UtcNow -lt $deadline) {
        try {
            $line = $Connection.Reader.ReadLine()
            if (-not [string]::IsNullOrWhiteSpace($line)) {
                return $line | ConvertFrom-Json
            }
        } catch [System.IO.IOException] {
            Start-Sleep -Milliseconds 50
        }
    }

    throw 'Timed out waiting for harness message'
}

function Invoke-HarnessCommand {
    <#
    .SYNOPSIS
        Sends one JSON command to a Trident harness connection and waits for its reply.
    .DESCRIPTION
        Writes -Command as a compact JSON line to -Connection.Writer, flushes, then
        reads lines (via the internal Read-HarnessMessage) until one carries a non-null
        `ok` field. Throws if that reply has `ok: false` (using its `.error` field in
        the message) or if -TimeoutMs elapses first.
        -Connection is the same shape display_disabled_rows.tests.ps1's local
        Connect-Harness builds: a hashtable with Client/Reader/Writer
        (TcpClient/StreamReader/StreamWriter). This module does not itself provide a
        Connect-Harness/Start-HarnessGame pair - none of the current smoke tests import
        those from the module (display_disabled_rows.tests.ps1 defines its own
        script:-scoped copies, matching the pattern documented for
        tests/cli/server.tests.ps1) - but Invoke-SqfEval/Invoke-SqfExec/
        Wait-HarnessEvent are exported here per issue #49's request so a future smoke
        test can pull them from the module instead of duplicating them again.
    .PARAMETER Connection
        Hashtable with Reader/Writer (and usually Client) for an open harness socket.
    .PARAMETER Command
        Hashtable to serialize as the JSON command, e.g. @{ cmd = 'eval'; code = '...' }.
    .PARAMETER TimeoutMs
        Overall timeout waiting for a reply carrying an `ok` field.
    .OUTPUTS
        The parsed reply object.
    #>
    param(
        [Parameter(Mandatory)]
        [hashtable]$Connection,
        [Parameter(Mandatory)]
        [hashtable]$Command,
        [int]$TimeoutMs = 10000
    )

    $Connection.Writer.WriteLine(($Command | ConvertTo-Json -Compress))
    $Connection.Writer.Flush()

    $deadline = [DateTime]::UtcNow.AddMilliseconds($TimeoutMs)
    while ([DateTime]::UtcNow -lt $deadline) {
        $msg = Read-HarnessMessage -Connection $Connection -TimeoutMs 1000
        if ($null -ne $msg.ok) {
            if (-not $msg.ok) {
                throw "Harness command failed: $($msg.error)"
            }
            return $msg
        }
    }

    throw 'Timed out waiting for harness response'
}

function Invoke-SqfEval {
    <#
    .SYNOPSIS
        Evaluates a Trident (`tri*`) SQF expression over the harness and returns its result.
    .DESCRIPTION
        Sends @{ cmd = 'eval'; code = $Code } via Invoke-HarnessCommand and returns the
        reply's `.result` as a string, stripping one layer of surrounding double quotes
        (Trident's `eval` returns SQF-string results already quoted, e.g. `"OK"`) so
        callers can write `Invoke-SqfEval -Connection $c -Code 'triClickText "OPTIONS"'
        | Should -Be 'OK'` directly.
    .PARAMETER Connection
        Open harness connection (see Invoke-HarnessCommand).
    .PARAMETER Code
        SQF expression to evaluate, e.g. 'triGetWindowMode'.
    .OUTPUTS
        System.String
    #>
    param(
        [Parameter(Mandatory)]
        [hashtable]$Connection,
        [Parameter(Mandatory)]
        [string]$Code
    )

    $resp = Invoke-HarnessCommand -Connection $Connection -Command @{ cmd = 'eval'; code = $Code }
    $result = [string]$resp.result
    if ($result -match '^"(.*)"$') {
        return $matches[1]
    }
    return $result
}

function Invoke-SqfExec {
    <#
    .SYNOPSIS
        Executes a Trident SQF statement over the harness (fire-and-forget).
    .DESCRIPTION
        Sends @{ cmd = 'exec'; code = $Code } via Invoke-HarnessCommand and discards the
        reply - for statements run for their side effect (e.g.
        'triSetLanguage "English";') rather than their return value.
    .PARAMETER Connection
        Open harness connection (see Invoke-HarnessCommand).
    .PARAMETER Code
        SQF statement to execute.
    #>
    param(
        [Parameter(Mandatory)]
        [hashtable]$Connection,
        [Parameter(Mandatory)]
        [string]$Code
    )

    Invoke-HarnessCommand -Connection $Connection -Command @{ cmd = 'exec'; code = $Code } | Out-Null
}

function Wait-HarnessEvent {
    <#
    .SYNOPSIS
        Blocks until the harness emits an event with a given name (and optional predicate).
    .DESCRIPTION
        Reads harness messages (via the internal Read-HarnessMessage) until one has
        `.event -eq $Name` and, if -Predicate is given, that predicate returns a truthy
        value for the message. Used e.g. to wait for the initial 'ready' event right
        after launching the game with --harness 0.
    .PARAMETER Connection
        Open harness connection.
    .PARAMETER Name
        Event name to wait for, e.g. 'ready'.
    .PARAMETER Predicate
        Optional scriptblock taking the message as its single argument; defaults to
        always-true.
    .PARAMETER TimeoutMs
        Overall timeout.
    .OUTPUTS
        The matching event message object.
    #>
    param(
        [Parameter(Mandatory)]
        [hashtable]$Connection,
        [Parameter(Mandatory)]
        [string]$Name,
        [scriptblock]$Predicate = { param($msg) $true },
        [int]$TimeoutMs = 30000
    )

    $deadline = [DateTime]::UtcNow.AddMilliseconds($TimeoutMs)
    while ([DateTime]::UtcNow -lt $deadline) {
        $msg = Read-HarnessMessage -Connection $Connection -TimeoutMs 1000
        if ($msg.event -eq $Name -and (& $Predicate $msg)) {
            return $msg
        }
    }

    throw "Timed out waiting for harness event '$Name'"
}

function Wait-Until {
    <#
    .SYNOPSIS
        Polls a scriptblock until it returns a truthy value, or throws on timeout.
    .DESCRIPTION
        Generic poll loop: calls -Condition every 100ms until it returns something
        PowerShell considers truthy, then returns that value. Used for UI-navigation
        waits like `Wait-Until -Description 'Options shell' -Condition { if
        ((Invoke-SqfEval ...) -eq 'OK') { return $true }; $null }`.
    .PARAMETER Condition
        Scriptblock to poll. Its return value is checked for truthiness and returned
        to the caller on success.
    .PARAMETER Description
        Human-readable label used in the timeout exception message.
    .PARAMETER TimeoutMs
        Overall timeout.
    .OUTPUTS
        Whatever -Condition returned on the successful call.
    #>
    param(
        [Parameter(Mandatory)]
        [scriptblock]$Condition,
        [string]$Description = 'condition',
        [int]$TimeoutMs = 10000
    )

    $deadline = [DateTime]::UtcNow.AddMilliseconds($TimeoutMs)
    while ([DateTime]::UtcNow -lt $deadline) {
        $result = & $Condition
        if ($result) {
            return $result
        }
        Start-Sleep -Milliseconds 100
    }

    throw "Timed out waiting for $Description"
}

Export-ModuleMember -Function @(
    'Get-RepoRoot',
    'Get-GameDataDir',
    'Get-GameExe',
    'Get-TriExe',
    'Test-HasAddons',
    'Invoke-GuiExe',
    'Get-JsonlLogs',
    'Find-LogMessage',
    'New-EphemeralGamePaths',
    'Remove-EphemeralGamePaths',
    'Read-CfgFile',
    'Invoke-HarnessCommand',
    'Invoke-SqfEval',
    'Invoke-SqfExec',
    'Wait-HarnessEvent',
    'Wait-Until'
)
