# Headless regression-test audit, 2026-10-15

Scope: ordinary Catch2 tests in `test_market.cpp` and
`test_portrait_service.cpp`. Production behavior is unchanged. No game executable,
Trident scenario, renderer, or game-data integration test was launched.

## Strengthened checks

- Dealer planning now compares the complete town/kind multiset for full coverage,
  so duplicates, omissions, and incorrect kinds cannot hide behind twelve rows.
  Authored replanning checks exact identities, positions and initial flags, with
  duplicate/case variants and unknown towns. Unknown-only lists suppress random
  selection for their own kind, with the other kind as a positive control.
- Dealer replanning starts with twelve existing rows before expecting the three
  authored rows. The empty-city case starts with four rows before expecting zero.
  Previously every output array was initially empty, so omitting `out.Clear()`
  passed the entire original market/portrait test selection.
- Portrait cancellation checks that the queued identity disappears while the
  completed identity and pixels remain. It checks nonempty pixels and pending
  work before cancellation, no work afterwards, and successful re-requesting of
  the cancelled identity. Previously deleting the ready portrait instead could
  satisfy the remaining-count assertion.
- Portrait teardown follows an actual warm-cache hit, requires the counter to
  start at one and end at zero, and verifies that requests work after teardown.
  Previously the reset assertion started and ended at zero.

## Executed results

Built with Clang using the existing `linux-x64-clang-rwdi` compile commands and
link libraries at source revision `d2b605063d4e7f3f2c79c8c14c0e9bba1791075c`.
An isolated executable linked both complete test files, the ordinary
`PoseidonTests` main/stubs, and the actual production libraries. Portrait tests
use their existing injected synthetic backend. All build and test writes were
under `/tmp/cwr-regression-audit`; no shared build outputs were modified.

| Build | Result |
| --- | --- |
| Original tests, unmodified production | 20 cases, 202 assertions passed |
| Strengthened tests, unmodified production | 20 cases, 262 assertions passed |
| Final clean rerun after removing scratch mutants, RNG seed 20261015 | 20 cases, 262 assertions passed |

For each mutation below, a copied **production translation unit** was recompiled
and linked in place of its archive member. The same mutation was run against both
versions of the complete two-file selection, `[market],[portrait]`.

| Deliberately wrong local mutation | Original tests | Strengthened tests |
| --- | --- | --- |
| Remove `Market::PlanDealers`' `out.Clear()` | 20 cases pass | 1 case fails, 3 assertions: stale replanning rows, wrong retained kind, nonempty empty-city result |
| In `PortraitService::Cancel`, erase a ready entry when present instead of the queued entry | 20 cases pass | 1 case fails, 2 assertions: queued identity remains and ready identity is missing |
| In `PortraitService::Teardown`, reset `_renders` but retain `_hits` | 20 cases pass | 1 case fails, 1 assertion: cache hits remain 1 |

The count-only dealer assertion is independently stronger, but two broader probes
(duplicate all full-share picks; assign all full-share rows the weapon kind) were
already caught elsewhere by the original save/load and nearest-dealer tests.
They are **not** claimed as newly detected suite failures.

Scratch mutation sources, objects and executables were removed before delivery;
no production changes are in the patch. Local commands and output logs remain in
`/tmp/cwr-regression-audit/evidence/`, with the scratch runner at
`/tmp/cwr-regression-audit/run.py`. The final clean command was:

```sh
/tmp/cwr-regression-audit/strengthened-clean '[market],[portrait]' --rng-seed 20261015 --reporter compact
```

The normal build can run the same selection with:

```sh
arma_CWR/build/linux-x64-clang-rwdi/tests/unit/engine/Poseidon/PoseidonTests '[market],[portrait]' --rng-seed 20261015 --reporter compact
```

This evidence establishes helper and ordinary unit-test behavior only. It does
not establish gameplay, real portrait rendering, or engine integration success.
