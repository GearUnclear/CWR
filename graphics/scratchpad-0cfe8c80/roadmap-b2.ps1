$p = 'D:\Arma_CWA_worktrees\graphics-remaster\roadmap\roadmap.yaml'
$t = [IO.File]::ReadAllText($p)
$pairs = @(
    @('and in twilight the fill lifts a wall at most to the visible sky (new: the prototype could light a wall above a dim dusk sky).',
      'and the fill lifts a wall at most to the visible sky wherever the fill runs, not only in twilight (new: the prototype could light a wall above a dim dusk sky; a clear sky does not bind the limit). The low-sun hue also tints the sun shafts, which take the rig''s sun colour.'),
    @('New debug view 6, the albedo (the research''s calibration view, kept).',
      'New debug view 7, the albedo (the research''s calibration view, kept; 6 on the lane).'),
    @('sunlit hull faces warmer than before everywhere and as warm as Classic or warmer (two Jeep values within 0.02);',
      'sunlit hull faces warmer than before everywhere and as warm as Classic or warmer, except two Jeep values slightly less warm than Classic on one ratio each (lowsun_box r:b 1.26 against 1.28, dawn_box5 g:r 0.59 against 0.58);'),
    @('the flat fill (not bounce light), no RTX 3060 measured."',
      'the flat fill (not bounce light), no RTX 3060 measured. Landed on B5 (e91b3f3) by a three-way apply (8 files in conflict: A6 took debug view 6 for the lights per tile, so the albedo view moved to 7 and the lit shaders'' debug branch takes 1 to 4; the comments of slots 22 and 23 side by side; the landed registry rows, landed=12/26; the post-stage debug-view test; the plan notes); merged unit gate 577/577, smoke_abel 1/1 (KHR_debug ids 131154, 131169 and 131218 only)."')
)
foreach ($pair in $pairs) {
    $n = ([regex]::Matches($t, [regex]::Escape($pair[0]))).Count
    if ($n -ne 1) { throw "expected one match, got $n for: $($pair[0])" }
    $t = $t.Replace($pair[0], $pair[1])
}
"crlf=" + ([regex]::Matches($t, "`r`n")).Count + " lf=" + ([regex]::Matches($t, "(?<!`r)`n")).Count
[IO.File]::WriteAllText($p, $t, (New-Object Text.UTF8Encoding($false)))
