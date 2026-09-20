# Literal, platform-independent mirroring for directories owned by our installer.
Set-StrictMode -Version Latest

function Assert-PlainPath {
    param([Parameter(Mandatory)][string]$Path)
    $cursor = [IO.Path]::GetFullPath($Path)
    while ($cursor) {
        if (Test-Path -LiteralPath $cursor) {
            $item = Get-Item -LiteralPath $cursor -Force
            if ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) {
                throw "Installer destination must not traverse a link/junction: $cursor"
            }
        }
        $cursor = [IO.Path]::GetDirectoryName($cursor)
    }
}

function Sync-InstallTree {
    param([Parameter(Mandatory)][string]$Source,
          [Parameter(Mandatory)][string]$Destination,
          [Parameter(Mandatory)][string]$OwnerRoot)
    $src = [IO.Path]::GetFullPath($Source)
    $dst = [IO.Path]::GetFullPath($Destination)
    $owner = [IO.Path]::GetFullPath($OwnerRoot).TrimEnd('/','\') + [IO.Path]::DirectorySeparatorChar
    if (-not $dst.StartsWith($owner, [StringComparison]::OrdinalIgnoreCase) -or $dst -eq $owner.TrimEnd('/','\')) {
        throw "Refusing to mirror outside the owned game directory: $dst"
    }
    Assert-PlainPath $dst
    foreach ($root in @($src, $dst)) {
        if (Test-Path -LiteralPath $root) {
            foreach ($item in Get-ChildItem -LiteralPath $root -Recurse -Force) {
                if ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) {
                    throw "Refusing to mirror a tree containing a link/junction: $($item.FullName)"
                }
            }
        }
    }
    [void][IO.Directory]::CreateDirectory($dst)
    foreach ($item in Get-ChildItem -LiteralPath $src -Recurse -Force) {
        $relative = $item.FullName.Substring($src.TrimEnd('/','\').Length + 1)
        $target = Join-Path $dst $relative
        if ($item.PSIsContainer) { [void][IO.Directory]::CreateDirectory($target) }
        else {
            [void][IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($target))
            [IO.File]::Copy($item.FullName, $target, $true)
        }
    }
    # Children before parents; these resolved targets are all inside $dst.
    foreach ($item in Get-ChildItem -LiteralPath $dst -Recurse -Force | Sort-Object { $_.FullName.Length } -Descending) {
        $relative = $item.FullName.Substring($dst.TrimEnd('/','\').Length + 1)
        if (-not (Test-Path -LiteralPath (Join-Path $src $relative))) {
            Remove-Item -LiteralPath $item.FullName -Force
        }
    }
}
Export-ModuleMember -Function Assert-PlainPath, Sync-InstallTree
