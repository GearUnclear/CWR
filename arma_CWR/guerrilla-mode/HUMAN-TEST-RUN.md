# Human gameplay test run

Copy this sheet for each acceptance run. No results are pre-filled as passed.

- Tester / date:
- Build / commit:
- Game data / mounted mods:
- Save / session identifier:
- Island / occupier roster / resistance roster / outfit:
- Difficulty / view distance / day or night:
- Road fixture used (and which case):

## Case results

Use the case IDs in [HUMAN-TESTS.md](HUMAN-TESTS.md). Record separate attempts
when restarting a campaign. Every case starts UNTESTED.

| Case ID | Session/save | Last completed step | Result | Actual versus expected / screenshot / defect |
|---|---|---|---|---|
| | | | UNTESTED | |

## New-game matrix

List each installed island with its defaults and each offered outfit, then
each offered occupier/resistance roster as described in the suite guide.
Do not collapse different roster or outfit runs into a single result.

| Island | Occupier | Resistance | Outfit | Start/preview | Garrison/recruit | Civilians/dealers | Natural traffic | Save/load | Result / evidence |
|---|---|---|---|---|---|---|---|---|---|
| Abel | | | | | | | | | UNTESTED |
| Eden | | | | | | | | | UNTESTED |
| Sinai | | | | | | | | | UNTESTED |
| Lebanon80 | | | | | | | | | UNTESTED |

Unavailable installed-content prerequisites are BLOCKED with a reason.
Complete every required case/row before describing the run as passed.

## Off-screen contract session (issue #71)

Follow [FULL-SCALE-SESSION.md](FULL-SCALE-SESSION.md) part A once on Malden
without mods (S1) and once on Sinai with `@LoBo` (S2).

| Row | S1 Malden | S2 Sinai + @LoBo | Evidence |
|---|---|---|---|
| OFF-a leave troops at a distant zone | UNTESTED | UNTESTED | |
| OFF-b wait in an untouched region | UNTESTED | UNTESTED | |
| OFF-c interrupt a convoy, leave, return | UNTESTED | UNTESTED | |
| OFF-d save near and far, reload, walk back | UNTESTED | UNTESTED | |

| Budget (hub scene, 1080p) | Threshold | S1 | S2 |
|---|---|---|---|
| Live AI bodies (`[gmsim] total=`) | <= 100 typical, 130 cap | | |
| Groups on one side (`maxSide=`) | <= 40 | | |
| Frame p95 | <= 25 ms | | |
| `gm` phase avg / p95 | <= 0.5 / 1.5 ms | | |
| `[gmsim] budget:` WARN lines | none | | |
