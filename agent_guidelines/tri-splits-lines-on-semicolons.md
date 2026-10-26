# tri splits a test line on every top-level semicolon

Verified 2026-10-18 at commit `bd831e7` (branch `occupier-forces`). Paths are
relative to `arma_CWR/`.

## What goes wrong

A `.test.sqf` line such as `gmCiConfirmBody = asRogue; n = count ([] call GM_AS_fnCandidates); gmCiConfirmBody = objNull`
looks atomic but is not. Trident's splitter (`parse_statements`, `engine/Trident/src/scenarios/integration.rs:660`;
the top-level `;` test is at `engine/Trident/src/scenarios/integration.rs:740`)
cuts a line at every top-level `;` (only `{}` blocks and strings stay whole) and sends each part as its own
harness command. The game pumps one command per frame and keeps simulating in between, so a running SQS
manager (civilian interaction at 4 Hz, the ambient civilian manager, shakedown, panic) can reset the global
you just set before the next part reads it. `guerrilla_assailants` failed about half its runs this way:
the exclusion body read `<nullptr-object>` one command after it was assigned.

## Why

The harness protocol is one statement per command; the splitter exists so a line of several statements
still maps to several commands. Nothing in the game pauses the schedulers between commands.

## Observed and inferred

- Observed in the code: `parse_statements` splits at every top-level `;` outside `{}` and strings
  (`engine/Trident/src/scenarios/integration.rs:740`), and the test loop sends each resulting statement
  as its own harness command (`engine/Trident/src/scenarios/integration.rs:1698-1701`).
- Observed in runs: `guerrilla_assailants` failed about half its runs, with the exclusion body reading
  `<nullptr-object>` one command after it was assigned. Commit `2c23d60` wrapped each sequence in one
  block (`tests/integration/scripting/guerrilla_assailants.test.sqf:131-138`) and the test went
  deterministic.
- Inferred, not traced: that the game serves one harness command per frame, and which SQS manager reset
  the global in between. The test's own comment blames the civilian-interaction manager; the other
  managers above run during that test too and are not ruled out.

## What to do

- Put a set / measure / clear sequence inside one block: `if (true) then { ... }` or the body of a
  `triSimUntil { ... }`. A block is one command and runs inside one frame.
- Never clear a manager's ledger arrays (`GM_PANIC_ZONES` / `GM_PANIC_UNTIL`, `GM_CIV_GROUPS`) from a test
  while the manager runs; expire the rows the manager's own way (`GM_PANIC_UNTIL set [i, time - 1]`) and
  wait for it to drop them.
- Registered assailants are armed and hostile: disable `AUTOTARGET` (not only `TARGET`) and set the group
  to combat mode BLUE, as the `guerrilla_assailant_fire_*` tests do, or they shoot the player and the
  autotest mission ends with a clean exit 0 that tri reports as "failed to read from harness".
