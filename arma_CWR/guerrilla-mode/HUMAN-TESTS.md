# Human gameplay acceptance suite

The **HUMAN TESTS** action in Showcase opens 43 guided cases (104 steps). These are player-operated acceptance tests of the actual Guerrilla implementation.

## Launch

From `arma_CWR`:

```powershell
.\guerrilla-mode\install-missions.ps1
.\run-game.ps1 -SkipBuild -Mission 'Missions\Showcase.Abel'
```

Or use the installed **SHOWCASE** main-menu reference mission, then **HUMAN TESTS: open guided suite** from the action menu. Use a current build and Classic 1.99 data. Start a **fresh** Showcase after installing changes: saved SQS scripts retain the old source text.

Showcase's bootstrap executes `\gmcore\init.sqs`, exactly like Guerrilla. Its description includes `Missions/Guerrilla.Abel/description.ext` from the installed game data. Install both templates together. This includes the main mode's CIV descriptor, global faction resolution, market and tuning; the old Showcase omitted civilians. Trident stages only the test mission, so the data-root include is intentional.

## Run a case

1. Open the suite before running a staged DEMO. A session that has run one is refused; restart it.
2. Browse with **next**, **previous**, or **next unpassed**, read the prerequisites and use **begin selected case**.
3. Perform the listed actions with normal game controls. Recruit, buy, talk, fight, drive and save through the campaign's own UI.
4. Choose **I observed this - check step** only after personally checking **every** requested effect. A state check also has to pass where one exists. Visual checks explicitly identify human observation as their evidence.
5. Use **FAIL** for an observed defect, **BLOCKED** when conditions/content prevent the check, or **stop without a verdict** for an incomplete attempt.

There is no automatic human pass, deadline pass or skipped-as-passed result. Failed state checks leave the step unchanged. Results are `UNTESTED`, `IN_PROGRESS`, `PASS (HUMAN)`, `FAIL`, `BLOCKED` or `INCOMPLETE`. Starting another attempt clears that case's current step evidence; completed/failed attempts remain in history. All unpassed statuses appear in **next unpassed**.

**Repeat instructions** restores the current card if campaign feedback replaces it. **Results report** lists case statuses in the radio log; browsing also shows each result. Save Game preserves the ledger, step receipts, attempt history and active case. Loading re-arms the overlay and clears pending test clicks. Merely detecting a load does not mark persistence passed.

Record a defect with case ID, step, build/commit, island, rosters, outfit, save, actual versus expected behavior and a screenshot. The [run sheet](HUMAN-TEST-RUN.md) records results across campaign restarts and new-game combinations.

## Test sessions and preparation

This is a suite, not one order-independent reel. The campaign keeps changing while you test. Use disposable saves and fresh missions for incompatible branches.

- First session: movement/equipment, map/journal, recruiting/training, HQ, arms/vehicle purchases, stash and garage; finish with save/load.
- Fresh combat session: garrison casualties/cache, cover, alerts/QRF, heat and contested consolidation; capture last. Test companion death in a separate saved branch.
- Town sessions: assessment/solicitation, preview/cancel/expiry, confirmed extortion, panic and recovery. Use separate residents/saves for donation/refusal/retaliation.
- Occupation scenes: shakedown intervention and uninterrupted/public-killing branches need separate occurrences. They use the real random director; an occurrence you did not witness is **BLOCKED**, not a pass.
- Long campaign: peaceful town liberation, ordinary income, gear unlock, group overflow, companion promotions, War Level and stronger force tiers. Let the real economy/progression run; the suite does not grant XP, funds or gear.
- Roads: ambient cars and commandeering work with the main campaign data. Enemy patrols/convoys require the explicit fixture below.
- AI: compare approaches from the same save with equivalent distance/light/weather, then test suppression, cover seeking, morale and mixed targets. These are human behavioral checks, not proof of every AI algorithm.

### Enemy-road fixture

Stock Abel starts with only one enemy zone, so native patrol/convoy route selection has no second enemy endpoint. In a **disposable fresh campaign**, walk to Village/Houdan, select **traffic_patrol** or **traffic_convoy**, and use **prepare road fixture (changes Camp owner)** before beginning the case.

The fixture makes Camp an enemy endpoint using production `gmZoneSet`, then calls production `gmTrafficForceSpawn`. That native service still owns route selection, road placement, hull/crew selection, driving, danger response, escort behavior and cleanup. Its force-spawn entry skips chance/caps, so these fixture cases test behavior **after admission**. Ordinary traffic admission/chance/caps must also be observed in campaign play on a multi-enemy-zone island (the new-game matrix).

The fixture records its old/new ownership and spawned object in `HT_SETUP_HISTORY`; step receipts include its count. It cannot run during an active case or mark a verdict. It refuses duplicate live traffic of the same kind. A placement failure is reported honestly. Restart before unrelated tests, since Camp remains an enemy endpoint. **No overlay script recreates traffic logic.**

## Coverage

[coverage.json](mission/Showcase.Abel/human/coverage.json) maps every shared core script and native Guerrilla gameplay source to cases. Offline descriptor lint, roster probing and island scaffolding are explicitly classified as tooling. The contract test fails when another core/native source is added without classification. The implementation and in-game text live in the single [case manifest](mission/Showcase.Abel/human/cases.sqs).

| ID | Area | Human test |
|---|---|---|
| `movement` | Soldier | Movement, stance and equipment |
| `squad` | Soldier | Squad commands and vehicle crew |
| `combat` | Soldier | Combat, wounds and weapon roles |
| `zones` | Territory | Map, discovery and town flags |
| `town_support` | Territory | Town support and peaceful liberation |
| `garrison` | Territory | Garrison spawn, casualties and distance cache |
| `capture` | Territory | Clear, consolidate and hold an outpost |
| `capture_contested` | Territory | Consolidation pauses under contest |
| `alert` | Occupation | Detection, investigation and QRF response |
| `undercover` | Occupation | Undercover suspicion, compromise and evasion |
| `vehicle_cover` | Occupation | Civilian and stolen military vehicle cover |
| `heat` | Occupation | Heat spike and recovery |
| `escalation` | Occupation | War Level and stronger occupier forces |
| `income` | Cell | Natural income and manpower |
| `recruit` | Cell | Recruit fighter and specialist |
| `train` | Cell | Squad training and rejection |
| `recruit_limits` | Cell | Recruitment funds, proximity and group overflow |
| `loot` | Cell | Combat loot and permanent gear unlock |
| `companion_progress` | Cell | Companion survival, kills and promotion |
| `companion_death` | Cell | Companion permadeath |
| `hq` | Logistics | Establish and relocate headquarters |
| `stash` | Logistics | Weapon cache deposit and retrieval |
| `market_arms` | Logistics | Arms dealer purchase and insufficient funds |
| `market_vehicle` | Logistics | Vehicle dealer and HQ delivery |
| `garage` | Logistics | Garage lock, release and persistence |
| `civilians` | Population | Ambient population, conversation and cache |
| `solicit` | Population | Assess, solicit, refuse and cooldown |
| `extortion` | Population | Extortion preview, cancel, commit and recovery |
| `panic` | Population | Civilian harm, panic and economic recovery |
| `shakedown` | Population | Occupier street shakedown and intervention |
| `reprisal` | Population | Occupation violence and delayed resentment |
| `assailant_rogue` | Population | Spontaneous armed civilian incident |
| `assailant_resist` | Population | Civilian resistance to extortion |
| `traffic_ambient` | Traffic | Road traffic, parking and distance lifecycle |
| `traffic_commandeer` | Traffic | Commandeering and abandoned vehicle cleanup |
| `traffic_patrol` | Traffic | Occupier vehicle patrols and danger response |
| `traffic_convoy` | Traffic | Supply convoy and escort |
| `journal` | Campaign | Journal, objectives and map navigation |
| `persistence` | Campaign | Save/load the live campaign and test ledger |
| `campaign_matrix` | Campaign | New-game islands, factions and outfits |
| `ai` | Soldier | AI perception, suppression, morale and target choice |

## New-game / content matrix

Use the **real GUERRILLA menu**, not the fixed Showcase player, for the `campaign_matrix` case. Keep results in the run sheet because new games intentionally replace the current campaign ledger.

1. On every installed primary template (Abel, Eden, Sinai and Lebanon80 when its mod is installed), test the default valid opposing rosters with each offered outfit.
2. On an installed island that supports the content, exercise every offered occupier roster against a valid resistance roster, then every resistance roster against a valid occupier, with each offered outfit. Record the actual names shown by the UI.
3. For each row, check preview/selection, start body and gear, garrison hostility, recruitment, civilian interaction, dealers/vehicles and Save/reload. Confirm missing content is gated or diagnosed; do not pretend an unavailable roster was played.
4. On islands with multiple occupier zones, watch natural patrol/convoy admission and bounded populations without the road fixture. Compare day/night and long/short view distance during traffic observation.
5. Mark the overall matrix passed only after all required rows are completed; list unavailable rows as blocked. Demo-data compatibility and future unimplemented occupation features are not claimed as covered gameplay.

## Automated verification of the suite

```powershell
python -m unittest discover -s tests/contracts -p test_human_suite.py -v
.\engine\Trident\target\debug\tri.exe test tests/integration/scripting/guerrilla_human_suite.test.sqf tests/integration/scripting/guerrilla_human_roads.test.sqf tests/integration/scripting/guerrilla_human_save.seq tests/integration/ui/guerrilla_human_visual.test.sqf --retries 0
```

The contract enforces shared bootstrap/data and observer ownership, with the road fixture audited separately. Integration tests exercise actual overlay actions, a rejected premature check, real production recruitment, explicit failure/blocking/stopping, staged-demo exclusion, native traffic movement, serialization/rearming and screenshot generation.

Automated confirmations in those tests verify the runner only. **They do not certify that a human completed the 43 gameplay cases or the content matrix.** The original `showcase_smoke` still exercises the staged reel and has its own separate result ledger.

