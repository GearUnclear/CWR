# Banded pixel differences between the run9 shots (identity of the prototypes).
# Output: max channel difference / pixels over 2 / pixels over 8, per row band.
param([string]$Dir)
$s = 'D:\Arma_CWA_worktrees\lighting\tmp\research\perf\scripts'
function Shot($label) { (Get-ChildItem $Dir -Filter "*_$label.png" | Select-Object -First 1).FullName }
$pairs = @(
    @('SG_m0all', 'SG_m0all_b', 'forest: gate repeat (noise floor)'),
    @('SG_cut', 'SG_cut_b', 'forest: cutout repeat (noise floor)'),
    @('SG_m0all', 'SG_cut', 'forest: gate vs gate + early cutout'),
    @('SG_m0', 'SG_m0all', 'forest: stock vs gate'),
    @('SG_m0', 'SG_m0all64', 'forest: stock vs gate + flag 64'),
    @('SF_m0', 'SF_m0_b', 'noon: stock repeat (noise floor)'),
    @('SF_m0', 'SF_m0all', 'noon: stock vs gate'),
    @('SF_m0', 'SF_m0all64', 'noon: stock vs gate + flag 64'),
    @('SF_classic', 'SF_classicall64', 'noon: classic vs classic + gate + flag 64'),
    @('SL_m0', 'SL_m0all64', 'lowsun: stock vs gate + flag 64'),
    @('SQ_m0', 'SQ_m0all64', 'sunrise: stock vs gate + flag 64')
)
foreach ($p in $pairs) {
    $a = Shot $p[0]; $b = Shot $p[1]
    if (-not $a -or -not $b) { "$($p[2]): missing shot"; continue }
    "$($p[2]) [$($p[0]) vs $($p[1])]: " + (python "$s\banddiff.py" $a $b)
}
