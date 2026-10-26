# AI: CommandSent on the shared center radio fails "msgCmd->GetFrom() == this" whenever another group of the side has a command queued there

Labels: engine, AI, upstream defect, strict mode

## Summary

`AIGroup::CommandSent` (all three overloads, `engine/Poseidon/AI/AIGroupImpl.cpp`)
scans either the group's own radio or, with `channelCenter == true`, the
side's **center** radio, and for every `RMTCommand` it finds it checks
`AI_ERROR(msgCmd->GetFrom() == this)`. The center radio is shared by every
group of the side, and the engine itself puts other groups' commands on it:
`AIGroup::SendCommand(cmd, list, channelCenter = true)` transmits a
`RadioMessageCommand(from = this group)` on `GetCenter()->GetRadio()`. So
while one group's command sits on the center channel (queued or being
spoken), the next fuel, repair, heal or ammo check of **any other group of
the same side** fails the check. Under strict mode (tri, `--dev`) the
`LOG_ERROR` is fatal and the game exits with code 3.

The same bare check sits in the GetIn / GetOut scan of
`AIGroup::SendCommand` (`engine/Poseidon/AI/AIGroup.cpp`), which reads the
same `channelCenter` radio.

This is upstream Bohemia code (unchanged since the reroot, 241ce1e) and
independent of the occupier-forces branch: the branch only made the
situation common (an infantry group riding in, and rearming from, a hull
crewed by a separate group).

## Where

- `AIGroupImpl.cpp` `AIGroup::CommandSent(AIUnit* to, Command::Message, bool channelCenter)`:
  the queue scan (line 204 at 16acfd5) and the actual-message check (line 217).
- The same pattern in `CommandSent(bool)` (lines 76, 90) and
  `CommandSent(Command::Message, bool)` (lines 143, 156).
- `AIGroup.cpp` `AIGroup::SendCommand`, GetIn / GetOut scan (line 1579).

Senders on the center channel (each sets `channelCenter` when the support
vehicle's commander is in another group, then `SendAutoCommandToUnit(...,
channelCenter)`): `AIGroup::CheckFuel`, `CheckArmor`, `CheckHealth`,
`CheckAmmo` (`AIGroupImplHealth.cpp`) and `AIUnit::CheckAmmo`
(`AIUnit.cpp`, a rifleman rearming from a nearby friendly vehicle).
Readers: the same four group checks call `CommandSent(unit, X, true)` for
every unit whose reported fuel / damage / health / ammo state is not normal.

## Observed (2026-10-16, branch occupier-forces, Remaster shadow)

Test `guerrilla_forces_breakoff_column` with its riflemen shielded (so the
fight never ended): a funded Ural column (infantry group EAST Golf Black,
carrier crew Foxtrot Black, BMP escort crew Hotel Black) fought four
shielded defenders at Houdan. About six minutes in, the infantry ran low on
ammo and Golf sent `Rearm` (Command::Message 8, CtxAuto) on the center
channel; the Rearm did not come from `AIGroup::CheckAmmo` (an instrumented
build logged nothing there), so it is `AIUnit::CheckAmmo`, the rifleman
rearming from a friendly hull commanded by another group. Within the same
frame groups Alpha, Bravo, Charlie (town garrisons), Foxtrot and Hotel ran
their checks and hit the queued message:

```
[AI]      D:\Arma_CWA\arma_CWR\engine\Poseidon\AI\AIGroupImpl.cpp(217): check failed: msgCmd->GetFrom() == this
[AI]      D:\Arma_CWA\arma_CWR\engine\Poseidon\AI\AIGroupImpl.cpp(217): check failed: msgCmd->GetFrom() == this
[AI]      D:\Arma_CWA\arma_CWR\engine\Poseidon\AI\AIGroupImpl.cpp(217): check failed: msgCmd->GetFrom() == this
[AI]      D:\Arma_CWA\arma_CWR\engine\Poseidon\AI\AIGroupImpl.cpp(217): check failed: msgCmd->GetFrom() == this
[CORE]    strict mode: ERROR logged above is fatal (dash) requesting clean shutdown (exit 3)
```

(the last line is the engine's strict-mode message; its dash is written out here). Unfixed: the
game ended 5 min 44 s after the column arrived. Fixed: the same scene ran
480 s and 900 s of fighting with at least 40 foreign center-channel commands (the diagnostic stopped counting at 40)
skipped and no check failure.

## Minimal repro (sketch, not yet run as its own mission)

1. One side (EAST), three groups near each other: A (infantry), B
   (infantry), C (the crew of a manned vehicle that carries ammo, e.g. a
   reammo truck or a hull with magazine cargo).
2. Let A's and B's riflemen run low on ammo (fire at an invulnerable
   target, or strip their magazines).
3. A rifleman of A rearms from C's vehicle (`AIUnit::CheckAmmo`:
   `channelCenter = commander's group != A`), so a `RadioMessageCommand`
   from A goes onto EAST's center radio.
4. While it is queued, B's `AIGroup::CheckAmmo` calls
   `CommandSent(unit, Command::Rearm, true)` for its low units: the check
   fails at `AIGroupImpl.cpp(204)` (queue) or `(217)` (actual message).

## Why skipping is the right semantics

`CommandSent` asks "did **this group** already send this command?". On the
group channel every command is the group's own (the check stays there). On
the center channel another group's message never answers that question:
`IsTo(unit)` compares unit pointers, so it can never match one of our units
anyway, and `CommandSent(message, true)` without a unit (and
`CommandSent(true)` with `IsToMainSubgroup`) would even give a false
"already sent" for another group's command of the same type. A sender
group that has left the center leaves a null `_from` link, which the old
check also failed on.

## Fix (landed on occupier-forces, uncommitted at the time of writing)

`bool RadioCommandIsFrom(RadioMessageCommand* msg, AIGroup* group, bool channelCenter)`
(declared in `AIRadio.hpp`, defined in `AIGroupImpl.cpp`): on the center
channel it returns `msg->GetFrom() == group` (another group's command, or
an orphan, is skipped); on the group channel it keeps
`AI_ERROR(msg->GetFrom() == group)` and returns true. The six CommandSent
sites and the SendCommand GetIn / GetOut scan go through it. Unit cases:
`[ai][radio]` in `tests/unit/engine/Poseidon/AI/test_ai.cpp`.
