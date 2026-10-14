# tri splits a test line on every top-level semicolon

## What goes wrong

A `.test.sqf` line such as `gmCiConfirmBody = asRogue; n = count ([] call GM_AS_fnCandidates); gmCiConfirmBody = objNull`
looks atomic but is not. Trident's splitter (`engine/Trident/src/scenarios/integration.rs`, around line 740)
cuts a line at every top-level `;` (only `{}` blocks and strings stay whole) and sends each part as its own
harness command. The game pumps one command per frame and keeps simulating in between, so a running SQS
manager (civilian interaction at 4 Hz, the ambient civilian manager, shakedown, panic) can reset the global
you just set before the next part reads it. `guerrilla_assailants` failed about half its runs this way:
the exclusion body read `<nullptr-object>` one command after it was assigned.

## Why

The harness protocol is one statement per command; the splitter exists so a line of several statements
still maps to several commands. Nothing in the game pauses the schedulers between commands.

## What to do

- Put a set / measure / clear sequence inside one block: `if (true) then { ... }` or the body of a
  `triSimUntil { ... }`. A block is one command and runs inside one frame.
- Never clear a manager's ledger arrays (`GM_PANIC_ZONES` / `GM_PANIC_UNTIL`, `GM_CIV_GROUPS`) from a test
  while the manager runs; expire the rows the manager's own way (`GM_PANIC_UNTIL set [i, time - 1]`) and
  wait for it to drop them.
- Registered assailants are armed and hostile: disable `AUTOTARGET` (not only `TARGET`) and set the group
  to combat mode BLUE, as the `guerrilla_assailant_fire_*` tests do, or they shoot the player and the
  autotest mission ends with a clean exit 0 that tri reports as "failed to read from harness".
