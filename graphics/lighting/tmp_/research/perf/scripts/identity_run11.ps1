# Banded pixel differences between the run11 shots (identity of the prototypes).
# Output: max channel difference / pixels over 2 / pixels over 8, per row band.
param([string]$Dir)
$s = 'D:\Arma_CWA_worktrees\lighting\tmp\research\perf\scripts'
function Shot($label) { (Get-ChildItem $Dir -Filter "*_$label.png" | Select-Object -First 1).FullName }
$pairs = @(
    @('SG_m0all', 'SG_m0all_b', 'forest: gate repeat (noise floor)'),
    @('SG_m0all', 'SG_cut', 'forest: gate vs + early cutout'),
    @('SG_m0all', 'SG_cutfol4', 'forest: gate vs + cutout + foliage 4 taps'),
    @('SG_m0all', 'SG_cutboth', 'forest: gate vs + cutout + foliage 4 taps + far tiers 4 taps'),
    @('SG_m0', 'SG_m0all192', 'forest: stock vs gate + 64 + 128')
)
foreach ($v in 'F', 'L', 'Q') {
    $pairs += , @("S$($v)_m0", "S$($v)_m0_b", "${v}: stock repeat (noise floor)")
    $pairs += , @("S$($v)_m0", "S$($v)_m0all192", "${v}: stock vs gate + 64 + 128")
    $pairs += , @("S$($v)_m0", "S$($v)_m0tocc", "${v}: stock vs terrain cull")
    $pairs += , @("S$($v)_m0", "S$($v)_m0all192tocc", "${v}: stock vs gate + 64 + 128 + terrain cull")
    $pairs += , @("S$($v)_classic", "S$($v)_classicall192", "${v}: classic vs classic + gate + 64 + 128")
    $pairs += , @("S$($v)_classic", "S$($v)_classictocc", "${v}: classic vs classic + terrain cull")
}
foreach ($p in $pairs) {
    $a = Shot $p[0]; $b = Shot $p[1]
    if (-not $a -or -not $b) { "$($p[2]): missing shot"; continue }
    "$($p[2]) [$($p[0]) vs $($p[1])]: " + (python "$s\banddiff.py" $a $b)
}
