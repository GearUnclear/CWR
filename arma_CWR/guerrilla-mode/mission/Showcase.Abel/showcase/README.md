# Showcase overlay (issue #9)

The oversight / verification mission for Guerrilla Mode: every gameplay system
demonstrated as a **narrated, self-verifying chapter**, runnable one at a time
from the action menu, as a full reel (`DEMO ALL`), or headlessly by the
Trident smoke test. This directory is the per-mission overlay; `init.sqs` +
`scripts/` next to it are the byte-identical shared core
(`test_mission_script_core.cpp`) and must not be touched from here except
through the sanctioned seams listed below.

## Why chapters (v2, replaces the v1 instant-demo menu)

v1 fired instant one-shot demos and painted a 6-line `titleText` HUD every
2 s. Two structural failures: `titleText` renders in the base-game title font
whose size is fixed by `RscTitlesText` in the global resource — unreachable
from script *and* from description.ext (`TitEffects.cpp:258-275`) — so the
"HUD" was an unreadable flashing wall; and instant demos complete before the
observer can see what the system did.

v2 principles:

- **Narration is `sideChat`** — the engine chat list renders small
  (`tahomaB24` @ 0.02, `Chat.cpp:275`), word-wraps, scrolls, and lines
  persist ~25 s. A readable rolling log instead of a title-font wall.
- **`titleText` survives only as one-line chapter banners** (that font is FOR
  titles).
- **Each chapter is paced** (`~SC_BEAT` between narration beats; 6 s
  interactive, 0.3 s when `SC_AUTO`) and **verifies its own outcome**: it
  polls for the observable effect with a deadline and reports
  `PASS` / `FAIL` / `SKIP <reason>` into the results ledger.
- **The event ticker replaces the HUD**: one chat line per state *change*
  (alert edges, ownership flips, War Level, cover, QRF, gear unlocks). Quiet
  when nothing happens, exact when something does.

## File map

| File | Role |
|---|---|
| `boot.sqs` | overlay bootstrap (exec'd from the player init line in `mission.sqm`, the sanctioned divergence point) |
| `lib.sqs` | `SC_fn*` helpers (narrate, banner, teleport, shield, grant, outcome) |
| `chapters.sqs` | **THE MANIFEST** — `SC_CH_IDS` + `SC_CH_TITLES`, extend here |
| `action.sqs` | thin addAction dispatcher → `SC_QUEUE` (+ ticker / time toggles) |
| `run.sqs` | queue consumer: banners, execs the chapter, enforces the timeout, records the ledger, re-arms the menu after a campaign load |
| `ticker.sqs` | the event ticker |
| `chapters/<id>.sqs` | one chapter per gameplay system |

## The chapter contract

The runner (run.sqs) pops an index off `SC_QUEUE`, prints the banner, resets
`SC_DONE=false` / `SC_OUTCOME="FAIL"` / `SC_OUTMSG`, and execs
`showcase/chapters/<SC_CH_IDS select i>.sqs`. The chapter:

1. narrates with `"..." call SC_fnSay` beats separated by `~SC_BEAT`;
2. checks its **preconditions** and reports `SKIP` with an honest reason when
   the world can no longer stage it (e.g. the Outpost was already liberated);
3. drives the systems **only through sanctioned seams** (below);
4. polls for the observable outcome with a bounded deadline
   (`#wait / ~1 / counter` pattern — never an unguarded `@`);
5. finishes with `[outcome, msg] call SC_fnDone` on **every** exit path, and
   restores anything it staged (shield, accTime) on every path too;
6. stages near a live garrison with `SC_fnTpSolo`, never `SC_fnTpGroup`:
   cover (`setCaptive`) is **personal** — squadmates parked near a garrison
   get engaged, the firefight provokes the player unit into returning fire,
   and his fired-EH breaks the campaign's cover mid-demo (found by the first
   smoke run, which is exactly what this mission is for).

The runner stamps the ledger (`SC_RES` / `SC_RESMSG`, parallel to the
manifest), posts a summary chat line, and sets `SC_LASTDONE` to the chapter
index — the completion handshake the smoke test waits on.

**To add a chapter:** create `chapters/<id>.sqs` following the contract and
append the id + menu title to the two arrays in `chapters.sqs`. Nothing else
— menu mount, reel, ledger, banners and the test loop all size off the
manifest. Ideas queued for later chapters: the AI tactics/perception rework
(mod-plans), voice-line playback, island/faction descriptor introspection.

## Sanctioned seams (everything else in core state is read-only)

- `gmResources` / `gmManpower` grants (clamped to `GM_MANPOWER_CAP`)
- `GM_COMP_XP` bumps (companions.sqs promotes on its own tick)
- `GM_fnLootItem` tallies (loot.sqs's own public tally entry)
- `gmReqId` / `gmReqPending` while the Camp menu is mounted (recruit.sqs's
  own dispatcher queue)
- `gmBreakUndercover`, `gmGarrisonForceDespawn`, `gmWarLevel` (transient by
  design — escalation.sqs recomputes it)
- `[aP, aP, -1] exec "scripts/campaign.sqs"` (the Save action's own dispatch)
- `setDammage 1` on garrison bodies (staged kills), `allowDammage` on the
  player group (the demo shield), `setAccTime`

Native event slots stay single-owner (one handler per event type, held by the
core), so the overlay registers **no** native handlers; campaign loads are
detected by polling `GM_pSaveAct` (campaign.sqs re-arms its Save action on
load, changing the id).

## Test integration

`tests/integration/scripting/showcase_smoke.test.sqf` sets `SC_AUTO = true`,
pushes each chapter index into `SC_QUEUE`, waits for `SC_LASTDONE`, and
asserts the ledger entry is `PASS` plus system-level postconditions (treasury
delta, unlock set, rank, zone owner...). Chapter order matters: **alert runs
before capture** so the FSM climbs against a live garrison; the alert chapter
ends by force-despawning the garrison (perception sources vanish → zone calms
→ qrf.sqs stands the QRF down), leaving the world quiet for capture.
