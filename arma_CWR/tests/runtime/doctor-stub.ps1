# Test double for setup orchestration only; real doctor is covered by engine tests.
if ($args.Count -ne 6 -or $args[0] -ne 'mod' -or $args[1] -ne 'doctor' -or
    $args[3] -ne '--fix' -or $args[4] -ne '--pbo' -or -not (Test-Path -LiteralPath $args[2])) {
    throw 'Incorrect doctor invocation'
}
exit 0
